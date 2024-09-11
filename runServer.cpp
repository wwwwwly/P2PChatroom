#include "server.hpp"
#include "ThreadPool.hpp"

using namespace std;

int main(int argc, char *argv[])
{

    Server server{"192.168.182.129", 6666};
    server.ConnectTo("192.168.182.129", 7777);
    string message;

    while (1)
    {
        server.Print();
        cin >> message;
        server.BoardCast(message);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // cout << test.get() << endl;
    }

    cin.get();
    return 0;
}