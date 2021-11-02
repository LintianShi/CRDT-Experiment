#include "list_exp.h"

#include "../exp_runner.h"
#include "list_generator.h"

exp_setting::default_setting list_exp::list_setting{
    .name = "List",
    .total_sec = 3,
    .total_servers = 3,
    .op_per_sec = 1000,
    };

void list_exp::exp_impl(const string& type, const string& pattern)
{
    list_log list(type);
    list_generator gen(type, list, pattern);
    list_ovhd_cmd ovhd(type, list);
    list_read_cmd read(type, list);
    exp_env env(3, 1, 1, 1);
    exp_runner runner(gen, env);
    runner.run();
}