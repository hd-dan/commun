#include <iostream>
#include <chrono>

#include "commun.h"
#include "serialcom.h"

#include "../util/joystick.h"

void runClient(){
//    std::string ip("127.0.0.1");
//    std::string ip("192.168.0.99");
    std::string ip("10.42.0.1");
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

        client.sendData(std::vector<double> (2,ii));
        ii++;
        ii=ii>255?0:ii;
        usleep(1e4);
    }
    client.stopRcv();
}


void runServer(){
//    std::string ip("127.0.0.1");
    std::string ip("10.42.0.1");
//    std::string ip("192.168.0.99");
    int port= 8888;
    commun server(ip,port,true);

    std::vector<double> dummy= {1,2.5,-2.2,1.0002};

    std::vector<double> data;
    std::chrono::high_resolution_clock::time_point t0,t1;
    t0= std::chrono::high_resolution_clock::now();
    double t=0;

    while(t<30){
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
        server.checkConnection();

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

void runPi(){
//    std::string ip("10.42.0.80");
    std::string ip("192.168.1.10");
    commun pi(ip,8888,0);
    std::vector<double> rcv;

    joystick js(0);
    boost::thread js_loop(&joystick::loopReadJs,&js);
    js_state state;


    std::vector<double> cmd= {1,0}; //{ena, dir}
    std::vector<double> stopCmd(5,-99);
    do{
        state= js.getState();

        if (pi.checkNewData()){
            rcv= pi.getData();

            for (unsigned int i=0;i<rcv.size();i++){
                printf("%.3f, ",rcv.at(i));
            }
            printf("\n");
        }

        double enaCmd=-1, dirCmd=-1;
        if (state.button.at(0))
            enaCmd=1;
        else if (state.button.at(1))
            enaCmd=0;
        else
            enaCmd=-1;

        if (state.button.at(4))
            dirCmd=1;
        else if (state.button.at(5))
            dirCmd=0;
        else
            dirCmd=-1;

        printf("cmd: {%.1f, %.1f}\n",enaCmd, dirCmd);
        cmd= {enaCmd, dirCmd};

        pi.sendData(cmd);
        usleep(1e4);

    }while(state.button.at(6)!=1);
    pi.sendData(stopCmd);
    pi.closeSock();
}

int main(){
    std::cout << "Hello World!" << std::endl;

//    runServer();
//    runClient();

//    runPi();
    while(1){
        printf(".");
        usleep(5e6);
    }

    return 0;
}
