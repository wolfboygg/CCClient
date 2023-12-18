//
// Created by 郭磊 on 2023/12/14.
//

#include "CStreamClient.h"
#include <iostream>

using namespace std;

static bool m_bThreadRunning;

CStreamClient::CStreamClient() {
    m_bThreadRunning = true;
    pthread_mutex_init(&m_recvMute, NULL);
    pthread_mutex_init(&m_sendMute, NULL);
    signal(SIGINT, clientExitSignalProcess);
}


void CStreamClient::StartClient(CC_NetConnectInfo *info) {
    // 建立与服务端的连接
    printf("client ip:%s, port:%d\n", info->server_ip, info->port);

    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sockfd < 0) {
        printf("socket error\n");
        return;
    }

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(info->port);
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);

    if (inet_pton(AF_INET, info->server_ip, &client_addr.sin_addr.s_addr) < 0) {
        printf("inet_pton error\n");
        return;
    }
    // 非阻塞模式
    int flags = fcntl(m_sockfd, F_GETFL, 0);
    fcntl(m_sockfd, F_SETFL, flags | O_NONBLOCK);

    // 这里使用的是非阻塞模式，会立刻返回-1，我们需要根据select读取socket的状态进行判断是否连接成功然后在处理
    int ret = connect(m_sockfd, (struct sockaddr *) &client_addr, sizeof(struct sockaddr));

    printf("connect success\n");
    // 开启
    detach_thread_create(NULL, (void *) StartRunClientProcess, this);

    // 这里可以通过设置不停的发送心跳包进行处理
    keepAliveHeartBeat();

}

void CStreamClient::StartRunClientProcess(void *args) {
    CStreamClient *client = static_cast<CStreamClient *>(args);
    if (client != NULL) {
        printf("CStream CommandConnectProcess\n");
        client->CommandConnectProcess();
    }
}

void CStreamClient::CommandConnectProcess() {
    // 通过设置读写模式来对套接字文件进行监听获取对应的状态
    int error = 0;
    fd_set r_set, w_set;
    // 清0操作
    FD_ZERO(&r_set);
    FD_ZERO(&w_set);
    // 设置
    FD_SET(m_sockfd, &r_set);
    FD_SET(m_sockfd, &w_set);

    struct timeval timeout = {10, 0};
    int retValue = select(m_sockfd + 1, &r_set, &w_set, NULL, &timeout);
    switch (retValue) {
        case -1: {
            printf("select 系统调用出错\n");
            return;
        }
            break;
        case 0: {
            printf("select 超时\n");
            return;
        }
            break;
        default: {
            if (FD_ISSET(m_sockfd, &r_set) && FD_ISSET(m_sockfd, &w_set)) {
                // 如果当前套接字可读可写 需要进一步判断
                socklen_t len = sizeof(error);
                if (getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) {
                    printf("get sock opt is error\n");
                    return;
                }
                printf("error = %d\n", error);
                if (error != 0) {
                    printf("connect failed\n");
                    return;
                }
                // 为0表示连接成功
                printf("connect success111\n");
                isConnected = true;
            }
            if (!FD_ISSET(m_sockfd, &r_set) && FD_ISSET(m_sockfd, &w_set)) {
                // 可写但不可读
                printf("connect success2222\n");
                isConnected = true;
            }
        }
            break;
    }
}

bool CStreamClient::recvSocketData(uint8_t *buff, unsigned int len) {
    signal(SIGPIPE, SIG_IGN);

    pthread_mutex_lock(&m_recvMute);

    // 将从socket中读取到的数据写到buffer中
    int recvLen = 0;
    int nRet = 0;
    while (recvLen < len) {
        nRet = recv(m_sockfd, buff, len - recvLen, 0);
        if (nRet < 0) {
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
                printf("recv data mRet:%d\n", nRet);
                usleep(10 * 1000);
                continue;
            }
        }
        if (-1 == nRet || 0 == nRet) {
            pthread_mutex_unlock(&m_recvMute);
            printf("recv data error\n");
            isConnected = false;
            return false;
        }
        recvLen += nRet;
        buff += nRet;
    }

    pthread_mutex_unlock(&m_recvMute);
    return true;
}

bool CStreamClient::sendSocketData(uint8_t *buff, unsigned int len) {
    signal(SIGPIPE, SIG_IGN);
    pthread_mutex_lock(&m_sendMute);
    // 将buffer中的数据发送到socket
    int sendLen = 0;
    int nRet = 0;
    while (sendLen < len) {
        nRet = send(m_sockfd, buff, len - sendLen, 0);

        if (nRet < 0) {
            if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
                printf("send data mRet:%d\n", nRet);
                usleep(10 * 1000);
                continue;
            }
        }
        if (-1 == nRet || 0 == nRet) {
            pthread_mutex_unlock(&m_sendMute);
            printf("send data error\n");
            return false;
        }

        sendLen += nRet;
        buff += nRet;
    }
    printf("send data is success, data length %d\n", sendLen);
    pthread_mutex_unlock(&m_sendMute);
    return true;
}

void CStreamClient::clientExitSignalProcess(int num) {
    m_bThreadRunning = false;
}

void CStreamClient::CloseSocketConnection() {
    printf("CloseSocketConnection\n");
    isConnected = false;
    m_bThreadRunning = false;
    close(m_sockfd);
}

void CStreamClient::keepAliveHeartBeat() {
    while (m_bThreadRunning) {
        if (!isConnected) {
            printf("current socket not connected\n");
            usleep(2000 * 1000);
            continue;
        }
        printf("send keep alive heart beat pack\n");
        CC_NetMsgHeader msgHeader;
        memset(&msgHeader, 0, sizeof(CC_NetMsgHeader));
        strncpy(msgHeader.header, "CCTC", sizeof(msgHeader.header));
        printf("msg header headerId:%s\n", msgHeader.header);
        msgHeader.type = NET_MESSAGE_TYPE_HEART_BEAT;
        msgHeader.contentLength = 0;
        printf("msg header:%s\n", (char *)&msgHeader);
        sendSocketData((uint8_t *) &msgHeader, sizeof(CC_NetMsgHeader));
        usleep(2000 * 1000);
    }
}

CStreamClient::~CStreamClient() {
    CloseSocketConnection();
    pthread_mutex_destroy(&m_recvMute);
    pthread_mutex_destroy(&m_sendMute);
}
