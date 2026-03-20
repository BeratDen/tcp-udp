#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, lws2_32.lib)

int main() {

    WSAData wsaData{};
    int wsaResult = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (wsaResult != 0) {std::cerr << "WSAStartup failed" << WSAGetLastError() << std::endl; return 1;}

    SOCKET clientSocket = socket(AF_INET,SOCK_STREAM, IPPROTO_TCP);
    if(clientSocket == INVALID_SOCKET) {std::cerr << "socket failed :" << WSAGetLastError() << std::endl; WSACleanup(); return 1;};

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    // deprecated old api
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int connectResult = connect(
        clientSocket,
        reinterpret_cast<sockaddr*>(&serverAddr),
        sizeof(serverAddr)
    );

    if(connectResult == SOCKET_ERROR) {std::cerr << "connect failed" << WSAGetLastError() << std::endl; closesocket(clientSocket); return 1;};

    std::cout << "client connected to 127.0.0.1:8080" << std::endl;

    std::string message = "Hello server this is a message from client!";
    
    int bytesSent = send(clientSocket,message.c_str(),static_cast<int>(message.size()),0);

    if(bytesSent == SOCKET_ERROR) {std::cerr << "send failed" << WSAGetLastError() << std::endl; closesocket(clientSocket); return 1;};

    std::cout << "sent " << bytesSent << std::endl;

    char buffer[1024]{};
    int bytesReceived = recv(clientSocket,buffer,sizeof(buffer),0);

    if (bytesReceived > 0) {std::string response(buffer, bytesReceived); std::cout << "received" << bytesReceived << " bytes: " << response << std::endl;}
    else if (bytesReceived == 0) {std::cout << "Server closed the connection" << std::endl;}
    else {std::cerr << "recv failed" << WSAGetLastError() << std::endl; return 1;};

    closesocket(clientSocket);
    WSACleanup();
    return 0;

}