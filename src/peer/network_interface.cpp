#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_net.h>
#include <iostream>
#include "network_interface.h"
#include "message_handler.h"

using namespace std;

network_interface::network_interface(int listen_port):handler(NULL), 
						      port(listen_port)
{
  
}

network_interface::~network_interface() {}

void network_interface::set_message_handler(message_handler* handler)
{
  this->handler = handler;
}

void network_interface::begin()
{
  s = SDLNet_UDP_Open(port);
  SDL_CreateThread(&network_interface::socket_function, this);
}

void network_interface::send(const message& m, player_id destination)
{
  UDPpacket* packet = SDLNet_AllocPacket(m.message_length);
  packet->address = destination;
  packet->len = m.message_length;
  memcpy(packet->data, &m, m.message_length);
  if(m.type == MESSAGE_SC_REGULAR_UPDATE)
    {
      memcpy(packet->data +  sizeof(message_type), m.data.update.data, 
	     m.message_length - sizeof(message_type));
	     }
 
  SDLNet_UDP_Send(s, -1, packet);
  SDLNet_FreePacket(packet);
}

int network_interface::socket_function(void* data)
{
  network_interface* interface = static_cast<network_interface*>(data);
  UDPpacket* packet = SDLNet_AllocPacket(1024);
  int packets_received = 0;
  while(true)
    {
      packets_received = SDLNet_UDP_Recv(interface->s, packet);
      if(packets_received == -1)
	return 0;
      else if(packets_received > 0)
	{
	  message m = *reinterpret_cast<message*>(packet->data);
	  switch(m.type)
	    {
	    case MESSAGE_CS_JOIN:
	      interface->handler->do_join(*interface, m, packet->address);
	      break;
	    case MESSAGE_CS_MOVE_UP:
	      cout << "Move Up" << endl;
	      break;
	    case MESSAGE_CS_MOVE_DOWN:
	      cout << "Move down" << endl;
	      break;
	    case MESSAGE_CS_MOVE_LEFT:
	      cout << "Move left" << endl;
	      break;
	    case MESSAGE_CS_MOVE_RIGHT:
	      cout << "Move right" << endl;
	    case MESSAGE_CS_USE:
	      cout << "Use" << endl;
	      break;
	    case MESSAGE_CS_ATTACK:
	      cout << "attack" << endl;
	      break;
	    default:
	      cout << "Received " << packet->len << endl;
	      break;
	    }
	  //cout << "Received " << packet->len << " bytes" << endl;
	}
    }
}