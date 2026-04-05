server build
g++ .\server\src\server.cpp -o server.exe

client build
g++ .\client\client.cpp -o client.exe -lws2_32

run server side
.\server.exe

run client side
.\client.exe