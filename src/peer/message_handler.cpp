#include "message_handler.h"
#include "network_interface.h"
#include "utils/Serializator.h"
#include <cstdlib>
#include <ctime>

using namespace std;

#define CELL_NONE	((char)0)
#define CELL_EMPTY	((char)1)
#define CELL_OBJECT	((char)2)
#define CELL_PLAYER	((char)3)

struct cell
{
  char occupied;
  char cell_type;
}

  static cells[25][25];
player_id player;
bool player_joined = false;


message_handler::message_handler()
{
  srand(time(0));
  for(int x = 0; x < 25; x++)
    for(int y = 0; y<25; y++)
      {
	cells[x][y].cell_type = CELL_EMPTY;
	cells[x][y].occupied  = (rand() % 10) == 9 ? 1 : 0;
      }
}

message_handler::~message_handler()
{

}

void message_handler::do_join(network_interface& out, const message& m, 
			      player_id source)
{
  message response;
  ok_join_data map_data;
  player_joined = true;
  player = source;
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

void message_handler::update(network_interface& out)
{
  if(!player_joined) return;
  Serializator s;
  s << 3;
  s << 3;
  s << 0;
  s << 0;
  s << 25;
  s << 25;
  s << 50; s << 50; s << 0;
  for(int x = 0; x < 25; x++)
    for(int y = 0; y<25; y++)
      {
	s << cells[x][y].occupied;
	s << cells[x][y].cell_type;
      }

  message m;
  update_data update;
  m.type = MESSAGE_SC_REGULAR_UPDATE;
  update.data = s.getBuffer();
  m.data.update = update;
  m.message_length = sizeof(message_type) + s.getSize();
  out.send(m, player);
}
