#include "client.hpp"

int Client::ConnectionInit()
{
    int status = SUCCESS;

    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == -1)
    {
        cerr << "ERROR: Failed to create client socket.\n";
        cout << "ERROR message: " << strerror(errno) << '\n';
        status = ERROR;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, sizeof(server_addr), 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server_addr.sin_port = htons(socket_port);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        cerr << "ERROR: Failed to connect server socket.\n";
        cout << "ERROR message: " << strerror(errno) << '\n';
        status = ERROR;
    }
}