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

#ifndef SAVEDATA_MI_H
#define SAVEDATA_MI_H

#include <fstream>
#include <stdexcept>

#define HASH_SIZE 64

extern char* OBJ_HASH;
class OptionsMI;

struct SaveDataMI {
    char hash[HASH_SIZE];
    uint32_t cycles;
    uint64_t num_samples_f_given_d;
    uint64_t num_samples_f_given_ds;
    uint64_t num_secrets;
    uint64_t num_runs;
    uint64_t duration_ms;
    uint64_t run_length;
    double* sumof_mi_f_s_given_d;

    SaveDataMI();
    ~SaveDataMI();
    void assert_integrity(const OptionsMI* opts, uint64_t rl);

    friend std::ifstream& operator>>(std::ifstream& in, SaveDataMI& data);
    friend std::ofstream& operator<<(std::ofstream& out, SaveDataMI& data);
    SaveDataMI& operator+=(const SaveDataMI& other);
};

struct SaveDataException : std::logic_error
{
    SaveDataException(const char* msg) : std::logic_error(msg) {};
};

#endif // SAVEDATA_MI_H
