
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
#include "AlgLightest.h"

/***************************************************************************************************
*
* Constructor / Destructor
*
***************************************************************************************************/

AlgLightest::AlgLightest(MapData &map_data, int port) : BasicLoadBalance(map_data, port)
{
}

AlgLightest::~AlgLightest()
{
}

/***************************************************************************************************
*
* Main algorithm
*
***************************************************************************************************/

bool AlgLightest::balance()
{
	int s1,s2;
	int nps1,nps2;

	/* find overloaded server (s1) */
	for ( s1 = 0; s1 < nservers; s1++ )
		if ( isServerOverloaded(s1) ) break;
	if ( s1 == nservers ) return false;

	/* find lightest server (s2) */
	s2 = getLightestServer();
	if ( s1 == s2 ) return false;	/* cannot load balance */

	/* select best region to move */
	int x = -1, y = -1, max = -1;
	for ( int i = 0; i < map_data.nregx; i++ )
		for ( int j = 0; j < map_data.nregy; j++ )
		{
			if ( layout[i][j] != s1 ) continue;
			int pl = players_per_region[i][j];
			if ( pl == 0 ) continue;

			nps1 = servers[s1].statistics.number_of_players;
			nps2 = servers[s2].statistics.number_of_players;
			if ( !isOverloaded(s2, nps2 + pl) && pl > max )
			{
				x = i;
				y = j;
				max = pl;
			}

		}
	if ( x < 0 || y < 0 ) return false;	/* no region found */
	printf("Moving region %d,%d (%d) from S%d (%d) to S%d (%d)\n",
		x,y, players_per_region[x][y],
		s1, servers[s1].statistics.number_of_players,
		s2, servers[s2].statistics.number_of_players);

	/* move region */
	servers[s1].statistics.number_of_players -= players_per_region[x][y];
	servers[s2].statistics.number_of_players += players_per_region[x][y];
	moveRegion(x,y, s1,s2);
	return true;
}
