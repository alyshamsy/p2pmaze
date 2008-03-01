
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

#ifndef __ALG_SPREAD_H
#define __ALG_SPREAD_H

#include "MasterModule.h"
#include "BasicLoadBalance.h"

/***************************************************************************************************
*
* SpRegion - region structure used to keep a vector of regions sorted by number of players
*
***************************************************************************************************/

struct SpRegion
{
  int x,y;  /* coordinates */
  int p;    /* number of players */
};

/***************************************************************************************************
*
* AlgSpread - MasterModule with spread partitioning algorithm
*
***************************************************************************************************/

class AlgSpread : public BasicLoadBalance
{
private:
  int **new_layout;   /* region layout after the load balancing */
          /* int **layout in MasterModule class holds
          the current layout */
  SpRegion *sorted_regions; /* regions sorted by number of players */
  int players_per_server[MAX_SERVERS];  /* number of players on each server on each
            step of the algorithm */
protected:
  virtual int getLightestServer();

public:
  /* Constructor / Destructor */
  AlgSpread(MapData &map_data, int port);
  ~AlgSpread();

  /* Algorithm name */
  virtual const char* getAlgorithm() { return "spread"; }

  /* Algorithm implementation */
  bool balance();
};

#endif
