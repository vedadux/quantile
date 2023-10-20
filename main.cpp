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

#include <iostream>
#include <string>

#include "common.h"
#include "labels.h"
#include "VCDStorage.h"
#include "CircuitGraph.h"
#include "OptionsCOV.h"
#include "Checker.h"
#include "BddChecker.h"
#include "SimChecker.h"

int main(int argc, const char* argv[]) {
    int ret_val = 0;
    Options* opts = nullptr;
    CircuitGraph* graph = nullptr;
    VCDStorage* vcd = nullptr;
    LabelStorage labels;
    Checker* checker = nullptr;

    try { opts = new Options(argc, argv); }
    catch (const OptionsException& e)
    {
        if (std::string("") != e.what())
        {
            std::cout << "Error while parsing options: " << e.what() << std::endl;
            ret_val = 1; goto cleanup;
        }
        return 0;
    }
    std::cout << *opts;

    try { graph = new CircuitGraph(opts->json_path.c_str()); }
    catch (const std::ifstream::failure& e)
    {
        std::cout << "Failed reading circuit: " << e.what() << std::endl;
        ret_val = 2; goto cleanup;
    }

    try { vcd = new VCDStorage(opts->vcd_path.c_str(), opts->clock_name, static_cast<SignalValue>(opts->clock_edge)); }
    catch (const std::ifstream::failure& e)
    {
        std::cout << "Failed reading VCD: " << e.what() << std::endl;
        ret_val = 2; goto cleanup;
    }
    try { make_labels(opts->label_path.c_str(), labels, graph); }
    catch (const std::ifstream::failure& e)
    {
        std::cout << "Failed reading labels: " << e.what() << std::endl;
        ret_val = 3; goto cleanup;
    } catch (const LabelsException& e)
    {
        std::cout << "Failed to create labels: " << e.what() << std::endl;
        ret_val = 3; goto cleanup;
    }

    checker = new SimChecker(graph, vcd, labels, opts);
    checker->check();

cleanup:
    delete opts;
    delete graph;
    delete vcd;
    delete checker;
    return ret_val;
}
