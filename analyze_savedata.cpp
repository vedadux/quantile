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

#include <fstream>
#include <iostream>
#include <filesystem>
#include <cmath>

#include "SaveDataMI.h"

char* OBJ_HASH = nullptr;

void free_obj_hash() { free(OBJ_HASH); }

void compute_errors(uint64_t ND, uint64_t NS, uint64_t NF_D, uint64_t NF_DS, double delta,
                    double& err_log_up, double& err_log_down, double& err_sqrt)
{
    double sigma_2_t1 = 1.0 / (4 * ND);

    double log2_fd = std::log2(NF_D);
    double sigma_2_t2 = (log2_fd * log2_fd) / (ND * NF_D);

    double log2_fds = std::log2(NF_DS);
    double sigma_2_t3 = (log2_fds * log2_fds) / (ND * NS * NF_DS);

    double sigma_2_t4 = 1.0 / (4 * ND * NS);

    double sigma_2 = (sigma_2_t1 + sigma_2_t4) + (sigma_2_t2 + sigma_2_t3);

    err_sqrt = std::sqrt(2 * sigma_2 * (-std::log(delta)));

    err_log_up = std::log2(1 + 1.0 / NF_D);
    err_log_down = std::log2(1 + 1.0 / NF_DS);
}

int main(int argc, char* argv[])
{
    atexit(free_obj_hash);
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " INPUT" << std::endl;
        return 1;
    }

    OBJ_HASH = (char*)malloc(64);

    SaveDataMI res_data;

    try {
        {
            std::ifstream f(argv[1], std::ios_base::binary);
            f.read(OBJ_HASH, 64);
            f.close();
        }
        {
            std::ifstream f(argv[1], std::ios_base::binary);
            f >> res_data;
            f.close();
        }
    } catch (std::ios_base::failure& fail)
    {
        std::cout << "Failed while reading \"" << argv[1] << "\": " << fail.what();
        return 3;
    }

    double err_log_up, err_log_down, err_sqrt;

    const double deltas[2] = {0.01, 0.00001};
    for (double delta : deltas)
    {
        compute_errors(res_data.num_runs, res_data.num_secrets, res_data.num_samples_f_given_d,
                       res_data.num_samples_f_given_ds, delta, err_log_up, err_log_down, err_sqrt);
        printf("delta:        %11.8f\n", delta);
        printf("err_log_up:   %11.8f\n", err_log_up);
        printf("err_log_down: %11.8f\n", err_log_down);
        printf("err_sqrt:     %11.8f\n", err_sqrt);
        printf("up_epsilon    %11.8f\n", err_log_up + err_sqrt);
        printf("down_epsilon  %11.8f\n", err_log_down + err_sqrt);
        printf("\n\n");
    }

    for


    return 0;
}