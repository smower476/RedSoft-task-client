#include "../include/connection.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>

using namespace std;

int connectToServer(const string &ip, int port, int timeout_sec) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl");
        close(sock);
        return -1;
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "Неверный адрес сервера" << endl;
        close(sock);
        return -1;
    }

    int res = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (res < 0 && errno != EINPROGRESS) {
        perror("connect");
        close(sock);
        return -1;
    }

    if (res != 0) {
        pollfd pfd{};
        pfd.fd = sock;
        pfd.events = POLLOUT;

        int poll_res = poll(&pfd, 1, timeout_sec * 1000);
        if (poll_res <= 0) {
            if (poll_res == 0) cerr << "connect timeout" << endl;
            else perror("poll");
            close(sock);
            return -1;
        }
    }

    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0) {
        cerr << "connect failed: " << strerror(err) << endl;
        close(sock);
        return -1;
    }

    if (fcntl(sock, F_SETFL, flags) < 0) {
        perror("fcntl restore");
        close(sock);
        return -1;
    }

    return sock;
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
