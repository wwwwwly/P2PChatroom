#include "server.hpp"
#include "ThreadPool.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    Server server{"192.168.182.129", 6666};
    // server.ConnectTo("192.168.182.129", std::atoi(argv[2]));
    string message{"hello client!"};

    while (1)
    {
        cout << 1 << endl;
        server.Print();
        server.BoardCast(message);
        cout << 2 << endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    cin.get();
    return 0;
}