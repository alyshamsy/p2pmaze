
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

/***************************************************************************************************
*
* Constructors, destructor and setup methods
*
***************************************************************************************************/

MapManagModule::MapManagModule(ServerData *server_data,
  char *master_name, int master_port)
{
  /* copy pointer to ServerData object */
  this->server_data = server_data;

  /* use DNS to find the masters address */
  if ( SDLNet_ResolveHost(&master_ip, master_name, master_port ) )
    throw "Cannot get the address of the master";

  /* open a connection with the master */
  sock = SDLNet_TCP_Open(&master_ip);
  if ( !sock ) throw "Cannot connect to the master";

  /* default no output queue */
  out = NULL;

  /* mutex to lock sending */
  send_mutex = SDL_CreateMutex();
}

MapManagModule::~MapManagModule()
{
  SDL_DestroyMutex(send_mutex);
  SDLNet_TCP_Close(sock);
}

void MapManagModule::setOutQueue(MessageQueue *out)
{
  this->out = out;
}

/***************************************************************************************************
*
* First function that must be called by the server
* - connects to the master and retrieves map information and layout
*
***************************************************************************************************/

void MapManagModule::retrieveMapData()
{
  printf("Contacting the master for initial data ...\n");

  /* send own IP address to the master */
  Uint32 message_type = SM_STARTED;
  int len;
  len = sizeof(Uint32);
  if ( SDLNet_TCP_Send2(sock, (void*)&message_type, len) < len )
    throw "Cannot send first message to master";
  len = sizeof(IPaddress);
  if ( SDLNet_TCP_Send2(sock, &server_data->own_ip, len) < len )
    throw "Cannot send first message to master";

  /* get data from master */
  MapData md;
  if ( SDLNet_TCP_Recv2( sock, &message_type, sizeof(Uint32) ) <= 0 )
    throw "Cannot receive configuration from master";
  if ( message_type != MS_CONFIG )
    if ( message_type == MS_EXIT )
      throw "Master denied this server";
    else
      throw "Cannot receive configuration from master";
  if ( SDLNet_TCP_Recv2( sock, &md, sizeof(MapData) ) <= 0 )
    throw "Cannot receive configuration from master";
  server_data->copyMapData(&md);
  server_data->alocateMemory();

  /* get layout from master */
  for ( int i = 0; i < md.nregx; i++ )
    for ( int j = 0; j < md.nregy; j++ )
    {
      if ( SDLNet_TCP_Recv2( sock, &server_data->layout[i][j],
        sizeof(IPaddress) ) <= 0 )
      throw "Cannot receive map layout from master";
    }

  /* generate regions */
  server_data->generateRegions();
}

/***************************************************************************************************
*
* Run method for this module ( main loop in this thread )
* - listens for messages on a TCP connection to the master
* - takes an action depeneding on the message type
*
***************************************************************************************************/

void MapManagModule::run()
{
  Uint32 message_type;
  Garbage *old_ptr = NULL;
  int numready,numused;
  SDLNet_SocketSet socket_set;

  /* create a socket set and add the only socket to it */
  socket_set = SDLNet_AllocSocketSet(1);
  if( !socket_set ) throw "Cannot create socket set";
  numused = SDLNet_TCP_AddSocket( socket_set, sock );
  if ( numused == -1 ) throw "Cannot add socket to the set";

  /* main loop */
  while ( true )
  {
    /* accept object to be deleted */
    server_data->trash->acceptIt(old_ptr);

    /* wait until socket has activity */
    numready = SDLNet_CheckSockets( socket_set, 5000 /* miliseconds */ );
    if( numready < 0 )
      throw "Error selecting sockets";
    if ( numready == 0 ) continue;

    /* receive message type */
    if ( SDLNet_TCP_Recv2( sock, &message_type, sizeof(Uint32) ) <= 0 )
      throw "Master has exited";

    /* take action depeneding on message_type */
    switch ( message_type )
    {
      case MS_TAKE_PLAYER:
        if ( !handle_TAKE_PLAYER() )
          throw "Cannot transfer player because there was an "
          "error in server-master communication";
        break;

      case MS_GIVE_REGION:
        if ( !handle_GIVE_REGION() )
          throw "Error in server-master communication";
        break;
      case MS_TAKE_REGION:
        if ( !handle_TAKE_REGION() )
          throw "Error in server-master communication";
        break;
      case MS_MOVING_REGION:
        if ( !handle_MOVING_REGION() )
          throw "Error in server-master communication";
        break;

      case MS_START_QUEST:
        if ( !handle_START_QUEST() )
          throw "Error in server-master communication";
        break;
      case MS_STOP_QUEST: server_data->quest.stop(); break;

      case MS_EXIT:
        printf("Master ordered to exit\n");
        exit(0);
        break;

      default:
        printf("[WARNING] Unknown message received from master (%d)\n",
          message_type);
        break;
    }
  }
}

/***************************************************************************************************
*
* Actions that require contacting the master
*
***************************************************************************************************/

void MapManagModule::sendPlayer(Player *p, int x, int y)
{
  char *buffer;
  Uint32 len;
  Uint32 message_type;

  message_type = SM_GIVE_PLAYER;
  p->lock();
  buffer = p->playerData();
  len = (Uint32)p->playerDataSize();
  p->unlock();

  SDL_LockMutex(send_mutex);
  if ( SDLNet_TCP_Send2( sock, (void*)&message_type, sizeof(Uint32) ) < (int)sizeof(Uint32)
    || SDLNet_TCP_Send2( sock, (void*)&len, sizeof(Uint32) ) < (int)sizeof(Uint32)
    || SDLNet_TCP_Send2( sock, (void*)buffer, len ) < (int)len
    || SDLNet_TCP_Send2( sock, (void*)&x, sizeof(int) ) < (int)sizeof(int)
    || SDLNet_TCP_Send2( sock, (void*)&y, sizeof(int) ) < (int)sizeof(int) )
      printf("[WARNING] Failed to transfer player '%s'. Player lost\n", p->name);

  SDL_UnlockMutex(send_mutex);
  delete[] buffer;
}

void MapManagModule::sendStatistics( ServerStatistics *ss, Serializator *se )
{
  Uint32 message_type = SM_STATISTICS;
  int len = se->getSize();

  SDL_LockMutex(send_mutex);
  if ( SDLNet_TCP_Send2( sock, (void*)&message_type, sizeof(Uint32) ) < (int)sizeof(Uint32)
    || SDLNet_TCP_Send2( sock, (void*)ss, sizeof(ServerStatistics) )
      < (int)sizeof(ServerStatistics)
    || SDLNet_TCP_Send2( sock, (void*)se->getBuffer(), len ) < (int)len )
      if ( server_data->display_all_warnings )
        printf("[WARNING] Failed to send statistics to master\n");
  SDL_UnlockMutex(send_mutex);
}

/***************************************************************************************************
*
* Handlers for messager received from the master
*
***************************************************************************************************/

bool MapManagModule::handle_TAKE_PLAYER()
/* returns false only if there was a socket error */
{
  Uint32 len;
  char *buffer;

  /* receive data */
  if ( SDLNet_TCP_Recv2( sock, &len, sizeof(Uint32) ) <= 0
    || ( ( buffer = new char[len] ) == NULL ) )
  {
    printf("[WARNING] Cannot receive client. Client lost\n");
    return false;
  }

  if ( SDLNet_TCP_Recv2( sock, buffer, len ) <= 0 )
  {
    delete buffer;
    printf("[WARNING] Cannot receive client. Client lost\n");
    return false;
  }

  /* create player and free buffer */
  Player *p = new Player(buffer);
  delete[] buffer;
  if ( p == NULL )
  {
    delete buffer;
    printf("[WARNING] Cannot create new player from received data. Player lost\n");
    return true;
  }

  /* add player to list and map */

  /* add player to list */
  server_data->migrating_players.insert(p);

  /* add player to map */
  if ( p->x < 0 || p->x >= server_data->mapx || p->y < 0 || p->y >= server_data->mapy )
  {
    printf("[WARNING] Invalid position for migrated player %d,%d. Player %s lost\n",
      p->x,p->y, p->name);
    return true;
  }
  int rx = p->x / server_data->regx;
  int ry = p->y / server_data->regy;
  Region *r = server_data->region[rx][ry];
  if ( r != NULL )
  {
    if ( r->grid[p->x % server_data->regx][p->y % server_data->regy].player != NULL )
      if ( server_data->display_all_warnings )
        printf("[WARNING] Migrated player steps over local player "
        "(%d,%d)\n", p->x, p->y);
    r->addPlayer(p, p->x % server_data->regx,p->y % server_data->regy);
  }

  /* send MESSAGE_SC_OK_MIGRATE to client */
  server_data->notifyClientOfMigration(p,out);

  return true;
}

bool MapManagModule::handle_START_QUEST()
{
  int x,y;

  if ( SDLNet_TCP_Recv2( sock, &x, sizeof(int) ) <= 0
    || SDLNet_TCP_Recv2( sock, &y, sizeof(int) ) <= 0 )
    return false;

  server_data->quest.setPosition(x,y);
  server_data->quest.start();
  return true;
}

/***************************************************************************************************
*
* Statistics related methods
*
***************************************************************************************************/

int MapManagModule::SDLNet_TCP_Recv2(TCPsocket sock, void *data, int maxlen)
{
  int p = 0,r;
  while ( p < maxlen )
  {
    r = SDLNet_TCP_Recv(sock, (void*)((char*)data + p), maxlen-p);
    if ( r <= 0 ) return r;
    p += r;
  }
  rm_recv.addValue(maxlen);
  return p;
}

int MapManagModule::SDLNet_TCP_Send2(TCPsocket sock, void *data, int len)
{
  int r = SDLNet_TCP_Send(sock, data, len);
  rm_send.addValue(len);
  return r;
}

float MapManagModule::getBPS_sent()
{
  return rm_send.getAverage();
}

float MapManagModule::getBPS_recv()
{
  return rm_recv.getAverage();
}

