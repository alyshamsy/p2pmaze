/*#include <SDL_net.h>
#include <SDL_thread.h>
#include <iostream>
#include <unistd.h>
#include "network_interface.h"
#include "message_handler.h"
*/

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <algorithm>
#include <SDL_net.h>
#include <SDL_thread.h>
#include "network_interface.h"
#include "message_handler.h"

static void init_peer ();
static void peer_listen ();
static void peer_send ();


int peer_socket, superpeer_socket;
char * superpeer_address;
int superpeer_port, peer_port;

int main(int argc, char * argv[])
{
  if (argc < 2)
  {
    perror ("Need to specify: peer address_of_peer port\n");
    exit(1);
  }
  superpeer_address = argv[1];
  superpeer_port    = atoi (argv[2]);
  peer_port = superpeer_port;

  init_peer ();

  peer_send ();
  peer_listen ();

  SDLNet_Init();
  network_interface network(6886);
  message_handler handler;
  network.set_message_handler(&handler);
  network.begin();
  while(true)
  {
    SDL_Delay(200);
    network.push_updates();
    handler.update(network);
  }
  SDLNet_Quit();
  return 0;
}

static void
init_peer()
{
  int sin_size = 0;
  sockaddr_in peer_address, superpeer_address;

  peer_socket = socket(PF_INET, SOCK_STREAM, 0);
  if ( peer_socket < 0 )
  {
    perror ("Failed to create peer socket\n");
    exit(1);
  }
  
  peer_address.sin_family = AF_INET;
  peer_address.sin_port = htons (peer_port);
  peer_address.sin_addr.s_addr = INADDR_ANY;
  memset(peer_address.sin_zero, '\0', sizeof peer_address.sin_zero);
  if(bind(peer_socket, (sockaddr*)&peer_address, sizeof(peer_address)) < 0)
  {
    perror("Could not bind the peer socket\n");
    exit(1);
  }

  if(listen(peer_socket, 10) < 0)
  {
    perror("Can't listen on the peer socket\n");
    exit(1);
  }

  // get connection from superpeer
  sin_size = sizeof (superpeer_address);
  superpeer_socket = accept(peer_socket, (struct sockaddr *)&superpeer_address, (socklen_t*)&sin_size);

  // get info of how to connect to superpeer
}

static void
peer_listen ()
{
}

static void
peer_send ()
{
  // send data to super peer
  //
  char * msg = "blah";
  int len, bytes_sent;

  char * type = "1";
  msg = "1:10x10:25x25:0,1,2";
  len = strlen (msg);
  bytes_sent = send (peer_socket, msg, len, 0);
}


