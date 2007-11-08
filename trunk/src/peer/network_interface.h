#ifndef __NETWORK_INTERFACE__
#define __NETWORK_INTERFACE__

#include <SDL_net.h>

enum message_type
    {
      /* no topic */
	MESSAGE_DEFAULT = 0,

	/* basic client messages */
	MESSAGE_CS_JOIN,
	MESSAGE_SC_OK_JOIN,			/* needs own class */
	MESSAGE_SC_NOK_JOIN,
	MESSAGE_CS_LEAVE,
	MESSAGE_SC_OK_LEAVE,

	/* migration messages */
	MESSAGE_SC_MIGRATE,
	MESSAGE_SC_OK_MIGRATE,
	MESSAGE_CS_ACK_OK_MIGRATE,

	/* quest messages */
	MESSAGE_SC_NEW_QUEST,			/* needs own class */
	MESSAGE_SC_QUEST_OVER,

	/* player actions */
	MESSAGE_CS_MOVE_UP,
	MESSAGE_CS_MOVE_DOWN,
	MESSAGE_CS_MOVE_LEFT,
	MESSAGE_CS_MOVE_RIGHT,
	MESSAGE_CS_USE,
	MESSAGE_CS_ATTACK,

	/* update game state messages */
	MESSAGE_SC_REGULAR_UPDATE,		/* needs own class */
	MESSAGE_SS_REQUEST_CLIENT_UPDATE,	/* needs own class */
	MESSAGE_SS_ANSWER_CLIENT_UPDATE		/* needs own class */
    };

struct ok_join_data
{
  int map_width;
  int map_height;
  int player_x;
  int player_y;
  char player_name[32];
};

struct empty_message {};

struct message
{
  message_type type;
  union message_data
  {
    empty_message nothing;
    ok_join_data initial_map_data;
  };
  message_data data;
  size_t message_length;
};

class message_handler;
typedef IPaddress player_id;

class network_interface
{
 public:
  network_interface(int listen_port);
  ~network_interface();
  void set_message_handler(message_handler*);
  void begin();
  void send(const message&, player_id destination);

 private:
  network_interface(const network_interface&);
  network_interface& operator=(const network_interface&);
  message_handler* handler;
  int port;
  UDPsocket s;
  static int socket_function(void*);
};

#endif
