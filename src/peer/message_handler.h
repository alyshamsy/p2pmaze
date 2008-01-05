#ifndef __MESSAGE_HANDLER__
#define __MESSAGE_HANDLER__

#include <SDL_net.h>
#include "map.h"


class network_interface;
struct message;
typedef IPaddress  player_id;

class message_handler
{
 public:
  message_handler();
  ~message_handler();

  void do_join(network_interface&, const message&, player_id);
  void do_move_right(network_interface&, player_id);
  void do_move_left(network_interface&, player_id);
  void do_move_up(network_interface&, player_id);
  void do_move_down(network_interface&, player_id);
  void update(network_interface&);
  
 private:
  message_handler(const message_handler&);
  message_handler& operator= (const message_handler&);
  map game_map;
};
#endif
