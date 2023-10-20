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

#ifndef VERILOGID_H
#define VERILOGID_H

#include <inttypes.h>
#include <string>

class VerilogId {
    const std::string* m_name;
    uint32_t m_pos;
    uint32_t m_depth;
public:
    constexpr VerilogId(const std::string& name, uint32_t pos);
    const std::string& name() const { return *m_name; }
    uint32_t pos() const { return m_pos; }
    uint32_t depth() const { return m_depth; }
    std::string display() const { return *m_name + " [" + std::to_string(m_pos) + "]"; }
};

constexpr VerilogId::VerilogId(const std::string& name, uint32_t pos)
    : m_name(&name), m_pos(pos), m_depth(0)
{
    size_t curr = 0;
    size_t found = 0;
    do {
        found = m_name->find('.', curr);
        curr = found + 1;
        m_depth += 1;
    } while(found != std::string::npos);
}

inline bool operator==(const VerilogId& a, const VerilogId& b)
{ return a.pos() == b.pos() && a.depth() == b.depth() && a.name() == b.name(); }

inline bool operator<(const VerilogId& a, const VerilogId& b)
{
    const size_t apos = a.name().find('_'),
                 bpos = b.name().find('_');
    if (bpos == 0 && apos != 0)
        return true;
    else if (bpos != 0 && apos == 0)
        return false;

    if (a.depth() < b.depth())
        return true;
    else if (a.depth() > b.depth())
        return false;

    return (a.name().length() < b.name().length());
}

#endif // VERILOGID_H
