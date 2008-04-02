
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
//#include <iostream>
#include <cassert>
#include <netinet/in.h>
#include <netdb.h>

#include "Master.h"
#include "MasterModule.h"

using namespace std;

/***************************************************************************************************
*
* Constructor / Destructor
*
***************************************************************************************************/

MasterModule::MasterModule(MapData &map_data, int port)
{
  /* init member variables */
  this->port = port;
  finished = false;
  connected = 0;
  nservers = 0;
  memcpy(&this->map_data, &map_data, sizeof(MapData));
  stats.player_migrations = 0;
  stats.region_migrations = 0;
  custom_quests = false;

  /* allocate memory for the message queue */
  message_queue = new MessageQueue();
  if ( message_queue == NULL )
    throw "Cannot create message queue";
  message_queue->setUnlimited();

  /* allocate memory for regions */
  layout = new int*[map_data.nregx];
  players_per_region = new int*[map_data.nregx];
  if ( layout == NULL || players_per_region == NULL )
    throw "Not enough memory for layout matrix";
  for ( int i = 0; i < map_data.nregx; i++ )
  {
    layout[i] = new int[map_data.nregy];
    players_per_region[i] = new int[map_data.nregy];
    if ( layout[i] == NULL || players_per_region[i] == NULL )
      throw "Not enough memory for layout matrix";
    for ( int j = 0; j < map_data.nregy; j++ )
    {
      players_per_region[i][j] = 0;
      layout[i][j] = 0;
    }
  }

  /* set time of first quest */
  time_of_next_quest = SDL_GetTicks() + map_data.quest_first * 1000;
}

MasterModule::~MasterModule()
{
  /* free memory from the region matrices */
  for ( int i = 0; i < map_data.nregx; i++ )
  {
    delete[] layout[i];
    delete[] players_per_region[i];
  }
  delete[] layout;
  delete[] players_per_region;
  delete message_queue;
}

/***************************************************************************************************
*
* Utility methods
*
***************************************************************************************************/

/*
* int SDLNet_TCP_Recv(TCPsocket sock, void *data, int maxlen)
* - The original function can return less than maxlen bytes. This method blocks until maxlen
*   have been read.
*/

int MasterModule::SDLNet_TCP_Recv(TCPsocket sock, void *data, int maxlen)
{
  int p = 0,r;
  while ( p < maxlen )
  {
    r = ::SDLNet_TCP_Recv(sock, (void*)((char*)data + p), maxlen-p);
    if ( r <= 0 ) return r;
    p += r;
  }
  return p;
}

/*
* const char* getAlgorithm()
* - The base MasterModule implements just static partitioning. Classes extending MasterModule
*   will overload this method to return the name of the algorithm they are implementing.
*/

const char* MasterModule::getAlgorithm()
{
  return "static";
}

/*
* IPaddress getHostAddress(TCPsocket s)
* - This method returns the IP address of the other peer in a TCP connection.
*/

IPaddress MasterModule::getHostAddress(TCPsocket s)
{
  IPaddress *ip = SDLNet_TCP_GetPeerAddress(s);
  if ( ip == NULL ) throw "Cannot obtain IP for host";
  return *ip;
}

/*
* MessageQueue* MasterModule::getMessageQueue()
* - This method returns the message queue so that other threads can use it.
*/

MessageQueue* MasterModule::getMessageQueue()
{
  return message_queue;
}

/*
* void MasterModule::setCustomQuests(char *str)
* - create a list of quests from the string
*/

void MasterModule::setCustomQuests(char *str)
{
  if ( str == NULL ) return;

  QuestPoint q;
  char *p;

  p = strtok(str, "() \t");
  while ( p != NULL )
  {
    sscanf(p, "%d,%d,%d", &q.x,&q.y,&q.duration);
    quest_points.insert(quest_points.end(), q);
    p = strtok(NULL, "() \t");
  }

  custom_quests = true;
}

/***************************************************************************************************
*
* Handlers for messages received from servers
*
***************************************************************************************************/

int MasterModule::handle_SM_STARTED(TCPsocket s)
{
  Uint32 message_type;
  int r;

  /* read UDP IP address */
  IPaddress udp_addr;
  int numreceived = SDLNet_TCP_Recv( s, &udp_addr, sizeof(IPaddress) );
  if ( numreceived <= 0 ) return numreceived;

  /* deny access if there are to many servers */
  if ( connected > map_data.num_servers )
  {
    printf("Too many servers ... sending exit to server\n");
    message_type = MS_EXIT;
    SDLNet_TCP_Send( s, &message_type, sizeof(Uint32) );
    return 1;
  }

  /* add a new server to the list */
  sockets.push_back(s);
  ConnectedServerInfo csi;// = &servers[nservers];
  nservers++;
  csi.tcp_connection = getHostAddress(s);
  csi.udp_connection = udp_addr;
  servers.push_back(csi);

  /* wait until all servers are conected */
  //if ( connected < map_data.num_servers ) return 1;

  /* if the first server is connected assign him to all regeons */
  if(nservers == 1){
    //int n_hosts = socket_list.size();
    int map_size = map_data.nregx * map_data.nregy;
    int regions_per_host = map_size; //map_size / n_hosts + ( ( map_size % n_hosts != 0 ) ? 1 : 0 );
    //int regions_for_this_host = regions_per_host;
    int i,j,k = 0;

    for ( i = 0; i < map_data.nregx; i++ )  /* for all regions */
      for ( j = 0; j < map_data.nregy; j++ )
	{
	  layout[i][j] = k;
	  /*regions_for_this_host--;
	  if ( regions_for_this_host == 0 )
	    {
	      k++;
	      regions_for_this_host = regions_per_host;
	      }*/
	    }

  }
  /* send the layout of regions to the server that just joined */
  /*
    - since this is the first message sent to the server, there are no synchronization
    problems so there is no need to put the message in the MessageQueue
  */
  //for ( k = 0; k < nservers; k++ )
  //{
    int k = nservers - 1; // last server
    //csi = &servers[k];
    message_type = MS_CONFIG;
    assert(sockets.size() == nservers);
    r = SDLNet_TCP_Send( sockets[k], &message_type, sizeof(Uint32) );
    if ( r < (int)sizeof(Uint32) ) throw "Error sending map to server";
    r = SDLNet_TCP_Send( sockets[k], &map_data, sizeof(MapData) );
    if ( r < (int)sizeof(MapData) ) throw "Error sending map to server";
    for ( int i = 0; i < map_data.nregx; i++ )
      for ( int j = 0; j < map_data.nregy; j++ )
      {
        r = SDLNet_TCP_Send( sockets[k],
          &servers[layout[i][j]].udp_connection,
          sizeof(IPaddress) );
        if ( r < (int)sizeof(IPaddress) )
          throw "Error sending map to server";
      }
    //}

  return 1;
}

int MasterModule::handle_SM_GIVE_PLAYER(TCPsocket s)
{
  Uint32 len;
  int x,y;
  char *buffer;

  /* receive data */
  if ( SDLNet_TCP_Recv( s, &len, sizeof(Uint32) ) <= 0
    || ( ( buffer = new char[len] ) == NULL )
    || SDLNet_TCP_Recv( s, buffer, len ) <= 0
    || SDLNet_TCP_Recv( s, &x, sizeof(int) ) <= 0
    || SDLNet_TCP_Recv( s, &y, sizeof(int) ) <= 0 )
  {
    printf("[WARNING] Cannot receive client. Client lost\n");
    return 0;
  }

  /* find the address of the next server */
  TCPsocket next;
  int rx,ry;
  rx = x / map_data.regx;
  ry = y / map_data.regy;
  if ( rx < 0 || ry < 0 || rx >= map_data.nregx || ry >= map_data.nregy )
  {
    printf("[WARNING] Coordinates for client are out of bounds\n");
    return 1;
  }
  next = sockets[layout[rx][ry]];

  /* send the player to the next server */
  Message *m = new MasterMessageWithBuffer(MS_TAKE_PLAYER, buffer, len, next);
  if ( m == NULL )
  {
    printf("[WARNING] Failed to transfer player. Player lost\n");
    delete[] buffer;
    return 0;
  }
  message_queue->putMessage(m);

  stats.player_migrations ++;
  return 1;
}

int MasterModule::handle_SM_MOVE_REGION(TCPsocket s)
{
  Uint32 len;
  int x,y;
  char *buffer;
  int players_in_region;

  /* receive data */
  if ( SDLNet_TCP_Recv( s, &len, sizeof(Uint32) ) <= 0
    || ( ( buffer = new char[len] ) == NULL )
    || SDLNet_TCP_Recv( s, buffer, len ) <= 0
    || SDLNet_TCP_Recv( s, &x, sizeof(int) ) <= 0
    || SDLNet_TCP_Recv( s, &y, sizeof(int) ) <= 0 )
      throw "Cannot receive region";

  /* find the address of the next server */
  TCPsocket next;
  if ( x < 0 || y < 0 || x >= map_data.nregx || y >= map_data.nregy )
  {
    printf("Region coordinates %d,%d ( map size: %dx%d )\n", x,y,
      map_data.nregx, map_data.nregy);
    throw "Coordinates for region are out of bounds";
  }
  next = sockets[layout[x][y]];

  /* get the number of players in this region */
  players_in_region = *((int*)( buffer + (len - sizeof(int)) ));

  /* send the player to the next server */
  Message *m = new MasterMessageWithBuffer(MS_TAKE_REGION, buffer, len, next);
  if ( m == NULL )
  {
    delete[] buffer;
    throw "Failed to transfer region";
  }
  message_queue->putMessage(m);

  stats.player_migrations += players_in_region;
  stats.region_migrations ++;
  return 1;
}

int MasterModule::handle_SM_STATISTICS(TCPsocket s)
{
  ServerStatistics ss;
  int ii,jj,n,i;
  IPaddress addr;

  /* receive data */
  if ( SDLNet_TCP_Recv( s, &ss, sizeof(ServerStatistics) ) <= 0 )
  {
    printf("[WARNING] Cannot receive statistics\n");
    return 0;
  }

  /* receive regions */
  for ( i = 0; i < ss.number_of_regions; i++ )
  {
    /* receive data */
    if ( SDLNet_TCP_Recv( s, &ii, sizeof(int) ) <= 0
      || SDLNet_TCP_Recv( s, &jj, sizeof(int) ) <= 0
      || SDLNet_TCP_Recv( s, &n, sizeof(int) ) <= 0 )
    {
      printf("[WARNING] Cannot receive statistics\n");
      return 0;
    }

    /* verify data */
    if ( ii < 0 || jj < 0 || ii >= map_data.nregx || jj >= map_data.nregy )
    {
      printf("[WARNING] Invalid statistics (region out of bounds)\n");
      continue;
    }

    /* update data */
    players_per_region[ii][jj] = n;
  }

  /* update statistics field for this server */
  addr = getHostAddress(s);
  for ( i = 0; i < nservers; i++ )
    if ( equalIP(addr, servers[i].tcp_connection) )
    {
      //memcpy(&servers[i].statistics, &ss, sizeof(ServerStatistics));
      ConnectedServerInfo info = servers[i];
      info.statistics = ss;
      servers[i] = info;		
      break;
    }

  return 1;
}

/***************************************************************************************************
*
* int receiveMessage(TCPsocket s)
* - This method waits for a message from one of the servers, then calls a handler
*   depending on the message type
*
***************************************************************************************************/

int MasterModule::receiveMessage(TCPsocket s)
{
  int numreceived;
  Uint32 message_type;

  /* receive message_type */
  numreceived = SDLNet_TCP_Recv( s, &message_type, sizeof(Uint32) );
  if ( numreceived <= 0 ) return numreceived;

  /* get the whole message depending on its type */
  switch ( message_type )
  {
    case SM_STARTED: numreceived = handle_SM_STARTED(s); break;
    case SM_GIVE_PLAYER: numreceived = handle_SM_GIVE_PLAYER(s); break;
    case SM_MOVE_REGION: numreceived = handle_SM_MOVE_REGION(s); break;
    case SM_STATISTICS: numreceived = handle_SM_STATISTICS(s); break;
    default:
      printf("[WARNING] Unknown message received from game server\n");
      break;
  }
  return numreceived;
}

/***************************************************************************************************
*
* Action to be taken when there are no messages received over a period of time
*
***************************************************************************************************/

/*
* void verifyQuest()
* - This method starts/stops quests.
* - When a quest is started/stopped all servers are informed.
*/

void MasterModule::verifyQuest()
{
  int k;
  int x,y;

  /* don't create any quest if not all servers are connected */
  if ( nservers != map_data.num_servers ) return;

  /* quest beginning / ending */
  if ( !quest.isActive() )
  {
    /* check if it is time to start a new quest */
    if ( SDL_GetTicks() > time_of_next_quest )
    {
      /* set location and duration */
      if ( custom_quests )
      {
        /* take next quest from list */
        if ( quest_points.size() > 0 )
        {
          QuestPoint q = quest_points.front();
          quest_points.pop_front();
          x = q.x; y = q.y;
          quest.setPosition(q.x,q.y);
          quest.start( q.duration );
        } else {
          time_of_next_quest += 3600000;  /* wait for a long time */
          return;
        }

      } else {

        /* create a random quest */
        x = rand() % map_data.mapx;
        y = rand() % map_data.mapy;
        quest.setPosition(x,y);
        quest.start( map_data.quest_min + rand() % map_data.quest_max );
      }

      /* tell all servers about the new quest */
      for ( k = 0; k < nservers; k++ )
      {
        Message *m = new MasterMessageXY(MS_START_QUEST, x,y, sockets[k]);
        if ( m == NULL )
        {
          printf("[WARNING] Failed to send MS_START_QUEST message\n");
          return;
        }
        message_queue->putMessage(m);
      }
    }

  } else {

    /* check if it is time to stop the quest */
    if ( quest.checkTimer() )
    {
      if ( custom_quests )
        time_of_next_quest = SDL_GetTicks() /* exactly that time */
          + map_data.quest_between * 1000;
      else
        time_of_next_quest = SDL_GetTicks() /* random time */
          + rand() % map_data.quest_between * 1000;

      /* tell all servers to stop quest */
      for ( k = 0; k < nservers; k++ )
      {
        Message *m = new MasterMessage(MS_STOP_QUEST, sockets[k]);
        if ( m == NULL )
        {
          printf("[WARNING] Failed to send MS_STOP_QUEST message\n");
          return;
        }
        message_queue->putMessage(m);
      }
    }
  }
}

/*
* void moveRegion(int x, int y, int old_server_id, int new_server_id )
* - Begins a region migration. First the master asks the old_server to
*   give the region, then it informs the rest of servers about the migration.
*   Later, not in this method, the old_server will give the region to the master and the
*   master will forward it to the new_server.
*/

void MasterModule::moveRegion(int x, int y, int old_server_id, int new_server_id )
{
  IPaddress new_server = servers[new_server_id].udp_connection;

  /* set new layout */
  layout[x][y] = new_server_id;

  /* send GIVE_REGION to new_server */
  Message *m = new MasterMessageXYServer
    (MS_GIVE_REGION, x,y,new_server, sockets[old_server_id]);
  if ( m == NULL )
  {
    printf("[WARNING] Failed to send MS_GIVE_REGION message\n");
    return;
  }
  message_queue->putMessage(m);

  /* send MOVING_REGION to all but new_server */
  for ( int k = 0; k < nservers; k++ )
  {
    if ( k == old_server_id ) continue;

    Message *m = new MasterMessageXYServer
      (MS_MOVING_REGION, x,y,new_server, sockets[k]);
    if ( m == NULL )
    {
      printf("[WARNING] Failed to send MS_GIVE_REGION message\n");
      return;
    }
    message_queue->putMessage(m);
  }
}

/*
* void initiate_action()
* - This method is called every MASTER_CHECK_INTERVAL (miliseconds).
* - Derived classes should overload this method to run load balancing algorithms.
* - See description of the run method for more details.
*/

void MasterModule::initiate_action()
{
  verifyQuest();
}

/***************************************************************************************************
*
* Main loop for the master
* - opens a socket for listening to connections from servers
* - waits until an event happens
* - if a new host is connecting, accept the connection
* - if a message is being received call the apropriare method
* - if nothing happens for a period of time call the 'initiate_action' method
*
***************************************************************************************************/
static bool operator==(IPaddress left, IPaddress right)
{
    return left.host == right.host && left.port == right.port;
}

void MasterModule::run()
{
  TCPsocket server_socket;
  TCPsocket new_socket;
  IPaddress ip,remoteIP;
  int numused,numready;
  SDLNet_SocketSet socket_set;
  list<TCPsocket>::iterator its,its2;
  Uint32 start_time;
  Uint32 interval,interval2;

  /* Create a new socket set (used to select sockets) */
  socket_set = SDLNet_AllocSocketSet(MAX_SERVERS);
  if( !socket_set ) throw "Cannot create socket set";

  /* Open a new TCP connection */
  if ( SDLNet_ResolveHost(&ip, NULL, port) )
    throw "Call to SDLNet_ResolveHost failed";
  if ( !( server_socket = SDLNet_TCP_Open(&ip) ) )
    throw "Cannot open connection";
  numused = SDLNet_TCP_AddSocket( socket_set, server_socket );
  if ( numused == -1 ) throw "Cannot add server socket to the set";

  printf("Master started (%s) ...\n", getAlgorithm());
  logMapData();

  /* main loop */
  interval = MASTER_CHECK_INTERVAL;
  while ( !finished )
  {
    /* wait until at least one socket has activity */
    start_time = SDL_GetTicks();
    numready = SDLNet_CheckSockets( socket_set, interval );

    if( numready == -1 )
      throw "Error selecting sockets";

    logGameState();

    if ( numready == 0 )
    {
      initiate_action();
      interval = MASTER_CHECK_INTERVAL;
    }

    else if(numready)
    {
      /* read the message from the socket */
      if( SDLNet_SocketReady(server_socket) )
      {
        /* accept a new connection */
        new_socket = SDLNet_TCP_Accept(server_socket);
        numused = SDLNet_TCP_AddSocket( socket_set, new_socket );
        if ( numused == -1 )
          throw "Cannot add socket to the set";
        socket_list.insert(socket_list.end(),new_socket);
        connected++;

        /* display the address of the new host */
        remoteIP = getHostAddress(new_socket);
        printf("Host connected: %X:%d\n",
          SDLNet_Read32(&remoteIP.host),
          SDLNet_Read16(&remoteIP.port));

      } else {

        /* iterate throgh the sockets to find the active ones */
	    vector<IPaddress> servers_to_delete;
        for (  its = socket_list.begin();
          its != socket_list.end();
          its++ )
          while ( SDLNet_SocketReady(*its) )
            if ( receiveMessage(*its) <= 0 )
            {
              printf("Host disconected\n");
	        servers_to_delete.push_back(getHostAddress(*its));
              its2 = its;
              its++;
              SDLNet_TCP_DelSocket(socket_set,*its2);
              socket_list.erase(its2);	
              connected--;
	           
              //if ( connected < map_data.num_servers )
	      //throw "Too few servers";
              break;
            }
     	for(vector<IPaddress>::iterator i = servers_to_delete.begin(); i!=servers_to_delete.end(); i++)
	    {
            int index = 0;
		    for(vector<ConnectedServerInfo>::iterator j = servers.begin(); j!=servers.end();)
		    {
			    if(j->tcp_connection == *i)
                {
                    change_layout_index(index, REGION_UNOCCUPIED);
                    j = servers.erase(j);
                    decrement_remaining_indecies(index);
                    nservers--;	
                    continue;
                }
                j++;
                index++;
		    }
	    }

        for(vector<IPaddress>::iterator i = servers_to_delete.begin(); i!=servers_to_delete.end(); i++)
	    {
		    for(vector<TCPsocket>::iterator j = sockets.begin(); j!=sockets.end();)
		    {
			    if(getHostAddress(*j) == *i)
                {
                    j = sockets.erase(j);
                    continue;
                }
                j++;
		    }
	    }
        assert(nservers == servers.size());
        assert(nservers == sockets.size());
      }

      /* compute timeout interval */
      interval2 = SDL_GetTicks() - start_time;
      if ( interval2 > interval ) interval = 0;
      else interval = interval - interval2;
    }

  }

  /* free resources */
  SDLNet_FreeSocketSet( socket_set );
  SDLNet_TCP_Close( server_socket );
}

/* thread termination */
void MasterModule::finish()
{
  finished = true;
}

void MasterModule::change_layout_index(int from, int to)
{
    for(int x = 0; x<map_data.nregx; x++)
    {
        for(int y = 0; y<map_data.nregy; y++)
        {
            if(layout[x][y] == from) layout[x][y] = to;
        }
    }
}

void MasterModule::decrement_remaining_indecies(int start)
{
    for(int x = 0; x<map_data.nregx; x++)
    {
        for(int y = 0; y<map_data.nregy; y++)
        {
            if(layout[x][y] > start) layout[x][y] = layout[x][y]-1;
        }
    }
}

/***************************************************************************************************
*
* Logging
*
***************************************************************************************************/

void MasterModule::setLogHost(const std::string& hostname)
{
  log_host = hostname;
  if(log_host != "")
    {
      if(log_host.find(':', 0) == std::string::npos) throw "Wrong format for log server name";
      std::string host = log_host.substr(0, log_host.find(':', 0));
      std::string port = log_host.substr(log_host.find(':', 0)+1);
      
      log_socket = socket(PF_INET, SOCK_STREAM, 0);
      if(log_socket == -1) throw "Could not create log socket";
      struct sockaddr_in dest_addr;
      struct hostent *he;
      he=gethostbyname(host.c_str());
	if(he == NULL)
	  throw "Could not resolve log server's hostname";
      
      dest_addr.sin_family = AF_INET;
      dest_addr.sin_port = htons(atoi(port.c_str()));
      dest_addr.sin_addr = *((struct in_addr *)he->h_addr);
      memset(dest_addr.sin_zero, '\0', sizeof dest_addr.sin_zero);
      if(connect(log_socket, (struct sockaddr *)&dest_addr, sizeof dest_addr) == -1)
	throw "Failed to connect to the log server";
      printf("Connected to the log server\n");
    }
}

static void sendall(int sock, const string& data)
{
  int bytes_sent = 0;
  while(bytes_sent != data.length())
    {
      int ret = send(sock, data.c_str() + bytes_sent, data.length() - bytes_sent, 0);
      if(ret == -1) throw "Could not send to the log server";
      bytes_sent += ret;
    }
}

bool MasterModule::logMapData()
{
  if(log_host != "")
    {
      std::ostringstream out;
      out << "dimensions " << map_data.nregx << " " << map_data.nregy << endl;
      sendall(log_socket, out.str());
    }
  return true;
}

bool MasterModule::logGameState()
{
  if(log_host == "") return true;

  std::ostringstream out;
  out << "begin"<<endl;
  out << "servers " << nservers << endl;
  for(int y = 0; y<map_data.nregy; y++)
    {
      for(int x = 0; x<map_data.nregx; x++)
	{
	  out << y*map_data.nregx + x << " " << layout[x][y] << " " << players_per_region[x][y] << endl;
	}
    }
  out << "end" << endl;
  sendall(log_socket, out.str());
  
  // if ( log_file == NULL ) return false;

//   FILE *f = fopen(log_file, "ab");
//   if ( f == NULL ) return false;

//   /* write timestamp */
//   Uint32 timestamp = SDL_GetTicks();
//   if ( fwrite(&timestamp, sizeof(Uint32), 1, f) != 1 )
//     return false;

//   /* write servers vector */
//   //FIXME: Dont write to the log for now
//   //if ( fwrite(servers, sizeof(ConnectedServerInfo), map_data.num_servers, f)
//   //  != (size_t)map_data.num_servers ) return false;

//   /* write quest data */
//   int x;
//   x = quest.isActive();
//   if ( fwrite(&x, sizeof(int), 1, f) != 1 ) return false;
//   x = quest.getX();
//   if ( fwrite(&x, sizeof(int), 1, f) != 1 ) return false;
//   x = quest.getY();
//   if ( fwrite(&x, sizeof(int), 1, f) != 1 ) return false;

//   /* write master statistics */
//   if ( fwrite(&stats, sizeof(MasterStatistics), 1, f) != 1 ) return false;

//   /* write map data */
//   for ( int i = 0; i < map_data.nregx; i++ )
//     for ( int j = 0; j < map_data.nregy; j++ )
//     {
//       if ( fwrite(&layout[i][j], sizeof(int), 1, f) != 1 ) return false;
//       if ( fwrite(&players_per_region[i][j], sizeof(int), 1, f) != 1 ) return false;
//     }

//   if ( fclose(f) != 0 ) return false;
  return true;
}

/***************************************************************************************************
*
* Limiting load balancing
* - The load balancind algorithm will not be run more often than the specified interval.
*
***************************************************************************************************/

void MasterModule::setLoadBalanceLimit(Uint32 seconds)
{
  min_balance_interval = seconds * 1000;
}

bool MasterModule::limitLoadBalance()
{
  if ( min_balance_interval == 0 ) return false;
  Uint32 now = SDL_GetTicks();
  if ( now - last_balance < min_balance_interval ) return true;
  last_balance = now;
  return false;

}
