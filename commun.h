#ifndef COMMUN_H
#define COMMUN_H

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> //inet_pton
#include <unistd.h>

#include <string.h>
#include <vector>
#include <boost/thread.hpp>
#include <chrono>

class commun{
private:

    std::string ip_;
    int port_;
    const int buffSize_;

    int serverfd_;
    int clientfd_;
    struct sockaddr_in server_;
    struct sockaddr_in client_;
    socklen_t clientLen_;

    bool fisServer_;
    bool fhvClient_;
    bool fstopRcv_;
    bool fnewData_;
    bool fstopClientBind_;

    std::string rcvStrBuff_;
    std::vector<double> rcvVect_;

    boost::thread threadRcvData_;
    boost::thread threadMonitorTimeout_;
    boost::thread threadWaitClientBind_;
    void monitorRcvTimeout();

    std::vector<double> processRcvStr();
    std::string processSendVect(std::vector<double> sendData);

    void waitClientLoop();

    std::chrono::high_resolution_clock::time_point dataRcvTime_;
    double timeout_;

public:
    commun(std::string ip, int port, bool isServer);
    ~commun();
    void closeSock();

    int waitForClient();
    void waitClientBindThread();

    void rcvData();
    void rcvDataThread();
    void rcvDataThread(double timeout);
    std::vector<double> getData();

    int sendData(std::vector<double> data);

    void stopRcv();
    bool checkNewData();

};

#endif // COMMUN_H
