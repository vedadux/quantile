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

#ifndef VERIFY_CONSTEXPR_HELPERS_H
#define VERIFY_CONSTEXPR_HELPERS_H

#include <cstdint>
#include <algorithm>
#include "apxmath.h"

constexpr uint64_t compute_ndata(double given_epsilon, double given_delta, uint64_t u_hist)
{
    double computed_eps_part_log = apxmath::log2(1.0 + 1.0 / (double)u_hist);
    double eps_part_sqrt = given_epsilon - computed_eps_part_log;

    // compute n_d from n_x and given_epsilon
    const double l2_u_hist = apxmath::log2((double)u_hist);
    const double sigma_div_n = ((2 * (0.25 * (double)u_hist) + 2 * (l2_u_hist * l2_u_hist)) / (double)u_hist);
    return (uint64_t)((sigma_div_n * (-2.0 * apxmath::log(given_delta))) / (eps_part_sqrt * eps_part_sqrt));
}

constexpr uint64_t compute_uhist(double given_epsilon, double given_delta, uint64_t parallel_size)
{
    double eps_part_log = given_epsilon / 3;
    // compute n_x from given_epsilon
    double d_hist = 1.0 / (apxmath::pow(2.0, eps_part_log) - 1.0);
    uint64_t u_hist = (uint64_t)d_hist;
    u_hist = std::max((u_hist / parallel_size), 1LU) * parallel_size;
    while (u_hist * compute_ndata(given_epsilon, given_delta, u_hist) >
           (u_hist + parallel_size) * compute_ndata(given_epsilon, given_delta, (u_hist + parallel_size)))
        u_hist += parallel_size;
    return u_hist;
}

#endif //VERIFY_CONSTEXPR_HELPERS_H
