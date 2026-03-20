#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

int main(){

    WSADATA wsaData{};

    int wsaResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (wsaResult != 0) {std::cerr << "WSA startup failed" << WSAGetLastError() << std::endl; return 1;}
    
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {std::cerr << "Socket creation failed " << WSAGetLastError() << std::endl; WSACleanup(); return 1;}

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET; // IPv4
    serverAddr.sin_port = htons(8080); // Host TO Network Short (Benim bilgisayarımdaki sayıyı (host), network formatına çevir)
    // old api deprecated
    // serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int result = inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if(result <= 0) {std::cerr << "inet_pton failed" << WSAGetLastError() << std::endl; closesocket(listenSocket); return 1;}

    int bindResult = bind(
        listenSocket,
        reinterpret_cast<sockaddr*>(&serverAddr),
        sizeof(serverAddr)
    );

    if (bindResult == SOCKET_ERROR) {std::cerr << "bind failed " << WSAGetLastError() << std::endl; std::cerr << WSAGetLastError() << std::endl; closesocket(listenSocket); return 1;}

    int listenResult = listen(listenSocket, SOMAXCONN); // SOMAXCONN listen fonksiyonunda kullanılan maksimum backlog değeri // backlog aynı anda bağlanmayı bekleyen client sayısı 

    if (listenResult == SOCKET_ERROR) {std::cerr << "listen failed" << WSAGetLastError() << std::endl; closesocket(listenSocket); return 1;}

    std::cout << "server is listening on 127.0.0.1:8080" << std::endl;

    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);
    
    // server should be open all the time
    SOCKET clientSocket = accept(
        listenSocket,
        reinterpret_cast<sockaddr*>(&clientAddr),
        &clientAddrSize
    );

    if(clientSocket == SOCKET_ERROR) {std::cerr << "accept failed" << WSAGetLastError() << std::endl; closesocket(listenSocket); return 1;}

    char clientIp[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIp,INET_ADDRSTRLEN);

    std::cout << "Client connected from " << clientIp << ":" << ntohs(clientAddr.sin_port) << std::endl; // nthost = network → host short

    char buffer[1024]{};

    int bytesReceived = recv(
        clientSocket,
        buffer,
        sizeof(buffer),
        0
    );

    if(bytesReceived > 0) {
        std::string message(buffer, bytesReceived);

        std::cout << "Recieved" << bytesReceived << "bytes : " << message << std::endl;
        
        // dont send the message back to same client
        int bytesSent = send(clientSocket,buffer, bytesReceived,0 );

        if(bytesSent == SOCKET_ERROR) std::cerr << "send failed with error" << WSAGetLastError() << std::endl;
        else std::cout << "Echoed back " << bytesSent << "bytes" << std::endl;
    } else if (bytesReceived == 0) std::cout << "Client disconnected greacefully" << std::endl;
    else std::cerr << "recv failed with error " << WSAGetLastError() << std::endl; 
    
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}