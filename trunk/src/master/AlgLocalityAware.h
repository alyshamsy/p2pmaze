
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

#ifndef __ALG_LOCALITY_AWARE_H
#define __ALG_LOCALITY_AWARE_H

#include "MasterModule.h"
#include "BasicLoadBalance.h"

/***************************************************************************************************
*
* AlgLocalityAware - MasterModule with spread partitioning algorithm
*
***************************************************************************************************/

class AlgLocalityAware : public BasicLoadBalance
{
private:
	int **new_layout;	/* region layout after the load balancing */
				/* int **layout in MasterModule class holds the current layout */
	int players_per_server[MAX_SERVERS];	/* number of players on each server on each
						step of the algorithm */
	bool **visited_nodes;	/* visited regions (for breadth first search) */
	int *node_list;		/* region queue (for breadth first search) */

public:
	/* Constructor / Destructor */
	AlgLocalityAware(MapData &map_data, int port);
	~AlgLocalityAware();

	/* Algoritm name */
	virtual const char* getAlgorithm() { return "locality aware"; }

	/* Utility methods ( see cpp file for details) */
	virtual int getLightestNeigbor(int s1);
	bool selectBorderingRegion(int s1, int s2, int *x, int *y);
	int stronglyConnectedComponents();
	virtual int getLightestServer();
	bool isRegionIsolated(int x, int y, int &neigh);
	int regionsForServer(int k);

	/* Algoritm implementation */
	void mapAggregation();
	void shedLoadToNeighbors(int s1);
	void shedLoadToLightest(int s1);
	bool balance();
};

#endif
