#ifndef __MAP__
#define __MAP__

#include "network_interface.h"
#include <map>

#define CELL_NONE	((char)0)
#define CELL_EMPTY	((char)1)
#define CELL_OBJECT	((char)2)
#define CELL_PLAYER	((char)3)

// entity is any non-static object on the map
// there is only one type for now, the other player
struct map_entity
{
  char type;
  player_id player;  // id of the player assosiated with this entity
};

struct cell
{
  char occupied; // true if there is a wall in this cell
  char cell_type;
  map_entity* entity; // NULL if there is no entity in the cell
};



enum player_direction
  {
    DOWN,
    RIGHT,
    UP,
    LEFT
  };

struct player_state
{
  player_id id;
  int x, y;
  player_direction direction;
};

class Serializator;

class map
{
 public:
  map();
  void add_player(player_id);
  void move_right(player_id);
  void move_left(player_id);
  void move_up(player_id);
  void move_down(player_id);

  void write_player_update(Serializator&, player_id);
  typedef std::map<player_id, player_state> player_list;
  const player_list& get_players() const;
  
 private:
  void place_player(player_id, player_state, int newx, int newy);
  cell cells[25][25];
  player_list players;
};



#endif

