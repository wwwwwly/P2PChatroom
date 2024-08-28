#include <iostream>
#include <string>
#include <cstring>
#include <ctime>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h> // 定义TCP/IP相关的一些结构体和函数，包含了IPPROTO_TCP的定义
#include <arpa/inet.h>  //主要处理IPV4地址的转换，网络字节序和主机字节序的转换，包含了inet_addr的定义

#include <mysql/mysql.h>

using namespace std;

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
    string ip;
    int socket_port;
    int server_socket;
    int ConnectionInit();
    int DatabaseInit();
    int GetMaxUserID();

protected:
    MYSQL *database;
    vector<int> clock; // 向量时钟
    int max_user_id;

public:
    Server() {}
    Server(const string &_ip, const int &_port) : ip{_ip}, socket_port{_port}
    {
        ConnectionInit();
        DatabaseInit();
        max_user_id = GetMaxUserID();
    }
    string GetServerTime();
};