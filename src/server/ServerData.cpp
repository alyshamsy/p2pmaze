
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

ServerData::ServerData(int nr_threads)
{
	region = NULL;
	layout = NULL;
	trash = new RecycleBin(nr_threads); /* nr_threads = number of active threads */
	if ( trash == NULL ) throw "Not enouth memory for garbage collection list";
	migrating_players.create(1);
}

void ServerData::setOwnIP(char *local_name, int local_port)
{
	/* get own address */
	memset(&own_ip, 0, sizeof(own_ip));
	if ( SDLNet_ResolveHost(&own_ip, local_name, local_port ) )
		throw "Cannot get own host";
}

void ServerData::copyMapData(MapData *md)
{
	this->num_servers = md->num_servers;
	this->stats_interval = md->stats_interval;
	this->regular_update_interval = md->regular_update_interval;
	this->regular_update_threads = md->regular_update_threads;
	this->world_update_threads = md->world_update_threads;

	this->display_all_warnings = md->display_all_warnings;
	this->display_quests = md->display_quests;
	this->display_actions = md->display_actions;
	this->display_user_on_off = md->display_user_on_off;
	this->display_migrations = md->display_migrations;

	this->mapx = md->mapx;
	this->mapy = md->mapy;
	this->nregx = md->nregx;
	this->nregy = md->nregy;
	this->regx = md->regx;
	this->regy = md->regy;

	this->blocks = md->blocks;
	this->resources = md->resources;
	this->min_res = md->min_res;
	this->max_res = md->max_res;

	this->player_min_life = md->player_min_life;
	this->player_max_life = md->player_max_life;
	this->player_min_attr = md->player_min_attr;
	this->player_max_attr = md->player_max_attr;

	this->quest_first = md->quest_first;
	this->quest_between = md->quest_between;
	this->quest_min = md->quest_min;
	this->quest_max = md->quest_max;
	this->quest_bonus = md->quest_bonus;
}

void ServerData::alocateMemory()
{
	int i,j;

	/* allocate memory for player_list */
	player_list.create(regular_update_threads);

	/* allocate memory for regions */
	region = new Region**[nregx];
	if ( region == NULL ) throw "Cannot create ServerData (not enough memory)";
	for ( i = 0; i < nregx; i++ )
	{
		region[i] = new Region*[nregy];
		if ( region[i] == NULL ) throw "Cannot create ServerData (not enough memory)";
		for ( j = 0; j < nregy; j++ ) region[i][j] = NULL;
	}

	/* allocate space for IP-s */
	layout = new IPaddress*[nregx];
	if ( layout == NULL ) throw "Cannot create ServerData (not enough memory)";
	for ( i = 0; i < nregx; i++ )
	{
		layout[i] = new IPaddress[nregy];
		if ( layout[i] == NULL ) throw "Cannot create ServerData (not enough memory)";
	}
}

void ServerData::generateRegions()
{
	/* generate all regions that belong to this server */
	for ( int i = 0; i < nregx; i++ )
		for ( int j = 0; j < nregy; j++ )
			if ( equalIP(own_ip, layout[i][j]) )
			{
				region[i][j] = new Region(regx,regy);
				if ( region[i][j] == NULL )
					throw "Cannot generate region (not enough memory)";
				region[i][j]->generate(blocks,resources, min_res,max_res);
				region[i][j]->setPosition(i,j);
			}
}

ServerData::~ServerData()
{
	/* free player_list */
	player_list.clear();

	/* free regions and addresses */
	for ( int i = 0; i < nregx; i++ )
	{
		for ( int j = 0; j < nregy; j++ )
			if ( region[i][j] != NULL ) delete region[i][j];
		delete[] region[i];
		delete[] layout[i];
	}
	delete[] region;
	delete[] layout;
}

void ServerData::getNewPosition(int *x, int *y)
{
	int rx,ry;

	if ( x == NULL || y == NULL ) return;

	/* first select a region that belongs to this server
	   ( perhaps it should select any region and send a migrate message
	   if the region belongs to another server ) */
	do
	{
		rx = rand() % nregx;
		ry = rand() % nregy;
	} while ( region[rx][ry] == NULL );

	/* then select new coodinates inside this region */
	Region *r = region[rx][ry];
	do
	{
		*x = rand() % r->getWidth();
		*y = rand() % r->getHeight();
	} while ( r->terrain[*x][*y] != 0
		|| r->grid[*x][*y].object != NULL
		|| r->grid[*x][*y].player != NULL );
		/* while selected cell is not empty */

	*x += rx * r->getWidth();
	*y += ry * r->getHeight();
}

void ServerData::getRandomRegion(int *x, int *y)
{
	if ( x == NULL || y == NULL ) return;
	do
	{
		*x = rand() % nregx;
		*y = rand() % nregy;
	} while ( region[*x][*y] == NULL );
}

bool ServerData::notifyClientOfMigration(Player *p, MessageQueue *out)
{
	/* notify client */
	Message *m = new Message(MESSAGE_SC_OK_MIGRATE);
	if ( m == NULL )
	{

		printf("[WARNING] Cannot notify player about migration. Player lost\n");
		return false;
	}
	m->setAddress(p->address);
	out->putPriorityMessage(m);
	return true;
}

void ServerData::setNumberOfThreads(int x)
{
	if (trash != NULL ) trash->setNumberOfThreads(x);
}
