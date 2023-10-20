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

#ifndef COMMON_MASK_UTILS_H
#define COMMON_MASK_UTILS_H

#include <cstdint>
#include <filesystem>

#include "../Simulator.h"

enum class MaskUse : uint32_t {FRESH_MASKS = 0, NO_MASKS = 1, ALWAYS_SAME_MASKS = 2, SAME_MASK_IN_N_CYCLES = 3,
                               SAME_MASKS_AFTER_N_ROUNDS};

#ifndef NUM_CYCLES_WITH_SAME_MASK
#define NUM_CYCLES_WITH_SAME_MASK 2
#endif

#ifndef NUM_ROUNDS_BEFORE_MASK_REPEAT
#define NUM_ROUNDS_BEFORE_MASK_REPEAT 1
#endif

#define SELF_PATH std::string(__FILE__)
#define DIR_PATH  ((SELF_PATH).substr(0, (SELF_PATH).find_last_of('/')))
#define TMP_PATH  std::string(std::filesystem::canonical((DIR_PATH) + "/../tmp"))

#ifndef GENERATED_CPP
#define GENERATED_CPP ((TMP_PATH) + "/tmp_runner.cpp")
#endif

#ifndef GENERATED_VCD
#define GENERATED_VCD ((TMP_PATH) + "/tmp_runner.vcd")
#endif

void fresh_masks(Simulator& sim, uint32_t& num_masks, MaskUse mask_use);

inline int common_tb(const std::string& circ_path, const std::string& top_module,
                     MaskUse mask_use, void (*simulate_circuit)(Simulator&, MaskUse))
{
    const std::string outf_path = GENERATED_CPP;
    const std::string vcdf_path = GENERATED_VCD;

    std::cout << "Circuit:       " << circ_path << std::endl;
    std::cout << "Generated CPP: " << outf_path << std::endl;
    std::cout << "Generated VCD: " << vcdf_path << std::endl;

    Circuit circ(circ_path, top_module);
    std::ofstream ofs(outf_path);
    Simulator sim(circ, &ofs);

    sim.emit_prologue();
    simulate_circuit(sim, mask_use);
    sim.emit_epilogue();

    Symbol::s_manager.unset_stream();
    ofs.close();
    sim.dump_vcd(vcdf_path);

    return 0;
}

#endif //COMMON_MASK_UTILS_H
