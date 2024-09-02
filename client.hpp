#include "server.hpp"

class Client : protected Server
{
private:
    int client_socket;
    int ConnectionInit();

protected:
    string client_ip;
    int client_port;

public:
    Client() {}
    Client(const string &ip, const int &port)
    {
        server_ip = ip;
        socket_port = port;
        ConnectionInit();
    }
    int Send(const string &message);
    int Receive(string &buffer);
    virtual ~Client()
    {
        close(client_socket);
    }
};