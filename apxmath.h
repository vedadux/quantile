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

#ifndef VERIFY_APXMATH_H
#define VERIFY_APXMATH_H

#include <cassert>
#include <cstdint>

namespace apxmath
{

constexpr double E_CON = 2.71828182845904523536028747135266249775724709369995957l;

constexpr double DIV_CONST    = E_CON;
constexpr double LN_DIV_CONST = 1.0;

/* implementation of log algorithm
https://math.stackexchange.com/questions/977586/is-there-an-approximation-to-the-natural-log-function-at-large-values
*/
constexpr double log(double x)
{
    assert(x > 0.0l);
    double sign = 1.0;
    if (x < 1.0l)
    {
        sign = -1.0l;
        x = 1.0l / x;
    }
    assert(x >= 1.0l);
    uint32_t n = 0;
    while (x / DIV_CONST >= 1.0l)
    {
        n += 1;
        x /= DIV_CONST;
    }
    const double y = (x - 1.0l) / (x + 1.0l);

    double res = 0;

    uint32_t k = 0;
    double y_pow_k = 1.0l;

    for (; true; k += 1, y_pow_k *= y)
    {
        const double numerator = y_pow_k * y_pow_k * y;
        const double denominator = k + k + 1;
        const double increase = numerator / denominator;
        if (increase == 0.0l) break;
        res += increase;
    }

    return sign * ((n * LN_DIV_CONST) + (2.0l * res));
}

constexpr double exp(double x)
{
    bool sign = x < 0.0l;
    if (sign) x = -x;
    assert(x >= 0.0l);
    uint32_t n = 0;
    while (x > 1.0l)
    {
        n += 1;
        x /= 2;
    }

    double res = 0;

    uint32_t k = 0;
    double numerator = 1.0l;
    double denominator = 1.0l;
    for(; true; k += 1, denominator *= k, numerator *= x)
    {
        const double increase = numerator / denominator;
        if (increase == 0.0l) break;
        res += increase;
    }

    for (uint32_t i = 0; i < n; i++)
        res = res * res;
    return sign ? 1.0l / res : res;
}

constexpr double pow(double base, double x)
{
    assert(base > 0.0l);
    return exp(x * log(base));
}

constexpr double log2(const double x)
{
    return log(x) / log(2);
}

}
#endif //VERIFY_APXMATH_H
