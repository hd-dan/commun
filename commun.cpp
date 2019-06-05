#include "commun.h"

commun::commun(std::string ip, int port, bool isServer):
    ip_(ip),port_(port),buffSize_(6000),
    fisServer_(isServer),fhvClient_(false),fstopRcv_(false),fnewData_(false),
    fstopClientBind_(false){

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
            printf("connection failed: %s\n",ip_.c_str()); exit(1);
        }
        commun::rcvDataThread();
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

        commun::waitClientBindThread();
    }
}

commun::~commun(){
    commun::closeSock();
}

void commun::closeSock(){
    fstopRcv_= true;
    fstopClientBind_= true;

    threadRcvData_.interrupt();
    threadRcvData_.join();
    threadMonitorTimeout_.interrupt();
    threadMonitorTimeout_.join();
    threadWaitClientBind_.interrupt();
    threadWaitClientBind_.join();

    close(clientfd_);
    close(serverfd_);
}


int commun::waitForClient(){
    if (!fisServer_){
        printf("Not Server\n");
        return -1;
    }
    printf("Waiting for Client..\n");
    clientfd_= accept(serverfd_, (struct sockaddr *)&client_, &clientLen_);
    if (clientfd_<0){
        printf("Error on accept\n"); return -1;
    }
    fhvClient_=true;
    commun::rcvDataThread();

    return clientfd_;
}

void commun::waitClientLoop(){
    while(!fstopClientBind_){
        if (!fhvClient_)
            commun::waitForClient();
        usleep(1e5);
        boost::this_thread::interruption_point();
    }
}

void commun::waitClientBindThread(){
    threadWaitClientBind_= boost::thread(&commun::waitClientLoop,this);
}

std::vector<double> commun::processRcvStr(){
    std::vector<double> parseVect;
    std::string parseStr= "";

    unsigned int i=0;
    for (;i<rcvStrBuff_.size();i++){
        if (rcvStrBuff_.at(i)==','){
            try{
                parseVect.push_back( std::stod(parseStr) );
            }catch(...){}
            parseStr= "";
        }else if (rcvStrBuff_.at(i)=='\n'){
            i++;
            break;
        }else{
            parseStr+=rcvStrBuff_.at(i);
        }
    }
    if (rcvStrBuff_.size()>=i){
        rcvStrBuff_= rcvStrBuff_.substr(i,rcvStrBuff_.size());
    }else{
        rcvStrBuff_="";
    }
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
        boost::this_thread::interruption_point();
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
        t=std::chrono::duration_cast<std::chrono::duration<double> >
                (timeCheck-dataRcvTime_).count();
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
    if (fisServer_ && !fhvClient_){
        return -1;
    }
    std::string sendStr= commun::processSendVect(sendData);
    const char* sendStrBuff= sendStr.c_str();
    int n= send(clientfd_,sendStrBuff,strlen(sendStrBuff),MSG_NOSIGNAL);
    if (fisServer_ && n<0){
        printf("Client Closed Connection\n");
        fhvClient_=false;
        threadRcvData_.interrupt();
        rcvStrBuff_="";
        return -1;
    }
    if (n<0){
        printf("Server Closed Connection\n");
        exit(1);
    }
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

bool commun::testConnection(){
    if (!fhvClient_){
        return 0;
    }
    const char* checkStr= "0";
    int n= send(clientfd_,checkStr,strlen(checkStr),MSG_NOSIGNAL);
    if (fisServer_ && n<0){
        printf("Client Closed Connection\n");
        fhvClient_=false;
        threadRcvData_.interrupt();
        rcvStrBuff_="";
        return 0;
    }
    if (n<0){
        printf("Server Closed Connection\n");
        exit(1);
    }
    return 1;
}

bool commun::checkConnection(){
    return fhvClient_;
}
