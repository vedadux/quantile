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

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <list>
#include <utility>
#include "Circuit.h"
#include "SymbolManager.h"
#include "Symbol.h"
#include "SymbolVector.h"

#include <map>
#include <fstream>
#include <chrono>

class Simulator : protected Circuit
{
private:
    bool m_prepared;
    std::ofstream* m_out_stream;

    using SimulatorValueMap = std::unordered_map<signal_id_t, Symbol>;
    std::list<SimulatorValueMap> m_trace;
    std::vector<std::pair<Range, uint32_t>> m_secret_ranges;
    std::vector<std::pair<Range, uint32_t>> m_data_ranges;
    std::vector<std::pair<Range, uint32_t>> m_mask_ranges;
    std::vector<std::pair<Range, uint32_t>> m_cycle_emits;
    std::vector<std::string> m_debug_info;

    inline void add_range(Range r, std::vector<std::pair<Range, uint32_t>>& ranges)
    { ranges.emplace_back(r, m_trace.size() - 1); }

    inline void add_debug_info(uint32_t pos, const std::string& info);

    std::unordered_map<size_t, SymbolVector> m_data;
    std::unordered_map<size_t, SymbolVector> m_secrets;
    std::unordered_map<size_t, Symbol> m_masks;

    inline void insert_consts(SimulatorValueMap& vals);

    void allocate_shared(Range range, size_t num_shares, bool is_secret);
    SymbolVector get_share_i(const Range target_range, const size_t which_share, const std::unordered_map<size_t, SymbolVector>& target);

    void emit_run_circuit();
    void emit_count_run();
    void emit_randomize_secrets();
    void emit_randomize_data();
    void emit_fix_random_secrets();
    void emit_fix_random_data();

    void emit_copy_secrets();
    void emit_copy_data();
    void emit_randomize_masks();
    void emit_debug_info();
    void emit_xor_runs();
    void emit_count_run_range();
    void emit_run_and_count_circuit();

public:
    /// Shallow copy constructor from a Circuit class, without pointer ownership transfer
    Simulator(const Circuit& circ, std::ofstream* stream);
    /// Custom destructor that obeys the pointer ownership, calling ~Circuit() implicitly
    ~Simulator();
    /// Emits the prologue of the generated CPP file
    void emit_prologue();
    /// Emits the epilogue of the generated CPP file
    void emit_epilogue();
    /// Prepares the execution of a cycle, defining inputs like in previous cycle
    void prepare_cycle();
    /// Executes one clock cycle and stores the results in \m m_trace
    void step_cycle();
    /// Performs both cycle preparation and stepping
    inline void step() { prepare_cycle(); step_cycle(); }

    /// Allocates secret identifiers in range \a range with \a num_shares many shares
    inline void allocate_secrets(Range range, size_t num_shares)
    { allocate_shared(range, num_shares, true); }
    /// Allocates data identifiers in range \a range with \a num_shares many shares
    inline void allocate_data(Range range, size_t num_shares)
    { allocate_shared(range, num_shares, false); }
    /// Allocates mask identifiers in range \a range
    void allocate_masks(Range range);

    /// Get the ith share of secrets in \a secrets_range
    SymbolVector secrets_share_i(const Range secrets_range, const size_t which_share)
    { return get_share_i(secrets_range, which_share, m_secrets); }
    /// Get the ith share of data in \a data_range
    SymbolVector data_share_i(const Range data_range, const size_t which_share)
    { return get_share_i(data_range, which_share, m_data); }
    /// Get the masks in \a range
    SymbolVector masks(Range range);

    // TODO: is it useful?
    // Get shares in \a share_range of the ith secret
    // SymbolVector ith_secret(Range share_range, size_t which_secret);

    /// Get the values of wires corresponding to \a name
    SymbolRefVector operator[](const std::string& name);

    /// Dumps the content of the current simulation into a VCD file
    void dump_vcd(const std::string& file_name);
};

inline void Simulator::add_debug_info(uint32_t pos, const std::string& info)
{
    if (m_debug_info.size() <= pos)
        m_debug_info.resize(pos+1, "");
    m_debug_info[pos] = info;
}

#endif // SIMULATOR_H
