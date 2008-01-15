#ifndef __LOAD_BALANCER_H__
#define __LOAD_BALANCER_H__

#include <vector>

enum message_type
  {
    TAKE_CONTROL
  };

struct take_control_message
{
  int length;
  message_type type;
  int seed;
  int width, height; // width and height are measured in regions
  int region_width, region_height; // size of a region
  // following this is the regions that a peer would control  
};

class load_balancer
{
 public:
  typedef std::vector<int> peer_list;
  load_balancer(int world_width, int world_height, int region_width, int region_height);
  ~load_balancer();
  void peer_connected(int peer_socket);
  void peer_disconnected(int peer_socket);
 private:
  void balance();
  void send_all(int peer, char* message, int length);
  int seed;
  int** world_regions;
  int world_width, world_height, region_width, region_height;

  peer_list known_peers, running_peers;
};
#endif
