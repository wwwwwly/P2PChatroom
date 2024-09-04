#include "server.hpp"

#include <iostream>
#include <memory>
#include <ctime>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>   // 定义TCP/IP相关的一些结构体和函数，包含了IPPROTO_TCP的定义
#include <arpa/inet.h>    //主要处理IPV4地址的转换，网络字节序和主机字节序的转换，包含了inet_addr的定义
#include <event2/event.h> //libevent

using std::cerr;
using std::cout;
using std::endl;
using std::string;

int Server::ConnectionInit()
{
    int status = SUCCESS;
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1)
    {
        cerr << "ERROR: Failed to create server socket.\n";
        cout << "ERROR message: " << strerror(errno) << '\n';
        status = ERROR;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 端口重用

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server_addr.sin_port = htons(socket_port); // 端口号需要从主机字节序转换为网络字节序  因为端口号在0~65535之间，故可以用htons处理int

    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        cerr << "ERROR: Failed to bind server socket.\n";
        cout << "ERROR message: " << strerror(errno) << '\n';
        status = ERROR;
    };

    if (listen(server_socket, 32) == -1) // 连接请求队列长度32
    {
        cerr << "ERROR: Failed to listen.\n";
        cout << "ERROR message: " << strerror(errno) << '\n';
        status = ERROR;
    };

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_addr_len);

    evutil_make_socket_nonblocking(server_socket); // 设置无阻赛

    // 创建event_base 事件的集合，多线程的话 每个线程都要初始化一个event_base
    event_base *base_event;
    base_event = event_base_new();
    string method = event_base_get_method(
        base_event); // 获取IO多路复用的模型，windows一般为IOCP,linux一般为epoll
    cout << "METHOD:" << method << endl;

    // 创建一个事件，类型为持久性EV_PERSIST，回调函数为do_accept（主要用于监听连接进来的客户端）
    // 将base_ev传递到do_accept中的arg参数
    event *ev;
    ev = event_new(base_event, server_socket, EV_TIMEOUT | EV_READ | EV_PERSIST,
                   do_accept, base_event);

    // 注册事件，使事件处于 pending的等待状态
    event_add(ev, NULL);

    // 事件循环
    event_base_dispatch(base_event);

    // 销毁event_base
    event_base_free(base_event);

    return status;
}

int Server::DatabaseInit()
{
    int status = SUCCESS;
    database = mysql_init(nullptr);
    if (database == nullptr)
    {
        cerr << "ERROR: Failed to init MySQL.\n";
        status = ERROR;
    }
    if (mysql_real_connect(database, server_ip.c_str(), "root", "12345678", "p2p", 3306, nullptr, 0) == nullptr)
    {
        cerr << "ERROR: Failed to connect MySQL.\n";
        status = ERROR;
    }

    string table_name{"chat_record_" + GetServerTime()}; // 每次启动服务器时都新建一张聊天记录表
    string sql_query = "create table " + table_name + " ("
                                                      "user_id int(9) not null,"
                                                      "user_name nvarchar(20) not null,"
                                                      "time_stamp int(9) not null,"
                                                      "message nvarchar(" +
                       std::to_string(msg_len) + ") not null,"
                                                 "primary key(user_id,time_stamp)"
                                                 ");";
    if (mysql_query(database, sql_query.c_str()))
    {
        cerr << "ERROR: Failed to create chat_record table.\n";
        status = ERROR;
    }
    return status;
}

string Server::GetServerTime()
{
    time_t t = time(nullptr);    // 获取时间戳
    tm *tm_info = localtime(&t); // 转化为tm结构体，包含年月日时分秒信息

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", tm_info); // 格式化为YYYYMMDD_HHMMSS形式

    return string(buffer);
}

int Server::GetMaxUserID()
{
    string sql_query = "select max(user_id) from user_info;";
    if (mysql_query(database, sql_query.c_str()))
    {
        cerr << "ERROR: Failed to get max user_id.\n";
        return ERROR;
    }

    MYSQL_RES *res = mysql_store_result(database);
    if (res == nullptr)
    {
        cerr << "ERROR: Failed to store result of max user_id.\n";
        return ERROR;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row || row[0])
    {
        cerr << "ERROR: Failed to fetch row of max user_id.\n";
        return ERROR;
    }

    mysql_free_result(res);

    return atoi(row[0]);
}

int Server::Send(const string &message, int client_socket)
{
    int status = SUCCESS;

    if (write(client_socket, message.c_str(), message.size()) == -1)
    {
        cerr << "ERROR: Server failed to send.\n";
        cout << "ERROR message: " << strerror(errno) << '\n';
        status = ERROR;
    }
    cout << "Server::Send:" << message << endl;
    close(client_socket);

    return status;
}

int Server::Receive(string &message, int client_socket)
{
    int status = SUCCESS;

    char *buffer = new char[msg_len];
    if (read(client_socket, buffer, msg_len) == -1)
    {
        cerr << "ERROR: Server failed to receive.\n";
        cout << "ERROR message: " << strerror(errno) << '\n';
        status = ERROR;
    }

    message = string(buffer);
    delete buffer;
    close(client_socket);

    return status;
}