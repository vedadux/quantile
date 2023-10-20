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

#ifndef SYMBOL_H
#define SYMBOL_H

#include "SymbolManager.h"

class Symbol
{
public:
    /// Manager in charge of this symbol
    static SymbolManager s_manager;
private:
    /// Variable identifier for this symbol
    var_t m_variable;

    /// Position at which this is emitted
    uint32_t m_position;

    /// Constructor from variable index
    explicit Symbol(var_t variable): m_variable(variable), m_position(s_manager.emission_slot(variable)) {}

public:
    /// Default constructor creating an invalid symbol
    Symbol() : m_variable(var_t::ZERO), m_position(INVALID_POS) {}

    /// Constructor from boolean
    explicit Symbol(bool value): m_variable(to_var(value)), m_position(INVALID_POS) {}

    /// We use the default assignment operator
    Symbol(const Symbol& other) = default;

    /// We use the default assignment operator
    Symbol& operator=(const Symbol& other) = default;

    /// We define a custom assignment operator from booleans
    Symbol& operator=(bool other_val) { m_variable = to_var(other_val); m_position = INVALID_POS; return *this; };
    Symbol& operator=(var_t other_val) = delete;
    Symbol& operator=(uint8_t other_val) = delete;
    Symbol& operator=(uint16_t other_val) = delete;
    Symbol& operator=(uint32_t other_val) = delete;
    Symbol& operator=(uint64_t other_val) = delete;

    /// Returns the variable index
    inline var_t var() const { return m_variable; }

    /// Returns the position index
    inline uint32_t pos() const { return m_position; }

    /// Creates a new symbolic value
    friend Symbol create_symbol();

    /// Emits symbol at a new position
    void emit(bool write = true);

    /// Simplifying operator PLUS (+)
    friend Symbol operator+(const Symbol& a)
    { return Symbol(a); }

    /// Simplifying operator NOT (!)
    friend Symbol operator!(const Symbol& a);

    /// Simplifying operator AND (&)
    friend Symbol operator&(const Symbol& a, const Symbol& b);

    /// Simplifying operator XOR (^)
    friend Symbol operator^(const Symbol& a, const Symbol& b);

    /// Simplifying operator OR (|)
    friend Symbol operator|(const Symbol& a, const Symbol& b);

    /// Simplifying operator MUX (?:)
    friend Symbol mux(const Symbol& cond, const Symbol& t_val, const Symbol& e_val);

    /// Compares two Symbols by variable index
    friend bool operator<(const Symbol& a, const Symbol& b);

    /// Compares two Symbols for equality
    friend bool operator==(const Symbol& a, const Symbol& b);
    friend bool operator!=(const Symbol& a, const Symbol& b);

    friend std::ostream& operator<<(std::ostream& out, const Symbol& sym);
};

Symbol create_symbol();
Symbol mux(const Symbol& s, const Symbol& t, const Symbol& e);

#endif // SYMBOL_H
