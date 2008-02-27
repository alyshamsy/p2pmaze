
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

#ifndef __MAP_MANAGEMENT_MODULE_H
#define __MAP_MANAGEMENT_MODULE_H

/***************************************************************************************************
*
* MapManagModule - Map management module
* - manages a connection between this server and the master
* - handles tasks like transfering players and map regions
* - the server must call the 'retrieveMapData' method from this class before it creates the
* other modules because this method fills the 'server_data' object whith data recived from the
* master
*
***************************************************************************************************/

class MapManagModule : public Module
{
protected:
  /* communication data */
  IPaddress master_ip;    /* the ip and port of the master */
  ServerData *server_data;  /* information share between all server threads */
  TCPsocket sock;     /* socket between this server and the master */
  MessageQueue *out;    /* queue for sending messages to clients */
  SDL_mutex *send_mutex;    /* doesn'a allow more than one thread to send at a time */

  /* statisctics */
  RateMonitor rm_send,rm_recv;

public:
  /* constructor, destructor and setup */
  MapManagModule(ServerData *server_data, char *master_name, int master_port);
  ~MapManagModule();
  void setOutQueue(MessageQueue *out);

  /* main loop for the map management thread */
  void run();

  /* first method called */
  void retrieveMapData();

  /* actions that requiere contacting the master */
  void sendPlayer( Player *p, int x, int y );
  void sendStatistics( ServerStatistics *ss, Serializator *se );

  /* handlers for messages received from the master */
  bool handle_TAKE_PLAYER();
  bool handle_START_QUEST();
  bool handle_GIVE_REGION();
  bool handle_TAKE_REGION();
  bool handle_MOVING_REGION();

private:
  /* region related */
  void packRegionCell(Region *r, int i, int j, Serializator &s);

  /* statistics methods */
protected:
  int SDLNet_TCP_Recv2(TCPsocket sock, void *data, int maxlen);
  int SDLNet_TCP_Send2(TCPsocket sock, void *data, int len);
  void updateStatistics_sent(int packet_size);
  void updateStatistics_recv(int packet_size);
public:
  float getBPS_sent();
  float getBPS_recv();
};

#endif
