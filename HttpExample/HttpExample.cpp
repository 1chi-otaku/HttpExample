#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

int main()
{
    string cityname;
    int i = 1;
    //1. инициализация "Ws2_32.dll" для текущего процесса
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {

        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }

    //инициализация структуры, для указания ip адреса и порта сервера с которым мы хотим соединиться

    char hostname[1024] = "api.openweathermap.org";

    addrinfo* result = NULL;

    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo(hostname, "http", &hints, &result);
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
    while (true)
    {
        system("cls");
        cout << "Enter 0 to exit." << endl;
        cout << "Enter \"Log\" to see request history." << endl;
        cout << "Enter City Name - ";
        cin >> cityname;
        system("cls");
        if (cityname == "0") {
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
            return 0;
        }
        else if (cityname == "Log") {
            system("cls");
            string line;

            std::ifstream in("history.txt"); 
            if (in.is_open())
            {
                while (getline(in, line))
                {
                    std::cout << line << std::endl;
                }
            }
            in.close();     
            cout << line;
            system("pause");
            continue;
        }
        string uri = "/data/2.5/weather?q=" + cityname + "&appid=75f6e64d49db78658d09cb5ab201e483&mode=JSON";

        string request = "GET " + uri + " HTTP/1.1\n";
        request += "Host: " + string(hostname) + "\n";
        request += "Accept: */*\n";
        request += "Accept-Encoding: gzip, deflate, br, utf-8\n";
        request += "Connection: \"keep-alive\"\n";
        request += "\n";

        //отправка сообщения
        if (send(connectSocket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
            cout << "send failed: " << WSAGetLastError() << endl;
            closesocket(connectSocket);
            WSACleanup();
            return 5;
        }

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
        const int DATA_AMOUNT = 9;
        string new_string;
        string subject_to_search[DATA_AMOUNT] = { "id","name","country","lat", "lon", "temp_min","temp_max", "sunrise","sunset" };
        string subject_to_show[DATA_AMOUNT] = { "City ID - ", "City Name - ", "Country - ", "x-coordinates - ", "y-coordinates - ","Minumum Temperature - ", "Maximum Temperature - ", "Sunrise-coordinates - ", "Sunset coordinates - " };
        ofstream out;
        system("cls");
        out.open("history.txt",std::ios::app);
        for (int i = 0; i < DATA_AMOUNT; i++)
        {
            size_t found = response.find(subject_to_search[i]);
            //cout << subject_to_show[i];
            new_string += subject_to_show[i];
            for (int j = found + subject_to_search[i].length() + 2; response[j] != '\n'; j++)
            {
                if (response[j] != '"') {
                    //cout << response[j];
                    new_string += response[j];
                }
            }
            new_string += '\n';
            //cout << endl;
        }
        out << i << "~----------------------------\n";
        out << new_string;
        cout << new_string;
        system("pause");
        i++;
       
    }
   

   


   
}