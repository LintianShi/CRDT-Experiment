//
// Created by admin on 2020/1/8.
//

#include "rpq_generator.h"
#include <math.h>

#define PA (pattern.PR_ADD)
#define PI (pattern.PR_ADD + pattern.PR_INC)
#define PM (pattern.PR_ADD + pattern.PR_INC + pattern.PR_MAX)
#define PS (pattern.PR_ADD + pattern.PR_INC + pattern.PR_MAX + pattern.PR_SCORE)
#define PAA (pattern.PR_ADD_CA)
#define PAR (pattern.PR_ADD_CA + pattern.PR_ADD_CR)
#define PRA (pattern.PR_REM_CA)
#define PRR (pattern.PR_REM_CA + pattern.PR_REM_CR)

rpq_generator::rpq_op_gen_pattern& rpq_generator::get_pattern(const string& name)
{
    static map<string, rpq_op_gen_pattern> patterns{{"default",
                                                     {.PR_ADD = 0.31,
                                                      .PR_INC = 0.25,
                                                      .PR_MAX = 0.1,
                                                      .PR_SCORE = 0.1,
                                                      .PR_REM = 0.24,
                                                      
                                                      .PR_ADD_CA = 0.15,
                                                      .PR_ADD_CR = 0.05,
                                                      .PR_REM_CA = 0.1,
                                                      .PR_REM_CR = 0.1}},

                                                    {"ardominant",
                                                     {.PR_ADD = 0.11,
                                                      .PR_INC = 0.6,
                                                      .PR_MAX = 0.1,
                                                      .PR_SCORE = 0.1,
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

struct invocation* rpq_generator::exec_incrby(redis_client& c, int element, int value)
{
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

struct invocation* rpq_generator::exec_max(redis_client& c) 
{
    //cout<<"max\n";
    ele.write_op_executed++;
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    redisReply_ptr reply = c.exec(new rpq_max_cmd(zt, ele));
    auto end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    invocation* inv = new invocation;
    inv->start_time = start;
    inv->end_time = end;
    if (reply->elements == 2) {
        string ret1 = reply->element[0]->str;
        string ret2 = reply->element[1]->str;
        string operation = "max," + ret1 + " " + ret2;
        inv->operation = operation;
    } else {
        string ret = "null";
        string operation = "max," + ret;
        inv->operation = operation;
    }
    return inv;
    return inv;
}

struct invocation* rpq_generator::exec_score(redis_client& c, int element) 
{
    //cout<<"score\n";
    ele.write_op_executed++;
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    redisReply_ptr reply = c.exec(new rpq_score_cmd(zt, ele, element));
    auto end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    invocation* inv = new invocation;
    inv->start_time = start;
    inv->end_time = end;

    if (reply->type == 1)
    {
        string operation = "score," + to_string(element) + "," + reply->str;
        inv->operation = operation;
    }
    else 
    {
        string operation = "score," + to_string(element) + "," + "null";
        inv->operation = operation;
    }

    
    return inv;
}

struct invocation* rpq_generator::gen_and_exec(redis_client& c)
{
    int e;
    double rand = decide();
    if (ele.write_op_executed < MAX_ELE / 2) {
        return normal_exec_add(c);
    }

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
        int d = intRand(-MAX_INCR, MAX_INCR);
        return exec_incrby(c, e, d);
        //redisReply_ptr reply = c.exec(new rpq_incrby_cmd(zt, ele, e, d));
    }
    else if (rand <= PM)
    {
        //cout<<"m\n";
        return exec_max(c);
    } else if (rand <= PS)
    {
        //cout<<"s\n";
        e = ele.random_get();
        if (e == -1) 
            return normal_exec_add(c);
        return exec_score(c, e);
    }
    else
    {
        // double conf = decide();
        // if (conf < PRA)
        // {
        //     e = add.get(-1);
        //     if (e == -1)
        //     {
        //         e = ele.random_get();
        //         if (e == -1) 
        //         {
        //             return normal_exec_add(c);
        //         }
        //     }
        //     rem.add(e);
        // }
        // else if (conf < PRR)
        // {
        //     e = rem.get(-1);
        //     if (e == -1)
        //     {
        //         e = ele.random_get();
        //         if (e == -1) {
        //             return normal_exec_add(c);
        //         }
        //         rem.add(e);
        //     }
        // }
        // else
        // {
            e = ele.random_get();
            if (e == -1) {
                return normal_exec_add(c);
            }
            rem.add(e);
        // }
        return exec_rem(c, e);
        //redisReply_ptr reply = c.exec(new rpq_remove_cmd(zt, ele, e));
    }
    return NULL;
}
rpq_add_cmd* rpq_generator::gen_add()
{
    int e;
    int d = intRand(0, MAX_INIT);
    //double conf = decide();
    // if (conf < PAA)
    // {
    //     e = add.get(-1);
    //     if (e == -1)
    //     {
    //         e = intRand(MAX_ELE);
    //         add.add(e);
    //     }
    // }
    // else if (conf < PAR)
    // {
    //     e = rem.get(-1);
    //     if (e == -1) e = intRand(MAX_ELE);
    //     add.add(e);
    // }
    // else
    // {
    e = intRand(MAX_ELE);
    //     add.add(e);
    // }
    return new rpq_add_cmd(zt, ele, e, d);
}
