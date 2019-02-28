#include "commun.h"

commun::commun(std::string ip, int port, bool isServer):
    ip_(ip),port_(port),buffSize_(6000),
    fisServer_(isServer),fhvClient_(false),fstopRcv_(false),fnewData_(false){

    server_.sin_family= AF_INET;
    server_.sin_port= htons(port_);

    serverfd_= socket(AF_INET,SOCK_STREAM,0);
    if (serverfd_<0){
        printf("Cannot Open Socket\n"); exit(1);
    }

    if (!fisServer_){
        clientfd_= serverfd_;
        if(inet_pton(AF_INET, ip_.c_str(), &server_.sin_addr)<=0){
            printf("Invalid address\n"); exit(1);
        }
        if(connect(clientfd_,(struct sockaddr *)&server_,sizeof(server_))<0){
            printf("connection failed\n"); exit(1);
        }
    }

    if (fisServer_){
        int reuse=1;
        if (setsockopt(serverfd_,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int))==-1) {
            printf("Failed to set socket to reuse\n"); exit(1);
        }

        server_.sin_addr.s_addr= INADDR_ANY;
        if (bind(serverfd_, (struct sockaddr *)&server_, sizeof(server_))<0){
            printf("bind failed.\n"); exit(1);
        }

        listen(serverfd_,5);
        clientLen_= sizeof(client_);
    }

}

commun::~commun(){
    commun::closeSock();
}

void commun::closeSock(){
    fstopRcv_= true;
    threadRcvData_.join();
    if (!threadRcvData_.timed_join(boost::posix_time::time_duration(0,0,1,0))){
      threadRcvData_.interrupt();
      threadRcvData_.timed_join(boost::posix_time::time_duration(0,0,1,0));
    }
    threadMonitorTimeout_.interrupt();
    close(clientfd_);

    close(serverfd_);
}


int commun::waitForClient(){
    if (!fisServer_){
        printf("Not Server\n");
        return -1;
    }
    clientfd_= accept(serverfd_, (struct sockaddr *)&client_, &clientLen_);
    if (clientfd_<0){
        printf("Error on accept\n"); return -1;
    }
    fhvClient_=true;
    return 1;
}


std::vector<double> commun::processRcvStr(){
    std::vector<double> parseVect;
    std::string parseStr= "";

    unsigned int i=0;
    for (;i<rcvStrBuff_.size();i++){
        if (rcvStrBuff_.at(i)==','){
            parseVect.push_back( std::stod(parseStr) );
            parseStr= "";
        }else if (rcvStrBuff_.at(i)=='\n'){
            i++;
            break;
        }else{
            parseStr+=rcvStrBuff_.at(i);
        }
    }
    rcvStrBuff_= rcvStrBuff_.substr(i,rcvStrBuff_.size());
    rcvVect_= parseVect;
    fnewData_= true;
    return rcvVect_;
}

void commun::rcvData(){
    while (!fstopRcv_){
        char buff[buffSize_];
        int n= read(clientfd_,buff,buffSize_);
        dataRcvTime_= std::chrono::high_resolution_clock::now();
        for (int i=0;i<n;i++){
            if (buff[i]!='\n'){
                rcvStrBuff_+=buff[i];
            }else{
                commun::processRcvStr();
            }
        }
    }
}

void commun::rcvDataThread(){
    threadRcvData_= boost::thread(&commun::rcvData,this);
}

void commun::monitorRcvTimeout(){
    std::chrono::high_resolution_clock::time_point timeCheck;
    double t= 0;
    while(!fstopRcv_ && t<timeout_){
        timeCheck= std::chrono::high_resolution_clock::now();
        t=std::chrono::duration_cast<std::chrono::duration<double> >(timeCheck-dataRcvTime_).count();
        usleep(1e5);
    }
    fstopRcv_=true;
}

void commun::rcvDataThread(double timeout){
    timeout_= timeout;
    threadRcvData_= boost::thread(&commun::rcvData,this);
    threadMonitorTimeout_= boost::thread(&commun::monitorRcvTimeout,this);
}

std::string commun::processSendVect(std::vector<double> sendData){
    std::string sendStrBuff;
    for (unsigned int i=0;i<sendData.size();i++){
        sendStrBuff+= std::to_string(sendData.at(i)) + ",";
    }
    sendStrBuff+= "\n";
    return sendStrBuff;
}

int commun::sendData(std::vector<double> sendData){
    std::string sendStr= commun::processSendVect(sendData);
    const char* sendStrBuff= sendStr.c_str();
    int n= send(clientfd_,sendStrBuff,strlen(sendStrBuff),MSG_NOSIGNAL);
    if (n<0)
        printf("Error Writing to Socket\n");
    return n;
}

std::vector<double> commun::getData(){
    fnewData_=false;
    return rcvVect_;
}

void commun::stopRcv(){
    fstopRcv_=true;
    return;
}

bool commun::checkNewData(){
    return fnewData_;
}
