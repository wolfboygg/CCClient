#include <iostream>

#include "CStreamClient.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    CC_NetConnectInfo info;
    memset(&info, 0, sizeof(CC_NetConnectInfo));

    char *server_ip = (char *) "172.16.238.53";
    strncpy(info.server_ip, server_ip, sizeof(info.server_ip));
    info.port = LISTEN_PORT;

    CStreamClient cStreamClient;
    cStreamClient.StartClient(&info);
    return 0;
}
