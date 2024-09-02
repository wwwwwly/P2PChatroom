#include "server.hpp"

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
                       to_string(msg_len) + ") not null,"
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

int Server::Send(const string &message)
{
    int status = SUCCESS;

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_addr_len);

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

int Server::Receive(string &message)
{
    cout << "Server::Receive1\n";

    int status = SUCCESS;

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_addr_len);

    cout << "Server::Receive2\n";

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