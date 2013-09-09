import socket
import sys

class DtnStats():
    fsock = None
    sock = None
    host = 'localhost'
    port = 4550

    def __init__(self, host, port):
        self.host = host
        self.port = port

    def readValueList(self):
        ret = {}
        data = self.fsock.readline()
        while (len(data) > 1):
            pair = data.strip().split(": ")
            ret[pair[0]] = pair[1]
            data = self.fsock.readline()
            
        return ret

    def info(self):
        self.sock.send("stats info\n")
        self.fsock.readline()
        return self.readValueList()

    def bundles(self):
        self.sock.send("stats bundles\n")
        self.fsock.readline()
        return self.readValueList()

    def convergencelayers(self):
        self.sock.send("stats convergencelayers\n")
        self.fsock.readline()
        return self.readValueList()

    def connect(self):
        ''' create a socket '''
        try:
            self.sock = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        except socket.error, msg:
            sys.stderr.write("[ERROR] %s\n" % msg)
            sys.exit(1)

        ''' connect to the daemon '''
        try:
            self.sock.connect((self.host, self.port))
            self.fsock = self.sock.makefile()
        except socket.error, msg:
            sys.stderr.write("[ERROR] %s\n" % msg)
            sys.exit(1)

        ''' read header '''
        self.fsock.readline()

        ''' switch into management protocol mode '''
        self.sock.send("protocol management\n")

        ''' read protocol switch '''
        self.fsock.readline()

    def close(self):
        self.sock.close()
        self.fsock.close()

