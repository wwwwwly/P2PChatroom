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

    int opt = 1;
    setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // 端口重用

    sockaddr_in server_addr;
    memset(&server_addr, sizeof(server_addr), 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server_addr.sin_port = htons(socket_port);

    if (connect(client_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        cerr << "ERROR: Failed to connect server socket.\n";
        cout << "ERROR message: " << strerror(errno) << '\n';
        status = ERROR;
    }

    return status;
}

int Client::Send(const string &message)
{
    int status = SUCCESS;

    sockaddr_in server_addr;
    memset(&server_addr, sizeof(server_addr), 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server_addr.sin_port = htons(socket_port);

    if (write(client_socket, message.c_str(), message.size()) == -1)
    {
        cerr << "ERROR: Client failed to send.\n";
        cout << "ERROR message: " << strerror(errno) << '\n';
        status = ERROR;
    }
    return status;
}

int Client::Receive(string &message)
{
    int status = SUCCESS;

    sockaddr_in server_addr;
    memset(&server_addr, sizeof(server_addr), 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    server_addr.sin_port = htons(socket_port);

    char *buffer = new char[msg_len];
    if (read(client_socket, buffer, msg_len) == -1)
    {
        cerr << "ERROR: Client failed to receive.\n";
        cout << "ERROR message: " << strerror(errno) << '\n';
        status = ERROR;
    }
    message = string(buffer);
    delete buffer;

    return status;
}