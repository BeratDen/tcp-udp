#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <atomic>
#include <thread>

#pragma comment(lib, lws2_32.lib)

std::atomic<bool> running = true;

void receiveMessage(SOCKET clientSocket)
{
    char buffer[1024];

    while (running)
    {
        ZeroMemory(buffer, sizeof(buffer));

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived > 0)
        {
            std::string message(buffer, bytesReceived);
            std::cout << "\n"
                      << message;
            std::cout << "\n> ";
            std::cout.flush();
        }
        else if (bytesReceived == 0)
        {
            std::cout << "\n [SERVER] closed connection" << std::endl;
            running = false;
            break;
        }
        else
        {
            std::cerr << "recv failed" << WSAGetLastError() << std::endl;
            running = false;
            break;
        }
    }
}

int main()
{

    WSAData wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed" << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "socket failed :" << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    };

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    // deprecated old api
    // serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0)
    {
        std::cerr << "inet_pton failed" << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        return 1;
    };

    if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "connect failed" << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        return 1;
    };

    std::cout << "connected to chat server" << std::endl;
    std::cout << "type 'exit' to close connection" << std::endl;
    std::cout << "> ";
    /*
    * thread şu şekilde çalışyor std::thread çağırılır sonraki thread değişken adı <receiver> daha sonra
    * std::thread <receiver>(clientSocket); ilk değişken thread'in ne yapacağı callback gibi
    * sonraki değişkenler ilk çağırılan callback'in alacağı değerler   
    */
    std::thread receiver(receiveMessage, clientSocket);

    while (running)
    {
        std::string input{};
        std::cout << "> ";
        std::getline(std::cin, input);

        if(!running) {break;}

        if (input == "exit") { running = false; shutdown(clientSocket, SD_BOTH); break; };

        if (input.empty()) { std::cerr << "> " << std::endl; continue; };

        int bytesSent = send(clientSocket, input.c_str(), static_cast<int>(input.size()), 0);

        if (bytesSent == SOCKET_ERROR)
        {
            std::cout << "send failed" << WSAGetLastError() << std::endl;
            break;
        };
        
    }

    if (receiver.joinable()) { receiver.join(); }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}