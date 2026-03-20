#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>

#include "../models/ClientInfo.hpp"

#pragma comment(lib, "ws2_32.lib")

void handleSocket(ClientInfo client)
{
    char buffer[1024]{};

    while (true)
    {
        ZeroMemory(buffer, sizeof(buffer));

        int bytesReceived = recv(client.socket, buffer, sizeof(buffer), 0);

        if (bytesReceived > 0)
        {
            std::string message(buffer, bytesReceived);
            std::cout << "Client " << client.clientId << " says: " << message << std::endl;

            std::string response = "Server recieved: " + message;

            int bytesSent = send(client.socket, response.c_str(), static_cast<int>(response.size()), 0);

            if (bytesSent == SOCKET_ERROR)
            {
                std::cerr << "send failed" << WSAGetLastError() << std::endl;
                break;
            }
        }
        else if (bytesReceived == 0)
        {
            std::cout << "Client disconnected greacefully" << std::endl;
            break;
        }
        else
        {
            std::cerr << "receive failed :" << WSAGetLastError() << std::endl;
            break;
        };
    }

    closesocket(client.socket);
    std::cout << "Client " << client.clientId << " disconnected" << std::endl;
}

int main()
{

    WSADATA wsaData{};

    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0)
    {
        std::cerr << "WSA startup failed" << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;   // IPv4
    serverAddr.sin_port = htons(8080); // Host TO Network Short (Benim bilgisayarımdaki sayıyı (host), network formatına çevir)
    // old api deprecated
    // serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int result = inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (result <= 0)
    {
        std::cerr << "inet_pton failed" << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return 1;
    }

    int bindResult = bind(
        listenSocket,
        reinterpret_cast<sockaddr *>(&serverAddr),
        sizeof(serverAddr));

    if (bindResult == SOCKET_ERROR)
    {
        std::cerr << "bind failed " << WSAGetLastError() << std::endl;
        std::cerr << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return 1;
    }

    int listenResult = listen(listenSocket, SOMAXCONN); // SOMAXCONN listen fonksiyonunda kullanılan maksimum backlog değeri // backlog aynı anda bağlanmayı bekleyen client sayısı

    if (listenResult == SOCKET_ERROR)
    {
        std::cerr << "listen failed" << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return 1;
    }

    std::cout << "server is listening on 127.0.0.1:8080" << std::endl;

    static int clientCounter = 0;

    while (true)
    {
        sockaddr_in clientAddr{};

        int clientAddrLen = sizeof(clientAddr);

        SOCKET clientSocket = accept(
            listenSocket,
            reinterpret_cast<sockaddr *>(&clientAddr),
            &clientAddrLen);

        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "accept failed" << WSAGetLastError() << std::endl;
            continue;
        };
        
        int clientId = clientCounter++;

        ClientInfo client{clientSocket, clientAddr, clientId};
        std::thread clientThread(handleSocket, client);
        clientThread.detach();
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}