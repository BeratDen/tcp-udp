#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

struct ClientInfo
{
    SOCKET socket;
    sockaddr_in address;
    int clientId;

    std::string getClientLabel()
    {
        return "Client " + std::to_string(clientId);
    }
};
