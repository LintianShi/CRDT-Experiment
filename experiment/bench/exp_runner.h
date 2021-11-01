//
// Created by admin on 2020/1/7.
//

#ifndef BENCH_EXP_RUNNER_H
#define BENCH_EXP_RUNNER_H

#include <future>
#include <thread>

#include "exp_env.h"
#include "exp_setting.h"
#include "util.h"

#if defined(__linux__)

#include <hiredis/hiredis.h>

#elif defined(_WIN32)

#include <direct.h>

#include "../../redis-6.0.5/deps/hiredis/hiredis.h"

#endif

constexpr int THREAD_PER_SERVER = 1;
#define RUN_CONDITION log.write_op_executed < exp_setting::total_ops

// time in seconds
#define INTERVAL_TIME ((double)TOTAL_SERVERS * THREAD_PER_SERVER / exp_setting::op_per_sec)
constexpr int TIME_OVERHEAD = 1;
constexpr int TIME_READ = 1;


using namespace std;
// extern const char *ips[];

class exp_runner
{
private:
    rdt_log &log;
    generator &gen;

    vector<thread> thds;
    vector<exec_trace*> traces;

    void conn_one_server_timed(const char *ip, int port, std::chrono::system_clock::time_point &thd_start_time)
    {
        for (int i = 0; i < THREAD_PER_SERVER; ++i)
        {
            thds.emplace_back([this, ip, port, thd_start_time] {
                redis_client c(ip, port);
                exec_trace* trace = new exec_trace;
                this_thread::sleep_until(thd_start_time);
                for (int t = 1; RUN_CONDITION; ++t)
                {
                    trace->insert(gen.gen_and_exec(c));
                }
                traces.push_back(trace);
                // trace->write_logfile(exp_setting::pattern_name, TOTAL_SERVERS, THREAD_PER_SERVER, exp_setting::op_per_sec);
            });
        }
    }

public:
    exp_runner(rdt_log &log, generator &gen) : gen(gen), log(log) {}

    void run()
    {
        exp_env e(3, 1, 1, 1);
        // start servers
        // construct replicas
        // ...
        // shutdown servers
        // clean log & rdb
        string ips[3] = {"172.24.81.133", "172.24.81.131", "172.24.81.135"};

        auto start = chrono::steady_clock::now();
        auto current_time = chrono::system_clock::now() + chrono::duration<int, std::milli>(2000);

        for (int i = 0; i < e.get_cluster_num(); i++) {
            for (int j = 0; j < e.get_replica_nums()[i]; j++) {
                cout<<ips[i]<<endl;
                conn_one_server_timed(ips[i].c_str(), BASE_PORT + j, current_time);
            }
        }
        
        thread progress_thread([this] {
            constexpr int barWidth = 50;
            double progress;
            while (RUN_CONDITION)
            {
                progress = log.write_op_executed / ((double)exp_setting::total_ops);
                cout << "\r[";
                int pos = barWidth * progress;
                for (int i = 0; i < barWidth; ++i)
                {
                    if (i < pos)
                        cout << "=";
                    else if (i == pos)
                        cout << ">";
                    else
                        cout << " ";
                }
                cout << "] " << (int)(progress * 100) << "%" << flush;
                this_thread::sleep_for(chrono::seconds(1));
            }
            cout << "\r[";
            for (int i = 0; i < barWidth; ++i)
                cout << "=";
            cout << "] 100%" << endl;
        });

        for (auto &t : thds)
            if (t.joinable()) t.join();
        if (progress_thread.joinable()) progress_thread.join();

        auto end = chrono::steady_clock::now();
        auto time = chrono::duration_cast<chrono::duration<double>>(end - start).count();
        cout << time << " seconds, " << log.write_op_generated / time << " op/s\n";
        cout << log.write_op_executed << " operations actually executed on redis." << endl;
        for (auto &t : traces) {
            printf("%d\n", traces.size());
            t->write_logfile(exp_setting::pattern_name, TOTAL_SERVERS, THREAD_PER_SERVER, exp_setting::op_per_sec);
        }
    }
};

#endif  // BENCH_EXP_RUNNER_H
