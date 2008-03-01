
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

#ifndef __ALG_LIGHTEST_H
#define __ALG_LIGHTEST_H

#include "MasterModule.h"
#include "BasicLoadBalance.h"

/***************************************************************************************************
*
* AlgLightest - MasterModule with lightest partitioning algorithm
*
***************************************************************************************************/

class AlgLightest : public BasicLoadBalance
{
public:
  /* Constructor / Destructor */
  AlgLightest(MapData &map_data, int port);
  ~AlgLightest();

  /* Algoritm name */
  virtual const char* getAlgorithm() { return "lightest"; }

  /* Algoritm implementation */
  bool balance();

};

#endif
