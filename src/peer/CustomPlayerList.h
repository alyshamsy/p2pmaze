
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

#ifndef __CUSTOM_PLAYER_LIST_H
#define __CUSTOM_PLAYER_LIST_H

#include <map>
#include <set>
using namespace std;

#include "PlayerBucket.h"

/***************************************************************************************************
*
* CustomPlayerList
* - a list for holding players (refered as a list but is in fact a more complex structure: a hash
* where each bucket is a tree)
* - implemented as a hash where there are as many buckets as threads
* - the key for the hash is the port from the client because the port is randomly generated and
* has a uniform spread accros all posible values
* - the complexity for each function in the cpp file treats the number of threads as a constant much
* smaller that the size of the collection
*
***************************************************************************************************/

class CustomPlayerList
{
private:
  int nthr;   /* number of buckets (same as threads) */
  PlayerBucket** buckets; /* vector of buckets */

public:
  /* Constructors / Destructor */
  CustomPlayerList();
  CustomPlayerList(int nthr);
  ~CustomPlayerList();
  void create(int nthr);

  /* operations */
  bool insert(Player *p);
  Player *find(IPaddress a);
  bool findValue(Player *p);
  bool erase(IPaddress a);
  void clear();
  int size();
  PlayerBucket* getBucket(int n);

  /* iterating in a one bucket list */
  inline void start() { buckets[0]->start(); }
  inline Player* next() { return buckets[0]->next(); }
};

#endif
