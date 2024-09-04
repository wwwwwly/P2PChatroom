#ifndef _SERVER_HPP_
#define _SERVER_HPP_

#include <string>
#include <vector>

#include <unistd.h> //用于unix/linux系统，定义了大量系统调用函数

#include <mysql/mysql.h>

enum STATUS
{
    ERROR = -1,
    INVALID = 0,
    SUCCESS = 1,
    INVALID_USER_NAME,
    INVALID_PASSWORD
};

class Server
{
private:
    int server_socket;
    int ConnectionInit();
    int DatabaseInit();
    int GetMaxUserID();

protected:
    MYSQL *database;
    std::vector<int> clock; // 向量时钟
    int max_user_id;
    std::string server_ip;
    int socket_port;
    int msg_len = 300; /// 消息长度

public:
    Server() {}
    Server(const std::string &_ip, const int &_port) : server_ip{_ip}, socket_port{_port}
    {
        ConnectionInit();
        DatabaseInit();
        max_user_id = GetMaxUserID();
    }
    std::string GetServerTime();
    virtual int Send(const std::string &message, int client_socket);
    virtual int Receive(std::string &message, int client_socket);
    virtual ~Server()
    {
        close(server_socket);
        mysql_close(database);
    }
};

#endif