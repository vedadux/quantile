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

#ifndef OPTIONS_MI_H
#define OPTIONS_MI_H

#include <string>
#include <stdexcept>
#include "constexpr_helpers.h"
#include "circuit_utils.h"

struct OptionsMI {
    static constexpr uint32_t DEFAULT_CYCLES = -1;
    static constexpr uint64_t DEFAULT_NUM_THREADS = 1;
    static const bool DEFAULT_EARLY_STOP = true;

    static constexpr double DEFAULT_DELTA = 0.00001; // 99.999% certainty
    static constexpr double DEFAULT_EPSILON = 0.001;

    static constexpr uint64_t DEFAULT_NUM_SAMPLES_FOR_HIST = compute_uhist(DEFAULT_EPSILON, DEFAULT_DELTA, PARALLEL_SIZE);
    static constexpr uint64_t DEFAULT_NUM_SAMPLES_F_GIVEN_D = DEFAULT_NUM_SAMPLES_FOR_HIST;
    static constexpr uint64_t DEFAULT_NUM_SAMPLES_F_GIVEN_DS = DEFAULT_NUM_SAMPLES_FOR_HIST;
    static constexpr uint64_t DEFAULT_NUM_SECRETS = 1;
    static constexpr uint64_t DEFAULT_NUM_DATA = compute_ndata(DEFAULT_EPSILON, DEFAULT_DELTA, DEFAULT_NUM_SAMPLES_FOR_HIST);
    static constexpr uint64_t DEFAULT_NUM_SAMPLES = DEFAULT_NUM_DATA * (DEFAULT_NUM_SAMPLES_F_GIVEN_D +
                                                                    DEFAULT_NUM_SECRETS * DEFAULT_NUM_SAMPLES_F_GIVEN_DS);
    static constexpr uint32_t DEFAULT_TIMEOUT = 0;
    static constexpr uint32_t DEFAULT_PRINT_BEST = 10;
    static constexpr uint32_t DEFAULT_PRINT_INTERVAL = 60;
    static constexpr const char* DEFAULT_LOAD_FILE = "";
    static constexpr const char* DEFAULT_STORE_FILE = "";
    static constexpr const char* DEFAULT_REPORT_FILE = "";

    uint32_t cycles;
    uint32_t num_threads;
    double delta;
    bool early_stop;
    uint64_t num_samples_f_given_d;
    uint64_t num_samples_f_given_ds;
    uint64_t num_secrets;
    uint64_t num_data;
    uint64_t num_samples;
    uint32_t timeout;
    uint32_t print_best;
    uint32_t print_interval;
    std::string load_file;
    std::string store_file;
    std::string report_file;

    OptionsMI(int argc, const char* argv[]);
    friend std::ostream& operator<<(std::ostream& out, OptionsMI& opts);
};

struct OptionsException : std::logic_error
{
    OptionsException(const char* msg) : std::logic_error(msg) {};
};

#endif // OPTIONS_MI_H
