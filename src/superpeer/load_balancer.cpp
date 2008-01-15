#include "load_balancer.h"
#include <algorithm>

using namespace std;

const size_t DIVISION_THREASHOLD = 4;


load_balancer::load_balancer(int world_width, int world_height, int region_width, int region_height)
{
  this->world_width = world_width;
  this->world_height = world_height;
  this->region_width = region_width;
  this->region_height = region_height;

  seed = rand();

  world_regions = new int*[world_width];
  for(int i = 0; i<world_width; i++)
    {
      world_regions[i] = new int[world_height];
    }
}

load_balancer::~load_balancer()
{
  for(int i = 0; i<world_width; i++)
    {
      delete[] world_regions[i];
    }
  delete[] world_regions;
}

void load_balancer::peer_connected(int peer_socket)
{
  known_peers.push_back(peer_socket);
  if(known_peers.size() <= DIVISION_THREASHOLD)
    {
      balance();
    }
}

void load_balancer::peer_disconnected(int peer_socket)
{
  remove(known_peers.begin(), known_peers.end(), peer_socket);
  if(find(running_peers.begin(), running_peers.end(), peer_socket) != running_peers.end())
    {
      balance();
    }
}

void load_balancer::balance()
{
  running_peers.clear();
  int peer_count = min(DIVISION_THREASHOLD, known_peers.size());
  int world_size = world_width * world_height;
  int regions_per_peer = world_size / peer_count;
  int remainder = world_size % peer_count;
  
  int regions_sent = 0;
  for(int i = 0; i < peer_count; i++)
    {
      take_control_message header;
      int regions_to_control = regions_per_peer + (i==peer_count - 1 ? remainder : 0);
      int peer = known_peers[i];
      header.length = sizeof(take_control_message) + regions_to_control * sizeof(int);
      header.type = TAKE_CONTROL;
      header.seed = seed;
      header.width = world_width;
      header.height= world_height;
      header.region_width = region_width;
      header.region_height = region_height;
      int* regions = new int[regions_to_control];
      for(int j = 0; j<regions_to_control; j++)
	{
	  regions[j] = regions_sent + j;
	}
      regions_sent += regions_to_control;
      char* buffer = new char[header.length];
      memcpy(buffer, &header, sizeof(take_control_message));
      memcpy(buffer + sizeof(take_control_message), regions, regions_to_control*sizeof(int));
      send_all(peer, buffer, header.length);
      running_peers.push_back(peer);
    }
}

void load_balancer::send_all(int peer, void* message, int length)
{

}



