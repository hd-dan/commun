#ifndef SERIALCOM_H
#define SERIALCOM_H

#include <fcntl.h>
#include <termios.h>
#include <string>
#include <vector>
#include <boost/thread.hpp>
#include <mutex>

//#include <stdio.h>
//#include <unistd.h>
//#include <iostream>


class serialCom{
private:
    std::string usbPort_;
    int usbfd_;
    int baudrate_;
    const int buffSize_;

    bool fsetup_;
    bool fstopRcv_;
    bool fnewData_;

    std::string rcvStrBuff_;
    std::vector<std::string> rcvStr_;
    std::vector<double> rcvVect_;

    boost::thread threadRcvData_;

    char delimiter_;
    std::vector<double> processRcvStr();
    std::string processSendVect(std::vector<double> sendData);

    std::mutex mtxRcv_;
    bool fsendInt_;

public:
    serialCom();
    serialCom(std::string usbPort, int baudrate=9600);
    ~serialCom();
    void setupUsb(std::string usbPort, int baudrate=9600);
    void closeUsb();

    void rcvData();
    bool checkNewData();
    std::vector<double> getData();
    std::vector<std::string> getRcvStr();

    int sendData(std::vector<double> data);
    void setDelimiter(char delimiter);
    void setSendInt(bool fsendInt=true);
};

#endif // SERIALCOM_H
