#include "list_exp.h"

#include "../exp_runner.h"
#include "list_generator.h"

exp_setting::default_setting list_exp::list_setting{
    .name = "List",
    .total_sec = 20,
    .delay = 50,
    .delay_low = 10,
    .total_servers = 3,
    .op_per_sec = 200,
    .speed_e = {.start = 50, .end = 1000, .step = 50},
    .replica_e = {.start = 1, .end = 5, .step = 1},
    .delay_e = {.start = 20, .end = 380, .step = 40}};

void list_exp::exp_impl(const string& type, const string& pattern)
{
    list_log list(type);
    list_generator gen(type, list, pattern);
    list_ovhd_cmd ovhd(type, list);
    list_read_cmd read(type, list);

    exp_runner runner(list, gen);
    runner.run();
}