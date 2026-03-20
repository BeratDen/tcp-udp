#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, lws2_32.lib)

int main() {

    WSAData wsaData{};
    if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {std::cerr << "WSAStartup failed" << WSAGetLastError() << std::endl; return 1;}

    SOCKET clientSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if(clientSocket == INVALID_SOCKET) {std::cerr << "socket failed :" << WSAGetLastError() << std::endl; WSACleanup(); return 1;};

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    // deprecated old api
    // serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int result = inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    if(result <= 0) {std::cerr << "inet_pton failed" << WSAGetLastError() << std::endl; closesocket(clientSocket); return 1;};

    if(connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR){std::cerr << "connect failed" << WSAGetLastError() << std::endl; closesocket(clientSocket); return 1;};

    std::cout << "client connected to server" << std::endl;

    while (true) {
        std::string message{};
        std::cout << "> ";
        std::getline(std::cin, message);

        if(message == "exit") {std::cout << "Closing connection" << std::endl; break;};
        if(message.empty()) {std::cerr << "message is empty" << std::endl; continue;};

        int bytesSent = send(clientSocket,message.c_str(),static_cast<int>(message.size()),0);

        if(bytesSent == SOCKET_ERROR) {std::cout << "send failed" << WSAGetLastError() << std::endl; break;};

        char buffer[1024]{};
        int bytesReceived = recv(clientSocket,buffer,sizeof(buffer),0);

        if(bytesReceived > 0) {std::string response(buffer,bytesReceived); std::cout << response << std::endl;}
        else if (bytesReceived == 0) {std::cout << "Server closed the connection" << std::endl; break;}
        else {std::cerr << "recv failed" << WSAGetLastError() << std::endl; break;};
    }
    
    closesocket(clientSocket);
    WSACleanup();
    return 0;

}