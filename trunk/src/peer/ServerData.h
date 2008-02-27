
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

#ifndef __SERVERDATA_H
#define __SERVERDATA_H

#include "../utils/RecycleBin.h"
#include <list>
using namespace std;

struct ServerData : public MapData
{
public:
  /* general information */
  IPaddress own_ip;

  /* map data ( besides the field in the MapData class ) */
  Region ***region;   /* region matrix (each cell is a region) */
  IPaddress **layout;   /* server for each region */

  /* players */
  CustomPlayerList player_list; /* list of players */
  CustomPlayerList migrating_players;

  /* data marked for deletion */
  RecycleBin *trash;

  /* quests */
  Quest quest;

public:
  ServerData(int nr_threads); /* nr_threads = number of threads to share this data */
  ~ServerData();

  void copyMapData(MapData *md);
  void setOwnIP(char *local_name, int local_port);
  void alocateMemory();
  void generateRegions();
  void getNewPosition(int *x, int *y);
  void getRandomRegion(int *x, int *y);

  bool notifyClientOfMigration(Player *p, MessageQueue *out);

  void setNumberOfThreads(int x);
};

#endif
