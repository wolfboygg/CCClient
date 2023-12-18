//
// Created by 郭磊 on 2023/12/14.
//

#ifndef CSTREAMCLIENT_CSTREAMCLIENT_H
#define CSTREAMCLIENT_CSTREAMCLIENT_H

#include "CStreamHeaderDef.h"
#include "CThread.h"


class CStreamClient {
public:
    CStreamClient();

    ~CStreamClient();

    void StartClient(CC_NetConnectInfo *info);

    void CommandConnectProcess();

    void CloseSocketConnection();

private:

    void keepAliveHeartBeat();

    bool recvSocketData(uint8_t *buff, unsigned int len);

    bool sendSocketData(uint8_t *buff, unsigned int len);


public:
    static void StartRunClientProcess(void *args);

    static void clientExitSignalProcess(int num);

private:
    int m_sockfd;
    bool isConnected = false;
    pthread_mutex_t  m_recvMute;
    pthread_mutex_t  m_sendMute;
};


#endif //CSTREAMCLIENT_CSTREAMCLIENT_H
