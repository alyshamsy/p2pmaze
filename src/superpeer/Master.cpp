
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
#include <unistd.h>

#include "Master.h"
#include "MasterModule.h"
#include "SenderModule.h"
#include "AlgLightest.h"
#include "AlgSpread.h"
#include "AlgLocalityAware.h"

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
* Initialization methods
*
***************************************************************************************************/

void dataFromConfigurator( MapData &map_data, Configurator &conf )
{
  /* servers */
  map_data.num_servers = conf.getIntAttribute("master.number_of_servers");
  if ( map_data.num_servers > MAX_SERVERS )
  {
    printf("The maximum number of servers is %d\n", MAX_SERVERS);
    throw "Config file: Too many serves";
  }
  map_data.stats_interval = conf.getIntAttribute("servers.stats_interval");
  map_data.regular_update_interval = conf.getIntAttribute("servers.regular_update_interval");
  if ( map_data.regular_update_interval < 0 )
    throw "Config file: Regular update interval must be positive";

  map_data.regular_update_threads = conf.getIntAttribute("servers.regular_update_threads");
  if ( map_data.regular_update_threads <= 0 )
    throw "Config file: There must be at least one regular update thread";
  if ( map_data.regular_update_threads > MAX_REGULAR_UPDATE_THREADS )
    throw "Config file: Too many regular update threads";
  map_data.world_update_threads = conf.getIntAttribute("servers.world_update_threads");
  if ( map_data.world_update_threads <= 0 )
    throw "Config file: There must be at least one world update thread";
  if ( map_data.world_update_threads > MAX_WORLD_UPDATE_THREADS )
    throw "Config file: Too many world update threads";

  map_data.display_all_warnings = conf.getIntAttribute("display.all_warnings");
  map_data.display_quests = conf.getIntAttribute("display.quests");
  map_data.display_actions = conf.getIntAttribute("display.actions");
  map_data.display_user_on_off = conf.getIntAttribute("display.user_on_off");
  map_data.display_migrations = conf.getIntAttribute("display.migrations");

  /* master */
  strcpy(map_data.algorithm_name, conf.getAttribute("master.balance"));
  map_data.overloaded_level = conf.getFloatAttribute("master.overloaded_level");
  if ( map_data.overloaded_level <= 1.0 )
  {
    printf("[WARNING] Config file error: overloaded_level must be greater than 1. "
      "Default value 1.2 used\n");
    map_data.overloaded_level = 1.2;
  }
  map_data.light_level = conf.getFloatAttribute("master.light_level");
  if ( map_data.light_level <= 0
    || map_data.light_level > map_data.overloaded_level )
  {
    printf("[WARNING] Config file error: light_level must be greater than 0 and "
      "smaller than overloaded_level. Default value 1.0 used\n");
    map_data.light_level = 1.0;
  }

  /* Map and region size */
  map_data.mapx = conf.getIntAttribute("map.width");
  map_data.mapy = conf.getIntAttribute("map.height");
  map_data.regx = conf.getIntAttribute("map.region_width");
  map_data.regy = conf.getIntAttribute("map.region_height");
  map_data.nregx  = map_data.mapx / map_data.regx;
  map_data.nregy  = map_data.mapy / map_data.regy;

  /* verify map data */
  if ( map_data.mapx <= 0 || map_data.mapy <= 0 )
    throw "Config file: Invalid map dimensions";
  if ( map_data.regx <= 0 || map_data.regy <= 0 )
    throw "Config file: Invalid region dimensions";
  if ( map_data.mapx % map_data.regx != 0 || map_data.mapy % map_data.regy != 0 )
    throw "Config file: Map dimension does not divide by region dimension";

  /* Region data */
  map_data.blocks   = conf.getIntAttribute("map.blocks");
  map_data.resources  = conf.getIntAttribute("map.resources");
  map_data.min_res  = conf.getIntAttribute("map.min_res");
  map_data.max_res  = conf.getIntAttribute("map.max_res");

  /* Player data */
  map_data.player_min_life = conf.getIntAttribute("player.min_life");
  map_data.player_max_life = conf.getIntAttribute("player.max_life");
  map_data.player_min_attr = conf.getIntAttribute("player.min_attr");
  map_data.player_max_attr = conf.getIntAttribute("player.max_attr");

  /* Quest data */
  map_data.quest_first  = conf.getIntAttribute("quest.first");
  map_data.quest_min  = conf.getIntAttribute("quest.min");
  map_data.quest_max  = conf.getIntAttribute("quest.max");
  map_data.quest_bonus  = conf.getIntAttribute("quest.bonus");
  map_data.quest_between  = conf.getIntAttribute("quest.between");
  if ( map_data.quest_first <= 0
    || map_data.quest_min <= 0 || map_data.quest_max <= map_data.quest_min
    || map_data.quest_bonus < 0 || map_data.quest_bonus > 100
    || map_data.quest_between <= 0 )
    throw "Invalid or incomplete values for quest properties";
}

void
startPeer (Configurator& conf)
{
  pid_t child_id = fork ();

  if (child_id > 0)
    return;

  if (child_id == 0) 
  {
    int ret = execl ("peer", 
        "peer", 
        ("localhost:"+ conf.getStringAttribute("master.server_port")).c_str(), 
        "localhost:5000", 
        NULL);
    if (ret < 0)
    {
      std::perror("Could not execl peer process");
      exit (1);
    }
  }
}

/***************************************************************************************************
*
* Main thread
*
***************************************************************************************************/

int main( int argc, char *argv[] )
{
  int port;
  MasterModule *m;
  Configurator conf;
  MapData map_data;
  char *log_file = NULL;
  char *config_file = NULL;

  try
  {
    /* set the seed for rand */
    srand((unsigned int)time(NULL));

    /* parse command line and configuration file */
    if (argc == 1) {
      port        = atoi (DEFAULT_MASTER_PORT);
      config_file = strdup (DEFAULT_CONFIG_FILE);
    }
    else if (argc == 2) {
      port        = atoi (DEFAULT_MASTER_PORT);
      config_file = strdup (DEFAULT_CONFIG_FILE);
      log_file = argv[1];
    }
    else if (argc == 3) {
      sscanf(argv[2], "%d", &port);
      config_file = strdup (argv[1]);
    }
    else if (argc == 4) {
      sscanf(argv[2], "%d", &port);
      config_file = strdup (argv[1]);
      log_file = strdup (argv[3]);
    }
    else throw 
      "Usage:   master <config_file> <port> [<log_file>]\nmaster log_file\nmaster";
    if ( port <= 0 ) throw "The port number must be greater than 0";

    /* Initialize SDL */
    if ( SDL_Init( SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE ) < 0 )
    {
      printf("Could not initialize SDL: %s\n", SDL_GetError());
      throw "Failed to initialize SDL (SDL_Init)";
    }
    if ( SDLNet_Init() < 0 ) throw "Cannot start SDL_net";

    /* parse configuration file */
    if ( !conf.addFile(config_file) ) throw "Invalid configuration file";
    dataFromConfigurator(map_data, conf); // get data from configuration file
    
    /* create master module */
    char *load_type = conf.getAttribute("master.balance");
    if ( !strcmp(load_type, "static") )
      m = new MasterModule(map_data, port);
    else if ( !strcmp(load_type, "lightest") )
      m = new AlgLightest(map_data, port);
    else if ( !strcmp(load_type, "spread") )
      m = new AlgSpread(map_data, port);
    else if ( !strcmp(load_type, "locality_aware") )
      m = new AlgLocalityAware(map_data, port);
    else throw "Partitioning algorithm not known "
      "(valid algorithms: static,lightest,spread,locality_aware)";
    if ( m == NULL ) throw "Cannot create the main master module";

    /* configure the master module */
    int load_balance_limit = conf.getIntAttribute("master.load_balance_limit");
    if ( load_balance_limit <= 0 )
    {
      printf("[WARNING] Invalid value for load_balance_limit. "
        "Must be greater than zero. Default value used\n");
      load_balance_limit = 10;
    }
    m->setLoadBalanceLimit(load_balance_limit);
    m->setCustomQuests(conf.getAttribute("quest.custom"));
    m->setLogHost(log_file);

    /* run the master module */
    SDL_Thread *master_thread = SDL_CreateThread(module_thread, (void*)m);
    if ( master_thread == NULL ) throw "Cannot create master thread";

    /* create and run the sender module */
    Module *sender_module = new SenderModule(m->getMessageQueue());
    if ( sender_module == NULL ) throw "Cannot create the sender module";
    SDL_Thread *sender_thread = SDL_CreateThread(module_thread, (void*)sender_module);
    if ( sender_thread == NULL ) throw "Cannot create sender thread";

    // fork a peer process
    startPeer (conf); // fork a peer

    /* User input loop (type 'quit' to exit) */
    while ( true )
    {
      char cmd[256];
      scanf("%s", cmd);
      if ( !strcmp(cmd, "exit") || !strcmp(cmd, "quit") || !strcmp(cmd, "q") )
        exit(0);
    }

    /* Close SDL */
    SDLNet_Quit();
    SDL_Quit();

    // Free memory
    // The memory that contains the locations of the config and log files
    if (config_file != NULL)
      free (config_file);
    if (log_file != NULL)
      free (log_file);

  } catch ( const char *err ) {
    /* display errors */
    printf("[ERROR] %s\n", err);
    return -1;
  }

  return 0;
}
