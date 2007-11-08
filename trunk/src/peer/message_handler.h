#ifndef __MESSAGE_HANDLER__
#define __MESSAGE_HANDLER__

#include <SDL_net.h>


class network_interface;
struct message;
typedef IPaddress  player_id;

class message_handler
{
 public:
  message_handler();
  ~message_handler();

  void do_join(network_interface&, const message&, player_id);
  
 private:
  message_handler(const message_handler&);
  message_handler& operator= (const message_handler&);
};
#endif
