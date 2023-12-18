//
// Created by 郭磊 on 2023/12/12.
//

#ifndef CSTREAMSERVER_CSTREAMSERVERHEADERDEF_H
#define CSTREAMSERVER_CSTREAMSERVERHEADERDEF_H

#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <strings.h>
#include <pthread.h>
#include <csignal>

#define LISTEN_PORT 30000

#define NET_MESSAGE_TYPE_HEART_BEAT 10001

#pragma pack(push, 1)

typedef struct netConnectInfo {
    char server_ip[16];
    int port;
} CC_NetConnectInfo;

typedef struct netMsgHeader {
    char header[4];
    int type;
    int contentLength;
} CC_NetMsgHeader;

#pragma pack(pop)

#endif //CSTREAMSERVER_CSTREAMSERVERHEADERDEF_H
