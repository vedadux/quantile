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

#include "common_tb_utils.h"
#include <filesystem>

extern void fresh_masks_good(Simulator& sim, uint32_t& num_masks);
extern void fresh_masks_none(Simulator& sim, uint32_t& num_masks);

bool USE_STALE = false;
void fresh_masks_stale(Simulator& sim, uint32_t& num_masks)
{
    if (!USE_STALE) {
        fresh_masks_good(sim, num_masks);
        USE_STALE = true;
    }
}

uint32_t NUM_FRESH_REQ = 0;
void fresh_masks_repeat_cycles(Simulator& sim, uint32_t& num_masks)
{
    if (NUM_FRESH_REQ % NUM_CYCLES_WITH_SAME_MASK == 0) {
        fresh_masks_good(sim, num_masks);
    }
    NUM_FRESH_REQ += 1;
}

extern void fresh_masks_unsafe(Simulator& sim, uint32_t& num_masks);
extern const char* ROUND_IDENTIFIER;

uint64_t CURR_RCON_VAL = 0xdeadbeef;
uint32_t NUM_ROUNDS_SEEN = 0;
void fresh_masks_repeat_rounds(Simulator& sim, uint32_t& num_masks)
{
    uint64_t curr_rcon = sim[ROUND_IDENTIFIER].as_uint64_t();
    if (curr_rcon != CURR_RCON_VAL && CURR_RCON_VAL != 0xdeadbeef)
        NUM_ROUNDS_SEEN += 1;
    CURR_RCON_VAL = curr_rcon;
    printf("DEBUG XX: %ld %ld %d %d\n", curr_rcon, CURR_RCON_VAL, NUM_ROUNDS_SEEN, NUM_ROUNDS_BEFORE_MASK_REPEAT);
    if (NUM_ROUNDS_SEEN == NUM_ROUNDS_BEFORE_MASK_REPEAT)
    {
        printf("Repeating masks!\n");
        NUM_ROUNDS_SEEN = 0;
        num_masks = 0;
    }
    fresh_masks_unsafe(sim, num_masks);
}

void fresh_masks(Simulator& sim, uint32_t& num_masks, const MaskUse mask_use)
{
    switch(mask_use)
    {
        case MaskUse::FRESH_MASKS:
            fresh_masks_good(sim, num_masks);
            break;
        case MaskUse::NO_MASKS:
            fresh_masks_none(sim, num_masks);
            break;
        case MaskUse::ALWAYS_SAME_MASKS:
            fresh_masks_stale(sim, num_masks);
            break;
        case MaskUse::SAME_MASK_IN_N_CYCLES:
            fresh_masks_repeat_cycles(sim, num_masks);
            break;
        case MaskUse::SAME_MASKS_AFTER_N_ROUNDS:
            fresh_masks_repeat_rounds(sim, num_masks);
            break;
        default:
            assert(false);
    }
}