
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

#include "Server.h"

/***************************************************************************************************
*
* Constructors / Destructor
*
***************************************************************************************************/

CustomPlayerList::CustomPlayerList()
{
  nthr = 0;
  buckets = NULL;
}

CustomPlayerList::CustomPlayerList(int nthr)
{
  create(nthr);
}

CustomPlayerList::~CustomPlayerList()
{
  for ( int i = 0; i < nthr; i++ ) delete buckets[i];
  delete[] buckets;
}

void CustomPlayerList::create(int nthr)
{
  if ( nthr <= 0 ) throw "Invalid number of threads in CustomPlayerList constructor";
  this->nthr = nthr;
  buckets = new PlayerBucket*[nthr];
  if ( buckets == NULL ) throw "Cannot create player buckets";
  for ( int i = 0; i < nthr; i++ )
  {
    buckets[i] = new PlayerBucket();
    if ( buckets[i] == NULL ) throw "Cannot create player buckets";
  }
}

/***************************************************************************************************
*
* Operations
*
***************************************************************************************************/

/*
* bool insert(Player *p)
* - inserts a player in the appropriate bucket
* - returns true if the player was inserted
*/

bool CustomPlayerList::insert(Player *p)
{
  /* verify parameters */
  if ( p == NULL ) return false;

  /* find key */
  Uint16 key = p->address.port % nthr;

  /* add player */
  bool result = buckets[key]->insert(p);

  return result;
}

/*
* Player* find(IPaddress a)
* - finds the player with the given IP address
* - first we select a bucket then the search is performed in that bucket
* - searching in a bucket is O(log n)
*/

Player* CustomPlayerList::find(IPaddress a)
{
  /* find key */
  Uint16 key = a.port % nthr;

  /* add player */
  return buckets[key]->find(a);
}

/*
* bool findValue(Player *p)
* - searches the whole collection to see if it contains the player pointer from the argument
* - complexity is O(n) where n is the size of the whole collection
* - returns true if the pointer p is in the list, false otherwise
*/

bool CustomPlayerList::findValue(Player *p)
{
  for ( int  i = 0; i < nthr; i++ )
    if ( buckets[i]->findValue(p) ) return true;
  return false;
}

/*
* bool erase(IPaddress a)
* - removes the player with the given IP address from the list
* - returns true if the player was removed, false if the player was not in the list
* - complexity is O(log n)
*/

bool CustomPlayerList::erase(IPaddress a)
{
  /* find key */
  Uint16 key = a.port % nthr;

  /* remove player */
  bool result = buckets[key]->erase(a);

  return result;
}

/*
* void clear()
* - removes all players from the list
* - complexity is O(1)
*/

void CustomPlayerList::clear()
{
  for ( int i = 0; i < nthr; i++ ) buckets[i]->clear();
}

/*
* int size()
* - returns the size of the collection
* - complexity is O(1)
*/

int CustomPlayerList::size()
{
  int s = 0;
  for ( int i = 0; i < nthr; i++ ) s += buckets[i]->size();
  return s;
}

/*
* PlayerBucket* getBucket(int n)
* - selects a bucket
*/

PlayerBucket* CustomPlayerList::getBucket(int n)
{
  if ( n < nthr )
    return buckets[n];
  else
    return NULL;
}
