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
#define NET_MESSAGE_TYPE_AVSTREAM 10002

#define NET_CONTENT_VIDEO 50001
#define NET_CONTENT_AUDIO 50002

#pragma pack(push, 1)

typedef struct netConnectInfo {
    char server_ip[16];
    int port;
} CC_NetConnectInfo;

typedef struct netMsgHeader {
    char header[4];
    int type;
    int subType;
    int contentLength;
} CC_NetMsgHeader;


typedef struct netAVStream {
    uint8_t *buffer;
    uint32_t size;
    uint16_t type;
} CC_AVStream;

#pragma pack(pop)

#endif //CSTREAMSERVER_CSTREAMSERVERHEADERDEF_H
