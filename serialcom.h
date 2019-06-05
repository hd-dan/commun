#ifndef SERIALCOM_H
#define SERIALCOM_H

#include <fcntl.h>
#include <string>
#include <vector>
#include <boost/thread.hpp>

//#include <stdio.h>
//#include <unistd.h>
//#include <iostream>


class serialCom{
private:
    std::string usbPort_;
    int usbfd_;
    const int buffSize_;

    bool fstopRcv_;
    bool fnewData_;

    std::string rcvStrBuff_;
    std::vector<double> rcvVect_;

    boost::thread threadRcvData_;

    std::vector<double> processRcvStr();
    std::string processSendVect(std::vector<double> sendData);

public:
    serialCom(std::string usbPort);
    ~serialCom();

    void rcvData();
    std::vector<double> getData();

    int sendData(std::vector<double> data);
};

#endif // SERIALCOM_H
