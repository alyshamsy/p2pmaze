
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

#ifndef __MASTER_MODULE_H
#define __MASTER_MODULE_H

#include <list>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

/***************************************************************************************************
*
* MasterModule - Main module for the master
*
***************************************************************************************************/
#define REGION_UNOCCUPIED -1

class MasterModule : public Module
{
protected:
  int port;   /* the port to listen for connections */
  bool finished;    /* flag to terminate the main loop */
  std::string log_host;   /* host name and port of the game monitor */
  int log_socket;
  MasterStatistics stats; /* migration statistics */

  /* list of connected hosts */
  int connected;    /* number of connected servers */
  list<TCPsocket> socket_list;
  MessageQueue *message_queue;  /* some messages are sent from another thead */
          /* (this module just puts them in this queue) */

  /* list of servers ( only those that will partition the map ) */
  //ConnectedServerInfo servers[MAX_SERVERS];
  vector<ConnectedServerInfo> servers;
  vector<TCPsocket> sockets;
  //TCPsocket sockets[MAX_SERVERS];
  int nservers;

  /* map information */
  MapData map_data;
  int **layout;
  int **players_per_region;

  /* quest data */
  bool custom_quests;
  list<QuestPoint> quest_points;
  Uint32 time_of_next_quest;
  Quest quest;

  /* members for limitinng load balancing */
  Uint32 last_balance,min_balance_interval;

protected:
  /* safe receive (receives the entire message) */
  int SDLNet_TCP_Recv(TCPsocket sock, void *data, int maxlen);

  void change_layout_index(int from, int to);
  void decrement_remaining_indecies(int start);

public:
  /* constructor / destructor */
  MasterModule(MapData &map_data, int port);
  virtual ~MasterModule();

  /* utility methods */
  virtual const char* getAlgorithm();
  IPaddress getHostAddress(TCPsocket s);
  MessageQueue* getMessageQueue();
  void setCustomQuests(char *str);

  /* limiting load balancing */
  void setLoadBalanceLimit(Uint32 seconds);
  bool limitLoadBalance();

  /* message handlers */
  int handle_SM_STARTED(TCPsocket s);
  int handle_SM_GIVE_PLAYER(TCPsocket s);
  int handle_SM_MOVE_REGION(TCPsocket s);
  int handle_SM_STATISTICS(TCPsocket s);
  int receiveMessage(TCPsocket s);

  /* action to be taken when no messages are received for a period of time */
  void verifyQuest();
  void moveRegion(int x, int y, int old_server_id, int new_server_id);
  virtual void initiate_action();

  /* main loop */
  void run();

  /* toggles the finished flag to terminate the main loop */
  void finish();

  /* logging */
  void setLogHost(const std::string& host);
  bool logMapData();
  bool logGameState();
};

#endif
