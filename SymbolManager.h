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

#ifndef SYMBOLMANAGER_H
#define SYMBOLMANAGER_H

#include "common.h"
#include <functional>
#include <fstream>

using unary_key_t  = var_t;
using binary_key_t = std::array<var_t, 2>;
using ternary_key_t = std::array<var_t, 3>;

constexpr const char* UNKNOWN_LITERAL = "Found unknown literal when adding clause";

#define INVALID_POS UINT32_MAX

template <> struct std::hash<binary_key_t>
{
    uint64_t operator() (const binary_key_t& key) const noexcept
    {
        return (static_cast<uint64_t>(std::get<1>(key)) << 32) | static_cast<uint64_t>(std::get<0>(key));
    }
};

template <> struct std::hash<ternary_key_t>
{
    uint64_t operator() (const ternary_key_t& key) const noexcept
    {
        return std::_Hash_impl::hash(key.data(), key.size() * sizeof(var_t));
    }
};

class SymbolManager
{
    /// The number of currently allocated variables
    uint32_t m_num_vars;
    /// The number of currently emitted positions
    uint32_t m_num_emitted;

    /// Pointer to the stream used as output, not owned
    std::ofstream* m_out_stream;

    template <class... Ts>
    void write_op(std::string op, var_t res, Ts... ins);

    template <class T>
    inline void write_tuple(std::ofstream* out, bool comma, T in_1);

    template <class T, class... Ts>
    inline void write_tuple(std::ofstream* out, bool no_comma, T in_1, Ts... ins);

protected:
    /// Cache for NOT gates
    std::unordered_map<unary_key_t, var_t>  m_not_cache;
    /// Cache for AND gates
    std::unordered_map<binary_key_t, var_t> m_and_cache;
    /// Cache for OR gates
    std::unordered_map<binary_key_t, var_t> m_or_cache;
    /// Cache for XOR gates
    std::unordered_map<binary_key_t, var_t> m_xor_cache;
    /// Cache for MUX gates
    std::unordered_map<ternary_key_t, var_t> m_mux_cache;

    /// Emission map
    std::unordered_map<var_t, uint32_t> m_emission_map;

public:
    static const std::string type_str;
    static const std::string storage_str;
    /// Allocates \a number many variables and returns the first one
    inline var_t new_vars(int number) noexcept;
    /// Allocates and returns a new variable
    inline var_t new_var() noexcept { return new_vars(1); };
    /// Returns the number of currently used variables
    inline uint32_t num_vars() const noexcept { return m_num_vars; };
    /// Allocates and returns a new slot
    inline uint32_t new_emission(var_t var, bool write) noexcept;
    /// Returns the emission slot for a given variable
    inline uint32_t emission_slot(var_t var) noexcept;
    /// Returns the number of currently emitted slots
    inline uint32_t num_emitted() const noexcept { return m_num_emitted; };

    /// Returns true if provided variable is known
    inline bool is_known(var_t a) const { return (static_cast<uint32_t>(a) <= m_num_vars) || a == var_t::ZERO || a == var_t::ONE; }

    /// Creates a new variable representing the and of \a a and \a b
    var_t make_not(var_t a);
    /// Creates a new variable representing the and of \a a and \a b
    var_t make_and(var_t a, var_t b);
    /// Creates a new variable representing the or of \a a and \a b
    var_t make_or(var_t a, var_t b);
    /// Creates a new variable representing the xor of \a a and \a b
    var_t make_xor(var_t a, var_t b);
    /// Creates a new variable representing the mux of \a s, \a t and \a e
    var_t make_mux(var_t s, var_t t, var_t e);

    inline void set_stream(std::ofstream* stream) { m_out_stream = stream; }
    inline void unset_stream() { set_stream(nullptr); }
    inline std::string idx(var_t x);
};

inline var_t SymbolManager::new_vars(const int number) noexcept
{
    const uint32_t var = m_num_vars;
    m_num_vars += number;
    return static_cast<var_t>(var);
}

#define TEMP_DEFINE 0
#define TEMP_CONST 1

#define TEMP_TYPE TEMP_DEFINE
#define DO_RELOAD


inline uint32_t SymbolManager::new_emission(var_t var, bool write) noexcept
{
    auto it = m_emission_map.find(var);
    if (it != m_emission_map.end()) return it->second;

    const uint32_t pos = m_num_emitted;
    m_num_emitted += 1;

    if (m_out_stream != nullptr)
    {
        if (write)
            *m_out_stream << "\t" << storage_str << "[" << pos << "] = " << idx(var) << ";\n";
        else
        #if TEMP_TYPE == TEMP_DEFINE
            *m_out_stream << "\t#define " << idx(var) << " " << storage_str << "[" << pos << "]" << "\n";
        #elif TEMP_TYPE == TEMP_CONST
            *m_out_stream << "\tconst " << type_str << "& " <<  idx(var) << " = " << storage_str << "[" << pos << "]" << ";\n";
        #endif
    }

    m_emission_map.emplace(var, pos);

    return pos;
}

inline uint32_t SymbolManager::emission_slot(var_t var) noexcept
{
    auto it = m_emission_map.find(var);
    if (it == m_emission_map.end()) return INVALID_POS;
    return it->second;
}

inline std::string SymbolManager::idx(var_t x)
{
    Assert(is_known(x), UNKNOWN_LITERAL);
#ifdef DO_RELOAD
    auto it = m_emission_map.find(x);
    if (it != m_emission_map.end())
    { return storage_str + "[" + std::to_string(it->second) + "]"; }
#endif
    return "t" + std::to_string(static_cast<uint32_t>(x));
}

template <class... Ts>
void SymbolManager::write_op(std::string op, var_t res, Ts... ins)
{
    if (m_out_stream == nullptr) return;

#if TEMP_TYPE == TEMP_DEFINE
    *m_out_stream << "\t#define " << idx(res) << " _" << op << "(";
    write_tuple(m_out_stream, false, ins...);
    *m_out_stream << ")\n";
#elif TEMP_TYPE == TEMP_CONST
    *m_out_stream << "\tconst " << type_str << " " << idx(res) << " = _" << op << "(";
    write_tuple(m_out_stream, false, ins...);
    *m_out_stream << ");\n";
#endif
}

template <class T, class... Ts>
inline void SymbolManager::write_tuple(std::ofstream* out, bool comma, T in_1, Ts... ins)
{
    if (out == nullptr) return;
    if (comma) *out << ", ";
    *out << SymbolManager::idx(in_1);
    write_tuple(out, true, ins...);
}

template <class T>
inline void SymbolManager::write_tuple(std::ofstream* out, bool comma, T in_1)
{
    if (out == nullptr) return;
    if (comma) *out << ", ";
    *out << SymbolManager::idx(in_1);
}

#endif // SYMBOLMANAGER_H
