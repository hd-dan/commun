#include "serialcom.h"


serialCom::serialCom():buffSize_(6000),fsetup_(false),fstopRcv_(false),
                        fnewData_(false),delimiter_(','){

}
serialCom::serialCom(std::string usbPort):
                        usbPort_(usbPort),buffSize_(6000),fsetup_(false),
                        fstopRcv_(false),fnewData_(false), delimiter_(','){
    serialCom::setupUsb(usbPort);
}

serialCom::~serialCom(){
    serialCom::closeUsb();
}

void serialCom::setupUsb(std::string usbPort){
    if (fsetup_)
        return;

    usbPort_= usbPort;
    usbfd_= open(usbPort_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (usbfd_<0){
        printf("Failed to open usbPort: %s\n",usbPort_.c_str());
    }
    threadRcvData_= boost::thread(&serialCom::rcvData,this);
    fsetup_=true;
    return;
}

void serialCom::closeUsb(){
    fsetup_= false;
    fstopRcv_= true;
    threadRcvData_.interrupt();
    threadRcvData_.join();
//    if (!threadRcvData_.timed_join(boost::posix_time::time_duration(0,0,1,0))){
//      threadRcvData_.interrupt();
//      threadRcvData_.timed_join(boost::posix_time::time_duration(0,0,1,0));
//    }
    close(usbfd_);
}

std::vector<double> serialCom::processRcvStr(){
    if (rcvStrBuff_.size()==0)
        return rcvVect_;

    std::vector<std::string> parseStrVect;
    std::vector<double> parseVect;
    std::stringstream parseBuff(rcvStrBuff_);
    while(parseBuff.good()){
        std::string parseStr;
        std::getline(parseBuff,parseStr, delimiter_);
        parseStrVect.push_back(parseStr);
        parseVect.push_back(std::atof(parseStr.c_str()));
    }
    rcvStrBuff_= "";

    if (parseVect.size()>0){
        std::lock_guard<std::mutex> rcvLock(mtxRcv_);
        rcvStr_= parseStrVect;
        rcvVect_= parseVect;
    }
    fnewData_= true;
    return rcvVect_;
}

void serialCom::rcvData(){
    while (!fstopRcv_){
        char buff[buffSize_];
        int n= read(usbfd_,buff,buffSize_);
        for (int i=0;i<n;i++){
//            printf("%c",buff[i]);
            if (buff[i]!='\n'){
                rcvStrBuff_+=buff[i];
            }else{
                serialCom::processRcvStr();
            }
        }
        boost::this_thread::interruption_point();
    }
}

bool serialCom::checkNewData(){
    return fnewData_;
}

std::vector<double> serialCom::getData(){
    std::lock_guard<std::mutex> rcvLock(mtxRcv_);
    fnewData_=false;
    return rcvVect_;
}

std::vector<std::string> serialCom::getRcvStr(){
    std::lock_guard<std::mutex> rcvLock(mtxRcv_);
    fnewData_=false;
    return rcvStr_;
}


std::string serialCom::processSendVect(std::vector<double> sendData){
    std::string sendStrBuff;
    for (unsigned int i=0;i<sendData.size();i++){
        sendStrBuff+= std::to_string(sendData.at(i)) + delimiter_;
    }
    sendStrBuff+= "\n";
    return sendStrBuff;
}

int serialCom::sendData(std::vector<double> sendData){
    std::string sendStr= serialCom::processSendVect(sendData);
    const char* sendStrBuff= sendStr.c_str();
    int n= write(usbfd_,sendStrBuff,strlen(sendStrBuff));
    if (n<0)
        printf("Error Writing to Socket\n");
    return n;
}

void serialCom::setDelimiter(char delimiter){
    delimiter_= delimiter;
    return;
}
