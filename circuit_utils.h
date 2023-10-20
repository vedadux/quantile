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

#ifndef CIRCUIT_UTILS_H
#define CIRCUIT_UTILS_H

#include <inttypes.h>
#include <random>

#if defined(__AVX512F__)
    #define PARALLEL_SIZE 512
#elif defined(__AVX2__)
    #define PARALLEL_SIZE 256
// #elif defined(__SSE2__)
    // #define PARALLEL_SIZE 128
#else
    #define PARALLEL_SIZE (__WORDSIZE)
#endif

#if PARALLEL_SIZE >= 128
#include <immintrin.h>
#endif

#define PARALLEL_BYTES ((PARALLEL_SIZE) / 8)

#if PARALLEL_SIZE == 512
    typedef __m512i wtype_t;
    const wtype_t ones = _mm512_set1_epi64((uint64_t)-1);
    const wtype_t zeros = _mm512_set1_epi64((uint64_t)0);
    inline __attribute__((always_inline)) wtype_t _and(wtype_t a, wtype_t b)
    { return _mm512_and_si512(a, b); }
    inline __attribute__((always_inline)) wtype_t _or(wtype_t a, wtype_t b)
    { return _mm512_or_si512(a, b); }
    inline __attribute__((always_inline)) wtype_t _xor(wtype_t a, wtype_t b)
    { return _mm512_xor_si512(a, b); }
    inline __attribute__((always_inline)) wtype_t _not(wtype_t a)
    { return _xor(ones, a); }
#elif PARALLEL_SIZE == 256
    typedef __m256i wtype_t;
    const wtype_t ones = _mm256_set1_epi64x((uint64_t)-1);
    const wtype_t zeros = _mm256_set1_epi64x((uint64_t)0);
    inline __attribute__((always_inline)) wtype_t _and(wtype_t a, wtype_t b)
    { return _mm256_and_si256(a, b); }
    inline __attribute__((always_inline)) wtype_t _or(wtype_t a, wtype_t b)
    { return _mm256_or_si256(a, b); }
    inline __attribute__((always_inline)) wtype_t _xor(wtype_t a, wtype_t b)
    { return _mm256_xor_si256(a, b); }
    inline __attribute__((always_inline)) wtype_t _not(wtype_t a)
    { return _xor(ones, a); }
#elif PARALLEL_SIZE == 128
    typedef __m128i wtype_t;
    const wtype_t ones = _mm128_set1_epi64x((uint64_t)-1);
    const wtype_t zeros = _mm128_set1_epi64x((uint64_t)0);
    inline __attribute__((always_inline)) wtype_t _and(wtype_t a, wtype_t b)
    { return _mm_and_si128(a, b); }
    inline __attribute__((always_inline)) wtype_t _or(wtype_t a, wtype_t b)
    { return _mm_or_si128(a, b); }
    inline __attribute__((always_inline)) wtype_t _xor(wtype_t a, wtype_t b)
    { return _mm_xor_si128(a, b); }
    inline __attribute__((always_inline)) wtype_t _not(wtype_t a)
    { return _xor(ones, a); }
#else // PARALLEL_SIZE is low
    typedef uintmax_t wtype_t;
    const wtype_t ones = (uintmax_t)-1;
    const wtype_t zeros = (uintmax_t)0;
    #define _xor(A, B) ((A) ^ (B))
    #define _and(A, B) ((A) & (B))
    #define _or(A, B) ((A) | (B))
    #define _not(A) (~(A))
#endif // PARALLEL_SIZE end

#define _mux(S, E, T) _or(_and(_not(S), (E)), _and((S), (T)))

#if PARALLEL_SIZE == 32
    #define _popcnt(X) (__builtin_popcount(X))
#else // PARALLEL_SIZE is high
    #define _popcnt(X) (__builtin_popcountll(X))
#endif // PARALLEL_SIZE end

#if PARALLEL_SIZE == 32
    #define rant_t std::mt19937
#else // PARALLEL_SIZE is high
    #define rand_t std::mt19937_64
#endif // PARALLEL_SIZE end

#define _low_64_from_128(X) _mm_cvtsi128_si64(X)
#define _shr_bytes_128(X, N) _mm_srli_si128((X),(N))
#define _select_64_from_256(X, N) _mm256_extract_epi64((X),(N))
#define _select_256_from_512(X, N) _mm512_extracti64x4_epi64((X),(N))

inline __attribute__((always_inline)) uintmax_t popcnt(wtype_t a)
{
#if PARALLEL_SIZE <= 64
    return _popcnt(a);
#elif PARALLEL_SIZE == 128
    const uint64_t res0 = _popcnt(_low_64_from_128(a));
    a = _shr_bytes_128(a, 8);
    const uint64_t res1 = _popcnt(_low_64_from_128(a));
    return res0 + res1;
#elif PARALLEL_SIZE == 256
    const uint64_t res0 = _popcnt(_select_64_from_256(a, 0));
    const uint64_t res1 = _popcnt(_select_64_from_256(a, 1));
    const uint64_t res2 = _popcnt(_select_64_from_256(a, 2));
    const uint64_t res3 = _popcnt(_select_64_from_256(a, 3));
    return res0 + res1 + res2 + res3;
#elif PARALLEL_SIZE == 512
    uint64_t res;
    {
        const auto a0 = _select_256_from_512(a, 0);
        const uint64_t res0 = _popcnt(_select_64_from_256(a0, 0));
        const uint64_t res1 = _popcnt(_select_64_from_256(a0, 1));
        const uint64_t res2 = _popcnt(_select_64_from_256(a0, 2));
        const uint64_t res3 = _popcnt(_select_64_from_256(a0, 3));
        res = res0 + res1 + res2 + res3;
    }
    {
        const auto a1 = _select_256_from_512(a, 1);
        const uint64_t res0 = _popcnt(_select_64_from_256(a1, 0));
        const uint64_t res1 = _popcnt(_select_64_from_256(a1, 1));
        const uint64_t res2 = _popcnt(_select_64_from_256(a1, 2));
        const uint64_t res3 = _popcnt(_select_64_from_256(a1, 3));
        res += res0 + res1 + res2 + res3;
    }
    return res;
#else
    assert(false);
#endif
}

inline __attribute__((always_inline)) wtype_t randbytes(rand_t& gen)
{
    constexpr uint32_t Y_SIZE = PARALLEL_BYTES / sizeof(uintmax_t);
    union { wtype_t x; uintmax_t y[Y_SIZE]; } un;
    for (uint32_t i = 0; i < Y_SIZE; i++)
        un.y[i] = gen();
    return un.x;
}

#endif // CIRCUIT_UTILS_H
