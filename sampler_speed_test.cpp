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

#include "circuit_utils.h"

extern void run_circuit(wtype_t* s, uint32_t cycles);
extern void randomize_secrets(wtype_t* s, uint32_t cycles, rand_t& gen);
extern void randomize_data(wtype_t* s, uint32_t cycles, rand_t& gen);
extern void randomize_masks(wtype_t* s, uint32_t cycles, rand_t& gen);

extern const uint32_t RUN_LENGTH;
extern const char* const DEBUG_INFO[];

volatile void simulate_circuit(wtype_t* run_data1, rand_t& generator)
{
    randomize_secrets(run_data1, -1U, generator);
    randomize_data(run_data1, -1U, generator);
    randomize_masks(run_data1, -1U, generator);
    run_circuit(run_data1, -1U);
}


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s NUM_SAMPLES\n", argv[0]);
        return 1;
    }

    rand_t generator;
    wtype_t* run_data1 = (wtype_t*) aligned_alloc(PARALLEL_BYTES, PARALLEL_BYTES * RUN_LENGTH);

    uint64_t num = atol(argv[1]);
    printf("Simulating %ld times\n", num);
    for (uint32_t i = 0; i < num; i+=PARALLEL_SIZE)
        simulate_circuit(run_data1, generator);
    free(run_data1);
    return 0;
}