import socket
import select

class SuperPeerData:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.region_info = [0] * (width*height)
        self.num_servers = 0

class UpdateServer:
    def __init__(self, port, monitor):
        self.monitor = monitor
        self.ssocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.ssocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.ssocket.bind(('', port))
        self.ssocket.listen(5)

        self.descriptors = [self.ssocket]
        self.superpeer_data = {}
        print 'Update server started on port %d' % port

    def run(self):
        while 1:
            (sread, swrite, sexc) = select.select(self.descriptors, [], [])
            for s in sread:
                if s == self.ssocket:
                    self.accept_connection()
                else:
                    str = s.recv(4096)
                    host, port = s.getpeername()
                    if str == '':
                        print '%s:%d disconnected' %(host, port)
                        s.close
                        self.descriptors.remove(s)
                        del self.superpeer_data[s]
                        action = self.monitor.generate_disconnect_action('%s:%d' %(host, port))
                        self.monitor.queue.put(action)
                    else:
                        self.handle(s, str, '%s:%d' % (host, port))

    def handle(self, s, str, client_name):
        for line in str.splitlines():
            if line =='begin': continue
            if line == 'end': 
                action = self.monitor.generate_update_action(client_name, self.superpeer_data[s])
                self.monitor.queue.put(action)
                continue
            
            words = line.split()
            if words[0] == 'servers': 
                self.superpeer_data[s].num_servers = int(words[1])
                continue

            if words[0] == 'dimensions':
                self.superpeer_data[s] = SuperPeerData(int(words[1]), int(words[2]))
            else:
                self.superpeer_data[s].region_info[int(words[0])] = [int(x) for x in words[1:]]
            
        

    def accept_connection(self):
        s, (host, port) = self.ssocket.accept()
        self.descriptors.append(s)

        print '%s:%d connected' %(host, port)
        self.monitor.queue.put(self.monitor.generate_connected_action('%s:%d' %(host, port)))

#server = UpdateServer(50007)
#server.run()
