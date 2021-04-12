//
// Created by admin on 2020/1/8.
//

#include "rpq_generator.h"

#define PA (pattern.PR_ADD)
#define PI (pattern.PR_ADD + pattern.PR_INC)
#define PAA (pattern.PR_ADD_CA)
#define PAR (pattern.PR_ADD_CA + pattern.PR_ADD_CR)
#define PRA (pattern.PR_REM_CA)
#define PRR (pattern.PR_REM_CA + pattern.PR_REM_CR)

rpq_generator::rpq_op_gen_pattern& rpq_generator::get_pattern(const string& name)
{
    static map<string, rpq_op_gen_pattern> patterns{{"default",
                                                    //  {.PR_ADD = 0.41,
                                                    //   .PR_INC = 0.2,
                                                    //   .PR_REM = 0.39,
                                                    {.PR_ADD = 0.11,
                                                      .PR_INC = 0.8,
                                                      .PR_REM = 0.09,
                                                      .PR_ADD_CA = 0.15,
                                                      .PR_ADD_CR = 0.05,
                                                      .PR_REM_CA = 0.1,
                                                      .PR_REM_CR = 0.1}},

                                                    {"ardominant",
                                                     {.PR_ADD = 0.11,
                                                      .PR_INC = 0.8,
                                                      .PR_REM = 0.09,

                                                      .PR_ADD_CA = 0.15,
                                                      .PR_ADD_CR = 0.05,
                                                      .PR_REM_CA = 0.1,
                                                      .PR_REM_CR = 0.1}}};
    if (patterns.find(name) == patterns.end()) return patterns["default"];
    return patterns[name];
}

struct invocation* rpq_generator::normal_exec_add(redis_client& c)
{
    ele.write_op_executed++;
    rpq_add_cmd* op_add = gen_add();
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    redisReply_ptr reply = c.exec(op_add); 
    auto end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    invocation* inv = new invocation;
    inv->start_time = start;
    inv->end_time = end;
    string operation = "add," + to_string(op_add->element) + "," + to_string(op_add->value) + ",null";
    inv->operation = operation;
    return inv;
}

struct invocation* rpq_generator::exec_incrby(redis_client& c, int element, double value)
{
    cout<<"incrby\n";
    ele.write_op_executed++;
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    redisReply_ptr reply = c.exec(new rpq_incrby_cmd(zt, ele, element, value));
    auto end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    invocation* inv = new invocation;
    inv->start_time = start;
    inv->end_time = end;
    string operation = "incrby," + to_string(element) + "," + to_string(value) + ",null";
    inv->operation = operation;
    return inv;
}

struct invocation* rpq_generator::exec_rem(redis_client& c, int element) 
{
    ele.write_op_executed++;
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    redisReply_ptr reply = c.exec(new rpq_remove_cmd(zt, ele, element));
    auto end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    invocation* inv = new invocation;
    inv->start_time = start;
    inv->end_time = end;
    string operation = "rem," + to_string(element) + ",null";
    inv->operation = operation;
    return inv;
}

struct invocation* rpq_generator::gen_and_exec(redis_client& c)
{
    int e;
    double rand = decide();
    if (rand <= PA) { 
        return normal_exec_add(c);
    }
    else if (rand <= PI)
    {
        e = ele.random_get();
        if (e == -1) 
        {
            return normal_exec_add(c);
        }
        double d = doubleRand(-MAX_INCR, MAX_INCR);
        return exec_incrby(c, e, d);
        //redisReply_ptr reply = c.exec(new rpq_incrby_cmd(zt, ele, e, d));
    }
    else
    {
        double conf = decide();
        if (conf < PRA)
        {
            e = add.get(-1);
            if (e == -1)
            {
                e = ele.random_get();
                if (e == -1) 
                {
                    return normal_exec_add(c);
                }
            }
            rem.add(e);
        }
        else if (conf < PRR)
        {
            e = rem.get(-1);
            if (e == -1)
            {
                e = ele.random_get();
                if (e == -1) {
                    return normal_exec_add(c);
                }
                rem.add(e);
            }
        }
        else
        {
            e = ele.random_get();
            if (e == -1) {
                return normal_exec_add(c);
            }
            rem.add(e);
        }
        return exec_rem(c, e);
        //redisReply_ptr reply = c.exec(new rpq_remove_cmd(zt, ele, e));
    }
    return NULL;
}
rpq_add_cmd* rpq_generator::gen_add()
{
    int e;
    double d = doubleRand(0, MAX_INIT);
    double conf = decide();
    if (conf < PAA)
    {
        e = add.get(-1);
        if (e == -1)
        {
            e = intRand(MAX_ELE);
            add.add(e);
        }
    }
    else if (conf < PAR)
    {
        e = rem.get(-1);
        if (e == -1) e = intRand(MAX_ELE);
        add.add(e);
    }
    else
    {
        e = intRand(MAX_ELE);
        add.add(e);
    }
    return new rpq_add_cmd(zt, ele, e, d);
}
