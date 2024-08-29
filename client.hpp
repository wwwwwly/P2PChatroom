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
    Client(string ip, int port) : client_ip{ip}, client_port{port}
    {
        ConnectionInit();
    }
};