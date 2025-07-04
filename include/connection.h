#pragma once
#include <string>
using namespace std;
int connectToServer(const string &ip, int port);
bool recvLine(int sock, string &out);
