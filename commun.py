
import numpy as np
import socket

import time

class commun:
    def __init__(self,ip,port,mode):
        self.TCP_ADDR= (ip,port)
        self.BUFFERSIZE= 6000

        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.fserver= False

        self.rcvStrBuff= ""
        self.rcvVect= np.array([])

        if mode == 'server':
            self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.sock.bind(self.TCP_ADDR)
            self.sock.listen(1)

            self.connectedSock= None
            self.connectedSockAddr= None
            self.fserver=True

        elif mode == 'client':
            self.sock.connect(self.TCP_ADDR)
        else:
            print("Unknown Mode.", mode)

    def waitForConnection(self):
        (self.connectedSock, self.connectedSockAddr)= self.sock.accept()
        return

    def vector2Str(self,vector):
        vectStr= b""
        for i in range(0,vector.shape[0]):
            vectStr= vectStr + str(vector[i]).encode() + b","

        vectStr= vectStr + b"\n"
        return vectStr



    def transmit(self, vector):
        vectStr= self.vector2Str(vector)
        # print(vectStr)
        if (self.fserver):
            self.connectedSock.send(vectStr)
        else:
            self.sock.send(vectStr)

    def str2vector(self):
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
        return

    def rcvData(self):
        rcvStr= self.sock.recv(self.BUFFERSIZE)
        for i in range(0,len(rcvStr)):
            if rcvStr[i]!='\n':
                self.rcvStrBuff+=rcvStr[i]
            else:
                self.str2vector()

        return self.rcvVect



    def closeSock(self):
        self.sock.close()
        return

def testServer():
    comm= commun(8888,"server")
    comm.waitForConnection()

    testData= np.arange(0,5).astype(np.double)*1.002
    t=0
    while(t<6):
        print("Sending ",t)
        comm.transmit(testData + t*0.5)
        time.sleep(t+0.1)
        t+=1
    return

def testClient():
    comm = commun(8888, "client")

    t = 0
    while (t < 2):
        print("Receiving ", t)
        rcvVect = comm.rcvData()
        print(rcvVect)
        t += 1

if __name__ == '__main__':
    # testServer()
    testClient()

