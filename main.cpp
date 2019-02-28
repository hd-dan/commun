#include <iostream>

#include "commun.h"
#include <chrono>

void runClient(){
    std::string ip("127.0.0.1");
    int port= 8888;
    commun pypp(ip,port,false);

    pypp.rcvDataThread();

    std::vector<double> data;
    std::chrono::high_resolution_clock::time_point t0,t1;
    t0= std::chrono::high_resolution_clock::now();
    double t=0;
    while(t<5){
        t1= std::chrono::high_resolution_clock::now();
        t=std::chrono::duration_cast<std::chrono::duration<double> >(t1-t0).count();

        if (pypp.checkNewData()){
            data= pypp.getData();
            printf("t: %.3f  | ",t);
            for (unsigned int i=0;i<data.size();i++){
                printf("%.3f, ",data.at(i));
            }
            printf("\n");
        }
        usleep(1e4);
    }
    pypp.stopRcv();
}


void runServer(){
    std::string ip("127.0.0.1");
    int port= 8888;
    commun pypp(ip,port,true);
    pypp.waitForClient();

    std::vector<double> dummy= {1,2.5,-2.2,1.0002};

    pypp.sendData(dummy);
    pypp.closeSock();
}

int main(){
    std::cout << "Hello World!" << std::endl;

//    runClient();
    runServer();

    return 0;
}
