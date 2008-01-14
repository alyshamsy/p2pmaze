#include "load_balancer.h"

const int DIVISION_THREASHOLD = 4;


load_balancer::load_balancer(int world_width, int world_height, int region_width, int region_height)
{
  this->world_width = world_width;
  this->world_height = world_height;
  this->region_width = region_width;
  this->region_height = region_height;

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

}

void load_balancer::peer_disconnected(int peer_socket)
{

}



