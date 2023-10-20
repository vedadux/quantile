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

#ifndef SYMBOLVECTOR_H
#define SYMBOLVECTOR_H

#include <vector>
#include "Symbol.h"

using SymbolVector = std::vector<Symbol>;

class SymbolRefVector
{
private:
    std::vector<Symbol*> m_refs;
    SymbolRefVector(const SymbolRefVector& other, size_t up, size_t down);
public:
    SymbolRefVector() = default;
    Symbol& operator[](const size_t id) { return *m_refs.at(id); }
    const Symbol& operator[](const size_t id) const { return *m_refs.at(id); }

    SymbolRefVector operator[](const Range range) const
    { return SymbolRefVector(*this, range.first, range.second); }
    void push_back(Symbol* elem) { m_refs.push_back(elem); }
    void pop_back() { m_refs.pop_back(); }
    // Symbol& back() { m_refs.back(); }
    // Symbol& front() { m_refs.front(); }
    size_t size() const { return m_refs.size(); }
    SymbolRefVector& operator=(uint64_t vals);
    SymbolRefVector& operator=(const SymbolVector& vals);
    SymbolRefVector& operator=(const SymbolRefVector&);
    bool is_const();
    uint64_t as_uint64_t();
    // TODO: maybe add this
    // friend std::ostream& operator<<(std::ostream& out, const SymbolRefVector& ref_vector);
};

#endif // SYMBOLVECTOR_H
