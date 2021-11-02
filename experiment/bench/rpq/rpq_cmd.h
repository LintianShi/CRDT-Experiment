//
// Created by admin on 2020/1/8.
//

#ifndef BENCH_RPQ_CMD_H
#define BENCH_RPQ_CMD_H

#include "../util.h"

class rpq_cmd : public cmd
{
protected:
    rpq_cmd(const string &type, const char *op)
    {
        stream << type << "z" << op << " " << type << "rpq";
        op_name = op;
    }

    void handle_redis_return(const redisReply_ptr &r) override { ; }
};

class rpq_add_cmd : public rpq_cmd
{
public:
    int element;
    int value;
    rpq_add_cmd(const string &type, int element, int value)
        : rpq_cmd(type, "add"), element(element), value(value)
    {
        add_args(element, value);
    }
};

class rpq_incrby_cmd : public rpq_cmd
{
public:
    int element;
    int value;

    rpq_incrby_cmd(const string &type, int element, int value)
        : rpq_cmd(type, "incrby"), element(element), value(value)
    {
        add_args(element, value);
    }
};

class rpq_rem_cmd : public rpq_cmd
{
public:
    int element;

    rpq_rem_cmd(const string &type, int element)
        : rpq_cmd(type, "rem"), element(element)
    {
        add_args(element);
    }
};

class rpq_max_cmd : public rpq_cmd
{
public:
    rpq_max_cmd(const string &type) : rpq_cmd(type,"max") {}
};

class rpq_score_cmd : public rpq_cmd
{    
public:
    int element;
    rpq_score_cmd(const string &type, int element)
         : rpq_cmd(type, "score"), element(element) 
         {
            add_args(element);
         }
};

#endif  // BENCH_RPQ_CMD_H
