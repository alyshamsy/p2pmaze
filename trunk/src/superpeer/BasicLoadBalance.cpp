
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

#include "Master.h"
#include "MasterModule.h"
#include "BasicLoadBalance.h"

/***************************************************************************************************
*
* Constructor / Destructor
*
***************************************************************************************************/

BasicLoadBalance::BasicLoadBalance(MapData &map_data, int port)
  : MasterModule(map_data,port)
{
  last_balance = SDL_GetTicks();
  min_balance_interval = 0;
}

BasicLoadBalance::~BasicLoadBalance()
{
}

/***************************************************************************************************
*
* Utility
*
***************************************************************************************************/

int BasicLoadBalance::getNumberOfPlayers()
{
  int sum = 0;
  for ( int i = 0; i < nservers; i++ )
    sum += servers[i].statistics.number_of_players;
  return sum;
}

inline double BasicLoadBalance::serverMachineFactor(int k)
{
  return ( servers[k].statistics.number_of_players == 0 ) ? 0 :
    ( (double)servers[k].statistics.average_real_regular_update_interval /
    (double)servers[k].statistics.number_of_players );
}

double BasicLoadBalance::getRatio(int k)
{
  #ifdef __SERVER_PLAYER_RATIO__
  if ( servers[k].statistics.average_real_regular_update_interval <
    map_data.regular_update_interval / 2 ) return 1;
  double all = 0;
  double mfk = serverMachineFactor(k);
  if ( mfk == 0 ) return 1;
  for ( int i = 0; i < nservers; i++ )
    all += serverMachineFactor(i);
  if ( all == 0 ) return 1;
  return all / nservers / mfk;
  #else
  return 1;
  #endif
}

int BasicLoadBalance::getLightestServer()
{
  double k;
  double pps = serverMachineFactor(0) * servers[0].statistics.number_of_players;
  int ls = 0;

  for ( int i = 1; i < nservers; i++ )
  {
    k = serverMachineFactor(i) * servers[i].statistics.number_of_players;
    if ( pps > k )
    {
      pps = k;
      ls = i;
    }
  }

  return ls;
}

/***************************************************************************************************
*
* Definitions
*
***************************************************************************************************/

inline bool BasicLoadBalance::isServerOverloaded(int s)
{
  return isOverloaded(s, servers[s].statistics.number_of_players);
}

inline bool BasicLoadBalance::isServerSafe(int s)
{
  return isSafe(s, servers[s].statistics.number_of_players);
}

inline bool BasicLoadBalance::isServerLight(int s)
{
  return isLight(s, servers[s].statistics.number_of_players);
}

inline bool BasicLoadBalance::isOverloaded(int s, int players)
{
  return (double)players / ( (double)getNumberOfPlayers() / nservers * getRatio(s) )
    > map_data.overloaded_level;
}

inline bool BasicLoadBalance::isSafe(int s, int players)
{
  return (double)players / ( (double)getNumberOfPlayers() / nservers * getRatio(s) )
    < map_data.overloaded_level;
}

inline bool BasicLoadBalance::isLight(int s, int players)
{
  return (double)players / ( (double)getNumberOfPlayers() / nservers * getRatio(s) )
    < map_data.light_level;
}

/***************************************************************************************************
*
* Master action ( see initiate_action in MasterModule class for more details )
*
***************************************************************************************************/

void BasicLoadBalance::initiate_action()
{
  verifyQuest();
  if ( limitLoadBalance() ) return;
  while ( balance() );
}
