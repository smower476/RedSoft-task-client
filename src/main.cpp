#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include "../include/validation.h"
#include "../include/connection.h"
#include "../include/commands.h"

using namespace std;

//TODO: safe send

bool parseArguments(int argc, char *argv[], string &server_ip, int &port, string &channel) {
    if (argc != 4) {
        cerr << "Использование: client <server_ip> <port> <channel>" << endl;
        return false;
    }

    server_ip = argv[1];
    port = stoi(argv[2]);
    channel = argv[3];

    if (channel.size() > 24) {
        cerr << "Имя канала слишком длинное (максимум 24 символа)" << endl;
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    string server_ip, channel;
    int port;

    if (!parseArguments(argc, argv, server_ip, port, channel)) return 1;

    string nick = inputNickname();
    int sock = connectToServer(server_ip, port);
    if (sock < 0) return 1;

    cout << "Подключен к серверу " << server_ip << ":" << port
         << ", начальный канал: " << channel << endl;

    commandLoop(sock, channel, nick);

    close(sock);
    cout << "Клиент завершил работу." << endl;
    return 0;
}
