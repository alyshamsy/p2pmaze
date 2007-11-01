
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

#include "Master.h"
#include "AlgSpread.h"

/***************************************************************************************************
*
* Constructor / Destructor
*
***************************************************************************************************/

AlgSpread::AlgSpread(MapData &map_data, int port) : BasicLoadBalance(map_data, port)
{
	/* allocate memory for layout matrix */
	new_layout = new int*[map_data.nregx];
	if ( new_layout == NULL )
		throw "Cannot start AlgSpread because not enough memory";
	for ( int i = 0; i < map_data.nregx; i++ )
	{
		new_layout[i] = new int[map_data.nregy];
		if ( new_layout[i] == NULL )
			throw "Cannot start AlgSpread because not enough memory";
	}

	/* allocate memory for region vector */
	sorted_regions = new SpRegion[map_data.nregx * map_data.nregy];
	if ( sorted_regions == NULL ) throw "Cannot start AlgSpread because not enough memory";
}

AlgSpread::~AlgSpread()
{
	for ( int i = 0; i < map_data.nregx; i++ )
		delete[] new_layout[i];
	delete[] new_layout;
	delete[] sorted_regions;
}

/***************************************************************************************************
*
* Region comparator ( used for qsort )
*
***************************************************************************************************/

int region_comparator(const void *o1, const void *o2)
{
	SpRegion *r1 = (SpRegion*)o1;
	SpRegion *r2 = (SpRegion*)o2;
	return r2->p - r1->p;
}


/***************************************************************************************************
*
* getLightestServer overload
*
***************************************************************************************************/

/*
* int getLightestServer()
* - This method finds the lightest server using the current values for the number of players
*   on each server
* - The getLightestServer from the base BasicLoadBalance class uses the last known real values
*   for the number of players on each server
*/

int AlgSpread::getLightestServer()
{
	double k,pps = serverMachineFactor(0) * players_per_server[0];
	int ls = 0;

	for ( int i = 1; i < nservers; i++ )	/* for all servers */
	{
		k = serverMachineFactor(i) * players_per_server[i];
		if ( pps > k )	/* less players than min */
		{
			pps = k;
			ls = i;		/* set this server as lightest */
		}
	}

	return ls;
}

/***************************************************************************************************
*
* Main algorithm implementation
*
***************************************************************************************************/

bool AlgSpread::balance()
{
	int i,j,k,r;
	int nregions;
	int s,pm;

	/* trigger condition ( at least one server is overloaded ) */
	for ( i = 0; i < nservers; i++ )
		if ( isServerOverloaded(i) ) break;
	if ( i == nservers ) return false;
	if ( servers[i].statistics.number_of_players == 0 ) return false;

	/* initialize algorithm data */
	for ( i = 0; i < MAX_SERVERS; i++ ) players_per_server[i] = 0;

	/* addd region to list, then sort */
	k = 0;
	nregions = map_data.nregx * map_data.nregy;
	for ( i = 0; i < map_data.nregx; i++ )	/* for all regions */
		for ( j = 0; j < map_data.nregy; j++ )
		{
			sorted_regions[k].x = i;
			sorted_regions[k].y = j;
			sorted_regions[k].p = players_per_region[i][j];
			k++;
		}
	qsort(sorted_regions, nregions, sizeof(SpRegion), region_comparator);

	/* most loaded region remains on its server */
	r = 0;
	i = sorted_regions[r].x;
	j = sorted_regions[r].y;
	new_layout[i][j] = layout[i][j];
	players_per_server[layout[i][j]] = players_per_region[i][j];

	/* reshuffle */
	for ( r = 1; r < nregions; r++ )	/* from the most loaded to the lightest */
	{
		/* select current region */
		i = sorted_regions[r].x;
		j = sorted_regions[r].y;

		/* don't move region with no players */
		if ( players_per_region[i][j] == 0 )
		{
			new_layout[i][j] = layout[i][j];
			continue;
		}

		/* choose the lightest server */
		s = 0;
		pm = players_per_server[s];
		for ( k = 1; k < nservers; k++ )
			if ( pm > players_per_server[k] )
			{
				s = k;
				pm = players_per_server[k];
			}

		/* give that region to the lightest server */
		new_layout[i][j] = s;
		players_per_server[s] += players_per_region[i][j];
	}

	/* apply the new layout */
	for ( i = 0; i < map_data.nregx; i++ )	/* for all regions */
		for ( j = 0; j < map_data.nregy; j++ )
			if ( layout[i][j] != new_layout[i][j] )	/* if region should be moved */
				moveRegion(i,j, layout[i][j], new_layout[i][j]);

	return false;
}
