import wx
import wx.html
import monitor_server
import threading
from Queue import *
from StringIO import *

ID_POLL_TIMER=100
HTML_COLOURS = ['aqua', 'blue', 'fuchsia', 'gray', 'green', 'lime', 'maroon', 'olive', 'purple', \
                'red', 'silver', 'teal', 'white','yellow']

class ServerView(wx.Notebook):
    def __init__(self, parent, client_name):
        wx.Notebook.__init__(self, parent)
        self.client_name = client_name
        self.servers = wx.html.HtmlWindow(self)
        self.load =  wx.html.HtmlWindow(self)
        self.AddPage(self.servers, 'Peers')
        self.AddPage(self.load, 'Load')

    def update(self, new_data):
        self.update_servers(new_data)
        self.update_load(new_data)

    def update_servers(self, new_data):
        out = StringIO()
       
        out.write('<html><body><font size="2"><table width="100%">')
        for y in range(new_data.height):
            out.write('<tr valign="center">')
            for x in range(new_data.width):
                out.write('<td bgcolor="%s" align="center">' % HTML_COLOURS[new_data.region_info[x+new_data.width*y][0]])
                out.write('(%d, %d)<br />' %( x,y))
                out.write('%d players' % new_data.region_info[x+new_data.width*y][1])
                out.write('</td>')
                
            out.write('</tr>')
        out.write('</table></font></body></html>')
        self.servers.SetPage(out.getvalue())
        out.close()

    def update_load(self, new_data):
        out = StringIO()
       
        out.write('<html><body><font size="2"><table width="100%">')
        players = reduce(lambda x,y: x+y, [info[1] for info in new_data.region_info])
        red_percentage = 0.0
        red = 0
        green = 255
        for y in range(new_data.height):
            out.write('<tr valign="center">')
            for x in range(new_data.width):
                current_region = new_data.region_info[x+new_data.width*y]
                if current_region[1] == 0:
                    red_percentage = 0
                    red = 0
                    green = 255
                else:
                    red_percentage = float(current_region[1])/players
                    red = int(red_percentage*255/2) + 128
                    green = 255-int(red_percentage*255)
                colour = '#%02x%02x00' % (red, green)
                out.write('<td bgcolor="%s" align="center">' % colour)
                out.write('(%d, %d)<br />' %( x,y))
                out.write('%d players' % current_region[1])
                out.write('</td>')
                
            out.write('</tr>')
        out.write('</table></font></body></html>')
        self.load.SetPage(out.getvalue())
        out.close()
        

class GameMonitorFrame(wx.Frame):
    
    def __init__(self):
        wx.Frame.__init__(self, None, wx.ID_ANY, 'Game Monitor')
        self.queue = Queue()
        self.SetSize((800, 600))
        self.create_controls()
        self.do_layout()
    
    def create_controls(self):
        self.tabs = wx.Notebook(self)
        self.timer = wx.Timer(self, ID_POLL_TIMER)
        wx.EVT_TIMER(self, ID_POLL_TIMER, self.on_timer)
        self.timer.Start(1000)

    def do_layout(self): pass

    def on_timer(self, event):
        while not self.queue.empty():
            action = self.queue.get()
            action()

    def generate_connected_action(self, client_name):
        def add_new_tab():
            htmlwin = ServerView(self.tabs, client_name)
            self.tabs.AddPage(htmlwin, client_name)
        return add_new_tab

    def generate_disconnect_action(self, client_name):
        def remove_client_tab():
            for i in range(0, self.tabs.GetPageCount()):
                page = self.tabs.GetPage(i)
                if page.client_name == client_name:
                    self.tabs.DeletePage(i)
                    return
        return remove_client_tab

    def generate_update_action(self, client_name, data):
        def update_tab():
             for i in range(0, self.tabs.GetPageCount()):
                page = self.tabs.GetPage(i)
                if page.client_name == client_name:
                    page.update(data)
                    return
        return update_tab
            
        

class ServerThread(threading.Thread):
    def __init__(self, monitor_window):
        threading.Thread.__init__(self)
        self.server = monitor_server.UpdateServer(50007, monitor_window)

    def run(self):
        self.server.run()

app = wx.App(0)
frame = GameMonitorFrame()
frame.Show()
thread = ServerThread(frame)
thread.start()
app.MainLoop()
