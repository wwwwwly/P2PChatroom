#include "server.hpp"
#include "ThreadPool.hpp"
#include <signal.h>

using namespace std;

int main(int argc, char *argv[])
{
    // signal(SIGPIPE, SIG_IGN); // 忽略SIGPIPE信号

    Server server{"192.168.182.129", 7777};
    // server.ConnectTo("192.168.182.129", 6666);
    string message{"I'm test."};

    while (1)
    {
        // cin >> message;
        server.Print();

        server.BoardCast(message);
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    cin.get();
    return 0;
}