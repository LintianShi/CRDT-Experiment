//
// Created by admin on 2020/6/30.
//

#include "rpq_exp.h"

#include "../exp_runner.h"
#include "rpq_generator.h"

exp_setting::default_setting rpq_exp::rpq_setting{
    .total_sec = 150,
    .delay = 50,
    .delay_low = 10,
    .total_servers = 3,
    .op_per_sec = 300,
    .speed_e = {.start = 500, .end = 10000, .step = 100},
    .replica_e = {.start = 1, .end = 5, .step = 1},
    .delay_e = {.start = 20, .end = 380, .step = 40}};

void rpq_exp::exp_impl(const string& type, const string& pattern)
{
    rpq_log qlog(type);
    rpq_generator gen(type, qlog, pattern);
    exp_runner runner(qlog, gen);
    runner.run();
}
