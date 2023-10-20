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

#include "Symbol.h"

SymbolManager Symbol::s_manager;

Symbol create_symbol()
{
    return Symbol(Symbol::s_manager.new_var());
}

void Symbol::emit(bool write)
{
    if (m_variable == var_t::ZERO || m_variable == var_t::ONE)
        return;
    m_position = s_manager.new_emission(m_variable, write);
}

Symbol operator!(const Symbol& a)
{
    return Symbol(Symbol::s_manager.make_not(a.m_variable));
}

Symbol operator&(const Symbol& a, const Symbol& b)
{
    return Symbol(Symbol::s_manager.make_and(a.m_variable, b.m_variable));
}

Symbol operator^(const Symbol& a, const Symbol& b)
{
    return Symbol(Symbol::s_manager.make_xor(a.m_variable, b.m_variable));
}

Symbol operator|(const Symbol& a, const Symbol& b)
{
    return Symbol(Symbol::s_manager.make_or(a.m_variable, b.m_variable));
}

Symbol mux(const Symbol& s, const Symbol& t, const Symbol& e)
{
    return Symbol(Symbol::s_manager.make_mux(s.m_variable, t.m_variable, e.m_variable));
}

bool operator<(const Symbol& a, const Symbol& b)
{
    return a.m_variable < b.m_variable;
}

bool operator==(const Symbol& a, const Symbol& b)
{
    return a.m_variable == b.m_variable;
}

bool operator!=(const Symbol& a, const Symbol& b)
{
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, const Symbol& sym)
{
    switch(sym.var())
    {
        case var_t::ZERO: out << '0'; break;
        case var_t::ONE:  out << '1'; break;
        default: out << 'x';
    }
    return out;
}