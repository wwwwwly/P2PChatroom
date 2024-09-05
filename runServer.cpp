#include "server.hpp"
#include "ThreadPool.hpp"

using namespace std;

int main()
{
    Server server{"192.168.182.129", 6666};
    string message{"hello client!"};
    server.BoardCast(message);

    cin.get();
    return 0;
}