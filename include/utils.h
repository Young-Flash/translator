#pragma once

#include <linux/tcp.h>
#include <arpa/inet.h>
#include <iostream>

int is_socket_connected(int sock)
{
    if (sock <= 0) return 0;
    struct tcp_info info;
    int len = sizeof(info);
    getsockopt(sock, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
    if ((info.tcpi_state == 1)) {
        std::cout << "socket connected\n" << std::endl;
        return 1;
    }
    else {
        std::cout << "socket disconnected\n" << std::endl;
        return 0;
    }
}