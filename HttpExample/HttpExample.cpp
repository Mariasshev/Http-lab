﻿#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>


using namespace std;

//vector<string> findWordsInJSON(const string& filename, const vector<string>& words) 
//{
//    ifstream file(filename);
//    vector<string> result;
//
//    if (!file.is_open()) 
//    {
//        cout << "Ошибка открытия файла" << endl;
//        return result;
//    }
//
//    string line;
//    while (getline(file, line)) 
//    {
//
//        for (const auto& word : words) 
//        {
//            size_t pos = line.find(word);
//            if (pos != string::npos) 
//            {
//                
//                result.push_back(line);
//                break; 
//            }
//        }
//    }
//
//    file.close();
//    return result;
//}



void printKeyValuePairs(const string& filename, const vector<string>& keys) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Ошибка открытия файла" << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        // Обработка каждой строки из файла
        for (const auto& key : keys) {
            size_t pos = line.find(key + ":");
            if (pos != string::npos) {
                // Найдена строка с нужным ключом
                pos += key.size() + 1; // Пропускаем ключ и символ ":"
                string value = line.substr(pos);
                cout << key << ": " << value << endl;
            }
        }
    }

    file.close();
}


int main()
{
    setlocale(0, "ru");

    //1. инициализация "Ws2_32.dll" для текущего процесса
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {

        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }  

    //инициализация структуры, для указания ip адреса и порта сервера с которым мы хотим соединиться
   
    char hostname[255] = "api.openweathermap.org";
    
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

    string uri = "/data/2.5/weather?q=Odessa&appid=75f6e64d49db78658d09cb5ab201e483&mode=JSON";

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

    /*-----------------*/
    //const string filename = "weather.json";
    //const vector<string> wordsToFind = { "date", "country", "name", "coord", "temp", "sunrise", "sunset" };
   

    //vector<string> linesWithWords = findWordsInJSON(filename, wordsToFind);

    //// Выводим найденные строки с найденным словом
    //for (const auto& line : linesWithWords) {
    //    cout << line << endl;
    //}

    const string filename = "weather.json";
    vector<string> keysToPrint = { "date", "temp", "sunrise", "sunset" };
    printKeyValuePairs(filename, keysToPrint);


    //cout << response << endl;

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