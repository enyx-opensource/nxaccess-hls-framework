import socketserver
from threading import Thread

def reset_all_sessions(manager):
    for i in range(0, manager.maxSessionCount()):
        manager.resetSession(i)

def close_all_sessions(manager):
    for i in range(0, manager.maxSessionCount()):
        manager.closeSession(i)

class ThreadedTCPServer(socketserver.ThreadingMixIn, socketserver.TCPServer):
    daemon_threads = True
    allow_reuse_address = True

class ThreadedMuteTCPRequestHandler(socketserver.BaseRequestHandler):
    def handle(self):
        while True:
            try:
                data = self.request.recv(4096)
                if len(data) == 0:
                    break
            except Exception:
                break

class MuteServerHelper:
    def __init__(self, serverIp, serverPort):
        self.server = ThreadedTCPServer((serverIp, serverPort), ThreadedMuteTCPRequestHandler)
        self.server_thread = Thread(target=self.server.serve_forever)
        self.server_thread.daemon = True
        self.server_thread.start()

    def stop(self):
        self.server.shutdown()
        self.server.server_close()
