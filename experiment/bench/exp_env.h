//
// Created by admin on 2020/4/16.
//
#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#ifndef BENCH_EXP_ENV_H
#define BENCH_EXP_ENV_H
#include <string>
#include <vector>
#include <chrono>
#include <sstream>
#include <thread>
using namespace std;
#include "exp_setting.h"

#define REDIS_SERVER "../../redis-6.0.5/src/redis-server"
#define REDIS_CONF "../../redis-6.0.5/redis.conf"
#define REDIS_CLIENT "../../redis-6.0.5/src/redis-cli"

#define IP_BETWEEN_CLUSTER "127.0.0.3"
#define IP_WITHIN_CLUSTER "127.0.0.2"
#define IP_SERVER "127.0.0.1"
constexpr int BASE_PORT = 6379;

class exp_env
{
private:
    int cluster_num;
    int replica_num;
    vector<ssh_session> sessions;
    string available_hosts[7] = {"n0.disalg.cn", "n1.disalg.cn", "n2.disalg.cn", "n3.disalg.cn", "n4.disalg.cn", "n5.disalg.cn", "n6.disalg.cn"};

    static void shell_exec(const char* cmd, bool sudo)
    {
//#define PRINT_CMD
#ifdef PRINT_CMD
        if (sudo)
            cout << "\nsudo " << cmd << endl;
        else
            cout << "\n" << cmd << endl;
#endif
        ostringstream cmd_stream;
        if (sudo) cmd_stream << "echo " << sudo_pwd << " | sudo -S ";
        cmd_stream << cmd << " 1>/dev/null";  // or " 1>/dev/null 2>&1"
        system(cmd_stream.str().c_str());
    }

    static inline void shell_exec(const string& cmd, bool sudo) { shell_exec(cmd.c_str(), sudo); }

    int connect_all() {
        for (int i = 0; i < cluster_num; i++) {
            if (connect_one_server(available_hosts[i].c_str()) != 0) {
                return -1;
            }
            
        }
        return 0;
    }

    int connect_one_server(const char* host) {
        ssh_session my_ssh_session = ssh_new();
        if (my_ssh_session == NULL)
            return -1;

        int port = 22;
        ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, host);
        ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "root");
        ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, &port);

        int rc;
        rc = ssh_connect(my_ssh_session);
        if (rc != SSH_OK)
        {
            fprintf(stderr, "Error connecting to localhost: %s\n",
            ssh_get_error(my_ssh_session));
            ssh_free(my_ssh_session);
            return rc;
        }

        rc = ssh_userauth_password(my_ssh_session, NULL, "disalg.root!");
        if (rc != SSH_AUTH_SUCCESS)
        {
            fprintf(stderr, "Error authenticating with password: %s\n", ssh_get_error(my_ssh_session));
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            return rc;
        }
        sessions.push_back(my_ssh_session);
        return 0;
    }

    int ssh_exec(char* cmd) {
        return 0;
    }

    static void start_servers()
    {
        ssh_exec("./server.sh")
    }

    static void construct_repl()
    {
        for (int i = 0; i < exp_setting::total_servers; ++i)
        {
            ostringstream repl_stream;    
            for (int k = 0; k < i; ++k) {
                repl_stream << " " << IP_SERVER << " " << BASE_PORT + k;
            } 

            ostringstream cmd_stream;
            cmd_stream << REDIS_CLIENT << " -h 127.0.0.1 -p " << BASE_PORT + i
                         << " REPLICATE " << TOTAL_SERVERS << " " << i << " exp_local"
                         << repl_stream.str();
            shell_exec(cmd_stream.str(), false);
        }
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }

    static void shutdown_servers()
    {
        for (int port = BASE_PORT; port < BASE_PORT + TOTAL_SERVERS; ++port)
        {
            ostringstream stream;
            stream << REDIS_CLIENT << " -h 127.0.0.1 -p " << port << " SHUTDOWN NOSAVE";
            shell_exec(stream.str(), false);
        }
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }

    static void clean() { shell_exec("rm -rf *.rdb *.log", false); }

public:
    static string sudo_pwd;

    exp_env(int cluster, int replica)
    {
        cluster_num = cluster;
        replica_num = replica;
        if (connect_all() != 0) {
            exit(-1);
        }
        exp_setting::print_settings();
        start_servers();
        cout << "server started, " << flush;
        construct_repl();
        cout << "replication constructed, " << flush;
    }

    ~exp_env()
    {
        shutdown_servers();
        cout << "server shutdown, " << flush;
        clean();
        cout << "cleaned\n" << endl;
        delete []sessions;
    }
};

#endif  // BENCH_EXP_ENV_H
