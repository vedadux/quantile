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
#include <cstdio>
#include <random>
#include <cmath>
#include <cstring>
#include <csignal>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <unistd.h>

#include "circuit_utils.h"
#include "OptionsMI.h"
#include "SaveDataMI.h"

extern void run_circuit(wtype_t* s, uint32_t cycles);
extern void copy_secrets(wtype_t* dst, wtype_t* src, uint32_t cycles);
extern void randomize_secrets(wtype_t* s, uint32_t cycles, rand_t& gen);
extern void copy_data(wtype_t* dst, wtype_t* src, uint32_t cycles);
// extern void randomize_data(wtype_t* s, uint32_t cycles, rand_t& gen);
extern void randomize_masks(wtype_t* s, uint32_t cycles, rand_t& gen);
extern void fix_random_data(wtype_t* s, uint32_t cycles, rand_t& gen);
extern void fix_random_secrets(wtype_t* s, uint32_t cycles, rand_t& gen);
// extern void xor_runs(wtype_t* result, wtype_t* other, uint32_t cycles);
extern void count_run(wtype_t* s, uintmax_t* count, uint32_t cycles);
extern void run_and_count_circuit(wtype_t* s, uintmax_t* count, uint32_t cycles);

extern const uint32_t RUN_LENGTH;
extern const char* const DEBUG_INFO[];

static volatile bool not_stopped = true;
void interupt_handler(int sig) {
    not_stopped = false;
}

struct SamplerMI
{
    static std::mutex mutex;
    static std::condition_variable cond_var;
    static uint32_t samplers_done;

    const uint64_t num_samples_f_given_d;
    const uint64_t num_samples_f_given_ds;
    const uint64_t num_secrets;
    double* lookup_f_given_d;
    double* lookup_f_given_ds;

    rand_t generator;
    wtype_t* run_data1;
    uint64_t* value_count;
    double* entropy_f_given_d;
    double* entropy_f_given_ds;
    double* sumof_mi_f_s_given_d;
    uint64_t run_id;
    SamplerMI(OptionsMI* opts);
    ~SamplerMI();
    inline __attribute__((always_inline)) std::thread run(uint32_t cycles, uint64_t num_runs);
private:
    inline __attribute__((always_inline)) void run_once(uint32_t cycles);
    void run_many(uint32_t cycles, uint64_t num_runs);
};

std::mutex SamplerMI::mutex;
std::condition_variable SamplerMI::cond_var;
uint32_t SamplerMI::samplers_done = 0;

SamplerMI::SamplerMI(OptionsMI* opts) :
        num_samples_f_given_d(opts->num_samples_f_given_d),
        num_samples_f_given_ds(opts->num_samples_f_given_ds),
        num_secrets(opts->num_secrets)
{
    assert(num_samples_f_given_d % PARALLEL_SIZE == 0);
    assert(num_samples_f_given_ds % PARALLEL_SIZE == 0);

    lookup_f_given_d = (double*) aligned_alloc(sizeof(double), sizeof(double) * (num_samples_f_given_d + 1));

    lookup_f_given_d[0] = lookup_f_given_d[num_samples_f_given_d] = 0.0;

    for (int cnt = 1; cnt < num_samples_f_given_d; cnt++)
    {
        const double p = (double)cnt / num_samples_f_given_d;
        lookup_f_given_d[cnt] = -(p * std::log2(p) + (1.0 - p) * std::log2(1.0 - p));
    }

    lookup_f_given_ds = (double*) aligned_alloc(sizeof(double), sizeof(double) * (num_samples_f_given_ds + 1));

    lookup_f_given_ds[0] = lookup_f_given_ds[num_samples_f_given_ds] = 0.0;

    for (int cnt = 1; cnt < num_samples_f_given_ds; cnt++)
    {
        const double p = (double)cnt / num_samples_f_given_ds;
        lookup_f_given_ds[cnt] = -(p * std::log2(p) + (1.0 - p) * std::log2(1.0 - p));
    }

    std::random_device r_dev;
    generator.seed(r_dev());
    run_data1 = (wtype_t*) aligned_alloc(PARALLEL_BYTES, PARALLEL_BYTES * RUN_LENGTH);
    value_count = (uint64_t*) aligned_alloc(sizeof(uint64_t), sizeof(uint64_t) * RUN_LENGTH);
    entropy_f_given_d  = (double*) aligned_alloc(sizeof(double), sizeof(double) * RUN_LENGTH);
    entropy_f_given_ds = (double*) aligned_alloc(sizeof(double), sizeof(double) * RUN_LENGTH);
    sumof_mi_f_s_given_d = (double*) aligned_alloc(sizeof(double), sizeof(double) * RUN_LENGTH);
    std::memset(value_count, 0, sizeof(uint64_t) * RUN_LENGTH);
    std::fill_n(sumof_mi_f_s_given_d, RUN_LENGTH, (double)0);
    run_id = 0;
}

SamplerMI::~SamplerMI()
{
    free(run_data1);
    free(value_count);
    free(entropy_f_given_d);
    free(entropy_f_given_ds);
    free(sumof_mi_f_s_given_d);

    free(lookup_f_given_d);
    free(lookup_f_given_ds);
}

inline __attribute__((always_inline)) void SamplerMI::run_once(uint32_t cycles)
{
    // Fix the data
    fix_random_data(run_data1, cycles, generator);

    // uint32_t num_runs_intern = 0;

    // Sample things for H(F|D=d)
    {
        std::memset(value_count, 0, sizeof(uint64_t) * RUN_LENGTH);
        // No need to zero out entropy_f_given_d because it is overwritten

        for (uint64_t i = 0; i < num_samples_f_given_d; i += PARALLEL_SIZE)
        {
            randomize_secrets(run_data1, cycles, generator);
            randomize_masks(run_data1, cycles, generator);
            run_circuit(run_data1, cycles);
            count_run(run_data1, value_count, cycles);
            // run_and_count_circuit(run_data1, value_count, cycles);
            // num_runs_intern += 1;
        }

        for (uint64_t run_pos = 0; run_pos < RUN_LENGTH; run_pos++)
        {
            entropy_f_given_d[run_pos] = lookup_f_given_d[value_count[run_pos]];
        }
    }

    // Sample things for avg over S=s of H(F|D=d,S=s)
    {
        std::fill_n(entropy_f_given_ds, RUN_LENGTH, (double)0);

        for (uint64_t secret_num = 0; secret_num < num_secrets; secret_num++)
        {
            // Sample things for H(F|D=d,S=s)
            fix_random_secrets(run_data1, cycles, generator);

            std::memset(value_count, 0, sizeof(uint64_t) * RUN_LENGTH);
            for (uint64_t i = 0; i < num_samples_f_given_ds; i += PARALLEL_SIZE)
            {
                randomize_masks(run_data1, cycles, generator);
                run_and_count_circuit(run_data1, value_count, cycles);
                // num_runs_intern += 1;
            }

            for (uint64_t run_pos = 0; run_pos < RUN_LENGTH; run_pos++)
            {
                entropy_f_given_ds[run_pos] += lookup_f_given_ds[value_count[run_pos]];
            }
        }

        // compute the average out of the sum of H(F|D=d,S=s)
        if (num_secrets != 1)
        {
            for (uint64_t run_pos = 0; run_pos < RUN_LENGTH; run_pos++)
                entropy_f_given_ds[run_pos] /= num_secrets;
        }

    }

    // assert(num_runs_intern * PARALLEL_SIZE == num_secrets * num_samples_f_given_ds + num_samples_f_given_d);

    for (uint64_t run_pos = 0; run_pos < RUN_LENGTH; run_pos++)
    {
        sumof_mi_f_s_given_d[run_pos] += entropy_f_given_d[run_pos] - entropy_f_given_ds[run_pos];
        // std::string info = DEBUG_INFO[run_pos];
        // if (info.find("unmasked") == std::string::npos) continue;
        // std::cout << info << " mi|d:" << sumof_mi_f_s_given_d[run_pos] << " / " << run_id + 1 << " f|d: " << entropy_f_given_d[run_pos] << " f|d,s: " << entropy_f_given_ds[run_pos] << std::endl;
    }

    run_id += 1;
}

void SamplerMI::run_many(uint32_t cycles, uint64_t num_runs)
{
    const uint64_t end_id = run_id + num_runs;
    while (not_stopped && run_id != end_id) run_once(cycles);
    std::unique_lock lock(SamplerMI::mutex);
    samplers_done += 1;
    lock.unlock();
    SamplerMI::cond_var.notify_one();
}

inline __attribute__((always_inline)) std::thread SamplerMI::run(uint32_t cycles, uint64_t num_runs)
{
    return std::thread(&SamplerMI::run_many, this, cycles, num_runs);
}

void write_save(uint64_t duration_ms, const OptionsMI* opts, const std::vector<SamplerMI>& samplers)
{
    if (opts->store_file == OptionsMI::DEFAULT_STORE_FILE) return;

    SaveDataMI data;
    data.cycles = opts->cycles;
    data.num_samples_f_given_d = opts->num_samples_f_given_d;
    data.num_samples_f_given_ds = opts->num_samples_f_given_ds;
    data.num_secrets = opts->num_secrets;
    data.num_runs = 0;
    for (uint32_t si = 0; si < samplers.size(); si++)
        data.num_runs += samplers[si].run_id;
    data.duration_ms = duration_ms;
    data.run_length = RUN_LENGTH;
    data.sumof_mi_f_s_given_d = (double*) aligned_alloc(sizeof(double), sizeof(double) * RUN_LENGTH);
    std::fill_n(data.sumof_mi_f_s_given_d, RUN_LENGTH, (double)0);
    for (const auto & sampler : samplers)
        for (uint64_t run_pos = 0; run_pos < RUN_LENGTH; run_pos++)
            data.sumof_mi_f_s_given_d[run_pos] += sampler.sumof_mi_f_s_given_d[run_pos];

    std::ofstream ofs(opts->store_file, std::ios_base::binary);
    ofs << data;
    ofs.close();
}

void show_info(double mi, uint64_t N, double down_sub, const char* name, FILE* fout)
{
    if (fout == stdout) fprintf(fout, "\033[1m\033[31m");
    fprintf(fout, "%19.16f (N=%ld) (DS=%19.16f)", mi, N, down_sub);
    if (fout == stdout) fprintf(fout, "\033[0m");
    fprintf(fout, " %s \n", name);
}

void compute_errors(uint64_t ND, uint64_t NS, uint64_t NF_D, uint64_t NF_DS, double delta,
                    double& err_log_up, double& err_log_down, double& err_sqrt)
{
    double sigma_2_t1 = 1.0 / (4 * ND);

    double log2_fd = std::log2(NF_D);
    double sigma_2_t2 = (log2_fd * log2_fd) / (ND * NF_D);

    double log2_fds = std::log2(NF_DS);
    double sigma_2_t3 = (log2_fds * log2_fds) / (ND * NS * NF_DS);

    double sigma_2_t4 = 1.0 / (4 * ND * NS);

    double sigma_2 = (sigma_2_t1 + sigma_2_t4) + (sigma_2_t2 + sigma_2_t3);

    err_sqrt = std::sqrt(2 * sigma_2 * (-std::log(delta)));

    err_log_up = std::log2(1 + 1.0 / NF_D);
    err_log_down = std::log2(1 + 1.0 / NF_DS);
}

void show_suspicious(const OptionsMI* opts, const std::vector<SamplerMI>& samplers, int32_t num_best, FILE* fout = stdout)
{
    if (num_best == 0) return;
    int64_t N = 0;
    for (const auto& s: samplers) N += s.run_id;

    double curr_log_up;
    double curr_log_down;
    double curr_sqrt;

    compute_errors(N, opts->num_secrets, opts->num_samples_f_given_d, opts->num_samples_f_given_ds, opts->delta,
                   curr_log_up, curr_log_down, curr_sqrt);

    double up_add = curr_log_up + curr_sqrt;
    double down_sub = curr_log_down + curr_sqrt;

    double final_log_up;
    double final_log_down;
    double final_sqrt;

    compute_errors(opts->num_data, opts->num_secrets, opts->num_samples_f_given_d, opts->num_samples_f_given_ds, opts->delta,
                   final_log_up, final_log_down, final_sqrt);

    assert(final_log_up = curr_log_up);
    assert(final_log_down = curr_log_down);

    double final_up_add = final_log_up + final_sqrt;
    double final_down_sub = final_log_down + final_sqrt;

    fprintf(fout, "N:        %ld\n", N);
    fprintf(fout, "log_up:   %19.16f\n", curr_log_up);
    fprintf(fout, "log_down: %19.16f\n", curr_log_down);
    fprintf(fout, "sqrt:     %19.16f\n", curr_sqrt);
    fprintf(fout, "up_add:   %19.16f\n", up_add);
    fprintf(fout, "down_sub: %19.16f\n", down_sub);
    fprintf(fout, "final sqrt:     %19.16f\n", final_sqrt);
    fprintf(fout, "final up_add:   %19.16f\n", final_up_add);
    fprintf(fout, "final down_sub: %19.16f\n", final_down_sub);

    double max_mi = -2;

    double* best_mi = nullptr;
    uint32_t* best_pos = nullptr;

    if (num_best > 0)
    {
        best_mi = (double*)aligned_alloc(sizeof(double), num_best * sizeof(double));
        best_pos = (uint32_t*)aligned_alloc(sizeof(uint32_t), num_best * sizeof(uint32_t));
        std::fill_n(best_mi, num_best, (double)0);
        std::memset(best_pos, 0, num_best * sizeof(uint32_t));
    }

    int32_t num_good = 0;
    for (uint32_t run_pos = 0; run_pos < RUN_LENGTH; run_pos++)
    {
        double mi = 0.0; int64_t N = 0;
        for (const auto& s: samplers)
        {
            mi += s.sumof_mi_f_s_given_d[run_pos];
            N += s.run_id;
        }
        mi = mi / N;
        std::string info = DEBUG_INFO[run_pos];
        if (info.find("unmasked") != std::string::npos &&
            info.find("secret") != std::string::npos) continue;

        if (mi > max_mi) max_mi = mi;

        const bool highlight = mi - down_sub > 0;
        if (!highlight) continue;

        if (num_best > 0)
        {
            if (best_mi[num_best - 1] > mi) continue;
            num_good += 1;
            uint32_t mi_pos = run_pos;
            for (uint32_t i = 0; i < num_best; i++)
            {
                if (mi > best_mi[i]) // do a swap
                {
                    uint32_t tmp_pos = best_pos[i];
                    double tmp_mi = best_mi[i];
                    best_mi[i] = mi;
                    best_pos[i] = mi_pos;
                    mi = tmp_mi;
                    mi_pos = tmp_pos;
                }
            }
        }
        else
        {
            show_info(mi, N, down_sub, info.c_str(), fout);
        }
    }

    if (num_best > 0 && num_good != 0)
    {
        fprintf(fout, "Best MI:\n");
        for (int i = 0; i < std::min(num_good, num_best); i++)
        {
            show_info(best_mi[i], N, down_sub, DEBUG_INFO[best_pos[i]], fout);
        }
    }
    else
    {
        fprintf(fout, "max_mi:   %19.16f\n", max_mi);
    }

    if (opts->early_stop && max_mi > 10 * down_sub)
    {
        if (fout == stdout) fprintf(fout, "\033[1m\033[31m");
        fprintf(fout, "Max MI substantially exceeds threshold, stopping ...");
        if (fout == stdout) fprintf(fout, "\033[0m");
        interupt_handler(SIGUSR1);
    }

    fprintf(fout, "\n"), fflush(fout);
}

void analyze(SaveDataMI* save_data, OptionsMI* opts)
{
    auto start_time = std::chrono::steady_clock::now();
    signal(SIGINT, interupt_handler);
    signal(SIGTERM, interupt_handler);
    signal(SIGHUP, interupt_handler);

    if (opts->timeout != 0)
    {
        signal(SIGALRM, interupt_handler);
        alarm(opts->timeout);
    }

    printf("DELTA: %19.16f\n", opts->delta);
    printf("SQRT(2log(1/DELTA)): %19.16f\n", std::sqrt(-2 * std::log(opts->delta)));
    printf("DEBUG_INFO: %p\n", DEBUG_INFO);

    const uint64_t samples_per_data = (opts->num_samples_f_given_d + opts->num_secrets * opts->num_samples_f_given_ds);
    const uint64_t samples_at_once = samples_per_data * opts->num_threads;
    uint64_t num_runs_per_thread = (opts->num_samples / samples_at_once) + (opts->num_samples % samples_at_once != 0);

    printf("num_samples: %ld\n", opts->num_samples);
    printf("samples_per_data: %ld\n", samples_per_data);
    printf("samples_at_once: %ld\n", samples_at_once);

    printf("runs_per_thread: %ld\n", num_runs_per_thread);

    SamplerMI::samplers_done = 0;
    std::vector<SamplerMI> samplers;
    samplers.reserve(opts->num_threads);
    for (uint32_t i = 0; i < opts->num_threads; i++)
        samplers.emplace_back(opts);

    // Extra special handling for save_data
    if (save_data != nullptr)
    {
        samplers[0].run_id = save_data->num_runs;
        memcpy(samplers[0].sumof_mi_f_s_given_d, save_data->sumof_mi_f_s_given_d, RUN_LENGTH * sizeof(double));
    }

    std::vector<std::thread> threads;
    threads.reserve(opts->num_threads);
    for (SamplerMI& s: samplers)
        threads.emplace_back(s.run(opts->cycles, num_runs_per_thread));

    auto is_done = [opts]{
        return SamplerMI::samplers_done == opts->num_threads;
    };

    while(true)
    {
        std::unique_lock lock(SamplerMI::mutex);
        if (opts->print_interval != 0)
            SamplerMI::cond_var.wait_for(lock, std::chrono::seconds(opts->print_interval), is_done);
        else
            SamplerMI::cond_var.wait(lock);

        if (is_done()) break;

        show_suspicious(opts, samplers, opts->print_best);
    }

    for (auto& t : threads) t.join();

    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    uint64_t duration_ms = elapsed_time.count();
    if (save_data != nullptr) duration_ms += save_data->duration_ms;

    printf("Finished analysis, writing results\n");
    write_save(duration_ms, opts, samplers);

    FILE* fout = stdout;
    if (opts->report_file != OptionsMI::DEFAULT_REPORT_FILE)
    {
        fout = fopen(opts->report_file.c_str(), "w+");
        if (fout == nullptr) fout = stdout;
    }
    show_suspicious(opts, samplers, -1, fout);
    if (fout != stdout)
        { fclose(fout); }

    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
}

int main(int argc, const char* argv[])
{
    int error_code = 0;
    OptionsMI* opts = nullptr;
    SaveDataMI* save_data = nullptr;

    try { opts = new OptionsMI(argc, argv); }
    catch (const OptionsException& e)
    {
        if (std::string("") != e.what())
        {
            std::cout << "Error while parsing OptionsMI: " << e.what() << std::endl;
            error_code = 1; goto cleanup;
        }
        return 0;
    }

    if (opts->load_file != OptionsMI::DEFAULT_LOAD_FILE) try {
        std::ifstream ifs(opts->load_file, std::ios_base::binary);
        save_data = new SaveDataMI();
        ifs >> *save_data;
        ifs.close();
        save_data->assert_integrity(opts, RUN_LENGTH);
    } catch (const SaveDataException& e)
    {
        std::cout << "Error while reading SaveDataMI: " << e.what() << std::endl;
        error_code = 2; goto cleanup;
    }

    std::cout << "Running verification with OptionsMI:\n" << *opts << std::endl;
    std::cout << "RUN_LENGTH:    " << RUN_LENGTH << std::endl;
    std::cout << "PARALLEL_SIZE: " << PARALLEL_SIZE << std::endl;

    analyze(save_data, opts);

    return 0;

    cleanup:
    delete opts;
    delete save_data;
    return error_code;
}