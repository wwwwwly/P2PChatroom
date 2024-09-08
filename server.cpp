#include "server.hpp"

#include <iostream>
#include <memory>
#include <ctime>
#include <cstring>
#include <memory>

#include <sys/socket.h>
#include <netinet/in.h> // 定义TCP/IP相关的一些结构体和函数，包含了IPPROTO_TCP的定义
#include <arpa/inet.h>  //主要处理IPV4地址的转换，网络字节序和主机字节序的转换，包含了inet_addr的定义

using std::cerr;
using std::cout;
using std::endl;
using std::string;

// 读取客户端
void do_read(evutil_socket_t fd, short event_type, void *arg)
{
    Server *server = (Server *)arg;
    string message;
    server->Receive(message, fd);
    if (!message.empty())
    {
        std::unique_lock<std::mutex> lock(server->mtx);
        server->records.emplace_back(std::move(message));
    }
}

// 回调函数，用于监听连接进来的客户端socket
void do_accept(evutil_socket_t fd, short event_type, void *arg)
{
    sockaddr_in client_address; // 客户端网络地址结构体
    socklen_t in_size = sizeof(client_address);

    // 客户端socket
    int client_socket = accept(fd, (sockaddr *)&client_address,
                               &in_size); // 等待接受请求，这边是阻塞式的

    if (client_socket < 0)
    {
        perror("ERROR:Fail to accept.\n");
        exit(1);
    }
    Server *server = (Server *)arg;
    // 类型转换
    event_base *base_event = event_base_new();

    server->client_sockets.push_back(client_socket);
    server->bases.push_back(base_event);

    server->thread_pool.submit([server](event_base *base_event, int client_socket)
                               {
        event *ev;
        ev = event_new(base_event, client_socket, EV_TIMEOUT | EV_READ | EV_PERSIST,
                   do_read, server); // 超时触发 socket可读时触发 保持未决
        // 注册事件，使事件处于 pending的等待状态 
        event_add(ev, NULL); 
        // 事件循环
        event_base_dispatch(base_event); }, base_event, client_socket);
}

void do_send(evutil_socket_t fd, short event_type, void *arg)
{
    SendArgs *args = (SendArgs *)arg;
    args->server->test.set_value(args->server->Send(args->message, args->client_socket));
}

std::vector<int> Server::client_sockets;
std::vector<event_base *> Server::bases;
std::mutex Server::mtx;
std::vector<std::string> Server::records;

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

    evutil_make_socket_nonblocking(server_socket); // 设置无阻赛

    // 创建event_base 事件的集合，多线程的话 每个线程都要初始化一个event_base
    event_base *base_event = event_base_new();
    bases.push_back(base_event);

    // 创建一个事件，类型为持久性EV_PERSIST，回调函数为do_accept（主要用于监听连接进来的客户端）
    // 将base_ev传递到do_accept中的arg参数
    event *ev;
    ev = event_new(base_event, server_socket, EV_TIMEOUT | EV_READ | EV_PERSIST,
                   do_accept, this);

    // 注册事件，使事件处于 pending的等待状态
    event_add(ev, NULL);

    // thread_pool.submit([this]
    //                    { this->Print(); });

    // 事件循环
    thread_pool.submit(event_base_dispatch, base_event);

    return status;
}

int Server::ConnectTo(const std::string &client_ip, const int &client_port)
{
    int status = SUCCESS;

    int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // 这个client_socket暂时用不到
    // 向服务器发起请求
    sockaddr_in client_address;
    memset(&client_address, 0, sizeof(client_address)); // 每个字节都⽤0填充

    client_address.sin_family = PF_INET;
    client_address.sin_addr.s_addr = inet_addr(client_ip.c_str()); // 将套接字与服务器绑定
    client_address.sin_port = htons(client_port);

    if (connect(client_socket, (sockaddr *)&client_address, sizeof(sockaddr)) < -1)
    {
        std::cerr << "ERROR:Failed to connect.\n";
        status = ERROR;
    }
    return status;
}

void Connect()
{
}

string Server::GetServerTime()
{
    time_t t = time(nullptr);    // 获取时间戳
    tm *tm_info = localtime(&t); // 转化为tm结构体，包含年月日时分秒信息

    char buffer[16];
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", tm_info); // 格式化为YYYYMMDD_HHMMSS形式

    return string(buffer);
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

    return status;
}

void Server::BoardCast(const string &message)
{
    for (auto s : client_sockets)
    {
        if (s != -1)
        {
            event_base *base_event = event_base_new();
            bases.push_back(base_event);
            std::shared_ptr<SendArgs> args = std::make_shared<SendArgs>(message, s, this);

            thread_pool.submit([args, base_event, s, message]()
                               {
                event *ev;
                ev = event_new(base_event, s, EV_TIMEOUT | EV_WRITE ,
                        do_send, args.get()); // 超时触发 socket可读时触发 不保持未决
                event_add(ev, NULL);
                event_base_dispatch(base_event); });
        }
    }
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

    return status;
}

void Server::Print()
{
    // while (1)
    // {

    if (!records.empty())
    {
        for (auto m : records) // todo:这里内存越界
            cout << m << endl;
        {
            std::unique_lock<std::mutex> lock(mtx);
            records.clear();
        }
    }
    // }
}
