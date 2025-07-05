#pragma once
#include <string>

using namespace std;

int connectToServer(const string &ip, int port, int timeout_sec);
bool recvLine(int sock, string &out);
