
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

#include "RegularUpdateModule.h"

#include <set>
using namespace std;

/***************************************************************************************************
*
* Constructors and setup methods
*
***************************************************************************************************/

RegularUpdateModule::RegularUpdateModule(ServerData *sd, int id)
{
  server_data = sd;
  out = NULL;
  average_regular_update_interval = -1;
  average_real_regular_update_interval = -1;
  barrier = NULL;
  rum_id = id;
  bucket = sd->player_list.getBucket(id);
}

void RegularUpdateModule::setOutQueue(MessageQueue *out)
{
  this->out = out;
}

/***************************************************************************************************
*
* Main loop
*
***************************************************************************************************/

void RegularUpdateModule::run()
{
  Uint32 start_time;
  int regular_update_interval;
  Garbage *old_ptr = NULL;
  Player *p;

  /* check queues */
  if ( out == NULL ) throw "Output message queue is NULL";
  printf("RegularUpdateModule started\n");

  /* main loop */
  while ( true )
  {
    start_time = SDL_GetTicks();

    /* send updates to clients (map state) */
    bucket->start();
    #ifndef __MESSAGE_BUFFER__
    while ( ( p = bucket->next() ) != NULL )
      updatePlayer(p);
    #else
    while ( ( p = bucket->next() ) != NULL )
    {
      updatePlayer(p);
      if ( bucket->isFirst() )
      {
        out->putMessages(&temp_message_list);
        temp_message_list.clear();
      }
    }
    out->putMessages(&temp_message_list);
    temp_message_list.clear();
    #endif

    /* accept to free memory for disconected clients */
    server_data->trash->acceptIt(old_ptr);

    /* wait for some time if messages were sent too fast */
    regular_update_interval = SDL_GetTicks() - start_time;

    if ( regular_update_interval < server_data->regular_update_interval )
      SDL_Delay( server_data->regular_update_interval - regular_update_interval );
    if ( average_real_regular_update_interval < 0 )
      average_real_regular_update_interval = regular_update_interval;
    else
      average_real_regular_update_interval
        = average_real_regular_update_interval * 0.95
        + (double)regular_update_interval * 0.05;

    /* update average regular update interval for statistic purpose */
    regular_update_interval = SDL_GetTicks() - start_time;
    if ( average_regular_update_interval < 0 )
      average_regular_update_interval = regular_update_interval;
    else
      average_regular_update_interval
        = average_regular_update_interval * 0.95
        + (double)regular_update_interval * 0.05;

  }
}

/***************************************************************************************************
*
* Client update method
*
***************************************************************************************************/

void RegularUpdateModule::updatePlayer(Player *p)
{
  int x1,y1,x2,y2;    /* rectangular region visible to the player */
  Serializator *s;
  MessageWithSerializator *m; /* message to be sent */
  int i,j;
  int rx,ry;      /* region coordinates */
  int ix,iy;      /* coordinates inside region */
  set<IPaddress,IpComparator> nghs; /* servers that own a region visible by this player */
  int px,py;      /* player coodinates */
  int attr,life,dir;    /* player attributes */
  Player *p2;     /* another player */
  Region *r;      /* region for current coodinates */

  /* copy player data */
  p->lock();
  px = p->x; py = p->y;
  dir = p->dir; attr = p->attr; life = p->life;

  /* region coordinates */
  rx = px / server_data->regx;
  ry = py / server_data->regy;
  /* coordinates inside region */
  ix = px % server_data->regx;
  iy = py % server_data->regy;

  /* verify that the region belongs to this server */
  r = server_data->region[rx][ry];

  if ( r != NULL )
  {
    /* add player to map if is not already */
    if ( r->grid[ix][iy].player == NULL )
      r->addPlayer(p, ix,iy);
  } else {
    /*
    - this might mean that the region with this player is being moved from
    another server so it should arrive for the next updates
    - we send the update message anyway because we don't want the player to think
    the server is disconected
    */
  }

  p->unlock();


  /* determine the region visible to the player */
  x1 = max(px - MAX_CLIENT_VIEW, 0);
  x2 = min(px + MAX_CLIENT_VIEW + 1, server_data->mapx);
  y1 = max(py - MAX_CLIENT_VIEW, 0);
  y2 = min(py + MAX_CLIENT_VIEW + 1, server_data->mapy);

  /* create a new message */
  m = new MessageWithSerializator(MESSAGE_SC_REGULAR_UPDATE);
  if ( m == NULL )
  {
    printf("[WARNING] Cannot create update message\n");
    return;
  }
  m->setAddress(p->address);
  s = m->getSerializator();
  if ( s == NULL )
  {
    printf("[WARNING] Cannot create update message with serializator\n");
    delete m;
    return;
  }

  /* pack data: position */
  *s << px; *s << py;
  *s << x1; *s << y1;
  *s << x2; *s << y2;

  /* pack data: attributes */
  *s << life;
  *s << attr;
  *s << dir;

  /* pack data: terrain, objects, players */
  for ( i = x1; i < x2; i++ )
  {
    for ( j = y1; j < y2; j++ )
    {
      /* region coordinates */
      rx = i / server_data->regx;
      ry = j / server_data->regy;
      /* coordinates inside region */
      ix = i % server_data->regx;
      iy = j % server_data->regy;

      /* check if region belongs to server */
      r = server_data->region[rx][ry];
      if ( r == NULL )
      {
        /* does not belong to current server */
        *s << ((char)0);  /* terrain is free */
        *s << CELL_NONE;  /* no player/object */
        if ( !equalIP(server_data->layout[rx][ry], server_data->own_ip) )
          nghs.insert(server_data->layout[rx][ry]);
      } else {
        /* region belongs to current server */
        *s << r->terrain[ix][iy];
        p2 = r->grid[ix][iy].player;
        if ( p2 != NULL && p2 != p )
        {
          *s << CELL_PLAYER;
          *s << p2->life;
          *s << p2->attr;
          *s << p2->dir;
          /* IPaddress used as an ID: */
          s->putBytes((char*)&p2->address, sizeof(IPaddress));

        } else if ( r->grid[ix][iy].object != NULL &&
          r->grid[ix][iy].object->quantity > 0 ) {
          *s << CELL_OBJECT;
          *s << r->grid[ix][iy].object->attr;
          *s << r->grid[ix][iy].object->quantity;

        } else {
          *s << CELL_EMPTY;
        }
      }
    }
  }

  /* send it to the client */
  m->prepare();
  #ifndef __MESSAGE_BUFFER__
  out->putMessage(m);
  #else
  temp_message_list.insert(temp_message_list.end(),m);
  #endif

  /* send message to other servers informing them to update this client */
  set<IPaddress,IpComparator>::iterator it;
  for ( it = nghs.begin(); it != nghs.end(); it++ )
  {
    Message *mrcu = new MessageRequestClientUpdate(p->x,p->y,p->address);
    mrcu->setAddress(*it);
    #ifndef __MESSAGE_BUFFER__
    out->putMessage(mrcu);
    #else
    temp_message_list.insert(temp_message_list.end(),mrcu);
    #endif
  }
}

