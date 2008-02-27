
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
#include "AlgLocalityAware.h"

/***************************************************************************************************
*
* Constructor / Destructor
*
***************************************************************************************************/

AlgLocalityAware::AlgLocalityAware(MapData &map_data, int port) : BasicLoadBalance(map_data, port)
{
	/* Allocate memory for region matrices */
	new_layout = new int*[map_data.nregx];
	visited_nodes = new bool*[map_data.nregx];
	if ( new_layout == NULL || visited_nodes == NULL )
		throw "Cannot start AlgLocalityAware because not enough memory";
	for ( int i = 0; i < map_data.nregx; i++ )
	{
		new_layout[i] = new int[map_data.nregy];
		visited_nodes[i] = new bool[map_data.nregy];
		if ( new_layout[i] == NULL || visited_nodes[i] == NULL )
			throw "Cannot start AlgLocalityAware because not enough memory";
	}

	/* Allocate node queue (for BFS) */
	node_list = new int[map_data.nregx * map_data.nregy];
	if ( node_list == NULL )
		throw "Cannot start AlgLocalityAware because not enough memory";
}

AlgLocalityAware::~AlgLocalityAware()
{
	for ( int i = 0; i < map_data.nregx; i++ )
	{
		delete[] new_layout[i];
		delete[] visited_nodes[i];
	}
	delete[] new_layout;
	delete[] visited_nodes;
	delete[] node_list;
}

/***************************************************************************************************
*
* Utility methods
*
***************************************************************************************************/

/* verify that a region is not out of bounds */
#define isRegionOk(x,y) ( (x) >= 0 && (x) < map_data.nregx && (y) >= 0 && (y) < map_data.nregy )

/* compares the number of players owned by the server with this region to previos servers */
#define compareRegion(x,y) \
	if ( players_per_server[layout[(x)][(y)]] < ps2 )	\
	{ \
		s2 = layout[(x)][(y)]; \
		ps2 = players_per_server[s2]; \
	}

/*
* int getLightestNeigbor(int s1)
* - For all regions on this server, select all neighbors and see which one of them is the lightest
*/
int AlgLocalityAware::getLightestNeigbor(int s1)
{
	int s2,ps2;
	int i,j;

	/* find lightest neighbor */
	s2 = -1;
	ps2 = getNumberOfPlayers() + 1;
	for ( i = 0; i < map_data.nregx; i++ )	/* for all regions from this server */
		for ( j = 0; j < map_data.nregy; j++ )
			if ( new_layout[i][j] == s1 )
			{
				if ( isRegionOk(i-1,j) ) compareRegion(i-1,j);
				if ( isRegionOk(i+1,j) ) compareRegion(i+1,j);
				if ( isRegionOk(i,j-1) ) compareRegion(i,j-1);
				if ( isRegionOk(i,j+1) ) compareRegion(i,j+1);
			}
	return s2;
}

/*
* int stronglyConnectedComponents()
* - Computes the number of strongly connected components in the graph defined by the region
*   layout matrix. Matriox cells are node and if two neighboring cells have the same value then
*   there is an arc between them.
* - Uses breadth first search.
*/

int AlgLocalityAware::stronglyConnectedComponents()
{
	int p,q,n;
	int i,j = 0;
	int x,y,x2,y2;
	int scc;
	int dxp[4] = { -1, 1, 0, 0 };
	int dyp[4] = { 0, 0, -1, 1 };

	/* init data */
	n = map_data.nregx * map_data.nregy;
	for ( i = 0; i < map_data.nregx; i++ )
		for ( j = 0; j < map_data.nregy; j++ )
			visited_nodes[i][j] = 0;
	p = q = 0;
	scc = 0;
	node_list[0] = 0;
	visited_nodes[0][0] = true;

	/* while there are still nodes */
	while ( p < n )
	{
		/* inside a strongly connected component */
		while ( p <= q )
		{
			/* select current node */
			x = node_list[p] / map_data.nregy;
			y = node_list[p] % map_data.nregy;
			p++;

			/* for all neighbors */
			for ( i = 0; i < 4; i++ )
			{
				/* get neighbor */
				x2 = x + dxp[i];
				y2 = y + dyp[i];

				/* verify neighbor */
				if ( !isRegionOk(x2,y2) ) continue;
				if ( new_layout[x2][y2] != new_layout[x][y] ) continue;
				if ( visited_nodes[x2][y2] ) continue;

				/* add neighbor */
				visited_nodes[x2][y2] = true;
				q++;
				node_list[q] = x * map_data.nregy + y;
			}
		}

		/* count number of components */
		scc++;

		/* if there are still components */
		if ( p < n )
		{
			/* find an unvisited node */
			x = y = -1;
			for ( i = 0; i < map_data.nregx; i++ )
			{
				for ( j = 0; j < map_data.nregy; j++ )
					if ( visited_nodes[i][j] == false )
					{
						x = i;
						y = j;
						break;
					}
				if ( x != -1 || y != -1 ) break;
			}

			/* add new node */
			q++;
			node_list[q] = x * map_data.nregy + y;
			visited_nodes[x][y] = true;
		}
	}

	return scc;
}

/*
* compareComponents(x,y) - macro
* - Compares the number of strongly connected components obtained by moving this region is
*   less than previos results.
*/
#define compareComponents(x,y) \
	if ( new_layout[(x)][(y)] == s2 ) \
	{ \
		new_layout[i][j] = s2; \
		scc = stronglyConnectedComponents(); \
		new_layout[i][j] = s1; \
		if ( scc < min ) \
		{ \
			min = scc; \
			x1 = i; \
			y1 = j; \
		} \
	}

/*
* bool selectBorderingRegion(int s1, int s2, int *x, int *y)
* - We select a region on this servers (s1) border with s2 in such a way that by moving
*   the region to s2 we obtain a minimum number of strongly connected components.
*/

bool AlgLocalityAware::selectBorderingRegion(int s1, int s2, int *x, int *y)
{
	int min,x1,y1;
	int i,j;
	int scc;
	x1 = y1 = 0;

	/* we cannot have more components than this */
	min = map_data.nregx * map_data.nregy;

	/* find best region */
	for ( i = 0; i < map_data.nregx; i++ )	/* for all regions from this server */
		for ( j = 0; j < map_data.nregy; j++ )
			if ( new_layout[i][j] == s1 )
			{
				if ( isRegionOk(i-1,j) ) compareComponents(i-1,j);
				if ( isRegionOk(i+1,j) ) compareComponents(i+1,j);
				if ( isRegionOk(i,j-1) ) compareComponents(i,j-1);
				if ( isRegionOk(i,j+1) ) compareComponents(i,j+1);
			}

	/* error */
	if ( min == map_data.nregx * map_data.nregy ) return false;

	/* return the region */
	*x = x1;
	*y = y1;
	return true;
}

/*
* int getLightestServer()
* - This method finds the lightest server using the current values for the number of players
*   on each server
* - The getLightestServer from the base BasicLoadBalance class uses the last known real values
*   for the number of players on each server
*/

int AlgLocalityAware::getLightestServer()
{
	double k,pps = serverMachineFactor(0) * players_per_server[0];
	int ls = 0;

	for ( int i = 1; i < nservers; i++ )	/* for all servers */
	{
		k = serverMachineFactor(i) * players_per_server[i];
		if ( pps > k )	/* less players than min */
		{
			pps = k;
			ls = i;				/* set this server as lightest */
		}
	}

	return ls;
}

/*
* bool isRegionIsolated(int x, int y, int &neigh)
* - Determines if a region is isolated.
*/

bool AlgLocalityAware::isRegionIsolated(int x, int y, int &neigh)
{
	int s1,i,j;
	int v[4];
	int ns;		/* number of borders with no server (map margin) */
	int n1,n2,min;

	s1 = new_layout[x][y]; /* current server */

	/* put all neigbors in a vector ( -1 for no server ) */
	ns = 0;
	if ( isRegionOk(x-1,y) ) v[0] = new_layout[x-1][y]; else v[0] = -1, ns++;
	if ( isRegionOk(x+1,y) ) v[1] = new_layout[x+1][y]; else v[1] = -1, ns++;
	if ( isRegionOk(x,y-1) ) v[2] = new_layout[x][y-1]; else v[2] = -1, ns++;
	if ( isRegionOk(x,y+1) ) v[3] = new_layout[x][y+1]; else v[3] = -1, ns++;

	/* verify if there is at least one neighbor from the same server */
	for ( i = 0; i < 4; i++ )
		if ( v[i] == s1 ) break;
	if ( i >= 3 )	/* no neighbor from the same server */
	{
		n1 = -1;
		min = getNumberOfPlayers() + 1;
		for ( i = 1; i < 4; i++ )
			if ( v[i] != -1 && min > players_per_server[v[i]] )
			{
				n1 = v[i];
				min = players_per_server[n1];
			}
		if ( n1 != -1 )
		{
			neigh = n1;
			return true;
		}
	}

	/* region is in the corner */
	if ( ns == 2 )
	{
		/* discover neighbor */
		n1 = n2 = -1;
		for ( i = 0; i < 4; i++ )
		{
			if ( v[i] != -1 )
				if ( n1 != -1 ) n2 = v[i];
				else n1 = v[i];
		}

		/* if there is only one neighbor and is different from current server */
		if ( n1 == n2 && n1 != s1 )
		{
			neigh = n1;
			return true;
		}
		return false;
	}

	/* region is on the border */
	if ( ns == 1 )
	{
		/* discover neighbor */
		n1 = n2 = -1;
		for ( i = 0; i < 4; i++ )
			for ( j = i+1; j < 4; j++ )
				if ( v[i] == v[j] )
				{
					n1 = v[i];
					break;
				}

		/* if there is a neighbor with two borders */
		if ( n1 != -1 && n1 != s1 )
		{
			neigh = n1;
			return true;
		}
		return false;
	}

	/* region is not corner */
	ns = 0;
	for ( i = 1; i < 4; i++ )
		if ( v[i] == v[0] ) ns++;
	if ( ns >= 2 )	/* v[0] is neighbor */
	{
		if ( v[0] != s1 )
		{
			neigh = v[0];
			return true;
		}
		return false;
	}
	ns = 0;
	for ( i = 2; i < 4; i++ )
		if ( v[i] == v[1] ) ns++;
	if ( ns >= 2 )	/* v[0] is neighbor */
		if ( v[1] != s1 )
		{
			neigh = v[1];
			return true;
		}

	return false;
}

/*
* int AlgLocalityAware::regionsForServer(int k)
* - returns the number of regions that belong to server k
*/

int AlgLocalityAware::regionsForServer(int k)
{
	int nr = 0,i,j;
	for ( i = 0; i < map_data.nregx; i++ )	/* for all regions from this server */
		for ( j = 0; j < map_data.nregy; j++ )
			if ( new_layout[i][j] == k )
				nr++;
	return nr;
}

/***************************************************************************************************
*
* Algorithm methods
*
***************************************************************************************************/

void AlgLocalityAware::shedLoadToNeighbors(int s1)
{
	int s2,x,y;

	printf("\tShed to neighbors\n");
	while ( isOverloaded(s1, players_per_server[s1]) )
	{
		/* get lightest neighbor */
		s2 = getLightestNeigbor(s1);
		if ( s2 == -1 ) return;
		if ( isOverloaded(s2, players_per_server[s2]) ) return;

		/* while the neighbor is not overloaded */
		printf("\t\tServer S%d is a neighbor and is not overloaded\n", s2);
		while ( isOverloaded(s1, players_per_server[s1])
			&& !isOverloaded(s2, players_per_server[s2]) )
		{
			/* select bordering region (x,y) */
			if ( !selectBorderingRegion(s1,s2, &x,&y) ) return;
			if ( isOverloaded( s2, players_per_server[s2] + players_per_region[x][y] ) )
				return;
			printf("\t\tRegion %d,%d is on the border with S%d\n", x,y, s2);

			/* move region */
			new_layout[x][y] = s2;
			players_per_server[s1] -= players_per_region[x][y];
			players_per_server[s2] += players_per_region[x][y];
		}
	}
}

void AlgLocalityAware::shedLoadToLightest(int s1)
{
	int s2;
	int nps1,nps2;

	printf("Shed to lightest\n");
	while ( isOverloaded(s1, players_per_server[s1]) )
	{
		/* get lightest server */
		s2 = getLightestServer();
		if ( s2 == s1 || isOverloaded(s2, players_per_server[s2]) ) return;
		printf("\t\tServer S%d is the lightest\n", s2);

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
		if ( x < 0 || y < 0 ) return;	/* no region found */
		printf("\t\tRegion %d,%d will be moved\n", x,y);

		/* move region */
		new_layout[x][y] = s2;
		players_per_server[s1] -= players_per_region[x][y];
		players_per_server[s2] += players_per_region[x][y];
	}
}

void AlgLocalityAware::mapAggregation()
{
	int x,y,s1,s2;

	for ( x = 0; x < map_data.nregx; x++ )	/* for all regions from this server */
		for ( y = 0; y < map_data.nregy; y++ )
		{
			/* verify that the region is isolated */
			s1 = new_layout[x][y];
			if ( !isRegionIsolated(x,y, s2) ) continue;
			if ( !isSafe(s2, players_per_server[s2] + players_per_region[x][y]) )
				continue;
			if ( regionsForServer(s1) <= 1 ) return;

			/* move region */
			printf("Moving isolated region %d,%d from S%d to S%d\n", x,y, s1,s2);
			new_layout[x][y] = s2;
			players_per_server[s1] -= players_per_region[x][y];
			players_per_server[s2] += players_per_region[x][y];
	}
}

/***************************************************************************************************
*
* Main algorithm
*
***************************************************************************************************/

bool AlgLocalityAware::balance()
{
	int i,j;

	/* initialize algorithm data */
	for ( i = 0; i < MAX_SERVERS; i++ )
		players_per_server[i] = servers[i].statistics.number_of_players;
	for ( i = 0; i < map_data.nregx; i++ )
		for ( j = 0; j < map_data.nregy; j++ )
			new_layout[i][j] = layout[i][j];

	/* trigger condition ( at least one server is overloaded ) */
	for ( i = 0; i < nservers; i++ )
		if ( isServerOverloaded(i) )
		{
			/* shed load */
			printf("Server S%d is overloaded\n", i);
			if ( servers[i].statistics.number_of_players == 0 ) return false;
			shedLoadToNeighbors(i);
			if ( isOverloaded(i, players_per_server[i]) ) shedLoadToLightest(i);
		}

	/* aggregate regions */
	mapAggregation();


	/* apply the new layout */
	for ( i = 0; i < map_data.nregx; i++ )	/* for all regions */
		for ( j = 0; j < map_data.nregy; j++ )
			if ( layout[i][j] != new_layout[i][j] )
				moveRegion(i,j, layout[i][j], new_layout[i][j]);

	return false;
}
