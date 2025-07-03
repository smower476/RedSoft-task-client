#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

static inline string trim(const string &s) {
    auto wsfront = find_if_not(s.begin(), s.end(), [](int c){ return isspace(c); });
    auto wsback = find_if_not(s.rbegin(), s.rend(), [](int c){ return isspace(c); }).base();
    if (wsback <= wsfront) return "";
    return string(wsfront, wsback);
}

bool recvLine(int sock, string &out) {
    out.clear();
    char c;
    while (true) {
        ssize_t r = recv(sock, &c, 1, 0);
        if (r <= 0) return false;
        if (c == '\n') break;
        if (c != '\r') out.push_back(c);
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "Использование: client <server_ip> <port> <channel>" << endl;
        return 1;
    }
    string server_ip = argv[1];
    int port = stoi(argv[2]);
    string channel = argv[3];
    if (channel.size() > 24) {
        cerr << "Имя канала слишком длинное (максимум 24 символа)" << endl;
        return 1;
    }

    cout << "Введите ваш ник (max 24 символа): ";
    string nick;
    getline(cin, nick);
    nick = trim(nick);
    if (nick.empty() || nick.size() > 24) {
        cerr << "Неверный ник" << endl;
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "Неверный адрес сервера" << endl;
        return 1;
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return 1;
    }
    cout << "Подключен к серверу " << server_ip << ":" << port 
         << ", канал: " << channel << endl;

    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;
        if (line == "quit") {
            break;
        }
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
            string request = "join " + channel + " " + nick + "\n";
            send(sock, request.c_str(), request.size(), 0);
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
    close(sock);
    cout << "Клиент завершил работу." << endl;
    return 0;
}
