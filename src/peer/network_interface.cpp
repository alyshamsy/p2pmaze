#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_net.h>
#include <iostream>
#include "network_interface.h"
#include "message_handler.h"
#include <cassert>

using namespace std;

network_interface::network_interface(int listen_port):handler(NULL), 
						      port(listen_port)
{
  received_queue_lock = SDL_CreateMutex();
}

void network_interface::enqueue_message(const message& m, player_id player)
{
  SDL_mutexP(received_queue_lock);
  received_messages.push(std::make_pair(player, m));
  SDL_mutexV(received_queue_lock);
}


void network_interface::push_updates()
{
  assert(handler != NULL);
  SDL_mutexP(received_queue_lock);
  while(!received_messages.empty())
    {
      std::pair<player_id, message> m = received_messages.front();
      received_messages.pop();
       switch(m.second.type)
	    {
	    case MESSAGE_CS_JOIN:
	      handler->do_join(*this, m.second, m.first);
	      break;
	    case MESSAGE_CS_MOVE_UP:
	      handler->do_move_up(*this, m.first);
	      break;
	    case MESSAGE_CS_MOVE_DOWN:
	      handler->do_move_down(*this, m.first);
	    
	      break;
	    case MESSAGE_CS_MOVE_LEFT:
	      handler->do_move_left(*this, m.first);
	      break;
	    case MESSAGE_CS_MOVE_RIGHT:
	      handler->do_move_right(*this, m.first);
	      break;
	    case MESSAGE_CS_USE:
	      cout << "Use" << endl;
	      break;
	    case MESSAGE_CS_ATTACK:
	      cout << "attack" << endl;
	      break;
	    default:
	      //cout << "Received " << packet->len << endl;
	      break;
	    }
	    //cout << "Received " << packet->len << " bytes" << endl;
      
    }
  SDL_mutexV(received_queue_lock);
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
	  m.message_length = packet->len;
	  /*switch(m.type)
	    {
	    case MESSAGE_CS_JOIN:
	      interface->handler->do_join(*interface, m, packet->address);
	      break;
	    case MESSAGE_CS_MOVE_UP:
	      interface->handler->do_move_up(*interface, packet->address);
	     
	      break;
	    case MESSAGE_CS_MOVE_DOWN:
	      interface->handler->do_move_down(*interface, packet->address);
	    
	      break;
	    case MESSAGE_CS_MOVE_LEFT:
	      interface->handler->do_move_left(*interface, packet->address);
	      break;
	    case MESSAGE_CS_MOVE_RIGHT:
	      interface->handler->do_move_right(*interface, packet->address);
	      break;
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
	    //cout << "Received " << packet->len << " bytes" << endl;*/
	  interface->enqueue_message(m, packet->address);
	}
    }
}
