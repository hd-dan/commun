#include "tcpcom.h"

tcpcom::tcpcom():buffSize_(6000),fsetup_(false),fhvClient_(false),fstopRcv_(false),
    fnewData_(false),fstopClientBind_(false),fhvServer_(0),
    fserverDc_(0),fstopServerWait_(0),fstayAlive_(1){

}

tcpcom::tcpcom(std::string ip, int port, bool isServer,bool fthread):
    ip_(ip),port_(port),buffSize_(6000), fsetup_(false),
    fisServer_(isServer),fhvClient_(false),fstopRcv_(false),fnewData_(false),
    fstopClientBind_(false), fthread_(fthread),fhvServer_(0),
    fserverDc_(0), fstopServerWait_(0),fstayAlive_(1){

    tcpcom::setup(ip, port,isServer,fthread);
}

tcpcom::~tcpcom(){
    tcpcom::closeSock();
}

void tcpcom::setup(){
    return tcpcom::setup(ip_,port_,fisServer_,fthread_);
}

void tcpcom::setup(std::string ip, int port, bool isServer,
                        bool fthread){
    if (fsetup_)
        return;

    fsetup_= true;
    ip_= ip;
    port_= port;
    fisServer_= isServer;
    fthread_= fthread;
    timeout_= 5;

    server_.sin_family= AF_INET;
    server_.sin_port= htons(static_cast<unsigned short>(port_));

    serverfd_= socket(AF_INET,SOCK_STREAM,0);
    if (serverfd_<0){
        printf("Cannot Open Socket\n"); exit(1);
    }

    if (!fisServer_){
        clientfd_= serverfd_;
        if(inet_pton(AF_INET, ip_.c_str(), &server_.sin_addr)<=0){
            printf("Invalid address\n"); exit(1);
        }

        if (!fthread_){
            if(connect(clientfd_,(struct sockaddr *)&server_,sizeof(server_))<0){
                printf("connection failed: %s\n",ip_.c_str());
                exit(1);
            }
            fhvServer_=1;
            threadRcvData_= boost::thread(&tcpcom::rcvData,this);
        }else{
            threadWaitServer_= boost::thread(&tcpcom::waitServerConnection,this);
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

        if(!fthread_){
            tcpcom::waitForClient();
        }
        threadWaitClientBind_= boost::thread(&tcpcom::waitClientLoop,this);
    }

    threadTestConnect_= boost::thread(&tcpcom::testConnectionLoop,this);
//    threadMonitorTimeout_= boost::thread(&tcpcom::monitorRcvTimeout,this);
}

void tcpcom::closeSock(){
    fsetup_= false;
    fstopRcv_= true;
    fstopClientBind_= true;
    fstopServerWait_= true;
    fstopTimeout_= true;

    threadRcvData_.interrupt();
    threadRcvData_.join();
    threadMonitorTimeout_.interrupt();
    threadMonitorTimeout_.join();
    threadWaitClientBind_.interrupt();
    pthread_cancel(threadWaitClientBind_.native_handle());
    threadWaitServer_.interrupt();
    threadWaitServer_.join();

    fstopTestConnect_= true;
    threadTestConnect_.interrupt();
    threadTestConnect_.join();

    close(clientfd_);
    close(serverfd_);
}


int tcpcom::waitForClient(){
    if (!fisServer_){
        printf("Not Server\n");
        return -1;
    }
    printf("Waiting for Client..\n");
    clientfd_= accept(serverfd_, (struct sockaddr *)&client_, &clientLen_);
    printf("Accepted\n");
    if (clientfd_<0){
        printf("Error on accept\n"); return -1;
    }
    fhvClient_=true;
    threadRcvData_= boost::thread(&tcpcom::rcvData,this);
//    threadMonitorTimeout_= boost::thread(&tcpcom::monitorRcvTimeout,this);

    return clientfd_;
}

void tcpcom::waitClientLoop(){
    while(!fstopClientBind_){
        if (!fhvClient_)
            tcpcom::waitForClient();
        usleep(1e5);
        boost::this_thread::interruption_point();
    }
}

void tcpcom::waitServerConnection(){
    printf("Waiting for Server..\n");

    clientfd_= socket(AF_INET,SOCK_STREAM,0);
    while(connect(clientfd_,(struct sockaddr *)&server_,sizeof(server_))<0
          && !fstopServerWait_){
        usleep(1e5);
        boost::this_thread::interruption_point();
    }
    fhvServer_=1;
    fserverDc_=0;
    threadRcvData_= boost::thread(&tcpcom::rcvData,this);
}


std::vector<double> tcpcom::processRcvStr(){
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

void tcpcom::rcvData(){
    while (!fstopRcv_){
        char buff[buffSize_];
        long n= read(clientfd_,buff,static_cast<unsigned long>(buffSize_));
        dataRcvTime_= std::chrono::high_resolution_clock::now();
        for (int i=0;i<n;i++){
            if (buff[i]!='\n'){
                rcvStrBuff_+=buff[i];
            }else{
                tcpcom::processRcvStr();
            }
        }
        boost::this_thread::interruption_point();
    }
}

void tcpcom::monitorRcvTimeout(){
    double t= 0;
    dataRcvTime_= std::chrono::high_resolution_clock::now();
    while(!fstopRcv_ && t<timeout_){
        if ((fisServer_ && !fhvClient_) || (!fisServer_ && !fhvServer_))
            dataRcvTime_= std::chrono::high_resolution_clock::now();

        t=std::chrono::duration_cast<std::chrono::duration<double> >
                (std::chrono::high_resolution_clock::now()-dataRcvTime_).count();


        usleep(1e5);
        boost::this_thread::interruption_point();
    }
    fstopRcv_=true;

    return;

}

std::string tcpcom::processSendVect(std::vector<double> sendData){
    std::string sendStrBuff;
    for (unsigned int i=0;i<sendData.size();i++){
        sendStrBuff+= std::to_string(sendData.at(i)) + ",";
    }
    sendStrBuff+= "\n";
    return sendStrBuff;
}

long tcpcom::sendData(std::vector<double> sendData){
    if (fisServer_ && !fhvClient_)
        return -1;
    if (!fisServer_ && !fhvServer_)
        return -1;

    std::string sendStr= tcpcom::processSendVect(sendData);
    const char* sendStrBuff= sendStr.c_str();
    long n= send(clientfd_,sendStrBuff,strlen(sendStrBuff),SEND_NOSIGNAL);
    return tcpcom::checkSent(n);
}

long tcpcom::checkSent(long n){
    if (fisServer_ && fhvClient_ && n<0){
        printf("Client Closed Connection\n");
        fhvClient_= false;
        threadRcvData_.interrupt();
        rcvStrBuff_="";
        return -1;
    }
    if (!fisServer_ && fhvServer_ && n<0){
        printf("Server Closed Connection\n");
        fhvServer_= false;
        fserverDc_= true;
        if (fstayAlive_)
            threadWaitServer_= boost::thread(&tcpcom::waitServerConnection,this);
        return -1;
    }
    return n;
}

std::vector<double> tcpcom::getData(){
    fnewData_=false;
    return rcvVect_;
}

void tcpcom::stopRcv(){
    fstopRcv_=true;
    return;
}

bool tcpcom::checkNewData(){
    return fnewData_;
}

bool tcpcom::testConnection(){
    if (fisServer_ && !fhvClient_)
        return 0;
    if(!fisServer_ && !fhvServer_)
        return 0;
    const char* checkStr= "0";
    long n= send(clientfd_,checkStr,strlen(checkStr),SEND_NOSIGNAL);
    return tcpcom::checkSent(n);
}

bool tcpcom::checkConnection(){
    if (fisServer_)
        return fhvClient_;
    else
        return fhvServer_;
}

bool tcpcom::checkServerDc(){
    return fserverDc_;
}

void tcpcom::testConnectionLoop(){
    fstopTestConnect_= false;
    while(!fstopTestConnect_){
        tcpcom::testConnection();

        usleep(1e4);
        boost::this_thread::interruption_point();
    }
}
