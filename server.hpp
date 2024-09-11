#ifndef SERVER_HPP_
#define SERVER_HPP_

#include <string>
#include <vector>
#include <future>
#include <unordered_set>
#include <unordered_map>

#include <unistd.h> //用于unix/linux系统，定义了大量系统调用函数

#include <mysql/mysql.h>       //-lmysqlclient
#include <event2/event.h>      //-levent
#include <boost/uuid/uuid.hpp> //-lboost_system -lboost_uuid
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "ThreadPool.hpp"

enum STATUS
{
    ERROR = -1,
    INVALID = 0,
    SUCCESS = 1,
    INVALID_USER_NAME,
    INVALID_PASSWORD
};
enum MUTEX
{
    FOR_RW = 0,
    FOR_CLOCK = 1,
    FOR_DB = 2
};

struct Message;
struct SendArgs;

class Server
{
private:
    // 局部变量
    int server_socket;
    std::string server_ip;
    int socket_port;
    boost::uuids::uuid peer_id;
    int msg_len = 300;       /// 消息长度
    int max_peer_num = 1024; // 最多接受1024个节点

    ThreadPool thread_pool;

    static std::unordered_map<int, std::vector<event_base *>> bases;
    static std::unordered_map<int, std::vector<event *>> events;
    static std::unordered_set<int> client_sockets;
    static std::vector<std::mutex> mtx;
    static std::vector<Message> records;

    static MYSQL *database;
    // static std::vector<int> clock; // 向量时钟
    static int clock;

    int ConnectionInit();
    std::string GetServerTime();
    boost::uuids::uuid GetPeerID();
    std::string Serialize(const Message &msg);
    Message Deserialize(const std::string &message);
    // int DatabaseInit();

protected:
    int ClearSocket(int client_socket);
    bool CheckSocket(int client_socket);
    int Receive(std::string &message, int client_socket);
    int Send(const Message &message, int client_socket);

public:
    std::promise<int> test;
    Server(const std::string &_ip, const int &_port);

    void BoardCast(const std::string &message);
    int ConnectTo(const std::string &client_ip, const int &client_port);
    void Connect();
    void Print();

    ~Server();

    friend void do_accept(evutil_socket_t fd, short event_type, void *arg);
    friend void do_read(evutil_socket_t fd, short event_type, void *arg);
    friend void do_send(evutil_socket_t fd, short event_type, void *arg);
};

// 暂定消息格式为:"user_id time_stamp message"
struct Message
{
    boost::uuids::uuid peer_id;
    // const std::vector<int> clock;
    int clock;
    std::string message;

    Message() {}
    Message(const boost::uuids::uuid id, const int clk, const std::string &msg) : peer_id{id}, clock{clk}, message{msg} {}

    // 序列化方法
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version)
    {
        ar & peer_id;
        ar & clock;
        ar & message;
    }
};

struct SendArgs
{
    const Message message;
    const int client_socket;
    Server *server;
    SendArgs(const Message &msg, int sock, Server *s) : message{msg}, client_socket{sock}, server{s} {}
};

#endif