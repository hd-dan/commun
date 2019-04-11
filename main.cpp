#include <iostream>

#include "commun.h"
#include <chrono>

#include "serialcom.h"

#include "../ur_robot/ur/joystick.h"
#include "../ur_robot/ur/util.hpp"


void runClient(){
//    std::string ip("127.0.0.1");
    std::string ip("192.168.0.99");
    int port= 8888;
    commun client(ip,port,false);

    std::vector<double> data;
    std::chrono::high_resolution_clock::time_point t0,t1;
    t0= std::chrono::high_resolution_clock::now();
    double t=0;
    int ii=0;
    while(t<10){
        t1= std::chrono::high_resolution_clock::now();
        t=std::chrono::duration_cast<std::chrono::duration<double> >(t1-t0).count();

        if (client.checkNewData()){
            data= client.getData();
            printf("t: %.3f  | ",t);
            for (unsigned int i=0;i<data.size();i++){
                printf("%.3f, ",data.at(i));
            }
            printf("\n");
        }
        client.sendData(std::vector<double>(1,ii));
        ii+=1;
        ii= (ii>255)?0:ii;

        usleep(1e4);
    }
    client.stopRcv();
}


void runServer(){
    std::string ip("127.0.0.1");
//    std::string ip("192.168.0.99");
    int port= 8888;
    commun server(ip,port,true);

    std::vector<double> dummy= {1,2.5,-2.2,1.0002};

    std::vector<double> data;
    std::chrono::high_resolution_clock::time_point t0,t1;
    t0= std::chrono::high_resolution_clock::now();
    double t=0;

    while(t<10){
        t1= std::chrono::high_resolution_clock::now();
        t=std::chrono::duration_cast<std::chrono::duration<double> >(t1-t0).count();

        if (server.checkNewData()){
            data= server.getData();
            printf("t: %.3f  | ",t);
            for (unsigned int i=0;i<data.size();i++){
                printf("%.3f, ",data.at(i));
            }
            printf("\n");
        }
        server.sendData(dummy);

        usleep(1e4);
    }
    server.stopRcv();
    server.closeSock();
}

void runSerialRcv(){
    serialCom serial("/dev/ttyACM0");

    double t=0;
    std::vector<double> rcvData;
    while(t<10){

        rcvData= serial.getData();
        for(unsigned int i=0;i<rcvData.size();i++){
            printf("%.3f,",rcvData.at(i));
        }
        if (rcvData.size()>0)
            printf("\n");
        usleep(1e3);
        t+=1e-3;
    }
}

void runSerialSend(){
    serialCom serial("/dev/ttyACM0");
    boost::thread tRcv(&serialCom::rcvDataThread,&serial);

    double t=0;
    std::vector<double> sendData= {1,0};
    serial.sendData(sendData);
    usleep(100e3);

    std::vector<double> rcvData;
    while(t<0.5){

        rcvData= serial.getData();
        for(unsigned int i=0;i<rcvData.size();i++){
            printf("%.3f,",rcvData.at(i));
        }
        usleep(1e3);
        t+=1e-3;
    }
}


int main(){
    std::cout << "Hello World!" << std::endl;

    runServer();
//    runClient();

    return 0;
}
