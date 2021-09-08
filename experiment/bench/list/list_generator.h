#ifndef BENCH_LIST_GENERATOR_H
#define BENCH_LIST_GENERATOR_H

#include "../util.h"
#include "list_cmd.h"
#include "list_log.h"

class list_generator : public generator
{
private:
    // TODO conflicts?
    struct list_op_gen_pattern
    {
        double PR_ADD;
        double PR_UPD;
        double PR_REM;
    };

    class id_gen
    {
    private:
        int count = 0;

        static int index(thread::id tid);

    public:
        id_gen() { 
            ;
        }

        string get()
        {
            int temp = index(this_thread::get_id()); 
            ostringstream stream;
            stream << temp << ":" << count;
            count++;
            return stream.str();
        }
    } new_id;

    list_op_gen_pattern &pattern;
    list_log &list;
    const string &type;
    // TODO record_for_collision<string> add;

    static list_op_gen_pattern &get_pattern(const string &name);

    list_insert_cmd *gen_insert();
    struct invocation* exec_insert(redis_client& c);
    struct invocation* exec_get(redis_client& c, string& prev);
    struct invocation* exec_remove(redis_client& c, string &id);

public:
    list_generator(const string &type, list_log &list, const string &p)
        : type(type), list(list), pattern(get_pattern(p))
    {
        // TODO add_record(add); start_maintaining_records();
    }

    struct invocation* gen_and_exec(redis_client &c) override;
};

#endif  // BENCH_LIST_GENERATOR_H