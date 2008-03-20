
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
#include "../comm/MessageModuleIN.h"
#include "../comm/MessageModuleOUT.h"
#include "MapManagModule.h"
#include "RegularUpdateModule.h"
#include "PeriodicEventsModule.h"
#include "WorldUpdateModule.h"
#include "StatisticsModule.h"

int intServerPort = 0, intMasterPort = 0;
string strServerPort, strMasterPort;
string strServerName, strMasterName;
Configurator *configurator = NULL;
ServerData *server_data = NULL;

/***************************************************************************************************
*
* Module thread
*
***************************************************************************************************/

int module_thread(void *data)
{
  Module *module = (Module*)data;

  try
  {
    module->run();
  } catch ( const char *err ) {
    printf("[ERROR] %s\n", err);
    exit(-1);
  }

  return 0;
}

/***************************************************************************************************
*
* Initialization
*
***************************************************************************************************/

void
startClient ()
{
  pid_t child_id = fork ();

  if (child_id > 0)
    return;

  if (child_id == 0) 
  {
    int ret = execl ("client", 
        "client", 
        (strServerName + ":" + strServerPort).c_str(), 
        NULL);
    if (ret < 0)
    {
      std::perror("Could not execl peer process");
      exit (1);
    }
  }
}

void init(int argc, char *argv[])
{
  /* interpret command line arguments */
  if (argc == 1) {
    strMasterName = DEFAULT_MASTER_NAME;
    strMasterPort = DEFAULT_MASTER_PORT;
    strServerName = DEFAULT_SERVER_NAME;
    strServerPort = DEFAULT_SERVER_PORT;
  }
  else if (argc == 3)
  {
    //char * pArg = NULL;
    //char * pPort = NULL;
    /*pArg = strdup (argv[1]);
    if ( pArg == NULL ) throw "Not enough memory";
    pPort = strchr (pArg, ':');
    if (pPort == NULL) throw "No port specified";
    *(pPort++) = 0;
    sscanf (pPort, "%s", &strMasterPort);
    */
    string strInputArg;
    char chInputArg[100];
    
    /* master name and port */
    // Copy argument to char[100] buffer and then copy to variable
    strcpy (chInputArg, argv[1]);
    strInputArg = chInputArg;
    int pos = 0;
    pos = strInputArg.find (":");
    strMasterName = strInputArg.substr (0, pos);
    if (strMasterName == "")
      throw "No name for master server specified!";
    strMasterPort = strInputArg.substr (pos+1);

    /* server name and port */
    // Copy argument to char[100] buffer and then copy to variable
    strcpy (chInputArg, argv[2]);
    strInputArg = chInputArg;
    if ( strInputArg == "" ) throw "Not enough memory";
    pos = strInputArg.find (":");
    strServerName = strInputArg.substr (0, pos);
    if (strServerName == "")
      throw "No name for local server specified!";
    strServerPort = strInputArg.substr (pos+1);
  } 
    /*strServerName = strdup(argv[2]);
    if ( strServerName == NULL ) throw "Not enough memory";
    port_ptr = strchr(strServerName, ':');
    if ( port_ptr == NULL ) throw "No port specified";
    *(port_ptr++) = 0;
    sscanf(port_ptr, "%d", &strServerPort);
    if ( strServerPort < 1 )
      throw "The port must be an integer larger than 0";
    */
  else {
    throw "Parameters: <strMasterName:port> <strServerName:port>";
  }

  // Convert server port and master port to type int
  intServerPort = atoi (strServerPort.c_str() );
  intMasterPort = atoi (strMasterPort.c_str() );

  if ( intMasterPort < 1 || intServerPort < 1)
    throw "The port must be an integer larger than 0";
  
  //cout << "Starting server " << strServerName << ":" << strServerPort << endl;
  printf("Starting server %s:%s\n", 
      strServerName.c_str(), strServerPort.c_str());
 
  /* initialize random number generator */
  srand((unsigned int)time(NULL));

  /* initialize SDL */
  if ( SDL_Init( SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE ) < 0 ) /* |SDL_INIT_VIDEO */
  {
    printf("Could not initialize SDL: %s.\n", SDL_GetError());
    throw "Failed to initialize SDL (SDL_Init)";
  }
  if ( SDLNet_Init() < 0 )
    throw "Failed to initialize SDL_net (SDLNet_Init)";

  /* initialize server data */
  server_data = new ServerData(0);
  if ( server_data == NULL ) throw "Cannot initialize server data";
  server_data->setOwnIP((char*)strServerName.c_str(), intServerPort);
}

void finish()
{
  /* finish SDL */
  SDLNet_Quit();
  SDL_Quit();

  /* free memory */
  delete server_data;
  delete configurator;
  //free(strServerName); // free pointers allocated using strdup()
}

/***************************************************************************************************
*
* Main
*
***************************************************************************************************/

int main(int argc, char *argv[])
{
  int i;
  try
  {
    #ifdef __COMPRESSED_MESSAGES__
    printf("Starting server with compressed messages\n");
    #endif

    /* initialize */
    init(argc, argv);

    /* create server modules */
    /* MapManagementModule */
    MapManagModule *mm_module = new MapManagModule(server_data, (char*)strMasterName.c_str(), intMasterPort);
    if ( mm_module == NULL ) throw "Cannot create map management module";
    mm_module->retrieveMapData(); /* first contact master and get map information */
    server_data->setNumberOfThreads(3
      + server_data->regular_update_threads
      + server_data->world_update_threads );

    /* IN */
    MessageModuleIN *in_module = new MessageModuleIN(intServerPort);
    if ( in_module == NULL ) throw "Cannot create input module";
    SDL_Thread *in_thread = SDL_CreateThread(module_thread, (void*)in_module);
    if ( in_thread == NULL ) throw "Cannot create input thread";

    /* OUT */
    MessageModuleOUT *out_module = new MessageModuleOUT(in_module->getUDPsocket());
    if ( out_module == NULL ) throw "Cannot create output module";
    SDL_Thread *out_thread = SDL_CreateThread(module_thread, (void*)out_module);
    if ( out_thread == NULL ) throw "Cannot create output thread";

    /* RegularUpdateModule */
    RegularUpdateModule **ru_module =
      new RegularUpdateModule*[server_data->regular_update_threads];
    SDL_Thread **ru_thread =
      new SDL_Thread*[server_data->regular_update_threads];
    if ( ru_module == NULL || ru_thread == NULL )
      throw "Cannot create thread for regular updates";
    SDL_barrier *barrier = SDL_CreateBarrier(server_data->regular_update_threads);
    for ( i = 0; i < server_data->regular_update_threads; i++ )
    {
      ru_module[i] = new RegularUpdateModule(server_data, i);
      if ( ru_module[i] == NULL ) throw "Cannot create regular update module";
      ru_module[i]->setOutQueue(out_module->getQueue());
      ru_module[i]->barrier = barrier;
      ru_thread[i] = SDL_CreateThread(module_thread, (void*)(ru_module[i]));
      if ( ru_thread[i] == NULL ) throw "Cannot create regular update thread";
    }

    /* PeriodicEventsModule */
    PeriodicEventsModule *pe_module = new PeriodicEventsModule(server_data);
    if ( pe_module == NULL ) throw "Cannot create regular update module";
    pe_module->setOutQueue(out_module->getQueue());
    SDL_Thread *pe_thread = SDL_CreateThread(module_thread, (void*)pe_module);
    if ( pe_thread == NULL ) throw "Cannot create regular update thread";

    /* WorldUpdateModule */
    WorldUpdateModule **wu_module =
      new WorldUpdateModule*[server_data->world_update_threads];
    SDL_Thread **wu_thread =
      new SDL_Thread*[server_data->world_update_threads];
    if ( wu_module == NULL || wu_thread == NULL )
      throw "Cannot create thread for regular updates";
    for ( i = 0; i < server_data->world_update_threads; i++ )
    {
      wu_module[i] = new WorldUpdateModule(server_data);
      if ( wu_module[i] == NULL ) throw "Cannot create world update module";
      wu_module[i]->setInQueue(in_module->getQueue());
      wu_module[i]->setOutQueue(out_module->getQueue());
      wu_module[i]->setMapManagModule(mm_module);
      wu_thread[i] = SDL_CreateThread(module_thread, (void*)(wu_module[i]));
      if ( wu_thread[i] == NULL ) throw "Cannot create world update thread";
    }

    mm_module->setOutQueue(out_module->getQueue());
    SDL_Thread *mm_thread = SDL_CreateThread(module_thread, (void*)mm_module);
    if ( mm_thread == NULL ) throw "Cannot create map management thread";

    /* StatisticsModule */
    StatisticsModule *stats_module = new StatisticsModule(server_data,
      mm_module, ru_module, in_module, out_module);
    if ( stats_module == NULL ) throw "Cannot create statistics module";
    SDL_Thread *stats_thread = SDL_CreateThread(module_thread, (void*)stats_module);
    if ( stats_thread == NULL ) throw "Cannot create statistics thread";

    // fork a client
    startClient ();

    /* User input loop (type 'quit' to exit) */
    while ( true )
    {
      char cmd[256] = "";
      scanf("%s", cmd);
      if ( !strcmp(cmd, "exit") || !strcmp(cmd, "quit") || !strcmp(cmd, "q") )
        exit(0);
    }

    finish();

  } catch ( const char *err ) {
    printf("[ERROR] %s\n", err);
    exit(-1);
  }

  return 0;
}

