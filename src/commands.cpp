#include "../include/commands.h"
#include "../include/connection.h"
#include "../include/validation.h"
#include <sys/socket.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;

void commandLoop(int sock, string &channel, const string &nick) {
    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;
        if (line == "quit") break;

        istringstream iss(line);
        string cmd;
        iss >> cmd;

        if (cmd == "send") {
            string msg;
            getline(iss, msg);
            msg = trim(msg);
            if (msg.empty()) {
                cout << "Использование: send <сообщение>" << endl;
                continue;
            }
            string request = "send " + channel + " " + nick + " " + msg + "\n";
            send(sock, request.c_str(), request.size(), 0);
        }
        else if (cmd == "read") {
            string request = "read " + channel + " " + nick + "\n";
            send(sock, request.c_str(), request.size(), 0);
        }
        else if (cmd == "join") {
            string new_channel;
            iss >> new_channel;
            if (new_channel.empty()) {
                cout << "Использование: join <канал>" << endl;
                continue;
            }

            if (new_channel.size() > 24) {
                cout << "Имя канала слишком длинное (максимум 24 символа)" << endl;
                continue;
            }

            string request = "join " + new_channel + " " + nick + "\n";
            send(sock, request.c_str(), request.size(), 0);

            string response;
            if (!recvLine(sock, response)) {
                cout << "Отключено от сервера." << endl;
                break;
            }

            if (response.rfind("OK", 0) == 0) {
                channel = new_channel;
                cout << "Вы присоединились к каналу: " << channel << endl;
            } else {
                cout << "Ошибка при присоединении: " << response << endl;
            }
            continue;
        }
        else if (cmd == "exit") {
            string request = "exit " + channel + " " + nick + "\n";
            send(sock, request.c_str(), request.size(), 0);
        }
        else {
            cout << "Неизвестная команда. Доступные: send, read, join, exit, quit." << endl;
            continue;
        }

        string response;
        if (!recvLine(sock, response)) {
            cout << "Отключено от сервера." << endl;
            break;
        }

        if (response.rfind("OK", 0) == 0) {
            if (cmd == "read") {
                istringstream rs(response);
                string ok;
                int count;
                rs >> ok >> count;
                cout << "Последние " << count << " сообщений в канале '" << channel << "':" << endl;
                for (int i = 0; i < count; i++) {
                    if (!recvLine(sock, response)) break;
                    cout << response << endl;
                }
            } else {
                cout << "OK" << endl;
            }
        }
        else if (response.rfind("ERROR", 0) == 0) {
            cout << response << endl;
        }
        else {
            cout << "Неожиданный ответ: " << response << endl;
        }
    }
}
