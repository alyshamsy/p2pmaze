
/***************************************************************************************************
*
* SUBJECT:
*    A Benckmark for Massive Multiplayer Online Games
*    Game Server and Client
*
* AUTHOR:
*    Mihai Paslariu
*    Politehnica University of Bucharest, Romania
*    mihplaesu@yahoo.com
*
* TIME AND PLACE:
*    University of Toronto, Toronto, Canada
*    March - August 2007
*
***************************************************************************************************/

#ifndef __STRUCTS_H
#define __STRUCTS_H

/***************************************************************************************************
*
* MapData class
* ( used as a structure )
*
***************************************************************************************************/

/* set alignment to 8 bytes so it will work well on both x86 and x86_64 */
/* more details at http://en.wikipedia.org/wiki/Byte_alignment */
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(4)     /* set alignment to 8 byte boundary */

class MapData
{
public:
	/* servers */
	int num_servers;		/* number of servers */
	int stats_interval;		/* interval for receiving statistics from servers */
	int regular_update_interval;	/* maximum time between two consecutive client updates */
	int regular_update_threads;	/* number of server threads */
	int world_update_threads;

	/* load-balance */
	char algorithm_name[64];	/* name of the load balancing algorithm */
	double overloaded_level;	/* overloaded and light server level */
	double light_level;

	/* messages to display */
	int display_all_warnings;	/* to display all warning messages */
	int display_quests;
	int display_actions;
	int display_user_on_off;
	int display_migrations;

	/* map and region */
	int mapx,mapy;			/* map dimensions */
	int nregx,nregy;		/* number of regions horizontaly,verticaly */
	int regx,regy;			/* size of regions */

	/* region properties */
	int blocks;			/* the number of blocked cells from 1000 cells */
	int resources;			/* the number of resources in a region */
	int min_res,max_res;		/* the minimun and maximum quantity
					a resource can have (min 1, max 10) */

	/* players */
	/* (values are between 1 and 100, exept for max_life which is between 41 and 100) */
	int player_min_life;
	int player_max_life;
	int player_min_attr;
	int player_max_attr;

	/* quests */
	int quest_first;		/* time until the first quest occurs */
	int quest_between;		/* maximum time between quests in seconds */
	int quest_min, quest_max;	/* the minimum and maximum duration of quests in seconds */
	int quest_bonus;		/* the bonus given for quests */
};

/***************************************************************************************************
*
* ServerStatistics
* - information about the load on each server
*
***************************************************************************************************/

struct ServerStatistics
{
	int number_of_regions;
	int number_of_players;
	int players_in_most_crowded_region;;
	int machine_cpu_usage;
	int machine_mem_usage;
	int process_cpu_usage;
	int process_mem_usage;
	int number_of_threads;
	double average_regular_update_interval;
	double average_real_regular_update_interval;
	double bps_tcp_recv,bps_tcp_sent;
	double bps_udp_recv,bps_udp_sent;
	double tcp_total,udp_total;
	int number_of_statistics;
};

/***************************************************************************************************
*
* MasterStatistics
* - information about player and region migration
*
***************************************************************************************************/

struct MasterStatistics
{
	int player_migrations;
	int region_migrations;
};

/***************************************************************************************************
*
* ConnectedServerInfo
* - used by the master to hold information about servers
*
***************************************************************************************************/

struct ConnectedServerInfo
{
	IPaddress tcp_connection;
	IPaddress udp_connection;
	ServerStatistics statistics;
};


/***************************************************************************************************
*
* QuestPoint
* - used by the master to hold a list of quests
*
***************************************************************************************************/

struct QuestPoint
{
	int x,y;
	int duration;
};

#pragma pack(pop)   /* restore original alignment from stack */

#endif
