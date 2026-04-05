#include <winsock2.h>
#include "SocketGuard.hpp"

namespace Sockets
{

    class SocketGuard
    {
    public:
        // Constructor
        explicit SocketGuard(SOCKET s) : sock(s) {}

        // Destructor
        ~SocketGuard()
        {
            if (sock != INVALID_SOCKET)
            {
                closesocket(sock);
            }
        }

        // copy protection (two objects cannot have the same socket)
        SocketGuard(const SocketGuard &) = delete;
        SocketGuard &operator=(const SocketGuard &) = delete;

        // expose the socket
        SOCKET get() const { return sock; }

    private:
        SOCKET sock;
    };

}