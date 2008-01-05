#include <iostream>
#include "map.h"
#include "utils/Serializator.h"
#include <cassert>

bool operator<(player_id left, player_id right)
{
  if(left.host == right.host)
    return left.port < right.port;
  else
    return left.host < right.host;
}


map::map() : players()
{
 srand(time(0));
  for(int x = 0; x<25; x++)
    for(int y = 0; y<25; y++)
      {
	cells[x][y].cell_type = CELL_EMPTY;
	cells[x][y].occupied  = (rand() % 10) == 9 ? 1 : 0;
	cells[x][y].entity = NULL;
      }
  cells[3][3].occupied = 0;
}


static int clamp_x_to_bounds(int x)
{
  if(x<0) return 0;
  if(x> 24) return 24;
  return x;
}

static int clamp_y_to_bounds(int y)
{
  if(y<0) return 0;
  if(y >24) return 24;
  return y;
}

void map::add_player(player_id player)
{
  player_state state;
  state.id = player;
  state.x = 3;
  state.y = 3;
  state.direction = RIGHT;
  cells[state.x][state.y].cell_type = CELL_PLAYER;
  map_entity* p = new map_entity;
  p->type = CELL_PLAYER;
  p->player = player;
  cells[state.x][state.y].entity = p;
  players.insert(std::make_pair(player, state));
}

void map::place_player(player_id id, player_state state, int newx, int newy)
{
  if(!cells[newx][newy].occupied && cells[newx][newy].cell_type != CELL_PLAYER)
    {
      delete cells[state.x][state.y].entity;
      cells[state.x][state.y].entity = NULL;
      cells[state.x][state.y].cell_type = CELL_EMPTY;
      cells[newx][newy].cell_type = CELL_PLAYER;
      map_entity* e = new map_entity;
      cells[newx][newy].entity = e;
      e->type = CELL_PLAYER;
      e->player = id;
      state.x = newx;
      state.y = newy;
      players[id] = state;
    }
}

void map::move_right(player_id id)
{
  player_state state = players[id];
  state.direction = RIGHT;
  int newx = clamp_x_to_bounds(state.x + 1);
  int newy = clamp_y_to_bounds(state.y);
  place_player(id, state, newx, newy);
}

void map::move_left(player_id id)
{
  player_state state = players[id];
  state.direction = LEFT;
  int newx = clamp_x_to_bounds(state.x-1);
  int newy = clamp_y_to_bounds(state.y);
  place_player(id, state, newx, newy);
}

void map::move_up(player_id id)
{
  player_state state = players[id];
  state.direction = UP;
  int newx = clamp_x_to_bounds(state.x);
  int newy = clamp_y_to_bounds(state.y-1);
  place_player(id, state, newx, newy);
}

void map::move_down(player_id id)
{
  player_state state = players[id];
  state.direction = DOWN;
  int newx = clamp_x_to_bounds(state.x);
  int newy = clamp_y_to_bounds(state.y+1);
  place_player(id, state, newx, newy);
}

const map::player_list& map::get_players() const
{
  return players;
}

void map::write_player_update(Serializator& s, player_id id)
{
  player_state player = players[id];
  s << player.x;
  s << player.y;
  int leftx = clamp_x_to_bounds(player.x - 7);
  int lefty = clamp_y_to_bounds(player.y - 7);
  int rightx = clamp_x_to_bounds(player.x + 8);
  int righty = clamp_y_to_bounds(player.y + 8);
  //std::cout << "Visible: ("<<leftx << "," << lefty << ") to (" << rightx 
  //    << "," <<righty << ")" << "pos (" << player.x << "," << player.y << ")" << std::endl;
  s << leftx;
  s<< lefty;
  s<<rightx;
  s<<righty;
  s << 50; s<<50;
  s << player.direction;
  for(int x = leftx; x < rightx; x++)
    for(int y = lefty; y < righty; y++)
      {
	s << cells[x][y].occupied;
	s << cells[x][y].cell_type;
	if(cells[x][y].cell_type == CELL_PLAYER)
	  {
	    assert(cells[x][y].entity != NULL);
	    s<< 50; // life
	    s<<50;  // attribute
	    s << players[cells[x][y].entity->player].direction;
	    s.putBytes(reinterpret_cast<char*>(&cells[x][y].entity->player), sizeof(player_id));
	  }
      }
}

// Serializator& operator<<(Serializator& s, const map& m)
// {
//   s << 3;
//   s << 3;
//   s << 0;
//   s << 0;
//   s << 11;
//   s << 11;
//   s << 50; s << 50; s << 1;
//   for(int x = 0; x < 11; x++)
//     for(int y = 0; y<11; y++)
//       {
// 	s << m.cells[x][y].occupied;
// 	s << m.cells[x][y].cell_type;
//       }
//   return s;
// }


