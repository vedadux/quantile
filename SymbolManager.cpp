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

#include "SymbolManager.h"

const std::string SymbolManager::type_str = "wtype_t";
const std::string SymbolManager::storage_str = "s";

var_t SymbolManager::make_not(const var_t a)
{
    Assert(is_known(a), UNKNOWN_LITERAL);

    // Standard rules for 'not' with constant
    if (a == var_t::ZERO) return var_t::ONE;
    if (a == var_t::ONE) return var_t::ZERO;

    auto res = m_not_cache.find(a);
    if (res != m_not_cache.end()) return res->second;

    var_t not_a = new_var();
    m_not_cache.emplace(a, not_a);
    m_not_cache.emplace(not_a, a);

    write_op("not", not_a, a);

    return not_a;
}

var_t SymbolManager::make_and(const var_t a, const var_t b)
{
    Assert(is_known(a), UNKNOWN_LITERAL);
    Assert(is_known(b), UNKNOWN_LITERAL);

    // Standard rules for 'and' with constant
    if (a == var_t::ZERO || b == var_t::ZERO) return var_t::ZERO;
    if (a == var_t::ONE) return b;
    if (b == var_t::ONE) return a;
    // Simplification rules for 'and'
    if (a == b) return a;
    auto not_b = m_not_cache.find(b);
    const bool exists_not_b = not_b != m_not_cache.end();
    if (exists_not_b && a == not_b->second) return var_t::ZERO;

    const binary_key_t key = {a < b ? a : b, a < b ? b : a};
    auto res = m_and_cache.find(key);
    if (res != m_and_cache.end()) return res->second;

    var_t c = new_var();
    m_and_cache.emplace(key, c);

    write_op("and", c, a, b);

    return c;
}

var_t SymbolManager::make_or(var_t a, var_t b)
{
    Assert(is_known(a), UNKNOWN_LITERAL);
    Assert(is_known(b), UNKNOWN_LITERAL);

    // Standard rules for 'or' with constant
    if (a == var_t::ONE || b == var_t::ONE) return var_t::ONE;
    if (a == var_t::ZERO) return b;
    if (b == var_t::ZERO) return a;
    // Simplification rules for 'or'
    if (a == b) return a;
    auto not_b = m_not_cache.find(b);
    const bool exists_not_b = not_b != m_not_cache.end();
    if (exists_not_b && a == not_b->second) return var_t::ONE;

    const binary_key_t key = {a < b ? a : b, a < b ? b : a};
    auto res = m_or_cache.find(key);
    if (res != m_or_cache.end()) return res->second;

    var_t c = new_var();
    m_or_cache.emplace(key, c);

    write_op("or", c, a, b);

    return c;
}

var_t SymbolManager::make_xor(var_t a, var_t b)
{
    Assert(is_known(a), UNKNOWN_LITERAL);
    Assert(is_known(b), UNKNOWN_LITERAL);

    // Standard rules for 'xor' with constant
    if (a == var_t::ZERO) return b;
    if (b == var_t::ZERO) return a;
    if (a == var_t::ONE) return make_not(b);
    if (b == var_t::ONE) return make_not(a);

    // Simplification rules for 'xor'
    if (a == b) return var_t::ZERO;
    auto not_b = m_not_cache.find(b);
    const bool exists_not_b = not_b != m_not_cache.end();
    if (exists_not_b && a == not_b->second) return var_t::ONE;

    const binary_key_t key = {a < b ? a : b, a < b ? b : a};
    auto res = m_xor_cache.find(key);
    if (res != m_xor_cache.end()) return res->second;

    var_t c = new_var();
    m_xor_cache.emplace(key, c);

    write_op("xor", c, a, b);

    return c;
}

var_t SymbolManager::make_mux(var_t s, var_t t, var_t e)
{
    Assert(is_known(s), UNKNOWN_LITERAL);
    Assert(is_known(t), UNKNOWN_LITERAL);
    Assert(is_known(e), UNKNOWN_LITERAL);

    // The formula representation is (s & t) | (-s & e)
    if (s == var_t::ONE) return t;  // ... = t | 0 = t
    if (s == var_t::ZERO) return e; // ... = 0 | e = e

    if (t == e) return t;               // ... = t & (-s | s) = t

    if (t == var_t::ONE) return make_or(s, e);    // ... = s | (-s & e) = s | e
    if (t == var_t::ZERO) return make_and(make_not(s), e); // ... = 0 | (-s & e) = (-s & e)

    if (e == var_t::ONE) return make_or(make_not(s), t);   // ... = (s & t) | -s  = -s | t
    if (e == var_t::ZERO) return make_and(s, t);  // ... = (s & t) | 0 = (s & t)

    auto not_e = m_not_cache.find(e);
    const bool exists_not_e = not_e != m_not_cache.end();
    if (exists_not_e && t == not_e->second) return make_xor(s, e); // ... = (s & -e) | (-s & e)

    if (t == s) return make_or(s, e);    // ... = s | (-s & e) = s | e

    auto not_s = m_not_cache.find(s);
    const bool exists_not_s = not_s != m_not_cache.end();
    if (exists_not_s && t == not_s->second) return make_and(not_s->second, e); // ... = 0 | (-s & e) = (-s & e);

    if (e == s) return make_and(s, t);   // ... = (s & t) | 0  = (s & t)

    if (exists_not_s && e == not_s->second) return make_or(not_s->second, t);  // ... = (s & t) | -s = -s | t

    // every simplification case should be handled by this point
    const ternary_key_t key = {s, t, e};
    auto res = m_mux_cache.find(key);
    if (res != m_mux_cache.end()) return res->second;

    var_t r = new_var();
    m_mux_cache.emplace(key, r);

    write_op("mux", r, s, e, t);

    return r;
}
