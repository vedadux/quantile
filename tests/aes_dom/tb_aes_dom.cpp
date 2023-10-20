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

#include "../common_tb_utils.h"
#include "../../Simulator.h"

#include <filesystem>

std::map<uint64_t, std::string> STATE_NAMES = {
        {0b00000, "STATE_0"},
        {0b00011, "STATE_1"},
        {0b00111, "STATE_2"},
        {0b01111, "STATE_3"},
        {0b11111, "STATE_4"},
        {0b11110, "STATE_5"},
        {0b11101, "STATE_6"},
        {0b11010, "STATE_7"},
        {0b10101, "STATE_8"},
        {0b01010, "STATE_9"},
        {0b10100, "STATE_10"},
        {0b01001, "STATE_11"},
        {0b10011, "STATE_12"},
        {0b00110, "STATE_13"},
        {0b01100, "STATE_14"},
        {0b11000, "STATE_15"},
        {0b10001, "STATE_16"},
        {0b00010, "STATE_17"},
        {0b00100, "STATE_18"},
        {0b01000, "STATE_19"},
        {0b10000, "STATE_20"},
};

void debug_cycle(Simulator& sim)
{
    std::cout << "RstxBI :" << sim["RstxBI"].as_uint64_t() << std::endl;
    std::cout << "StartxSI :" << sim["StartxSI"].as_uint64_t() << std::endl;
    std::cout << "DonexSO :" << sim["DonexSO"].as_uint64_t() << std::endl;
    std::cout << "lsfrxdp : " << STATE_NAMES[sim["controller_selection_g.aes_ctrl_lsfr_1.lsfrxdp"].as_uint64_t()] << std::endl;
    std::cout << "rcon_1.rconxdp : " << sim["rcon_1.rconxdp"].as_uint64_t() << std::endl;
}

void aes_dom_fresh_masks_common(Simulator& sim, uint32_t& num_masks)
{
    sim["Binv1xDI"][{3, 0}] = sim.masks({num_masks +  3, num_masks +  0});
    sim["Binv2xDI"][{3, 0}] = sim.masks({num_masks +  7, num_masks +  4});
    sim["Binv3xDI"][{3, 0}] = sim.masks({num_masks + 11, num_masks +  8});
    sim["Bmul1xDI"][{7, 0}] = sim.masks({num_masks + 19, num_masks + 12});

    sim["Zinv1xDI"][{1, 0}] = sim.masks({num_masks + 21, num_masks + 20});
    sim["Zinv2xDI"][{1, 0}] = sim.masks({num_masks + 23, num_masks + 22});
    sim["Zinv3xDI"][{1, 0}] = sim.masks({num_masks + 25, num_masks + 24});
    sim["Zmul1xDI"][{3, 0}] = sim.masks({num_masks + 29, num_masks + 26});
    sim["Zmul2xDI"][{3, 0}] = sim.masks({num_masks + 33, num_masks + 30});
    sim["Zmul3xDI"][{3, 0}] = sim.masks({num_masks + 37, num_masks + 34});
}

void fresh_masks_good(Simulator& sim, uint32_t& num_masks)
{
    sim.allocate_masks({num_masks + 37, num_masks});
    aes_dom_fresh_masks_common(sim, num_masks);
    num_masks += 38;
}

void fresh_masks_unsafe(Simulator& sim, uint32_t& num_masks)
{
    try{ sim.allocate_masks({num_masks + 37, num_masks}); } catch (std::logic_error& err) { }
    aes_dom_fresh_masks_common(sim, num_masks);
    num_masks += 38;
}

void fresh_masks_none(Simulator& sim, uint32_t& num_masks)
{
    sim["Binv1xDI"][{3, 0}] = 0;
    sim["Binv2xDI"][{3, 0}] = 0;
    sim["Binv3xDI"][{3, 0}] = 0;
    sim["Bmul1xDI"][{7, 0}] = 0;
    sim["Zinv1xDI"][{1, 0}] = 0;
    sim["Zinv2xDI"][{1, 0}] = 0;
    sim["Zinv3xDI"][{1, 0}] = 0;
    sim["Zmul1xDI"][{3, 0}] = 0;
    sim["Zmul2xDI"][{3, 0}] = 0;
    sim["Zmul3xDI"][{3, 0}] = 0;
}

const char* ROUND_IDENTIFIER = "rcon_1.rconxdp";

void setup_aes_dom(Simulator& sim, uint32_t& num_masks, MaskUse mask_use)
{
    sim.prepare_cycle();
    sim["RstxBI"] = 0;
    sim["StartxSI"] = 0;
    sim.step_cycle();
    debug_cycle(sim);

    sim.step();
    debug_cycle(sim);

    sim.prepare_cycle();
    sim["RstxBI"] = 1;
    sim["StartxSI"] = 1;
    sim.step_cycle();
    debug_cycle(sim);

    for (uint32_t i = 0; i < 16; i++)
    {
        const uint32_t start = i * 8;

        sim.prepare_cycle();
        sim.allocate_secrets({start + 7, start}, 2);
        sim.allocate_data({start + 7, start}, 2);

        sim["StartxSI"] = 0;
        sim["KxDI"][{7,  0}] = sim.secrets_share_i({start + 7, start}, 0);
        sim["KxDI"][{15, 8}] = sim.secrets_share_i({start + 7, start}, 1);
        sim["PTxDI"][{7,  0}] = sim.data_share_i({start + 7, start}, 0);
        sim["PTxDI"][{15, 8}] = sim.data_share_i({start + 7, start}, 1);
        fresh_masks(sim, num_masks, mask_use);
        sim.step_cycle();
        debug_cycle(sim);
    }

    sim.prepare_cycle();
    sim["KxDI"][{7, 0}] = 0;
    sim["KxDI"][{15, 8}] = 0;
    sim["PTxDI"][{7, 0}] = 0;
    sim["PTxDI"][{15, 8}] = 0;
    sim.step_cycle();
    debug_cycle(sim);
}

bool early_break(Simulator& sim, uint32_t cycles)
{
    return false;
    // return cycles >= 60;
}

void simulate_aes_dom(Simulator& sim, MaskUse mask_use)
{
    uint32_t num_masks = 0;
    setup_aes_dom(sim, num_masks, mask_use);

    uint32_t cycles = 20;
    while (sim["DonexSO"].as_uint64_t() != 1)
    {
        if (early_break(sim, cycles)) return;
        std::cout << "Cycle " << cycles << ":" << std::endl;

        sim.prepare_cycle();
        fresh_masks(sim, num_masks, mask_use);
        sim.step_cycle();
        debug_cycle(sim);

        cycles += 1;
    }

    // std::array<uint8_t, 16> ct = {};
    for (int i = 0; i < 16; i++)
    {
        // ct[i] = sim["CxDO"][{7,0}].as_uint64_t() ^ sim["CxDO"][{15,8}].as_uint64_t();
        if (i == 15) break;
        sim.prepare_cycle();
        fresh_masks(sim, num_masks, mask_use);
        sim.step_cycle();
    }

    // for (int i = 0; i < 16; i++)
    //     printf("%02x ", ct[i]);
    // printf("\n");
}

#ifndef MASK_USE
#define MASK_USE MaskUse::FRESH_MASKS
#endif

#ifndef JSON_FILE
#define JSON_FILE ""
#endif

int main()
{
    return common_tb(JSON_FILE, "aes_top", MASK_USE, simulate_aes_dom);
}