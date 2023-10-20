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

#include "../../Simulator.h"
#include "../common_tb_utils.h"

void simulate_aes_2bit(Simulator& sim, MaskUse mask_use)
{
    sim.prepare_cycle();
    sim.allocate_secrets({127, 0}, 1);
    sim.allocate_masks({1, 0});

    SymbolVector masks = sim.masks({1, 0});
    Symbol m0 = masks[0];
    Symbol m1 = masks[1];
    Symbol m01 = m0 ^ m1;

    SymbolVector key_shares = sim.secrets_share_i({127, 0}, 0);
    for (uint32_t i = 0; i < 16; i++)
    {
        key_shares[8 * i + 0] = key_shares[8 * i + 0] ^ m1;
        key_shares[8 * i + 1] = key_shares[8 * i + 1] ^ m01;
        key_shares[8 * i + 2] = key_shares[8 * i + 2] ^ m01;
        key_shares[8 * i + 3] = key_shares[8 * i + 3] ^ m0;
        key_shares[8 * i + 4] = key_shares[8 * i + 4] ^ m0;
        key_shares[8 * i + 5] = key_shares[8 * i + 5] ^ m1;
        key_shares[8 * i + 6] = key_shares[8 * i + 6] ^ m0;
        key_shares[8 * i + 7] = key_shares[8 * i + 7] ^ m1;
    }

    SymbolVector pt_shares;
    pt_shares.resize(128); // plaintext is 0s

    for (uint32_t i = 0; i < 16; i++)
    {
        pt_shares[8 * i + 0] = pt_shares[8 * i + 0] ^ m1;
        pt_shares[8 * i + 1] = pt_shares[8 * i + 1] ^ m01;
        pt_shares[8 * i + 2] = pt_shares[8 * i + 2] ^ m01;
        pt_shares[8 * i + 3] = pt_shares[8 * i + 3] ^ m0;
        pt_shares[8 * i + 4] = pt_shares[8 * i + 4] ^ m0;
        pt_shares[8 * i + 5] = pt_shares[8 * i + 5] ^ m1;
        pt_shares[8 * i + 6] = pt_shares[8 * i + 6] ^ m0;
        pt_shares[8 * i + 7] = pt_shares[8 * i + 7] ^ m1;
    }

    sim["key_in"] = key_shares;
    sim["pt_in"] = pt_shares;
    sim["m0"][0] = m0;
    sim["m1"][0] = m1;
    sim.step_cycle();
}

int main()
{
    return common_tb(JSON_FILE, "aes_top", MaskUse::NO_MASKS, simulate_aes_2bit);
}
