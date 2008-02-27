
/***************************************************************************************************
*
* SUBJECT:
*    A Benckmark for Massive Multiplayer Online Games
*    Game Server and Client
*
* AUTHOR:
*    Mihai Paslariu
*    Politehnica University of Bucharest, Bucharest, Romania
*    mihplaesu@yahoo.com
*
* TIME AND PLACE:
*    University of Toronto, Toronto, Canada
*    March - August 2007
*
***************************************************************************************************/

#include "WorldUpdateModule.h"

/***************************************************************************************************
*
* Constructors and setup methods
*
***************************************************************************************************/

WorldUpdateModule::WorldUpdateModule(ServerData *sd)
{
  server_data = sd;
  in = out = NULL;

  min_life = server_data->player_min_life;
  max_life = server_data->player_max_life;
  min_attr = server_data->player_min_attr;
  max_attr = server_data->player_max_attr;
}

void WorldUpdateModule::setInQueue(MessageQueue *in)
{
  this->in = in;
}

void WorldUpdateModule::setOutQueue(MessageQueue *out)
{
  this->out = out;
}

void WorldUpdateModule::setMapManagModule(MapManagModule *m)
{
  this->mm_module = m;
}

/***************************************************************************************************
*
* Main loop
*
***************************************************************************************************/

void WorldUpdateModule::run()
{
  Message *m;
  Uint32 addr;
  bool delete_flag; /* received messages are deleted by default */
  Garbage *old_ptr = NULL;

  /* check queues */
  if ( in == NULL ) throw "Input message queue is NULL";
  if ( out == NULL ) throw "Output message queue is NULL";
  printf("WorldUpdateModule started\n");

  /* main loop */
  while ( true )
  {
    m = in->getMessage();
    addr = m->getIP();
    delete_flag = true;

    switch ( m->getType() )
    {
      case MESSAGE_CS_JOIN: handleClientJoinRequest(m); break;
      case MESSAGE_CS_LEAVE: handleClientLeaveRequest(m); break;
      case MESSAGE_CS_ACK_OK_MIGRATE: handle_ACK_OK_MIGRATE(m); break;

      case MESSAGE_SS_REQUEST_CLIENT_UPDATE:
        handle_REQUEST_CLIENT_UPDATE(m);
        break;
      case MESSAGE_SS_ANSWER_CLIENT_UPDATE:
        handle_ANSWER_CLIENT_UPDATE(m);
        delete_flag = false;
        break;

      case MESSAGE_CS_MOVE_UP: handle_MOVE_UP(m); break;
      case MESSAGE_CS_MOVE_DOWN: handle_MOVE_DOWN(m); break;
      case MESSAGE_CS_MOVE_LEFT: handle_MOVE_LEFT(m); break;
      case MESSAGE_CS_MOVE_RIGHT: handle_MOVE_RIGHT(m); break;
      case MESSAGE_CS_USE: handle_USE(m); break;
      case MESSAGE_CS_ATTACK: handle_ATTACK(m); break;

      default:
        printf("[WARNING] Unknown message (%d) received from"
          " %u.%u.%u.%u:%d\n",
          m->getType(),
          addr & 0xFF,  (addr >> 8) & 0xFF,
          (addr >> 16) & 0xFF, addr >> 24,
          m->getPort());
    }

    if ( delete_flag ) delete m;

    server_data->trash->acceptIt(old_ptr);
  }
}

/***************************************************************************************************
*
* Handle client requests
*
***************************************************************************************************/

void WorldUpdateModule::handleClientJoinRequest(Message *m)
/* generate a new player, update player list, send an ok message */
{
  Player *p;
  Region *r;

  /* check if the player is already on this server */
  /* then send a NOK_JOIN message */
  Player *existing_player = server_data->player_list.find(m->getAddress());
  if ( existing_player != NULL )
  {
    /* uncomment these lines if you want the server to refuse clients
    that are already connected */

    /*
    Message *mnok = new Message(MESSAGE_SC_NOK_JOIN);
    if ( mnok == NULL )
    {
      printf("[WARNING] Cannot answer client (JoinNotOkMessage)\n");
      return;
    }
    mnok->setAddress(m->getAddress());
    out->putMessage( mnok );

    printf("[WARNING] Player already on server '%s' "
      "(send not ok to join message)\n", existing_player->name);
    return;
    */

    p = existing_player;
  } else {
    /* create a new player */
    p = new Player(m->getAddress());
    if ( p == NULL )
    {
      printf("[WARNING]Cannot create new player\n");
      return;
    }

    /* set player name and initial attributes */
    char pname[MAX_PLAYER_NAME];
    sprintf(pname, "Player_%X_%X", m->getIP(), m->getPort());
    p->setName(pname);
    p->life = min_life + rand() % ( max_life - min_life + 1 );
    p->attr = min_attr + rand() % ( max_attr - min_attr + 1 );

    r = NULL;
    while ( r == NULL )
    {
      /* set starting position for the new player */
      server_data->getNewPosition(&p->x, &p->y);
      r = server_data->region[p->x / server_data->regx]
        [p->y / server_data->regy];
    }

    /* add player to map */
    r->addPlayer(p,p->x % server_data->regx,p->y % server_data->regy);

    /* add player to list */
    server_data->player_list.insert(p);

    /* Display message */
    if ( server_data->display_user_on_off )
      printf("New player: %s (%d,%d)\n", p->name, p->x, p->y);
  }

  /* send a message back to the player */
  MessageOkJoin *mok = new MessageOkJoin();
  if ( mok == NULL )
  {
    printf("[WARNING] Cannot answer client (JoinOkMessage)\n");
    return;
  }
  mok->setAddress(p->address);
  mok->setParam(p->name, p->x, p->y);
  mok->setMapSize(server_data->mapx, server_data->mapy);
  out->putMessage( (Message*)mok );
}

void WorldUpdateModule::handleClientLeaveRequest(Message *m)
/* remove client from list and send an ok_leave message */
{
  /* get player and check if client is present on this server */
  Player *p = server_data->player_list.find(m->getAddress());
  if ( p == NULL )
  {
    printf("[WARNING] Client sent leave message but is not on this server\n");
    return;
  }

  /* display message */
  if ( server_data->display_user_on_off )
    printf("Removing player %s\n", p->name);

  /* create an ok_leave message */
  Message *mok = new Message(MESSAGE_SC_OK_LEAVE);
  if ( mok == NULL )
  {
    printf("[WARNING] Cannot answer client (LeaveOkMessage)\n");
    return;
  }
  mok->setAddress(p->address);

  /* remove player from map */
  p->lock();
  Region *r = server_data->region[p->x / server_data->regx][p->y / server_data->regy];
  if ( r != NULL )
    r->removePlayer(p->x % server_data->regx,p->y % server_data->regy);
  p->unlock();

  /* remove player from list */
  server_data->player_list.erase(m->getAddress());  /* remove from list */
  server_data->trash->add(p);

  /* send message */
  out->putMessage( mok );
}

void WorldUpdateModule::handle_ACK_OK_MIGRATE(Message *m)
/* remove player from migrating_players list and add it to the regulare players list */
{
  Player *p = server_data->migrating_players.find(m->getAddress());
  if ( p == NULL )
  {
    if ( server_data->display_all_warnings )
      printf("[WARNING] Player sent ACK_OK_MIGRATE "
      "but is not in migrating list\n");
    return;
  }

  server_data->player_list.insert(p);
  server_data->migrating_players.erase(m->getAddress());
}

void WorldUpdateModule::handle_REQUEST_CLIENT_UPDATE(Message *m)
/* Another server needs to update a client ==> reply with map information */
{
  int x1,y1,x2,y2;    /* rectangular region visible to the player */
  Serializator *s;
  MessageWithSerializator *ans; /* message to be sent */
  int i,j;
  int rx,ry;      /* region coordinates */
  int ix,iy;      /* coordinates inside region */

  MessageRequestClientUpdate *mrcu = (MessageRequestClientUpdate*)m;

  /* determine the regiom visible to the player */
  x1 = max(mrcu->x - MAX_CLIENT_VIEW, 0);
  x2 = min(mrcu->x + MAX_CLIENT_VIEW + 1, server_data->mapx);
  y1 = max(mrcu->y - MAX_CLIENT_VIEW, 0);
  y2 = min(mrcu->y + MAX_CLIENT_VIEW + 1, server_data->mapy);

  /* create a new message */
  ans = new MessageWithSerializator(MESSAGE_SS_ANSWER_CLIENT_UPDATE);
  if ( ans == NULL )
  {
    printf("[WARNING] Cannot create update message\n");
    return;
  }
  ans->setAddress(mrcu->addr);
  s = ans->getSerializator();
  if ( s == NULL )
  {
    printf("[WARNING] Cannot create update message with serializator\n");
    delete ans;
    return;
  }

  /* pack data: position */
  s->putBytes((char*)&mrcu->addr, sizeof(IPaddress));
  *s << mrcu->x;
  *s << mrcu->y;
  *s << x1; *s << y1;
  *s << x2; *s << y2;

  /* pack data: terrain, objects, players */
  for ( i = x1; i < x2; i++ )
    for ( j = y1; j < y2; j++ )
    {
      /* region coordinates */
      rx = i / server_data->regx;
      ry = j / server_data->regy;
      /* coordinates inside region */
      ix = i % server_data->regx;
      iy = j % server_data->regy;

      /* check if region belongs to server */
      Region *r = server_data->region[rx][ry];
      if ( r == NULL )
      {
        /* does not belong to current server */
        *s << ((char)0);  /* terrain is free */
        *s << CELL_NONE;  /* no player/object */

      } else {
        /* region belongs to current server */
        *s << r->terrain[ix][iy];
        Player *p2 = r->grid[ix][iy].player;
        if ( p2 != NULL && !(i == mrcu->x && j == mrcu->y) )
        {
          *s << CELL_PLAYER;
          *s << p2->life;
          *s << p2->attr;
          *s << p2->dir;
          s->putBytes((char*)&p2->address, sizeof(IPaddress));

        } else if ( r->grid[ix][iy].object != NULL
          && r->grid[ix][iy].object->quantity > 0 ) {
          *s << CELL_OBJECT;
          *s << r->grid[ix][iy].object->attr;
          *s << r->grid[ix][iy].object->quantity;

        } else {
          *s << CELL_EMPTY;
        }
      }
    }

  /* send it to the client */
  ans->prepare();
  out->putMessage(ans);
}

void WorldUpdateModule::handle_ANSWER_CLIENT_UPDATE(Message *m)
/* forward message to client */
{
  #ifdef __COMPRESSED_MESSAGES__
  printf("[WARNING] ANSWER_CLIENT_UPDATE does not work with compressed messages\n");
  return;
  #endif

  MessageWithSerializator *ans = (MessageWithSerializator*)m;
  IPaddress addr;

  /* get serializator */
  Serializator *s = ans->getSerializator();
  if ( s == NULL )
  {
    printf("[WARNING] Invalid message (AnswerClientUpdate)\n");
    return;
  }

  /*  get client IP address and set it as the destination for the message */
  s->getBytes((char*)&addr, sizeof(IPaddress));
  ans->setAddress(addr);

  /* send the message */
  out->putMessage(ans);

}

/***************************************************************************************************
*
* Handle client actions
*
***************************************************************************************************/

void WorldUpdateModule::handle_move(Player *p, int new_x, int new_y)
{
  /* the player is on the edge of the map */
  if ( new_x < 0 ) return;
  if ( new_x >= server_data->mapx ) return;
  if ( new_y < 0 ) return;
  if ( new_y >= server_data->mapy ) return;

  p->lock();

  /* get region and position inside region (old) */
  int rx_old = p->x / server_data->regx;
  int ry_old = p->y / server_data->regy;
  int ix_old = p->x % server_data->regx;
  int iy_old = p->y % server_data->regy;

  /* get region and position inside region (new) */
  int rx_new = new_x / server_data->regx;
  int ry_new = new_y / server_data->regy;
  int ix_new = new_x % server_data->regx;
  int iy_new = new_y % server_data->regy;

  /* get old and new region */
  Region *r_old = server_data->region[rx_old][ry_old];
  Region *r_new = server_data->region[rx_new][ry_new];

  if ( r_new == NULL )  /* PLAYER HANDOFF */
  {
    if ( equalIP(server_data->layout[rx_new][ry_new], server_data->own_ip) )
    {
      /* the region belongs to current server but is not transfered yet */
      p->unlock();
      if ( server_data->display_all_warnings )
        printf("[WARNING] Player moves into a region that is not "
        "transfered. Move canceled\n");
      return;
    }

    /* the player is in a region that does not belong to the current server */

    /* remove player */
    if ( r_old != NULL ) r_old->removePlayer(ix_old,iy_old);
    p->x = new_x;
    p->y = new_y;

    /* send a migrate message to the player */
    Message *m = new MessageWithIP(MESSAGE_SC_MIGRATE, server_data->own_ip);
    if ( m == NULL )
    {
      printf("[WARNING] Cannot send MIGRATE message to player\n");
      p->unlock();
      return;
    }
    m->setAddress(p->address);
    out->putPriorityMessage(m);

    /* send a give player message to the master */
    mm_module->sendPlayer( p, p->x,p->y );

    /* remove player from list */
    server_data->player_list.erase(p->address); /* remove from list */
    server_data->trash->add(p);

    p->unlock();
    return;
  }

  /* client tries to move to a blocked area */
  if ( r_new->terrain[ix_new][iy_new] != 0 )
  {
    p->unlock();
    return;
  }
  /* client tries to move over another player */
  if ( r_new->grid[ix_new][iy_new].player != NULL )
  {
    p->unlock();
    return;
  }

  /* move player */
  if ( r_old != NULL ) r_old->removePlayer(ix_old,iy_old);
  p->x = new_x;
  p->y = new_y;
  r_new->addPlayer(p,ix_new,iy_new);
  p->unlock();
}

void WorldUpdateModule::handle_MOVE_UP(Message *m)
{
  /* get pointer to player */
  Player *p = server_data->player_list.find(m->getAddress());
  if ( p == NULL ) return;
  handle_move( p, p->x, p->y - 1 );
  p->dir = 2;
}

void WorldUpdateModule::handle_MOVE_DOWN(Message *m)
{
  Player *p = server_data->player_list.find(m->getAddress());
  if ( p == NULL ) return;
  handle_move( p, p->x, p->y + 1 );
  p->dir = 0;
}

void WorldUpdateModule::handle_MOVE_LEFT(Message *m)
{
  Player *p = server_data->player_list.find(m->getAddress());
  if ( p == NULL ) return;
  handle_move( p, p->x - 1, p->y );
  p->dir = 3;
}

void WorldUpdateModule::handle_MOVE_RIGHT(Message *m)
{
  Player *p = server_data->player_list.find(m->getAddress());
  if ( p == NULL ) return;
  handle_move( p, p->x + 1, p->y );
  p->dir = 1;
}

void WorldUpdateModule::handle_USE(Message *m)
{
  /* get player */
  Player *p = server_data->player_list.find(m->getAddress());
  if ( p == NULL ) return;

  /* get player coordinates */
  int rx = p->x / server_data->regx;
  int ry = p->y / server_data->regy;
  int ix = p->x % server_data->regx;
  int iy = p->y % server_data->regy;

  /* get resource resource */
  Region *r = server_data->region[rx][ry];
  if ( r == NULL ) return;
  GameObject *o = r->grid[ix][iy].object;
  if ( o == NULL || o->quantity <= 0 ) return;
  if ( o->quantity > 0 && p->life < 100 )
    o->quantity --, p->life++;
}

void WorldUpdateModule::handle_ATTACK(Message *m)
{
  /* get player */
  Player *p1 = server_data->player_list.find(m->getAddress());
  if ( p1 == NULL ) return;

  /* get second player */
  for ( int i = -1; i <= 1; i++ )
    for ( int j = -1; j <= 1; j++ )
    {
      /* exclude this player */
      if ( i == 0 && j == 0) continue;

      /* neighbour coordinates */
      int x = p1->x + i;
      int y = p1->y + j;

      /* check if coordinates are inside the map  */
      if ( x < 0 ) return;
      if ( x >= server_data->mapx ) return;
      if ( y < 0 ) return;
      if ( y >= server_data->mapy ) return;

      /* get player2 coordinates */
      int rx2 = x / server_data->regx;
      int ry2 = y / server_data->regy;
      int ix2 = x % server_data->regx;
      int iy2 = y % server_data->regy;

      /* get second player */
      Region *r = server_data->region[rx2][ry2];
      if ( r == NULL ) continue;
      Player *p2 = r->grid[ix2][iy2].player;
      if ( p2 == NULL ) continue; /* no player here */
      if ( server_data->player_list.find(m->getAddress()) == NULL )
        continue; /* player is gone */

      /* update players */
      if ( p1->life < p2->life ) break;
      if ( server_data->display_actions )
        printf("Player %s attacks %s\n", p1->name, p2->name);
      if ( p2->life > 1 ) p2->life--, p2->dir =  - p1->dir;
      if ( p1->life < 100 ) p1->life++;
    }

}
