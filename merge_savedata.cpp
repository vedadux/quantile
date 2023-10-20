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

#include "SaveDataMI.h"

char* OBJ_HASH = nullptr;

void free_obj_hash() { free(OBJ_HASH); }

int main(int argc, char* argv[])
{
    atexit(free_obj_hash);
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " [INPUTS] OUTPUT" << std::endl;
        return 1;
    }

    if (std::filesystem::exists(std::filesystem::path(argv[argc - 1])))
    {
        std::cout << "Output file exists and would be overwritten, aborting" << std::endl;
        return 2;
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

    for (int i = 2; i < argc - 1; i++) try {
        SaveDataMI curr_data;
        std::ifstream f(argv[i], std::ios_base::binary);
        f >> curr_data;
        f.close();
        res_data += curr_data;
    } catch (SaveDataException& e)
    {
        std::cout << "Exception while processing \"" << argv[i] << "\": " << e.what() << std::endl;
        return 4;
    }

    try {
        std::ofstream f(argv[argc - 1], std::ios_base::binary);
        f << res_data;
        f.close();
    } catch (std::ios_base::failure& fail)
    {
        std::cout << "Failed while writing \"" << argv[argc - 1] << "\": " << fail.what();
        return 5;
    }

    return 0;
}