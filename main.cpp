#include <iostream>
#include <chrono>

#include "tcpcom.h"
#include "serialcom.h"

//#include "../util/joystick.h"

#include <gattlib.h>


void runClient(){
    std::string ip("127.0.0.1");
//    std::string ip("192.168.0.99");
    tcpcom client(ip,8888,false,true);

    std::chrono::high_resolution_clock::time_point t0=
                std::chrono::high_resolution_clock::now();
    double t=0;
    int ii=0;
    while(t<60){
        t=std::chrono::duration_cast<std::chrono::duration<double> >(
                    std::chrono::high_resolution_clock::now()-t0).count();

        if (client.checkNewData()){
            std::vector<double> data= client.getData();
            printf("t: %.3f  | ",t);
            for (unsigned int i=0;i<data.size();i++){
                printf("%.3f, ",data.at(i));
            }
            printf("\n");
        }

        client.sendData(std::vector<double> (2,ii));
        ii++; ii=ii>255?0:ii;
        usleep(1e4);
    }
}


void runServer(){
    std::string ip("127.0.0.1");
//    std::string ip("192.168.0.99");
    tcpcom server(ip,8888,true,true);

    std::vector<double> dummy= {1,2.5,-2.2,1.0002};

    std::chrono::high_resolution_clock::time_point t0=
                    std::chrono::high_resolution_clock::now();
    double t=0;
    while(t<60){
        t=std::chrono::duration_cast<std::chrono::duration<double> >(
                    std::chrono::high_resolution_clock::now()-t0).count();

        if (server.checkNewData()){
            std::vector<double> data= server.getData();
            printf("t: %.3f  | ",t);
            for (unsigned int i=0;i<data.size();i++){
                printf("%.3f, ",data.at(i));
            }
            printf("\n");
        }
        server.sendData(dummy);

        usleep(1e4);
    }

}

void runSerialRcv(){
//    serialCom serial("/dev/ttyACM0");
    serialCom serial("/dev/tty.usbmodem14101");

    serial.setDelimiter(' ');

    double t=0;
    std::vector<double> rcvData;
    std::vector<std::string> rcvStr;
    while(t<10){

        if (serial.checkNewData()){
//            rcvData= serial.getData();
//            for(unsigned int i=0;i<rcvData.size();i++){
//                printf("%.3f,",rcvData.at(i));
//            }
            rcvStr= serial.getRcvStr();
            for(unsigned int i=0;i<rcvStr.size();i++){
                printf("%s,",rcvStr.at(i).c_str());
            }
            printf("\n");
        }

        usleep(1e3);
        t+=1e-3;
    }
}


void testBle(){
    std::string btmac= "34:B1:F7:D4:5D:D4";

    std::string cmdClose= "0x32DA";
    std::string cmdOpen= "0x31DA";

    gatt_connection_t* bluetooth= gattlib_connect(NULL,btmac.c_str(),
                                        GATTLIB_CONNECTION_OPTIONS_LEGACY_DEFAULT);

    if (bluetooth!=NULL){
        std::string cmd= cmdClose;
        cmd= cmdOpen;
        long int data= strtol(cmd.c_str(),NULL,16);
        int ret= gattlib_write_char_by_handle(bluetooth,37,&data,sizeof(data));
    }
    gattlib_disconnect(bluetooth);

}

int main(int argc, char * argv[]){
    std::cout << "Hello World!" << std::endl;

    runServer();
//    runClient();

//    runSerialRcv();

//    testBle();

    return 0;
}
