
import numpy as np
import socket
import sys

# if sys.version_info[0]<3:
#     import thread
# else:
#     import _thread
from threading import Thread

import time

class commun:
    def __init__(self,ip,port,mode):
        self.TCP_ADDR= (ip,port)
        self.BUFFERSIZE= 6000

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.fisServer= False

        self.fstopRcv= False
        self.fnewData= False
        self.rcvStrBuff= ""
        self.rcvVect= np.array([])

        self.threadRcv= None
        self.threadWaitClient= None
        
        self.fhvClient= False
        self.fstopClientBind= False


        if mode == 'client':
            self.sock.connect(self.TCP_ADDR)
            self.connectedSock= self.sock

            self.rcvDataThread()

        elif mode == 'server':
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.sock.bind(self.TCP_ADDR)
            self.sock.listen(1)

            self.connectedSock= None
            self.connectedSockAddr= None
            self.fisServer=True

            self.waitForConnectionThread()

        else:
            print("Unknown Mode.", mode)

    def waitForConnection(self):
        if not self.fisServer:
            print("Not server mode\n")
        print("Waiting for connection..\n")
        (self.connectedSock, self.connectedSockAddr)= self.sock.accept()
        self.fhvClient= True

        self.rcvDataThread()
        
        return

    def _waitForConnectionLoop(self):
        while (not self.fstopClientBind):
            if (not self.fhvClient):
                self.waitForConnection()
            time.sleep(0.1)
        return

    def waitForConnectionThread(self):
        self.threadWaitClient= Thread(target=self._waitForConnectionLoop).start()
        return

    def _vector2Str(self,vector):
        vectStr= b""
        for i in range(0,vector.shape[0]):
            vectStr= vectStr + str(vector[i]).encode() + b","
        vectStr= vectStr + b"\n"
        return vectStr


    def sendData(self, vector):
        if (self.fisServer and not self.fhvClient):
            return -1

        vectStr= self._vector2Str(vector)
        n= self.connectedSock.send(vectStr)

        if (self.fisServer and n<0):
            print("Client closed connection..\n")
            self.fhvClient= False
            self.threadRcv.interrupt()
        #     rcvthread interrrupt
            return -1

        if (n<0):
            print("Server closed connection..\n")
            exit(1)
            return -1

        return n

    def _str2vector(self):
        parseStr=""
        parseList= []
        for i in range(0,len(self.rcvStrBuff)):
            if self.rcvStrBuff[i]!=',':
                parseStr+=self.rcvStrBuff[i]
            else:
                parseList.append(float(parseStr))
                parseStr= ""
        self.rcvStrBuff= ""
        self.rcvVect= np.array(parseList)
        self.fnewData= True
        return

    def rcvData(self):
        while (not self.fstopRcv):
            rcvStr= self.sock.recv(self.BUFFERSIZE)
            for i in range(0,len(rcvStr)):
                if rcvStr[i]!='\n':
                    self.rcvStrBuff+=rcvStr[i]
                else:
                    self._str2vector()
        return

    def rcvDataThread(self):
        # _thread.start_new_thread(self.rcvData,self)
        self.threadRcv= Thread(target=self.rcvData).start()
        return

    def getData(self):
        self.fnewData= False
        return self.rcvVect

    def checkNewData(self):
        return self.fnewData

    def closeSock(self):
        self.fstopRcv= True
        self.fstopClientBind= True

        self.threadRcv.interrupt()
        if (self.fisServer):
            self.threadWaitClient.interrupt()

        self.sock.close()
        return

def testServer():
    comm= commun("127.0.0.1",8881,"server")
    # comm.waitForConnection()

    testData= np.arange(0,5).astype(np.double)*1.002
    t=0
    while(t<6):
        print(t, ": Sending ",testData)
        comm.sendData(testData + t*0.5)
        time.sleep(t+0.1)
        t+=1
    return

def testClient():
    comm = commun("127.0.0.1",8881, "client")

    t = 0
    while (t < 5):
        print(t, ": Receiving ")
        if (comm.checkNewData()):
            rcvVect= comm.getData()
            print(rcvVect)
        t += 1
    return

if __name__ == '__main__':
    # testServer()
    testClient()

