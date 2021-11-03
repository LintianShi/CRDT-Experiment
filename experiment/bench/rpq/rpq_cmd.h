//
// Created by admin on 2020/1/8.
//

#ifndef BENCH_RPQ_CMD_H
#define BENCH_RPQ_CMD_H

#include "../util.h"

class rpq_cmd : public cmd
{
public:
    rpq_cmd(const string &type, const char *op, int round)
    {
        stream << type << "z" << op << " " << type << "rpq" << round;
        op_name = op;
    }

    void handle_redis_return(const redisReply_ptr &r) override { ; }
};

class rpq_add_cmd : public rpq_cmd
{
public:
    int element;
    int value;
    rpq_add_cmd(const string &type, int element, int value, int round)
        : rpq_cmd(type, "add", round), element(element), value(value)
    {
        add_args(element, value);
    }
};

class rpq_incrby_cmd : public rpq_cmd
{
public:
    int element;
    int value;

    rpq_incrby_cmd(const string &type, int element, int value, int round)
        : rpq_cmd(type, "incrby", round), element(element), value(value)
    {
        add_args(element, value);
    }
};

class rpq_rem_cmd : public rpq_cmd
{
public:
    int element;

    rpq_rem_cmd(const string &type, int element, int round)
        : rpq_cmd(type, "rem", round), element(element)
    {
        add_args(element);
    }
};

class rpq_max_cmd : public rpq_cmd
{
public:
    rpq_max_cmd(const string &type, int round) : rpq_cmd(type,"max", round) {}
};

class rpq_score_cmd : public rpq_cmd
{    
public:
    int element;
    rpq_score_cmd(const string &type, int element, int round)
         : rpq_cmd(type, "score", round), element(element) 
         {
            add_args(element);
         }
};

#endif  // BENCH_RPQ_CMD_H
