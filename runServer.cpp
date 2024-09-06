#include "server.hpp"
#include "ThreadPool.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    Server server{"192.168.182.129", std::atoi(argv[0])};
    server.ConnectTo("192.168.182.129", std::atoi(argv[1]));
    string message{"hello client!"};
    server.BoardCast(message);

    cin.get();
    return 0;
}