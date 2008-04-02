import wx
import wx.html
import monitor_server
import threading
from Queue import *
from StringIO import *

ID_POLL_TIMER=100
HTML_COLOURS = ['aqua', 'blue', 'fuchsia', 'gray', 'green', 'lime', 'maroon', 'olive', 'purple', \
                'red', 'silver', 'teal', 'white','yellow']

class ServerView(wx.html.HtmlWindow):
    def __init__(self, parent, client_name):
        wx.html.HtmlWindow.__init__(self, parent)
        self.client_name = client_name

    def update(self, new_data):
        out = StringIO()
       
        out.write('<html><body><table>')
        for y in range(new_data.height):
            out.write('<tr>')
            for x in range(new_data.width):
                out.write('<td bgcolor=%s >(%d, %d)</td>' \
                              %(HTML_COLOURS[new_data.region_info[x+new_data.width*y][0]], x,y))
                
            out.write('</tr>')
        out.write('</table></body></html>')
        self.SetPage(out.getvalue())
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
