
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

#include "PeriodicEventsModule.h"

#include <set>
using namespace std;

/***************************************************************************************************
*
* Constructors and setup methods
*
***************************************************************************************************/

PeriodicEventsModule::PeriodicEventsModule(ServerData *sd)
{
  server_data = sd;
  out = NULL;
}

void PeriodicEventsModule::setOutQueue(MessageQueue *out)
{
  this->out = out;
}

/***************************************************************************************************
*
* Main loop
*
***************************************************************************************************/

void PeriodicEventsModule::run()
{
  Uint32 start_time;
  int regular_update_interval;
  Garbage *old_ptr = NULL;
  Player *p;
  int i,j;

  /* check queues */
  if ( out == NULL ) throw "Output message queue is NULL";
  printf("RegularUpdateModule started\n");

  /* main loop */
  while ( true )
  {
    start_time = SDL_GetTicks();

    /* create a new quest or destroy an existing one (if needed) */
    checkQuests();

    /* add new resources */
    regenerateResources();

    /* free memory for disconected clients */
    server_data->trash->acceptIt(old_ptr);
    server_data->trash->empty();

    /* resend MESSAGE_SC_OK_MIGRATE to clients that haven't answerd to previous
    migration messages */
    server_data->migrating_players.start();
    while ( ( p = server_data->migrating_players.next() ) != NULL )
    {
      server_data->notifyClientOfMigration(p,out);
      p->count++;
      if ( p->count == RETRY_COUNT )
      {
        Region *r = server_data->region[p->x / server_data->regx]
          [p->y / server_data->regy];
        if ( r != NULL ) r->removePlayer(p->x % server_data->regx,
          p->y % server_data->regy);
        if ( server_data->migrating_players.erase(p->address) )
          server_data->trash->add(p);
        printf("[WARNING] Migration message limit reached for player %s."
          " Removing player\n", p->name);
      }
    }

    /* clean map */
    for ( i = 0; i < server_data->nregx; i++ )
      for ( j = 0; j < server_data->nregy; j++ )
        cleanRegion(server_data->region[i][j]);

    /* wait for some time if this thread was too fast */
    regular_update_interval = SDL_GetTicks() - start_time;
    if ( regular_update_interval < server_data->regular_update_interval )
      SDL_Delay( server_data->regular_update_interval - regular_update_interval );
  }
}

/***************************************************************************************************
*
* Quest notifications
*
***************************************************************************************************/

void PeriodicEventsModule::checkQuests()
{
  int qx,qy;    /* quest location */
  int i;
  list<Player*> pl;
  list<Player*>::iterator it;
  Player *p;
  Message *m;
  Quest *q = &server_data->quest;

  /* get coordinates */
  qx = q->getX();
  qy = q->getY();

  /* send new quest message */
  if ( q->mustSendStart() )
  {
    /* send message to all players informing the of the new quest */
    for ( i = 0; i < server_data->regular_update_threads; i++ )
    {
      server_data->player_list.getBucket(i)->copyPlayers(&pl);
      for ( it = pl.begin(); it != pl.end(); it++ )
      {
        p = *it;
        m = (Message*)new MessageXY(MESSAGE_SC_NEW_QUEST,qx,qy);
        if ( m != NULL )
        {
          m->setAddress(p->address);
          out->putMessage(m);
        }
      }
    }

    if ( server_data->display_quests )
      printf("New quest %d,%d\n", qx,qy);
  }

  /* send quest over message */
  if ( q->mustSendStop() )
  {
    /* send message to all players that the quest is finished */
    for ( i = 0; i < server_data->regular_update_threads; i++ )
    {
      server_data->player_list.getBucket(i)->copyPlayers(&pl);
      for ( it = pl.begin(); it != pl.end(); it++ )
      {
        p = *it;
        m = new Message(MESSAGE_SC_QUEST_OVER);
        if ( m != NULL )
        {
          m->setAddress(p->address);
          out->putMessage(m);
        }
      }
    }

    /* give a small bonus to all players in quest area */
    bonusToPlayers(qx,qy, server_data->quest_bonus);

    if ( server_data->display_quests )
      printf("Quest over\n");
  }
}

void PeriodicEventsModule::bonusToPlayers( int x, int y, int bonus )
{
  int i,j;
  int x1,x2,y1,y2;
  int rx,ry,ix,iy;
  Region *r;
  Player *p;

  x1 = max(x - MAX_CLIENT_VIEW, 0);
  x2 = min(x + MAX_CLIENT_VIEW + 1, server_data->mapx);
  y1 = max(y - MAX_CLIENT_VIEW, 0);
  y2 = min(y + MAX_CLIENT_VIEW + 1, server_data->mapy);

  for ( i = x1; i < x2; i++ )
    for ( j = y1; j < y2; j++ )
    {
      /* region coordinates */
      rx = i / server_data->regx;
      ry = j / server_data->regy;
      r = server_data->region[rx][ry];
      if ( r == NULL ) continue;

      /* coordinates inside region */
      ix = i % server_data->regx;
      iy = j % server_data->regy;
      p = r->grid[ix][iy].player;
      if ( p == NULL ) continue;

      p->life = min(p->life + bonus, 100);
    }
}

/***************************************************************************************************
*
* Management of map resources (food)
*
***************************************************************************************************/

void PeriodicEventsModule::regenerateResources()
{
  if ( rand() % 10000 < 10000 - server_data->regular_update_interval ) return;

  /* select a random region */
  int rx,ry;
  server_data->getRandomRegion(&rx,&ry);
  Region *r = server_data->region[rx][ry];

  /* grow all resources in that region */
  for ( int i = 0; i < server_data->regx; i++ )
    for ( int j = 0; j < server_data->regy; j++ )
    {
      GameObject *o = r->grid[i][j].object;
      if ( o != NULL && o->quantity < server_data->max_res )
        o->quantity++;
    }
}

void PeriodicEventsModule::cleanRegion(Region *r)
{
  if ( r == NULL ) return;

  r->lock();
  for ( int i = 0; i < r->sizex; i++ )
    for ( int j = 0; j < r->sizey; j++ )
      if ( r->grid[i][j].player != NULL
      && !server_data->player_list.findValue(r->grid[i][j].player)
      && !server_data->migrating_players.findValue(r->grid[i][j].player) )
        r->grid[i][j].player = NULL;
  r->unlock();
}
