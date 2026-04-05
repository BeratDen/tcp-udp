

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>

#include "../models/ClientInfo.hpp"
#include "../Sockets/SocketGuard.hpp"
using namespace Sockets;

#pragma comment(lib, "ws2_32.lib")

std::vector<ClientInfo> clients{};
std::mutex clientsMutex{};

void removeClient(SOCKET clientSocket ) {
    std::lock_guard<std::mutex> lock(clientsMutex);

    clients.erase(
        std::remove_if(
            clients.begin(),
            clients.end(),
            [clientSocket](const ClientInfo& client) { return client.socket == clientSocket; }
        ),
        clients.end()
    );
}

void broadcastMessage(const std::string& message,SOCKET senderSocket) {
    std::lock_guard<std::mutex> lock(clientsMutex);

    for(const auto& client: clients) {
        if(client.socket == senderSocket) {continue;}

        int result = send(client.socket,message.c_str(),static_cast<int>(message.size()),0);

        if(result == SOCKET_ERROR) { std::cerr << "send failed" << WSAGetLastError() << std::endl; }
    }
}

void handleSocket(ClientInfo client)
{
    SocketGuard guard(client.socket);
    std::string clientLabel = client.getClientLabel();
    std::cout << clientLabel << " connected" << std::endl;
    std::string joinMessage = "[SERVER] " + clientLabel + " joined the chat" + "\n";
    broadcastMessage(joinMessage,client.socket);

    char buffer[1024]{};

    while (true)
    {
        ZeroMemory(buffer, sizeof(buffer));

        int bytesReceived = recv(client.socket, buffer, sizeof(buffer), 0);

        if (bytesReceived > 0)
        {
            std::string message(buffer, bytesReceived);
            std::string fullMessage = "[" + clientLabel + "] " + message + "\n";
            
            std::cout << fullMessage;
            broadcastMessage(fullMessage,client.socket);
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
    removeClient(client.socket);

    std::string leaveMessage = "[SERVER] " + clientLabel + " left the chat" + "\n";
    broadcastMessage(leaveMessage,client.socket);

    std::cout << clientLabel << " disconnected" << std::endl;
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

    if(listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) { std::cerr << "listen failed" << WSAGetLastError() << std::endl; closesocket(listenSocket); return 1; };

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

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(client);
        }

        std::thread clientThread(handleSocket, client);
        clientThread.detach();
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}