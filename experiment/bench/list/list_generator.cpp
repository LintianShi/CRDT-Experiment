#include "list_generator.h"

constexpr auto MAX_FONT_SIZE = 100;
constexpr auto TOTAL_FONT_TYPE = 10;
constexpr auto MAX_COLOR = 1u << 25u;

#define PA (pattern.PR_ADD)
#define PU (pattern.PR_ADD + pattern.PR_UPD)

list_generator::list_op_gen_pattern &list_generator::get_pattern(const string &name)
{
    static map<string, list_op_gen_pattern> patterns{
        {"default", {.PR_ADD = 0.60, .PR_UPD = 0.20, .PR_REM = 0.20}},
        {"upddominant", {.PR_ADD = 0.60, .PR_UPD = 0.20, .PR_REM = 0.20}}};
    if (patterns.find(name) == patterns.end()) return patterns["default"];
    return patterns[name];
}

int list_generator::id_gen::index(thread::id tid)
{
    static int next_index = 0;
    static mutex my_mutex;
    static map<thread::id, int> ids;
    lock_guard<mutex> lock(my_mutex);
    if (ids.find(tid) == ids.end()) ids[tid] = next_index++;
    return ids[tid];
}

struct invocation* list_generator::exec_insert(redis_client& c) {
    //list.write_op_executed++;
    list_insert_cmd* op_insert = gen_insert();
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    redisReply_ptr reply = c.exec(op_insert);
    auto end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    if (reply == NULL || reply->type == 6) {
        return NULL;
    }
    list.insert(op_insert->prev, op_insert->id, op_insert->content, op_insert->font, op_insert->size, op_insert->color, op_insert->bold, op_insert->italic, op_insert->underline);
    invocation* inv = new invocation;
    inv->start_time = start;
    inv->end_time = end;
    inv->operation = "insert," + op_insert->prev + "," + op_insert->id + "," + op_insert->content + ",null";
    return inv;
}

struct invocation* list_generator::exec_remove(redis_client& c, string &id) {
    //list.write_op_executed++;
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    redisReply_ptr reply = c.exec(new list_remove_cmd(type, list, id));
    auto end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    
    if (reply == NULL || reply->type == 6) {
        return NULL;
    }
    list.remove(id);
    invocation* inv = new invocation;
    inv->start_time = start;
    inv->end_time = end;
    inv->operation = "remove," + id + ",null";
    return inv;
}

struct invocation* list_generator::exec_getNext(redis_client& c, string& prev) {
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    redisReply_ptr reply = c.exec(new list_read_cmd(type, list));
    auto end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (reply == NULL || reply->type == 6) {
        return NULL;
    }

    int r_len = reply->elements;
    string name = "null";
    if (r_len != 0) {
        for (int i = 0; i < r_len; i++) {
            string temp_name = reply->element[i]->element[0]->str;
            if (temp_name.compare(prev) == 0 && i < r_len - 1) {
                name = reply->element[i+1]->element[0]->str;
                name += ":";
                name += + reply->element[i+1]->element[1]->str;
                break;
            } else if (temp_name.compare(prev) == 0 && i == r_len - 1) {
                name = "end";
            }
        }
        
    }

    if (name.compare("null") == 0) {
        return NULL;
    }
    invocation* inv = new invocation;
    inv->start_time = start;
    inv->end_time = end;
    inv->operation = "get," + prev + "," + name;
    return inv;
}

struct invocation* list_generator::exec_getIndex(redis_client& c, int index) {
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    redisReply_ptr reply = c.exec(new list_read_cmd(type, list));
    auto end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (reply == NULL || reply->type == 6) {
        return NULL;
    }

    int r_len = reply->elements;
    string name = "null";
    if (index < r_len) {
            name = reply->element[index]->element[0]->str;
            name += ":";
            name += + reply->element[index]->element[1]->str;
    }

    if (name.compare("null") == 0) {
        return NULL;
    }
    invocation* inv = new invocation;
    inv->start_time = start;
    inv->end_time = end;
    inv->operation = "get," + to_string(index) + "," + name;
    return inv;
}

struct invocation* list_generator::gen_and_exec(redis_client &c)
{
    int rand = intRand(0, 100);
    // TODO conflicts?
    if (rand <= 50) { 
        return exec_insert(c); 
    }
    else if (rand <= 75) {
        // string prev = list.random_get();
        // if (prev.empty()) return exec_insert(c);
        // return exec_getNext(c, prev);
        int index = list.random_get_index();
        if (index == -1) return exec_insert(c);
        return exec_getIndex(c, index);
    }
    else
    {
        string id = list.random_get();
        if (id.empty()) return exec_insert(c);
        return exec_remove(c, id);
    }
}

list_insert_cmd *list_generator::gen_insert()
{
    string prev = list.random_get(), id = new_id.get(), content = strRand();
    int font = intRand(TOTAL_FONT_TYPE), size = intRand(MAX_FONT_SIZE), color = intRand(MAX_COLOR);
    bool bold = boolRand(), italic = boolRand(), underline = boolRand();
    if (prev.empty()) prev = "null";
    return new list_insert_cmd(type, list, prev, id, content, font, size, color, bold, italic,
                               underline);
}