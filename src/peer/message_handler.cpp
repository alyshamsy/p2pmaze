#include "message_handler.h"
#include "network_interface.h"
#include "utils/Serializator.h"
#include <cstdlib>
#include <ctime>

using namespace std;




message_handler::message_handler()
{
  
}

message_handler::~message_handler()
{

}

void message_handler::do_join(network_interface& out, const message& m, 
			      player_id source)
{
  message response;
  ok_join_data map_data;
  game_map.add_player(source);
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

void message_handler::do_move_right(network_interface&, player_id id)
{
  game_map.move_right(id);
}

void message_handler::do_move_left(network_interface&, player_id id)
{
  game_map.move_left(id);
}

void message_handler::do_move_up(network_interface&, player_id id)
{
  game_map.move_up(id);
}

void message_handler::do_move_down(network_interface&, player_id id)
{
  game_map.move_down(id);
}

void message_handler::update(network_interface& out)
{
  if(game_map.get_players().empty()) return;
  for(::map::player_list::const_iterator i = game_map.get_players().begin();
      i != game_map.get_players().end(); i++)
    {
      Serializator s; 
      message m;
      update_data update;
      m.type = MESSAGE_SC_REGULAR_UPDATE;
      game_map.write_player_update(s, i->first);
      update.data = s.getBuffer();
      m.data.update = update;
      m.message_length = sizeof(message_type) + s.getSize();
      out.send(m, i->first);
    }
}
