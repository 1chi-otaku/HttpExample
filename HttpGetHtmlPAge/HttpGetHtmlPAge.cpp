#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <string>
using namespace std;

int main()
{

    //1. инициализация "Ws2_32.dll" для текущего процесса
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {

        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }

    //инициализация структуры, для указания ip адреса и порта сервера с которым мы хотим соединиться
    //3277fc1b38e64592b4d130355222012
    string hostname = "api.weatherapi.com";

    addrinfo* result = NULL;

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo(hostname.c_str(), "http", &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << endl;
        WSACleanup();
        return 3;
    }


    SOCKET connectSocket = INVALID_SOCKET;
    addrinfo* ptr = NULL;

    //Пробуем присоединиться к полученному адресу
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        //2. создание клиентского сокета
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        //3. Соединяемся с сервером
        iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    //4. HTTP Request

    string uri = "/v1/current.xml?key=3277fc1b38e64592b4d130355222012&q=London&aqi=no";

    string request = "GET " + uri + " HTTP/1.1\n";
    request += "Host: " + string(hostname) + "\n";
    request += "Accept: */*\n";
    request += "Accept-Encoding: gzip, deflate, br\n";
    request += "Connection: close\n";
    request += "\n";

    //отправка сообщения
    if (send(connectSocket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
        cout << "send failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 5;
    }
    cout << "send data" << endl;

    //5. HTTP Response

    string response;

    const size_t BUFFERSIZE = 1024;
    char resBuf[BUFFERSIZE];

    int respLength;

    do {
        respLength = recv(connectSocket, resBuf, BUFFERSIZE, 0);
        if (respLength > 0) {
            response += string(resBuf).substr(0, respLength);
        }
        else {
            cout << "recv failed: " << WSAGetLastError() << endl;
            closesocket(connectSocket);
            WSACleanup();
            return 6;
        }

    } while (respLength == BUFFERSIZE);
    for (int i = 0; i < response.length(); i++)
    {
        if (response[i + 1] == ',' || response[i] == '{' || response[i + 1] == '}') {
            response.insert(i + 1, 1, '\n');
            i++;
        }
    }

    cout << response << endl;

    //отключает отправку и получение сообщений сокетом
    iResult = shutdown(connectSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
        cout << "shutdown failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 7;
    }

    closesocket(connectSocket);
    WSACleanup();
}