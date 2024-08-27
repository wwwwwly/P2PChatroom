#include <iostream>
#include <string>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h> // 定义TCP/IP相关的一些结构体和函数，包含了IPPROTO_TCP的定义
#include <arpa/inet.h>  //主要处理IPV4地址的转换，网络字节序和主机字节序的转换，包含了inet_addr的定义
using namespace std;

int main(int argc, char *argv[]) // argv[0]->ip,argv[1]->port
{
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[0]);
    server_addr.sin_port = htons(stoi(argv[1])); // 端口号需要从主机字节序转换为网络字节序  因为端口号在0~65535之间，故可以用htons处理int
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, 32); // 连接请求队列长度32

    return 0;
}