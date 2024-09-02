#include "server.hpp"

int main()
{
    Server server{"192.168.182.129", 6666};
    string message{"hello client!"};
    server.Send(message);
    string recv;
    cout << server.Receive(recv) << endl;
    cout << "Message received: " << recv << endl;

    cin.get();
    return 0;
}