#include "../include/commands.h"
#include "../include/validation.h"
#include "../include/connection.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sstream>

using namespace std;

bool handleSend(int sock, const string &channel, const string &nick, istringstream &iss) {
    string msg;
    getline(iss, msg);
    msg = trim(msg);
    if (msg.empty()) {
        cout << "Использование: send <сообщение>" << endl;
        return true;
    }
    string request = "send " + channel + " " + nick + " " + msg + "\n";
    if (send(sock, request.c_str(), request.size(), 0) < 0) {
        perror("send");
        return false;
    }
    return true;
}

bool handleRead(int sock, const string &channel, const string &nick) {
    string request = "read " + channel + " " + nick + "\n";
    if (send(sock, request.c_str(), request.size(), 0) < 0) {
        perror("send");
        return false;
    }

    string response;
    if (!recvLine(sock, response)) {
        cout << "Отключено от сервера." << endl;
        return false;
    }

    if (response.rfind("OK", 0) == 0) {
        istringstream rs(response);
        string ok;
        int count;
        rs >> ok >> count;
        cout << "Последние " << count << " сообщений в канале '" << channel << "':" << endl;
        for (int i = 0; i < count; ++i) {
            if (!recvLine(sock, response)) break;
            cout << response << endl;
        }
    } else {
        cout << "Ошибка: " << response << endl;
    }

    return true;
}

bool handleJoin(int sock, string &channel, const string &nick, istringstream &iss) {
    string new_channel;
    iss >> new_channel;
    if (new_channel.empty()) {
        cout << "Использование: join <канал>" << endl;
        return true;
    }

    if (new_channel.size() > 24) {
        cout << "Имя канала слишком длинное (максимум 24 символа)" << endl;
        return true;
    }

    string request = "join " + new_channel + " " + nick + "\n";
    if (send(sock, request.c_str(), request.size(), 0) < 0) {
        perror("send");
        return false;
    }

    string response;
    if (!recvLine(sock, response)) {
        cout << "Отключено от сервера." << endl;
        return false;
    }

    if (response.rfind("OK", 0) == 0) {
        channel = new_channel;
        cout << "Вы присоединились к каналу: " << channel << endl;
    } else {
        cout << "Ошибка при присоединении: " << response << endl;
    }

    return true;
}

bool handleExit(int sock, const string &channel, const string &nick) {
    string request = "exit " + channel + " " + nick + "\n";
    return send(sock, request.c_str(), request.size(), 0) >= 0;
}

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

        bool ok = true;
        if (cmd == "send") {
            ok = handleSend(sock, channel, nick, iss);
        } else if (cmd == "read") {
            ok = handleRead(sock, channel, nick);
        } else if (cmd == "join") {
            ok = handleJoin(sock, channel, nick, iss);
        } else if (cmd == "exit") {
            ok = handleExit(sock, channel, nick);
        } else {
            cout << "Неизвестная команда. Доступные: send, read, join, exit, quit." << endl;
        }

        if (!ok) {
            cout << "Соединение прервано." << endl;
            break;
        }
    }
}

