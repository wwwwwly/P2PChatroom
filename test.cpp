#include "server.hpp"
#include "ThreadPool.hpp"

using namespace std;

int main(int argc, char *argv[])
{
    Server server{"192.168.182.129", 7777};
    server.ConnectTo("192.168.182.129", 6666);
    string message;
    std::future<int> test = server.test.get_future();

    while (1)
    {
        cin >> message;
        server.Print();

        server.BoardCast(message);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cout << test.get() << endl;
    }

    cin.get();
    return 0;
}