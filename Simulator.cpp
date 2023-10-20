//
//   Copyright 2023 Vedad Hadžić, Graz University of Technology
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//

#include "Simulator.h"
#include "Symbol.h"

#include <algorithm>

inline void Simulator::insert_consts(SimulatorValueMap& vals)
{
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_0), std::forward_as_tuple(false));
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_1), std::forward_as_tuple(true));
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_X), std::forward_as_tuple(false));
    vals.emplace(std::piecewise_construct, std::forward_as_tuple(signal_id_t::S_Z), std::forward_as_tuple(false));
}

void Simulator::prepare_cycle()
{
    // Prepare the inputs for the next cycle
    m_trace.emplace_back();
    SimulatorValueMap& second_cycle = m_trace.back();
    const SimulatorValueMap& first_cycle = *(++m_trace.crbegin());

    // prepare constants
    insert_consts(second_cycle);

    // prepare inputs
    for (const signal_id_t sig : m_in_ports)
    { second_cycle.emplace(sig, first_cycle.at(sig)); }

    // prepare registers
    for (const Cell* cell : m_cells)
    {
        if (!is_register(cell->type())) continue; // wires are handled in step_cycle
        cell->eval<Symbol, Symbol&>(first_cycle, second_cycle);
    }

    m_prepared = true;

    const std::string cycle_num_str = std::to_string(m_trace.size() - 2);
    *m_out_stream << "\ninline __attribute__ ((always_inline)) void run_circuit_cycle_" << cycle_num_str << "(";
    *m_out_stream << SymbolManager::type_str << "* " << SymbolManager::storage_str;
    *m_out_stream << ") \n{\n";
}

Simulator::Simulator(const Circuit& circ, std::ofstream* stream) :
        Circuit(circ), m_prepared(false), m_out_stream(stream)
{
    // Create a dummy pre-cycle that is computed with all zero inputs and registers
    m_trace.emplace_back();
    SimulatorValueMap& first_cycle = m_trace.back();
    for (const signal_id_t sig : m_in_ports)
    { first_cycle.emplace(std::piecewise_construct, std::forward_as_tuple(sig), std::forward_as_tuple(false)); }
    for (const signal_id_t sig : m_reg_outs)
    { first_cycle.emplace(std::piecewise_construct, std::forward_as_tuple(sig), std::forward_as_tuple(false)); }

    insert_consts(first_cycle);

    for (const Cell* cell : m_cells)
    {
        if (is_register(cell->type())) continue;
        cell->eval<Symbol, Symbol&>(first_cycle, first_cycle);
    }
}

void Simulator::step_cycle()
{
    if (!m_prepared)
    { throw std::logic_error("Trying to step an unprepared cycle."); }
    SimulatorValueMap& curr_cycle = m_trace.back();
    const SimulatorValueMap& prev_cycle = *(++m_trace.crbegin());

    Range emit_range {Symbol::s_manager.num_emitted(), 0};

    const std::string cycle_num_str = std::to_string(m_trace.size() - 2);

    for (const Cell* cell : m_cells)
    {
        if (is_register(cell->type())) continue; // registers are handled in init_cycle

        cell->eval<Symbol, Symbol&>(prev_cycle, curr_cycle);

        const signal_id_t sig = cell->m_ports.m_unr.m_out_y;
        assert(&cell->m_ports.m_unr.m_out_y == &cell->m_ports.m_bin.m_out_y);
        assert(&cell->m_ports.m_unr.m_out_y == &cell->m_ports.m_mux.m_out_y);
        var_t v = curr_cycle.at(sig).var();
        uint32_t p = curr_cycle.at(sig).pos();
        std::string info = m_bit_name.at(sig).display() + " @" + cycle_num_str;
        const bool output = m_out_stream != nullptr && v != var_t::ZERO && v != var_t::ONE && p == INVALID_POS;
        if (output)
        {
            *m_out_stream << "\t// Saving " << info << "\n";
        }
        curr_cycle.at(sig).emit();

        if (output)
        {
            p = curr_cycle.at(sig).pos();
            assert(p != INVALID_POS);
            add_debug_info(p, info);
        }
    }
    *m_out_stream << "}\n";

    emit_range.second = Symbol::s_manager.num_emitted();
    m_cycle_emits.emplace_back(emit_range, m_trace.size() - 1);

    // if (m_out_stream != nullptr)
    //     *m_out_stream << "\tif (cycles == " << m_trace.size() - 1 << ") return;\n";
    m_prepared = false;
    std::cout << "There are " << Symbol::s_manager.num_vars() << " variables." << std::endl;
}

Simulator::~Simulator()
{
    m_cells.clear();
}

void Simulator::allocate_shared(const Range range, const size_t num_shares, bool is_secret)
{
    if (!m_prepared)
    { throw std::logic_error("Trying to allocate in unprepared cycle."); }
    const size_t low  = range.first < range.second ? range.first : range.second;
    const size_t high = range.first < range.second ? range.second : range.first;

    if (num_shares == 0)
    { throw std::logic_error("A secret cannot have no shares"); }
    SymbolVector value_vector;
    value_vector.reserve(num_shares);

    std::unordered_map<size_t, SymbolVector>& dest_map = is_secret ? m_secrets : m_data;
    auto& dest_ranges = is_secret ? m_secret_ranges : m_data_ranges;
    const std::string debug_info = is_secret ? "secret" : "data";
    for (size_t i = low; i <= high; i++)
    {
        if (dest_map.find(i) != dest_map.end())
        { throw std::logic_error("Attempting to redefine a secret"); }

        Symbol sym = create_symbol();
        sym.emit(false);
        add_debug_info(sym.pos(), debug_info + " " + std::to_string(i) + " unmasked");
        dest_map[i] = {sym};
    }

    const Range dest_range = {dest_map[low][0].pos(), dest_map[high][0].pos()};
    add_range(dest_range, dest_ranges);

    for (size_t sh = 1; sh < num_shares; sh++)
    {
        for (size_t i = low; i <= high; i++)
        {
            Symbol sym = create_symbol();
            sym.emit(false);
            add_debug_info(sym.pos(), debug_info + " " + std::to_string(i) + " share " + std::to_string(sh));
            dest_map[i].push_back(sym);
        }
    }

    const Range mask_range = {dest_map[low][1].pos(), dest_map[high][num_shares - 1].pos()};
    add_range(mask_range, m_mask_ranges);

    for (size_t i = low; i <= high; i++)
    {
        SymbolVector& secret = dest_map[i];
        for (size_t sh = 1; sh < num_shares; sh++)
        {
            secret[0] = secret[0] ^ secret[sh];
            secret[0].emit();
            if (sh != num_shares - 1)
                add_debug_info(secret[0].pos(), "intern secret " + std::to_string(i) + "share xor");
            else
                add_debug_info(secret[0].pos(), debug_info + " " + std::to_string(i) + " share 0");
        }
    }
}

void Simulator::allocate_masks(Range range)
{
    const size_t low  = range.first < range.second ? range.first : range.second;
    const size_t high = range.first < range.second ? range.second : range.first;
    for (size_t i = low; i <= high; i++)
    {
        if (m_masks.find(i) != m_masks.end())
        { throw std::logic_error("Attempting to redefine a mask"); }

        Symbol sym = create_symbol();
        sym.emit(false);
        add_debug_info(sym.pos(), "mask " + std::to_string(i));
        m_masks.emplace(i, sym);
    }
    const Range mask_range = {m_masks[low].pos(), m_masks[high].pos()};
    add_range(mask_range, m_mask_ranges);
}

SymbolRefVector Simulator::operator[](const std::string& name)
{
    const std::vector<signal_id_t>& signals = m_name_bits.at(name);
    SymbolRefVector ret_vector;
    for (const signal_id_t sig : signals)
    {
        const auto it = m_trace.back().find(sig);
        if (it != m_trace.back().end())
        {
            ret_vector.push_back(&m_trace.back().at(sig));
        }
        else
        {
            ret_vector.push_back(&m_trace.back().at(signal_id_t::S_X));
        }
    }
    return ret_vector;
}



SymbolVector Simulator::get_share_i(const Range target_range, const size_t which_share, const std::unordered_map<size_t, SymbolVector>& target)
{
    const size_t front = target_range.second;
    const size_t back = target_range.first;
    const size_t direction = (front < back) ? 1 : -1;

    SymbolVector result;
    result.reserve(std::abs((int64_t)front - (int64_t)back) + 1);
    for (size_t i = front; ; i += direction)
    {
        result.push_back(target.at(i).at(which_share));
        if (i == back) break;
    }

    return result;
}

/*
SymbolVector Simulator::ith_secret(const Range share_range, const size_t which_secret)
{
    const size_t front = share_range.second;
    const size_t back = share_range.first;
    const size_t direction = (front < back) ? 1 : -1;

    SymbolVector result;
    result.reserve(std::abs((int64_t)front - (int64_t)back) + 1);
    for (size_t i = front; ; i += direction)
    {
        result.push_back(m_secrets.at(which_secret).at(i));
        if (i == back) break;
    }

    return result;
}
*/

SymbolVector Simulator::masks(const Range range)
{
    const size_t front = range.second;
    const size_t back = range.first;
    const size_t direction = (front < back) ? 1 : -1;

    SymbolVector result;
    result.reserve(std::abs((int64_t)front - (int64_t)back) + 1);
    for (size_t i = front; ; i += direction)
    {
        result.push_back(m_masks.at(i));
        if (i == back) break;
    }

    return result;
}

void Simulator::emit_prologue()
{
    if (m_out_stream == nullptr) return;
    // TODO reset the manager here
    Symbol::s_manager.set_stream(m_out_stream);

    /*
    // #define PARALLEL_SIZE x
    __builtin_cpu_init();
    bool supports_128 = __builtin_cpu_supports("sse2"); // ignored because slow
    bool supports_256 = __builtin_cpu_supports("avx2");
    bool supports_512 = __builtin_cpu_supports("avx512f");

    if      (supports_512) *m_out_stream << "#define PARALLEL_SIZE 512\n";
    else if (supports_256) *m_out_stream << "#define PARALLEL_SIZE 256\n";
    else                   ; // leave it undefined, goes back to default on arch
    */

    // include circuit utilities here that define the operations
    write_license(*m_out_stream, "// ");
    *m_out_stream << "\n#include \"" << LIB_ROOT << "/circuit_utils.h\"\n";
}

void Simulator::emit_run_circuit()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid run_circuit(";
    *m_out_stream << SymbolManager::type_str << "* " << SymbolManager::storage_str;
    *m_out_stream << ", " << "uint32_t cycles";
    *m_out_stream << ")\n{\n";

    for (uint32_t cycle = 2; cycle < m_trace.size(); cycle++)
    {
        *m_out_stream << "\tif (cycles == " << cycle - 2 << ") return;\n";
        *m_out_stream << "\trun_circuit_cycle_" << cycle - 2 << "(" << SymbolManager::storage_str << ");\n";
    }

    *m_out_stream << "}\n";
}

void Simulator::emit_count_run_range()
{
    *m_out_stream << "\ninline __attribute__((always_inline)) void count_run_range(";
    *m_out_stream << SymbolManager::type_str << "* " << SymbolManager::storage_str;
    *m_out_stream << ", uintmax_t* count";
    *m_out_stream << ", uint32_t from";
    *m_out_stream << ", uint32_t to";
    *m_out_stream << ")\n{\n";
    *m_out_stream << "\tfor (uint32_t i = from; i < to; i++)\n";
    *m_out_stream << "\t\tcount[i] += popcnt(" << SymbolManager::storage_str << "[i]);\n";
    *m_out_stream << "}\n";
}

void Simulator::emit_count_run()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid count_run(" << SymbolManager::type_str << "* " << SymbolManager::storage_str;
    *m_out_stream << ", uintmax_t* count";
    *m_out_stream << ", uint32_t cycles";
    *m_out_stream << ")\n{\n";
    uint32_t start = 0;
    for (const auto& range : m_cycle_emits)
    {
        *m_out_stream << "\tif (cycles == " << range.second - 1 << ") return;\n";
        *m_out_stream << "\tcount_run_range(" << SymbolManager::storage_str << ", count, ";
        *m_out_stream << start << ", " << range.first.second << ");\n";
        start = range.first.second;
    }
    *m_out_stream << "}\n";
}

void Simulator::emit_run_and_count_circuit()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid run_and_count_circuit(";
    *m_out_stream << SymbolManager::type_str << "* " << SymbolManager::storage_str;
    *m_out_stream << ", uintmax_t* count";
    *m_out_stream << ", uint32_t cycles";
    *m_out_stream << ")\n{\n";

    uint32_t start = 0;
    for (const auto& range : m_cycle_emits)
    {
        *m_out_stream << "\tif (cycles == " << range.second - 1 << ") return;\n";
        *m_out_stream << "\trun_circuit_cycle_" << range.second - 1 << "(" << SymbolManager::storage_str << ");\n";
        *m_out_stream << "\tcount_run_range(" << SymbolManager::storage_str << ", count, ";
        *m_out_stream << start << ", " << range.first.second << ");\n";
        start = range.first.second;
    }
    *m_out_stream << "}\n";
}

void Simulator::emit_randomize_secrets()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid randomize_secrets(" << SymbolManager::type_str << "* " << SymbolManager::storage_str;
    *m_out_stream << ", uint32_t cycles, rand_t& gen)\n{\n";
    for (const auto& range : m_secret_ranges)
    {
        *m_out_stream << "\tif (cycles < " << range.second << ") return;\n";
        *m_out_stream << "\tfor (uint32_t i = " << range.first.first << "; i <= " << range.first.second << "; i++)\n";
        *m_out_stream << "\t\t" << SymbolManager::storage_str << "[i] = randbytes(gen);\n";
    }
    *m_out_stream << "}\n";
}

void Simulator::emit_fix_random_secrets()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid fix_random_secrets(" << SymbolManager::type_str << "* " << SymbolManager::storage_str;
    *m_out_stream << ", uint32_t cycles, rand_t& gen)\n{\n";
    *m_out_stream << "\tuintmax_t rand_bits = gen();\n";
    *m_out_stream << "\tuint32_t rand_pos = sizeof(uintmax_t) * 8 - 1;\n";
    for (const auto& range : m_secret_ranges)
    {
        *m_out_stream << "\tif (cycles < " << range.second << ") return;\n";
        *m_out_stream << "\tfor (uint32_t i = " << range.first.first << "; i <= " << range.first.second << "; i++)\n\t{\n";
        *m_out_stream << "\t\t" << SymbolManager::storage_str << "[i] = (rand_bits >> rand_pos) & 1 ? ones : zeros;\n";
        *m_out_stream << "\t\tif (rand_pos == 0) { rand_bits = gen(); rand_pos = sizeof(uintmax_t) * 8 - 1; }\n";
        *m_out_stream << "\t}\n";
    }
    *m_out_stream << "}\n";
}

void Simulator::emit_fix_random_data()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid fix_random_data(" << SymbolManager::type_str << "* " << SymbolManager::storage_str;
    *m_out_stream << ", uint32_t cycles, rand_t& gen)\n{\n";
    *m_out_stream << "\tuintmax_t rand_bits = gen();\n";
    *m_out_stream << "\tuint32_t rand_pos = sizeof(uintmax_t) * 8 - 1;\n";
    for (const auto& range : m_data_ranges)
    {
        *m_out_stream << "\tif (cycles < " << range.second << ") return;\n";
        *m_out_stream << "\tfor (uint32_t i = " << range.first.first << "; i <= " << range.first.second << "; i++)\n\t{\n";
        *m_out_stream << "\t\t" << SymbolManager::storage_str << "[i] = (rand_bits >> rand_pos) & 1 ? ones : zeros;\n";
        *m_out_stream << "\t\tif (rand_pos == 0) { rand_bits = gen(); rand_pos = sizeof(uintmax_t) * 8 - 1; }\n";
        *m_out_stream << "\t}\n";
    }
    *m_out_stream << "}\n";
}

void Simulator::emit_randomize_data()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid randomize_data(" << SymbolManager::type_str << "* " << SymbolManager::storage_str;
    *m_out_stream << ", uint32_t cycles, rand_t& gen)\n{\n";
    for (const auto& range : m_data_ranges)
    {
        *m_out_stream << "\tif (cycles < " << range.second << ") return;\n";
        *m_out_stream << "\tfor (uint32_t i = " << range.first.first << "; i <= " << range.first.second << "; i++)\n";
        *m_out_stream << "\t\t" << SymbolManager::storage_str << "[i] = randbytes(gen);\n";
    }
    *m_out_stream << "}\n";
}

void Simulator::emit_copy_secrets()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid copy_secrets(" << SymbolManager::type_str << "* dst, " << SymbolManager::type_str << "* src";
    *m_out_stream << ", uint32_t cycles)\n{\n";
    for (const auto& range : m_secret_ranges)
    {
        *m_out_stream << "\tif (cycles < " << range.second << ") return;\n";
        *m_out_stream << "\tfor (uint32_t i = " << range.first.first << "; i <= " << range.first.second << "; i++)\n";
        *m_out_stream << "\t\tdst[i] = src[i];\n";
    }
    *m_out_stream << "}\n";
}

void Simulator::emit_copy_data()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid copy_data(" << SymbolManager::type_str << "* dst, " << SymbolManager::type_str << "* src";
    *m_out_stream << ", uint32_t cycles)\n{\n";
    for (const auto& range : m_data_ranges)
    {
        *m_out_stream << "\tif (cycles < " << range.second << ") return;\n";
        *m_out_stream << "\tfor (uint32_t i = " << range.first.first << "; i <= " << range.first.second << "; i++)\n";
        *m_out_stream << "\t\tdst[i] = src[i];\n";
    }
    *m_out_stream << "}\n";
}

void Simulator::emit_randomize_masks()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid randomize_masks(" << SymbolManager::type_str << "* " << SymbolManager::storage_str;
    *m_out_stream << ", uint32_t cycles, rand_t& gen)\n{\n";
    for (const auto& range : m_mask_ranges)
    {
        *m_out_stream << "\tif (cycles < " << range.second << ") return;\n";
        *m_out_stream << "\tfor (uint32_t i = " << range.first.first << "; i <= " << range.first.second << "; i++)\n";
        *m_out_stream << "\t\t" << SymbolManager::storage_str << "[i] = randbytes(gen);\n";
    }
    *m_out_stream << "}\n";
}

void Simulator::emit_debug_info()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nuint32_t RUN_LENGTH = " << m_cycle_emits.back().first.second << ";\n";
    *m_out_stream << "const char* DEBUG_INFO[" << m_debug_info.size() << "] = {\n";
    assert(m_cycle_emits.back().first.second == m_debug_info.size());
    if (m_debug_info.empty())
        *m_out_stream << "\n};\n";
    else
    {
        for (int32_t pos = 0; pos <= (int32_t)m_debug_info.size() - 2; pos++)
        {
            *m_out_stream << "\t\"" << m_debug_info.at(pos) << "\",\n";
        }
        *m_out_stream << "\t\"" << m_debug_info.at(m_debug_info.size() - 1) << "\"\n};\n";
    }
}

void Simulator::emit_xor_runs()
{
    if (m_out_stream == nullptr) return;

    *m_out_stream << "\nvoid xor_runs(" << SymbolManager::type_str << "* result, " << SymbolManager::type_str << "* other";
    *m_out_stream << ", uint32_t cycles)\n{\n";
    uint32_t start = 0;
    for (const auto& range : m_cycle_emits)
    {
        *m_out_stream << "\tif (cycles < " << range.second << ") return;\n";
        *m_out_stream << "\tfor (uint32_t i = " << start << "; i < " << range.first.second << "; i++)\n";
        *m_out_stream << "\t\t result[i] = _xor(result[i], other[i]);\n";
        start = range.first.second;
    }
    *m_out_stream << "}\n";
}

void Simulator::emit_epilogue()
{
    emit_run_circuit();
    emit_count_run_range();
    emit_count_run();
    emit_run_and_count_circuit();

    emit_xor_runs();

    emit_randomize_secrets();
    emit_randomize_data();
    emit_randomize_masks();

    emit_fix_random_secrets();
    emit_fix_random_data();

    emit_copy_secrets();
    emit_copy_data();

    emit_debug_info();
}

void Simulator::dump_vcd(const std::string& file_name)
{
    std::ofstream out(file_name);

    // Print out licensing information
    {
        out << "$comment" << std::endl;
        write_license(out, "\t");
        out << "$end" << std::endl;
    }

    // Print the current time
    {
        auto now_timepoint = std::chrono::system_clock::now();
        std::time_t curr_time = std::chrono::system_clock::to_time_t(now_timepoint);
        out << "$date" << std::endl;
        out << "\t" << std::ctime(&curr_time) << std::endl;
        out << "$end" << std::endl;
    }

    // Print the generator version
    {
        out << "$version" << std::endl;
        out << "\t" << "Quantile: QUANTifier of Information LEakage v1.0" << std::endl;
        out << "$end" << std::endl;
    }

    // Print the timescale
    {
        out << "$timescale" << std::endl;
        out << "\t" << "1ps" << std::endl;
        out << "$end" << std::endl;
    }

    // Get all named signals
    std::unordered_map<signal_id_t, const std::string> signals_in_vcd;
    using scope_entry_t = std::tuple<const char*, std::string, std::string, uint32_t>;
    std::vector<scope_entry_t> scope_data;
    for (auto& it : m_name_bits)
    {
        std::string name = it.first;
        std::replace(name.begin(), name.end(), ':', '.');
        std::replace(name.begin(), name.end(), '$', '_');
        std::replace(name.begin(), name.end(), ' ', '_');

        const std::vector<signal_id_t> bits = it.second;
        for (uint32_t pos = 0; pos < bits.size(); pos++)
        {
            const signal_id_t sig_id = bits[pos];
            const std::string sig_str = vcd_identifier(sig_id);
            signals_in_vcd.emplace(sig_id, sig_str);
            const char* type = "wire 1";
            scope_data.emplace_back(type, sig_str, name, pos);

        }
    }

    if (m_sig_clock != signal_id_t::S_0)
        signals_in_vcd.erase(m_sig_clock);

    out << "$scope module " << m_module_name << " $end" << std::endl;
    for (const auto& entry : scope_data)
    {
        out << "\t$var " << std::get<0>(entry) << " " << std::get<1>(entry);
        out << " " << std::get<2>(entry) << "[" << std::get<3>(entry) << "]" << " $end" << std::endl;
    }
    out << "$upscope $end" << std::endl;
    out << "$enddefinitions $end" << std::endl;

    if (m_trace.empty())
    {
        out.close();
        return;
    }

    // Dump the first cycle
    auto prev_ptr = ++(m_trace.cbegin());
    auto curr_ptr = ++(m_trace.cbegin());
    uint32_t curr_tick = 0;

    while(curr_ptr != m_trace.end())
    {
        out << "#" << curr_tick << std::endl;
        if (curr_tick == 0) out << "$dumpvars" << std::endl;

        const SimulatorValueMap& curr_map = *curr_ptr;
        const SimulatorValueMap& prev_map = *prev_ptr;

        if (m_sig_clock != signal_id_t::S_0)
        {
            out << "b1 " << vcd_identifier(m_sig_clock) << std::endl;
            out << "b1 " << vcd_identifier(m_sig_clock) << std::endl;
        }

        for (auto it: signals_in_vcd)
        {
            auto curr_find_it = curr_map.find(it.first);
            auto prev_find_it = prev_map.find(it.first);
            const std::string vcd_id = it.second;
            if (curr_tick == 0)
            {
                if (curr_find_it != curr_map.end())
                { out << "b" << curr_find_it->second << " " << vcd_id << std::endl; }
                else
                { out << "bx " << vcd_id << std::endl; }

            }
            else if (curr_find_it != curr_map.end())
            {
                if (prev_find_it == prev_map.end() || curr_find_it->second != prev_find_it->second)
                { out << "b" << curr_find_it->second << " " << vcd_id << std::endl; }
            }
        }

        if (curr_tick == 0) out << "$end" << std::endl;

        if (m_sig_clock != signal_id_t::S_0)
        {
            out << "#" << curr_tick + 500 << std::endl;
            out << "b0 " << vcd_identifier(m_sig_clock) << std::endl;
        }

        curr_tick += 1000;
        prev_ptr = curr_ptr;
        curr_ptr++;
    }

    out << "#" << curr_tick << std::endl;
}