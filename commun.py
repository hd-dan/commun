
import numpy as np
import socket
import threading
import time

import sys

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

        self.threadRcv= threading.Thread(target=self.rcvData)
        self.threadWaitClient= threading.Thread(target=self._waitForConnectionLoop)
        
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
            # self.waitForConnection()

        else:
            print("Unknown Mode.", mode)

    def __del__(self):
        self.closeSock()

    def waitForConnection(self):
        if not self.fisServer:
            print("Not server mode")
        print("Waiting for connection..")
        try:
            self.sock.settimeout(5)
            (self.connectedSock, self.connectedSockAddr)= self.sock.accept()

            self.fhvClient = True
            self.fstopRcv = False
            self.rcvDataThread()

        except socket.timeout:
            pass


        
        return

    def _waitForConnectionLoop(self):
        while (not self.fstopClientBind):
            if (not self.fhvClient):
                self.waitForConnection()
            time.sleep(0.1)
        return

    def waitForConnectionThread(self):
        self.threadWaitClient.start()
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
        try:
            n= self.connectedSock.send(vectStr)
        except:
            n=-1

        if (self.fisServer and n<0):
            print("Client closed connection..")
            self.fhvClient= False
            self.fstopRcv= True

            return -1

        if (n<0):
            print("Server closed connection..")
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
            rcvStr= self.connectedSock.recv(self.BUFFERSIZE)
            for i in range(0,len(rcvStr)):
                if rcvStr[i]!='\n':
                    self.rcvStrBuff+=rcvStr[i]
                else:
                    self._str2vector()

            time.sleep(0.001)

        return self.rcvVect

    def rcvDataThread(self):
        self.threadRcv= threading.Thread(target=self.rcvData)
        self.threadRcv.start()
        return

    def getData(self):
        self.fnewData= False
        return self.rcvVect

    def checkNewData(self):
        return self.fnewData

    def closeSock(self):
        self.fstopRcv= True
        self.fstopClientBind= True

        self.sock.close()
        return


def testServer():
    comm= commun("127.0.0.1",8885,"server")

    t=0
    while(t<30):
        comm.sendData(np.arange(0,5) + t*0.5)

        if (comm.checkNewData()):
            rcvData= comm.getData()
            print(t, ": ", rcvData)

        time.sleep(0.5)
        t+=0.5

    comm.closeSock()
    return

def testClient():
    comm= commun("127.0.0.1",8885, "client")

    t=0
    while(t<5):
        comm.sendData(np.array([t,t*0.5]))

        if (comm.checkNewData()):
            rcvData= comm.getData()
            print(t, ": ", rcvData)

        time.sleep(0.5)
        t+=0.5

    comm.closeSock()
    return

if __name__ == '__main__':
    if sys.argv[1]=="server":
        testServer()
    elif sys.argv[1]=="client":
        testClient()

