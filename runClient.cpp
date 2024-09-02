#include "client.hpp"

int main()
{
    Client client{"192.168.182.129", 6666};
    string message{"hello server"};
    cout << client.Send(message) << endl;
    string recv;
    client.Receive(recv);
    cout << "mseeage received: " << recv << endl;

    cin.get();
    return 0;
}