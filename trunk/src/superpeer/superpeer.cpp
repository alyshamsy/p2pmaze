#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <algorithm>


static void start_peer();
static void start_superpeer();

int main()
{
  pid_t child_id = fork();
  if(child_id == 0)
    {
      start_peer();
    }
  else if(child_id > 0)
    {
      start_superpeer();
    }
  else
    {
      std::perror("Failed to fork peer process");
      exit(1);
    }
  return 0;
}

static void start_peer()
{
  int ret = execl("peer", "peer", "localhost" "6886", NULL);
  if(ret < 0)
    {
      std::perror("Could not execl peer process");
      exit(1);
    }
}


static void superpeer_accept(int server_socket);
static void start_superpeer()
{
  int server_socket = socket(PF_INET, SOCK_STREAM, 0);
  if(server_socket < 0)
    {
      perror("Failed to create server socket");
      exit(1);
    }
  
  sockaddr_in socket_address;
  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(6886);
  socket_address.sin_addr.s_addr = INADDR_ANY;
  if(bind(server_socket, (sockaddr*)&socket_address, sizeof(socket_address)) < 0)
    {
      perror("Could not bind the server socket");
      exit(1);
    }

  if(listen(server_socket, 10) < 0)
    {
      perror("Can't listen on the server socket");
      exit(1);
    }

  superpeer_accept(server_socket);
  
}

struct select_read_ready_p
{
  fd_set* set;
  select_read_ready_p(fd_set* set)
  {
    this->set = set;
  }

  bool operator()(int socket)
  {
    return FD_ISSET(socket, set);
  }
};

typedef std::vector<int> socket_list;

static void superpeer_accept(int server_socket)
{
  int max_socket = server_socket;
  fd_set read_set, read_ready_set;
  FD_ZERO(&read_set);
  FD_SET(server_socket, &read_set);
  socket_list sockets;
  sockets.push_back(server_socket);
  while(1)
    {
      read_ready_set = read_set;
      if(select(max_socket + 1, &read_ready_set, NULL, NULL, NULL) < 0)
	{
	  perror("select call failed");
	  exit(1);
	}
      if(FD_ISSET(server_socket, &read_ready_set))
	{
	  int client_socket = accept(server_socket, NULL, NULL);
	  if(client_socket < 0)
	    {
	      perror("accept failed");
	      exit(1);
	    }
	  max_socket = std::max(max_socket, client_socket);
	  sockets.push_back(client_socket);
	  FD_SET(client_socket, &read_set);
	  //printf("Someone connected: %d\n", client_socket);
	  //printf("now have %d sockets\n", sockets.size());
	  continue;
	}

      socket_list::iterator i = find_if(sockets.begin(), sockets.end(), 
					select_read_ready_p(&read_ready_set));
      if(i != sockets.end())
	{
	  char buffer[1024];
	  int length = recv(*i, buffer, sizeof(buffer), 0);
	  if(length < 0)
	    {
	      perror("error reading socket");
	      FD_CLR(*i, &read_set);
	      close(*i);
	      sockets.erase(i);
	      max_socket = *max_element(sockets.begin(), sockets.end());
	    }
	  else if(length == 0)
	    {
	      //printf("%d disconnected\n", *i);
	      FD_CLR(*i, &read_set);
	      close(*i);
	      sockets.erase(i);
	      max_socket = *max_element(sockets.begin(), sockets.end());
	      //printf("%d sockets remain\n", sockets.size());
	    }
	  else
	    {
	      //printf("someone sent %d bytes\n", length);
	    }
	}
    }
  
}


