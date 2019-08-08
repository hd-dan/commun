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
    std::string rcvStr_;
    std::vector<double> rcvVect_;

    boost::thread threadRcvData_;

    char delimiter_;
    std::vector<double> processRcvStr();
    std::string processSendVect(std::vector<double> sendData);

public:
    serialCom(std::string usbPort);
    ~serialCom();

    void rcvData();
    bool checkNewData();
    std::vector<double> getData();
    std::string getRcvStr();

    int sendData(std::vector<double> data);
    void setDelimiter(char delimiter);
};

#endif // SERIALCOM_H
