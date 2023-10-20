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

#include <cassert>
#include <filesystem>

#include "OptionsMI.h"
#include "cxxopts.hpp"
#include "circuit_utils.h"
#include "apxmath.h"

OptionsMI::OptionsMI(int argc, const char* argv[]):
    cycles(DEFAULT_CYCLES),
    num_threads(DEFAULT_NUM_THREADS),
    delta(DEFAULT_DELTA),
    early_stop(DEFAULT_EARLY_STOP),
    num_samples_f_given_d(DEFAULT_NUM_SAMPLES_F_GIVEN_D),
    num_samples_f_given_ds(DEFAULT_NUM_SAMPLES_F_GIVEN_DS),
    num_secrets(DEFAULT_NUM_SECRETS),
    num_data(DEFAULT_NUM_DATA),
    num_samples(DEFAULT_NUM_SAMPLES),
    timeout(DEFAULT_TIMEOUT),
    print_best(DEFAULT_PRINT_BEST),
    print_interval(DEFAULT_PRINT_INTERVAL),
    load_file(),
    store_file(),
    report_file()
{
    cxxopts::Options parser(argv[0], "Verify");
    parser.add_options()
        ("h,help", "Show this help menu")
        ("c,cycles", "Number of cycles to verify",
         cxxopts::value<uint32_t>()->default_value(std::to_string(cycles)))
        ("epsilon", "Epsilon distance of approximation from reality",
         cxxopts::value<double>())
        ("delta", "Delta confidence of approximation in interval",
         cxxopts::value<double>()->default_value(std::to_string(delta)))
        ("early-stop", "Stop execution when far above/below detectable threshold " + std::string(early_stop ? "(default)" : ""))
        ("no-early-stop", "Do not stop execution when far above/below detectable threshold " + std::string(!early_stop ? "(default)" : ""))
        ("num-samples-f-given-d", "Number of samples for entropy H(F|D=d)",
         cxxopts::value<uint64_t>()->default_value(std::to_string(num_samples_f_given_d)))
        ("num-samples-f-given-ds", "Number of samples for entropy H(F|D=d,S=s)",
         cxxopts::value<uint64_t>()->default_value(std::to_string(num_samples_f_given_ds)))
        ("s,num-secrets", "Number of secret values s when averaging H(F|D=d,S=s)",
         cxxopts::value<uint64_t>()->default_value(std::to_string(num_secrets)))
        ("d,num-data", "Number of data values d when averaging MI(S;F|D=d)",
         cxxopts::value<uint64_t>()->default_value(std::to_string(num_data)))
        ("n,num-samples", "Total number of samples taken",
         cxxopts::value<uint64_t>()->default_value(std::to_string(num_samples)))
        ("t,num-threads", "Number of threads to run sampling",
         cxxopts::value<uint32_t>()->default_value(std::to_string(num_threads)))
        ("x,timeout", "Terminate the program after this many seconds",
         cxxopts::value<uint32_t>()->default_value(std::to_string(timeout)))
        ("print-best", "Number of best leaks to continuously print",
         cxxopts::value<uint32_t>()->default_value(std::to_string(print_best)))
        ("print-interval", "Number of seconds in-between printing leaks",
         cxxopts::value<uint32_t>()->default_value(std::to_string(print_interval)))
        ("i,load-file", "File from which to load results",
         cxxopts::value<std::string>()->default_value(DEFAULT_LOAD_FILE))
        ("o,store-file", "File to which to store results",
         cxxopts::value<std::string>()->default_value(DEFAULT_STORE_FILE))
        ("r,report-file", "File to which to print final report",
         cxxopts::value<std::string>()->default_value(DEFAULT_REPORT_FILE))
         ;
    try
    {
        const int orig_argc = argc;
        cxxopts::ParseResult opts = parser.parse(argc, argv);
        if(opts.count("help"))
        {
            std::cout << parser.help() << std::endl;
            throw OptionsException("");
        }

        if(opts.count("timeout"))
            timeout = opts["timeout"].as<uint32_t>();

        if(opts.count("timeout"))
            timeout = opts["timeout"].as<uint32_t>();

        if(opts.count("print-best"))
            print_best = opts["print-best"].as<uint32_t>();

        if(opts.count("print-interval"))
            print_interval = opts["print-interval"].as<uint32_t>();

        if(opts.count("cycles"))
            cycles = opts["cycles"].as<uint32_t>();

        if(opts.count("delta"))
            delta = opts["delta"].as<double>();

        if(opts.count("early-stop") && opts.count("no-early-stop"))
            throw cxxopts::OptionException("Contradicting early-stop");

        if(opts.count("early-stop"))
            early_stop = true;

        if(opts.count("no-early-stop"))
            early_stop = false;

        if(opts.count("epsilon"))
        {
            double epsilon = opts["epsilon"].as<double>();
            if (epsilon <= 0)
                throw OptionsException("epsilon must be positive");

            uint64_t u_hist = compute_uhist(epsilon, delta, PARALLEL_SIZE);

            num_samples_f_given_d = u_hist;
            num_samples_f_given_ds = u_hist;
            num_secrets = 1;
            num_data = compute_ndata(epsilon, delta, u_hist);

            num_samples = num_data * (num_samples_f_given_d + num_secrets * num_samples_f_given_ds);
        }
        else
        {
            if(opts.count("num-samples-f-given-d"))
                num_samples_f_given_d = opts["num-samples-f-given-d"].as<uint64_t>();
            if(opts.count("num-samples-f-given-ds"))
                num_samples_f_given_ds = opts["num-samples-f-given-ds"].as<uint64_t>();
            if(opts.count("num-secrets"))
                num_secrets = opts["num-secrets"].as<uint64_t>();

            const uint64_t factor = (num_samples_f_given_d + num_secrets * num_samples_f_given_ds);
            if (num_samples_f_given_d == 0 || num_samples_f_given_ds == 0 || num_secrets == 0)
                throw cxxopts::OptionException("Sampling number cannot be zero.");

            {
                const uint64_t mod = num_samples_f_given_d % PARALLEL_SIZE;
                if (mod != 0)
                {
                    std::cout << "Overriding num-samples-f-given-d from " << num_samples_f_given_d << " to ";
                    num_samples_f_given_d = num_samples_f_given_d - mod + PARALLEL_SIZE;
                    std::cout << num_samples_f_given_d << std::endl;
                }
            }

            {
                const uint64_t mod = num_samples_f_given_ds % PARALLEL_SIZE;
                if (mod != 0)
                {
                    std::cout << "Overriding num-samples-f-given-ds from " << num_samples_f_given_ds << " to ";
                    num_samples_f_given_ds = num_samples_f_given_ds - mod + PARALLEL_SIZE;
                    std::cout << num_samples_f_given_ds << std::endl;
                }
            }
            assert(factor != 0);

            if(opts.count("num-data") && opts.count("num-samples"))
            {
                num_data = opts["num-data"].as<uint64_t>();
                num_samples = opts["num-samples"].as<uint64_t>();
                const uint64_t computed_ns = num_data * factor;
                if (num_samples != computed_ns)
                    throw cxxopts::OptionException("Specified number of samples (" +
                                                    std::to_string(num_samples) +
                                                    ") mismatches derived number of samples (" +
                                                    std::to_string(computed_ns) + ")");
            }
            else if (opts.count("num-data"))
            {
                num_data = opts["num-data"].as<uint64_t>();
                num_samples = num_data * factor;
            }
            else if (opts.count("num-samples"))
            {
                num_samples = opts["num-samples"].as<uint64_t>();
                num_data = (num_samples / factor) + ((num_samples % factor) != 0);
            }

            if (num_samples == 0 || num_data == 0)
                throw cxxopts::OptionException("Sampling number cannot be zero.");

        }
        if (opts.count("num-threads"))
        {
            num_threads = opts["num-threads"].as<uint32_t>();
            if (num_threads == 0) throw cxxopts::OptionException("Number of threads cannot be zero");
        }

        if (opts.count("load-file"))
        {
            load_file = opts["load-file"].as<std::string>();
            std::filesystem::path file_path{load_file};
            if (!std::filesystem::exists(file_path))
            {
                std::cout << "Warning: Load file \"" << load_file << "\" does not exist" << std::endl;
                load_file = DEFAULT_LOAD_FILE;
            }
        }

        if (opts.count("store-file"))
        {
            store_file = opts["store-file"].as<std::string>();
            std::filesystem::path file_path{store_file};
            if (load_file != store_file && std::filesystem::exists(file_path))
                throw cxxopts::OptionException("Store file exists and would be overwritten");
        }

        if (opts.count("report-file"))
        {
            report_file = opts["report-file"].as<std::string>();
            if (report_file == load_file || report_file == store_file)
            {
                throw cxxopts::OptionException("Report would overwrite either store or load file");
            }
            std::filesystem::path file_path{report_file};
            if (std::filesystem::exists(file_path))
            {
                std::cout << "Warning: Overwriting report file \"" << report_file << "\"" << std::endl;
            }
        }
    }
    catch (const cxxopts::OptionException& e)
    {
        throw OptionsException(e.what());
    }
}

std::ostream& operator<<(std::ostream& out, OptionsMI& opts)
{
    out << "cycles:                 " << opts.cycles << " ";
    out << ((opts.cycles == OptionsMI::DEFAULT_CYCLES) ? "(default)" : "(custom)") << std::endl;
    out << "delta:                  " << opts.delta << " ";
    out << ((opts.delta == OptionsMI::DEFAULT_DELTA) ? "(default)" : "(custom)") << std::endl;
    out << "num-samples-f-given-d:  " << opts.num_samples_f_given_d << " ";
    out << ((opts.num_samples_f_given_d == OptionsMI::DEFAULT_NUM_SAMPLES_F_GIVEN_D) ? "(default)" : "(custom)") << std::endl;
    out << "num-samples-f-given-ds: " << opts.num_samples_f_given_ds << " ";
    out << ((opts.num_samples_f_given_ds == OptionsMI::DEFAULT_NUM_SAMPLES_F_GIVEN_DS) ? "(default)" : "(custom)") << std::endl;
    out << "num-secrets:            " << opts.num_secrets << " ";
    out << ((opts.num_secrets == OptionsMI::DEFAULT_NUM_SECRETS) ? "(default)" : "(custom)") << std::endl;
    out << "num-data:               " << opts.num_data << " ";
    out << ((opts.num_data == OptionsMI::DEFAULT_NUM_DATA) ? "(default)" : "(custom)") << std::endl;
    out << "num-samples:            " << opts.num_samples << " ";
    out << ((opts.num_samples == OptionsMI::DEFAULT_NUM_SAMPLES) ? "(default)" : "(custom)") << std::endl;
    out << "num-threads:            " << opts.num_threads << " ";
    out << ((opts.num_threads == OptionsMI::DEFAULT_NUM_THREADS) ? "(default)" : "(custom)") << std::endl;
    out << "timeout:                " << opts.timeout << " ";
    out << ((opts.timeout == OptionsMI::DEFAULT_TIMEOUT) ? "(default)" : "(custom)") << std::endl;
    out << "print_best:             " << opts.print_best << " ";
    out << ((opts.print_best == OptionsMI::DEFAULT_PRINT_BEST) ? "(default)" : "(custom)") << std::endl;
    out << "print-interval:        " << opts.print_interval << " ";
    out << ((opts.print_interval == OptionsMI::DEFAULT_PRINT_INTERVAL) ? "(default)" : "(custom)") << std::endl;
    out << "load-file:              " << opts.load_file << " ";
    out << ((opts.load_file == OptionsMI::DEFAULT_LOAD_FILE) ? "(default)" : "(custom)") << std::endl;
    out << "store-file:             " << opts.store_file << " ";
    out << ((opts.store_file == OptionsMI::DEFAULT_STORE_FILE) ? "(default)" : "(custom)") << std::endl;
    out << "report-file:            " << opts.report_file << " ";
    out << ((opts.report_file == OptionsMI::DEFAULT_REPORT_FILE) ? "(default)" : "(custom)") << std::endl;

    return out;
}