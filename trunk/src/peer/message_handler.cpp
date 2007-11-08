#include "message_handler.h"
#include "network_interface.h"


message_handler::message_handler()
{}

message_handler::~message_handler()
{

}

void message_handler::do_join(network_interface& out, const message& m, 
			      player_id source)
{
  message response;
  ok_join_data map_data;
  response.type = MESSAGE_SC_OK_JOIN;
  map_data.map_width = 25;
  map_data.map_height = 25;
  map_data.player_x = 3;
  map_data.player_y = 3;
  strcpy(map_data.player_name, "Sucker");
  response.data.initial_map_data = map_data;
  response.message_length = sizeof(map_data) + sizeof(message_type);
  out.send(response, source);
}
