#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#ifndef SD_SEND
#define SD_SEND 1
#endif

using namespace std;

// Инициализация Windows Sockets DLL
int WinSockInit()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 0); /* Требуется WinSock ver 2.0*/
    printf("Запуск Winsock...\n");
    // Проинициализируем Dll
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        printf("\nОшибка: Не удалось найти работоспособную Winsock Dll\n");
        return 1;
    }
    // Проверка версии Dll
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 0)
    {
        printf("\nОшибка: Не удалось найти работоспособную WinSock DLL\n");
        WSACleanup(); // Отключение Windows Sockets DLL
        return 1;
    }
    //printf(" Winsock запущен.\n");
    return 0;
}

// Отключение Windows Sockets DLL
void WinSockClose()
{
    WSACleanup();
    printf("WinSock Closed...\n");
}

// Остановка передачи данных
void stopTCP(SOCKET s)
{
    shutdown(s, SD_SEND); // Остановка передачи данных
    closesocket(s); // Закрытие сокета
    printf("Socket %ld closed.\n", s);
}


int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "rus");

    if (argc != 2)
    {
        cerr << "Использование: server <имя_хоста>\n";
        return 1;
    }

    const char* hostname = argv[1];

    if (WinSockInit() != 0)
    {
        return 1;
    }

    char buffer[1024] = "";

    // Преобразование имени хоста в IP-адрес
    hostent* host = gethostbyname(hostname);
    if (host == NULL)
    {
        cerr << "Ошибка: Не удалось разрешить имя хоста.\n";
        WinSockClose();
        return 1;
    }

    SOCKET listenSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) 
    {
        cerr << "socket failed: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = *(unsigned long*)host->h_addr_list[0];
    serverAddr.sin_port = htons(80);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        cerr << "bind failed: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Вывод информации о сервере
    sockaddr_in serverInfo;
    int serverInfoSize = sizeof(serverInfo);
    if (getsockname(listenSocket, (sockaddr*)&serverInfo, &serverInfoSize) == 0) 
    {
        char ipStr[16]; // Достаточно для IPv4 "xxx.xxx.xxx.xxx\0"
        inet_ntoa(serverInfo.sin_addr);
        strcpy_s(ipStr, inet_ntoa(serverInfo.sin_addr));
        cout << "Сервер запущен на IP: " << ipStr << ", порт: " << ntohs(serverInfo.sin_port) <<endl;
    }
    else 
    {
        cerr << "Не удалось получить информацию о сокете сервера: " << WSAGetLastError() << endl;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) 
    {
        cerr << "listen failed: " << WSAGetLastError() << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    cout << "Ожидание подключений..." << endl;

    while (true)
    {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) 
        {
            cerr << "accept failed: " << WSAGetLastError() <<endl;
            continue;
        }

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        //cout << "Получено " << bytesReceived << " байт от клиента: " << buffer << endl;
        string response = buffer;
        send(clientSocket, response.c_str(), response.length(), 0);

        memset(buffer, 0, sizeof(buffer)); // Очистка буфера
        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}

