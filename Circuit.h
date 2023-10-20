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

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <string>
#include <cstdint>
#include <utility>
#include <cassert>
#include <cstring>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "Cell.h"
#include "VerilogId.h"

class Circuit
{
private:
    void add_bit_names(std::string& name, std::vector<signal_id_t>& typed_signals);
protected:
    std::unordered_set<signal_id_t> m_in_ports;
    std::unordered_set<signal_id_t> m_out_ports;
    std::unordered_set<signal_id_t> m_reg_outs;
    std::unordered_set<signal_id_t> m_signals;
    std::vector<const Cell*> m_cells;
    std::unordered_map<std::string, std::vector<signal_id_t>> m_name_bits;
    std::unordered_map<signal_id_t, VerilogId> m_bit_name;
    std::string m_module_name;
    std::string m_license;
    signal_id_t m_sig_clock;
    template <typename T> signal_id_t get_signal_any(T& bit);
    void write_license(std::ostream& out, std::string delim);
    Circuit(const Circuit& circ) = default;
public:
    Circuit(const std::string& json_file_path, const std::string& top_module_name);
    bool has(const std::string& name);

    const std::vector<signal_id_t>& operator[](const std::string& name);
    ~Circuit();
};

template <typename T> signal_id_t Circuit::get_signal_any(T& bit)
{
    if(bit.is_number_unsigned())
        return static_cast<signal_id_t>((uint32_t)bit);
    else if(bit.is_string()) // it must be a constant
        return signal_from_str(bit);
    throw std::logic_error(ILLEGAL_SIGNAL_TYPE);
}

#endif // CIRCUIT_H
