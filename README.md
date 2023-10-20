# Quantile - Quantifier of Information Leakage

## Getting started

The whole project is configured and built using CMake. To get started, create a build directory and run CMake to set everything up.

```bash
mkdir -p build               &&
cd build                     &&
cmake .. -G "Unix Makefiles"
```

Afterwards, you will have make targets for each hardware design. Importantly, it seems like the `gcc`/`g++` compiler has trouble with the kinds of files generated throughout Quantile's workflow. If you experience long compilation times, try switcing your compiler to `clang++`. We found that version 14 seems to work well, i.e., replacing the `cmake` part with:
```bash
cmake .. -G "Unix Makefiles" -D CMAKE_C_COMPILER=clang-14 -D CMAKE_CXX_COMPILER=clang++-14 -D CMAKE_MAKE_PROGRAM=make
```

---

The verification procedure consists of two steps, each of which has its own make target.

#### 1. Symbolic Simulation (target `tb_[design]`)

As the first, step, you are required to write a small testbench that simulates you design symbolically and specifies secrets, masks, and control signals the same way you would expect the design to be used in a real-world setting. Most importantly, the purpose of the testbench is to generate a `[design].cpp` file, containing functions that enable a highly efficient parallelized simulation of your hardware design on concrete inputs. This file is required in the next step. For more information on how to write such a symbolic testbench, consult the dedicated section in the README.

#### 2. Verification (target `quantile_[design]`)

After your testbench works and generates the appropriate `[design].cpp` file, you just need to link it together with the unchanging verification sources. Running the generated program will run quantitative masking verification for your hardware design and report back the results incrementally. Here, you can specify some verification options that are outlined in the dedicated section of the README. If the verification does not report any leaks, for small enough values of epsilon, you gain confidence in its power-analysis resistance. Otherwise, if the tool consistently reports leakage with values higher than epsilon, your design is almost surely susceptible to power-analysis attacks.

## Verification Options

You can get an overview of the available options by running `quantile_[design] --help`. However, here is another quick overview of what each option does.

* `-h`, `--help` Show this help menu
* `-c`, `--cycles` `arg` Sets the number of cycles to run within the verification. Every concrete simulation done by the verifier is terminated after the specified amount of cycles.
* `--epsilon` `arg` Epsilon specifies the target error between the approximated mutual information and the actual mutual information. Specifying epsilon, automatically computes the number of samples required, and all other `--num-samples*`, `--num-data` and `--num-secrets` options are ignored.
* `--delta` `arg` Delta specifies the probability that the approximated amount is outside of the epsilon range of the actual mutual information.
* `--(no-)early-stop` Whether to stop execution when far above/below detectable threshold
* `--num-samples-f-given-d` `arg` Number of samples used for the estimation of entropy H(X|D=d)
* `--num-samples-f-given-ds` `arg` Number of samples used for the estimation of entropy H(X|D=d,S=s)
* `-s`, `--num-secrets` `arg` Number of secret values s for S when averaging H(F|D=d,S=s)
* `-d`, `--num-data` `arg` Number of data values d when averaging MI(S;F|D=d)
* `-n`, `--num-samples` `arg` Total number of samples of X taken for all the estimations together.
* `-t`, `--num-threads` `arg` Number of threads that will run the sampling. Make sure to optimize this, because using as many thready as there are cores on your machine is not always optimal.
* `-x`, `--timeout` `arg` Terminate the program after a specified number of seconds
* `--print-best` `arg` Continuously print this number of best leaks whose lower bound for MI(X;S|D) is above 0.
* `--print-interval` `arg` Number of seconds in-between printing best found leaks and statistics.
* `-i`, `--load-file` `arg` File from which to load previously computed results and continue computation
* `-o`, `--store-file` `arg` File to which to store computed results when terminating program
* `-r`, `--report-file` `arg` File to which to print final report

## Writing a Testbench

Writing a testbench is a straightforward process, similar to writing a hardware testbench with any other framework.
For inspiration, we highly suggest that you look at the examples in the tests directory.

#### 1. Boilerplate

Testbenches are all alike. In this framework, the standard code to load a design circuit, initialize a simulator and finalize the simulation looks as follows:
```cpp
Circuit circ(path_to_circuit_json, "top_module_name");
std::ofstream ofs(path_to_generated_cpp_file);
Simulator sim(circ, &ofs);

sim.emit_prologue();
simulate_design(sim);
sim.emit_epilogue();
ofs.close();
```
It is not really necessary to understand all the details here, and the heart of the testbench is actually in the `simulate_design` function that you have to write yourself. This function actially specifies the interaction with the design.

#### 2. Simulation Basics

In order to simulate a full clock cycle with the `Simulator` object `sim`, you can do the following API call:
```cpp
sim.step();
```
This is very convenient if you do not need to change any inputs of the circuit and just want to continue running it. However, if you would like to set the values of inputs, you will need to break this down into two seperate API calls.
```cpp
sim.prepare_cycle();
sim.step_cycle();
```
Calling `prepare_cycle` creates a fresh cycle and you can manipulate all input ports of the design. You can access and update the current values of signals using the overloaded operator `operator[]` that takes a string input:
```cpp
auto s_vector = sim["in_data"];
s_vector = 0xfff;
s_vector[0] = false;
s_vector[{15, 8}] = 18;
std::cout << s_vector.as_uint64_t();
```
The simulation value is returned per reference, so any changes to `s_vector` are reflected in the simulators internal state of the circuit. The API supports both assignment with `uint64_t`, access of specific bits and even slicing using a `Range` object, i.e., `s_vector[{15, 8}]` returns the second byte of the multi-bit signal. After setting the values, a call to `step_cycle` simulates the rest of the cycle.

#### 3. Allocating Symbolic Values

Most importantly, this framework supports symbolic simulation, which is also necessary for the verification later on. Here, you can define secrets and masks using the API and assign them to the desired parts of the circuit. The following call allocates 32 secrets with 3 shares each, and 8 data bits with 2 shares each.
```cpp
sim.allocate_secrets({31, 0}, 3);
sim.allocate_data({7, 0}, 2);
```
You can now retrieve the ith shares for a range of secrets or data and assign them to circuit signals.
```cpp
sim["in_key1"] = sim.secrets_share_i({31, 0}, 0);
sim["in_key2"] = sim.secrets_share_i({31, 0}, 1);
sim["in_key3"] = sim.secrets_share_i({31, 0}, 2);
sim["in_data1"][{7,0}] = sim.data_share_i({7, 0}, 0);
sim["in_data2"][{3,0}] = sim.data_share_i({7, 4}, 1);
sim["in_data2"][{7,4}] = sim.data_share_i({3, 0}, 1);
```
Uniformly random masks have a similar interface.
```cpp
sim.allocate_masks({15, 0});
sim["in_random"][{47, 40}] = sim.masks({7, 0});
sim["in_random"][{7, 0}] = sim.masks({15, 8});
```

Operations on symbolic values all have overloaded operators in API, so you could, for example XOR a secret share, data share and mask:
```cpp
SymbolVector masks      = sim.masks({1, 0});
SymbolVector key_share  = sim.secrets_share_i({1, 0}, 0);
SymbolVector data_share = sim.data_share_i({6, 5}, 0);
SymbolVector res = masks ^ key_share ^ data_share;
```

From the point you assign such symbolic values to your signals, the computations they are involved with are performed symbolically. This also means that you cannot access their values as plain `uint64_t` numbers througn `as_uint64_t()`, and you will instead get an exception.

#### 4. Creating the Targets

We provide a single CMake function that allows you to easily specify the two targets required in the Quantile workflow. You can add a testbench target `tb_design` and verifier target `quantile_design` with:
```cmake
add_quantile(
  NAME design                          # Name of your design
  DIR tmp_dir                          # Existing directory for generated files
  SOURCES tb_design.cpp other_src.cpp  # Testbench sources go here
  DEFINES JSON_FILE="design.json"      # Compile time defines go here 
)
```

All the dependencies are reflected properly, so you can just run
`make quantile_[design]` to build the quantile verifier for your design. This will in turn build the testbench target `tb_[design]`, run it to generate the CPP runner in the specified directory, and then link it all into the `quantile_[design]` target. The built target `quantile_[design]` will be in (a sub-directory of) `build`, from which you can run it with, e.g., `./build/tests/my_design/design`.

## License

Quantile is released under the permissive Apache v2.0 license, as detailed in [LICENSE](LICENSE).

Crutially, the Apache v2.0 license exclusively covers files that belong to Quantile itself. Hardware designs located in sub-directories  `designs` and `tests` are subject to their own licensing terms. Furthermore, any sub-modules of this repository are subject to their own licensing terms and are not part of Quantile.

Although the Quantile testbenches we provide are licensed under Apache v2.0, the code they generate from hardware design is licensed under the same terms as the design itself, as are all resulting binaries. Therefore, every file in this repository contains copyright and licensing information to clear up any confusion.

---

Copyright 2023 Vedad Hadžić, Graz University of Technology