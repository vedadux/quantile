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

#include "SymbolVector.h"
#include "Cell.h"
#include <cassert>
#include <iostream>

static Symbol SymZero = Symbol(false);
static Symbol SymOne = Symbol(true);


SymbolRefVector::SymbolRefVector(const SymbolRefVector& other, const size_t up, const size_t down)
{
    if (up >= down)
    {
        const auto begin = other.m_refs.cbegin() + down;
        const auto end = other.m_refs.cbegin() + up + 1;
        m_refs = std::vector<Symbol*>(begin, end);
    }
    else
    {
        const auto begin = other.m_refs.crbegin() + (other.m_refs.size() - down - 1);
        const auto end = other.m_refs.crbegin() + (other.m_refs.size() - up);
        m_refs = std::vector<Symbol*>(begin, end);
    }
    assert(m_refs.size() == (std::abs((int64_t)up - (int64_t)down) + 1L));
}

SymbolRefVector& SymbolRefVector::operator=(uint64_t vals)
{
    if (size() < 64 && (vals >> size()) != 0)
    { std::cout << "Warning: ValueViewVector assignment overflow " << size() << std::endl; }

    for (size_t i = 0; i < size(); i++)
    { *m_refs.at(i) = ((bool)((vals >> i) & 1)) ? SymOne : SymZero; }

    return *this;
}

SymbolRefVector& SymbolRefVector::operator=(const SymbolVector& vals)
{
    if (vals.size() > size())
    { std::cout << "Warning: ValueViewVector assignment overflow " << vals.size() << std::endl; }

    for (size_t i = 0; i < size() && i < vals.size(); i++)
    { *m_refs.at(i) = vals.at(i); }

    if (vals.size() < size())
    { std::cout << "Warning: Assigning " << size() - vals.size() << " upper ValueViewVector bits to zero" << std::endl; }

    for (size_t i = vals.size(); i < size(); i++)
    { *m_refs.at(i) = SymZero; }

    return *this;
}

SymbolRefVector& SymbolRefVector::operator=(const SymbolRefVector& vals)
{
    if (vals.size() > size())
    { std::cout << "Warning: ValueViewVector assignment overflow " << vals.size() << std::endl; }

    for (size_t i = 0; i < size() && i < vals.size(); i++)
    { *m_refs.at(i) = vals[i]; }

    if (vals.size() < size())
    { std::cout << "Warning: Assigning " << size() - vals.size() << "upper ValueViewVector bits to zero" << std::endl; }

    for (size_t i = vals.size(); i < size(); i++)
    { *m_refs.at(i) = SymZero; }

    return *this;
}

bool SymbolRefVector::is_const()
{
    for (uint32_t i = 0; i < size(); i++)
    {
        if (m_refs[i]->var() == var_t::ZERO || m_refs[i]->var() == var_t::ONE) return true;
    }
    return false;
}

uint64_t SymbolRefVector::as_uint64_t()
{
    if (!is_const())
    { throw std::logic_error(ILLEGAL_VALUE_NOT_CONST); }

    uint64_t res = 0;
    for (uint32_t i = 0; i < size() && i < 64; i++)
    { res |= ((uint64_t)(m_refs[i]->var() == var_t::ONE)) << i; }
    return res;
}

/*
std::ostream& operator<<(std::ostream& out, const SymbolRefVector& ref_vector)
{
    for(uint32_t i = 0; i < ref_vector.size(); i++)
    { out << *ref_vector.m_refs[i] << " "; }
    return out;
}
*/