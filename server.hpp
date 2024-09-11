#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <string>
#include <vector>
#include <future>
#include <unordered_set>
#include <unordered_map>

#include <unistd.h> //用于unix/linux系统，定义了大量系统调用函数

#include <mysql/mysql.h>
#include <event2/event.h> //libevent

#include "ThreadPool.hpp"

// 暂定消息格式为:"user_id user_name time_stamp message"

enum STATUS
{
    ERROR = -1,
    INVALID = 0,
    SUCCESS = 1,
    INVALID_USER_NAME,
    INVALID_PASSWORD
};
// 收到消息时查看是否有id，没有的话发送的就是id，否则是消息

class Server
{
private:
    int server_socket;
    int ConnectionInit();
    // int DatabaseInit();
    // int GetMaxUserID();
    ThreadPool thread_pool;

    static std::unordered_map<int, std::vector<event_base *>> bases;
    static std::unordered_map<int, std::vector<event *>> events;
    static std::unordered_set<int> client_sockets;
    static std::mutex mtx;
    static std::vector<std::string> records;

    // static MYSQL *database;
    // static std::vector<int> clock; // 向量时钟
    std::string server_ip;
    int socket_port;
    int msg_len = 300; /// 消息长度

public:
    std::promise<int> test;
    Server() {}
    Server(const std::string &_ip, const int &_port) : server_ip{_ip}, socket_port{_port}
    {
        ConnectionInit();
        // DatabaseInit();
        // max_user_id = GetMaxUserID();
        thread_pool.start();
        thread_pool.setMode(PoolMode::MODE_CACHED);
    }
    std::string GetServerTime();
    int Send(const std::string &message, int client_socket);
    void BoardCast(const std::string &message);
    int Receive(std::string &message, int client_socket);
    int ConnectTo(const std::string &client_ip, const int &client_port);
    void Connect();
    void Print();
    int ClearSocket(int client_socket);
    bool CheckSocket(int client_socket);

    ~Server()
    {
        close(server_socket);
        // mysql_close(database);

        for (auto s : client_sockets)
        {
            if (s != -1)
            {
                close(s);
                for (auto b : bases.at(s))
                    event_base_free(b); // 销毁event_base
            }
        }
    }
    friend void do_accept(evutil_socket_t fd, short event_type, void *arg);
    friend void do_read(evutil_socket_t fd, short event_type, void *arg);
    friend void do_send(evutil_socket_t fd, short event_type, void *arg);
};

struct SendArgs
{
    const std::string &message;
    int client_socket;
    Server *server;
    SendArgs(const std::string &msg, int sock, Server *s) : message{msg}, client_socket{sock}, server{s} {}
};

#endif