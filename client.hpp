#ifndef _CLIENT_HPP
#define _CLIENT_HPP

#include "server.hpp"

class Client : protected Server
{
private:
    int client_socket;
    int ConnectionInit();

protected:
    std::string client_ip;
    int client_port;

public:
    Client() {}
    Client(const std::string &ip, const int &port)
    {
        server_ip = ip;
        socket_port = port;
        ConnectionInit();
    }
    int Send(const std::string &message);
    int Receive(std::string &buffer);
    virtual ~Client()
    {
        close(client_socket);
    }
};

#endif