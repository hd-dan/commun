#ifndef TCPCOM_H
#define TCPCOM_H

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

#ifdef __APPLE__
#define SEND_NOSIGNAL 0
#else
#define SEND_NOSIGNAL MSG_NOSIGNAL
#endif

class tcpcom{
private:

    std::string ip_;
    int port_;
    const int buffSize_;

    int serverfd_;
    int clientfd_;
    struct sockaddr_in server_;
    struct sockaddr_in client_;
    socklen_t clientLen_;

    bool fsetup_;
    bool fisServer_;
    bool fhvClient_;
    bool fstopRcv_;
    bool fnewData_;
    bool fstopClientBind_;

    bool fthread_;
    bool fhvServer_;
    bool fstopServerWait_;
    bool fstayAlive_;

    std::string rcvStrBuff_;
    std::vector<double> rcvVect_;

    boost::thread threadRcvData_;
    boost::thread threadMonitorTimeout_;
    boost::thread threadWaitClientBind_;
    boost::thread threadWaitServer_;

    std::vector<double> processRcvStr();
    std::string processSendVect(std::vector<double> sendData);

    void waitClientLoop();
    void waitServerConnection();

    bool fstopTimeout_;
    double timeout_;
    void monitorRcvTimeout();
    std::chrono::high_resolution_clock::time_point dataRcvTime_;

    bool fstopTestConnect_;
    void testConnectionLoop();
    boost::thread threadTestConnect_;

    long checkSent(long n);

public:
    tcpcom();
    ~tcpcom();
    tcpcom(std::string ip, int port, bool isServer, bool fthread=0);
    void setup(std::string ip, int port, bool isServer, bool fthread=0);
    void setup();
    void closeSock();

    int waitForClient();

    void rcvData();
    void rcvDataThread(double timeout);
    std::vector<double> getData();

    long sendData(std::vector<double> data);
    bool testConnection();
    bool checkConnection();

    void stopRcv();
    bool checkNewData();

};

#endif // TCPCOM_H
