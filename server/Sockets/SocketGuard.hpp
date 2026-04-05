#pragma once 

#include <winsock2.h>

namespace Sockets
{
    class SocketGuard
    {
        public: 
            explicit SocketGuard(SOCKET s) : sock(s) {}
            ~SocketGuard()
            {
                if (sock != INVALID_SOCKET)
                {
                    closesocket(sock);
                }
            }

            SocketGuard(const SocketGuard &) = delete;
            SocketGuard &operator=(const SocketGuard &) = delete;
            SOCKET get() const { return sock; }

        private:
            SOCKET sock;
    };
}