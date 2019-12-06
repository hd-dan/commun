#include "serialcom.h"


serialCom::serialCom():baudrate_(9600),buffSize_(6000),fsetup_(false),fstopRcv_(false),
                        fnewData_(false),fsendInt_(false),delimiter_(','){

}
serialCom::serialCom(std::string usbPort, int baudrate):
                        usbPort_(usbPort),baudrate_(baudrate),buffSize_(6000),
                        fsetup_(false),fstopRcv_(false),fnewData_(false),
                        fsendInt_(false),delimiter_(','){
    serialCom::setupUsb(usbPort, baudrate);
}

serialCom::~serialCom(){
    serialCom::closeUsb();
}

void serialCom::setupUsb(std::string usbPort, int baudrate){
    if (fsetup_)
        return;

    baudrate_= baudrate;
    usbPort_= usbPort;
    usbfd_= open(usbPort_.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (usbfd_<0){
        printf("Failed to open usbPort: %s\n",usbPort_.c_str());
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    if(tcgetattr(usbfd_, &tty)!=0)
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));

    tty.c_cflag &= ~PARENB; // Disables the Parity Enable bit(PARENB),So No Parity
    tty.c_cflag &= ~CSTOPB; // CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit
    tty.c_cflag &= ~CSIZE;	 // Clears the mask for setting the data size
    tty.c_cflag |= CS8; // Set the data bits = 8

    tty.c_cflag &= ~CRTSCTS; //No Hardware flow Control
    tty.c_cflag |= CREAD|CLOCAL; //Enable receiver,Ignore Modem Control lines
    tty.c_iflag &= ~(IXON|IXOFF|IXANY); // Disable XON/XOFF flow control both i/p o/p
    tty.c_iflag &= ~(ICANON|ECHO|ECHOE|ISIG);  // Non Cannonical mode
    tty.c_oflag &= ~OPOST; //No Output Processing


    if (baudrate_==115200){
        cfsetispeed(&tty, B115200);
        cfsetospeed(&tty, B115200);
        baudrate_= 115200;
    }else{
        cfsetispeed(&tty, B9600);
        cfsetospeed(&tty, B9600);
        baudrate_= 9600;
    }

    if (tcsetattr(usbfd_, TCSANOW, &tty)!=0)
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));

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

    if (rcvStrBuff_.back()==',')
        rcvStrBuff_.pop_back();

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
        if (n>0) printf("=> %s\n",buff);
        for (int i=0;i<n;i++){
            if (buff[i]!='\n'){
                if (buff[i]!='\r') rcvStrBuff_+=buff[i];
//                printf("%c",buff[i]);
            }else{
                serialCom::processRcvStr();
            }
        }
        boost::this_thread::interruption_point();
        usleep(1e-5);
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
        if (fsendInt_)
            sendStrBuff+= std::to_string(int(sendData.at(i))) + delimiter_;
        else
            sendStrBuff+= std::to_string(sendData.at(i)) + delimiter_;
    }
    sendStrBuff+= "\n";
    return sendStrBuff;
}

int serialCom::sendData(std::vector<double> sendData){
    std::string sendStr= serialCom::processSendVect(sendData);
    const char* sendStrBuff= sendStr.c_str();
    int n= write(usbfd_,sendStrBuff,strlen(sendStrBuff));
//    unsigned char returnBytes[2]={0x0D,0x0A};
//    write(usbfd_,&returnBytes,2);
//    printf("Send: %s\n",sendStrBuff);
    if (n<0)
        printf("Error Writing to Socket\n");
    usleep(1e-4);
    return n;
}

void serialCom::setDelimiter(char delimiter){
    delimiter_= delimiter;
    return;
}

void serialCom::setSendInt(bool fsendInt){
    fsendInt_= fsendInt;
}
