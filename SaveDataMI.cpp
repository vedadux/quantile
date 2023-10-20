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

#include "SaveDataMI.h"
#include "OptionsMI.h"

#include <cstring>

SaveDataMI::SaveDataMI():
    cycles(0),
    num_samples_f_given_d(0),
    num_samples_f_given_ds(0),
    num_secrets(0),
    num_runs(0),
    duration_ms(0),
    run_length(0),
    sumof_mi_f_s_given_d(nullptr)
{
    memcpy(hash, OBJ_HASH, HASH_SIZE);
}

SaveDataMI::~SaveDataMI()
{
    if (sumof_mi_f_s_given_d != nullptr) free(sumof_mi_f_s_given_d);
}

void SaveDataMI::assert_integrity(const OptionsMI* opts, uint64_t rl)
{
    if (cycles != opts->cycles)
        throw SaveDataException("Unexpected cycles mismatch");
    if (num_samples_f_given_d != opts->num_samples_f_given_d)
        throw SaveDataException("Unexpected num_samples_f_given_d mismatch");
    if (num_samples_f_given_ds != opts->num_samples_f_given_ds)
        throw SaveDataException("Unexpected num_samples_f_given_ds mismatch");
    if (num_secrets != opts->num_secrets)
        throw SaveDataException("Unexpected num_secrets mismatch");
    if (run_length != rl)
        throw SaveDataException("Unexpected run_length mismatch");
}

std::ifstream& operator>>(std::ifstream& in, SaveDataMI& data)
{
    in.read(data.hash, sizeof(data.hash));
    if (memcmp(data.hash, OBJ_HASH, 64) != 0)
        throw SaveDataException("Loaded hash does not match");
    in.read(reinterpret_cast<char*>(&data.cycles), sizeof(data.cycles));
    in.read(reinterpret_cast<char*>(&data.num_samples_f_given_d), sizeof(data.num_samples_f_given_d));
    in.read(reinterpret_cast<char*>(&data.num_samples_f_given_ds), sizeof(data.num_samples_f_given_ds));
    in.read(reinterpret_cast<char*>(&data.num_secrets), sizeof(data.num_secrets));
    in.read(reinterpret_cast<char*>(&data.num_runs), sizeof(data.num_runs));
    in.read(reinterpret_cast<char*>(&data.duration_ms), sizeof(data.duration_ms));
    if (data.run_length != 0) free(data.sumof_mi_f_s_given_d);
    in.read(reinterpret_cast<char*>(&data.run_length), sizeof(data.run_length));
    data.sumof_mi_f_s_given_d = (double*) aligned_alloc(sizeof(double), data.run_length * sizeof(double));
    in.read(reinterpret_cast<char*>(data.sumof_mi_f_s_given_d), data.run_length * sizeof(double));

    return in;
}

std::ofstream& operator<<(std::ofstream& out, SaveDataMI& data)
{
    out.write(reinterpret_cast<char*>(&data.hash), sizeof(data.hash));
    out.write(reinterpret_cast<char*>(&data.cycles), sizeof(data.cycles));
    out.write(reinterpret_cast<char*>(&data.num_samples_f_given_d), sizeof(data.num_samples_f_given_d));
    out.write(reinterpret_cast<char*>(&data.num_samples_f_given_ds), sizeof(data.num_samples_f_given_ds));
    out.write(reinterpret_cast<char*>(&data.num_secrets), sizeof(data.num_secrets));
    out.write(reinterpret_cast<char*>(&data.num_runs), sizeof(data.num_runs));
    out.write(reinterpret_cast<char*>(&data.duration_ms), sizeof(data.duration_ms));
    out.write(reinterpret_cast<char*>(&data.run_length), sizeof(data.run_length));
    out.write(reinterpret_cast<char*>(data.sumof_mi_f_s_given_d), data.run_length * sizeof(double));

    return out;
}

SaveDataMI& SaveDataMI::operator+=(const SaveDataMI& other)
{
    if (cycles != other.cycles)
        throw SaveDataException(("Unexpected cycles mismatch"));
    if (num_samples_f_given_d != other.num_samples_f_given_d)
        throw SaveDataException("Unexpected num_samples_f_given_d mismatch");
    if (num_samples_f_given_ds != other.num_samples_f_given_ds)
        throw SaveDataException("Unexpected num_samples_f_given_ds mismatch");
    if (num_secrets != other.num_secrets)
        throw SaveDataException("Unexpected num_secrets mismatch");
    if (run_length != other.run_length)
        throw SaveDataException("Unexpected run_length mismatch");

    num_runs += other.num_runs;
    duration_ms += other.duration_ms;

    if (run_length != 0)
    {
        if (other.sumof_mi_f_s_given_d == nullptr)
            throw SaveDataException("Unexpected nullptr in other");
        for (uint32_t i = 0; i < run_length; i++)
            sumof_mi_f_s_given_d[i] += other.sumof_mi_f_s_given_d[i];
    }

    return *this;
}