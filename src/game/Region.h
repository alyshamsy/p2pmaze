
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

#ifndef __REGION_H
#define __REGION_H

#include "GameObject.h"
#include "Player.h"
#include "../utils/Garbage.h"

/***************************************************************************************************
*
* The game map is in fact a matrix. Each cell in the matrix is of type GridElement.
*
***************************************************************************************************/

struct GridElement
{
	GameObject *object;	/* object that can be modified */
	Player *player;		/* player identified by an IP address */
};

/***************************************************************************************************
*
* Region class
* - a region is a rectangular section of the game map.
*
***************************************************************************************************/

class Region : public Garbage
{
public:
	int sizex,sizey;	/* size */
	int px,py;		/* position */
	int number_of_players;
	int number_of_objects;

	char **terrain;		/* 0 - free, 1 - blocked (but can be extended to represent altitude,water ...) */
	GridElement **grid;	/* the matrix for the game map that contains information about non-fixed objects */

	/* synchronization */
	SDL_mutex *mutex;

public:
	/* constructors/destructors */
	Region();
	Region(int x, int y);
	~Region();

	/* public methods */
	void setPosition(int x, int y);
	int getWidth();
	int getHeight();
	int getX();
	int getY();

	/* map generation */
	void generate(int blocks, int resources, int min_res, int max_res);

	/* methods for transfering terrain over the network */
	char *getSerializedTerrain();
	void deserializeTerrain(char *buffer);

	/* region not initialized */
	bool isEmpty() { return terrain == NULL; }

	/* player and object management */
	void addPlayer(Player *p, int x, int y);
	void removePlayer(int x, int y);
	void addPlayerNoSync(Player *p, int x, int y);
	void removePlayerNoSync(int x, int y);
	bool movePlayer(int x, int y, int xd, int yd);
	int getNumberOfPlayers();
	int getNumberOfObjects();

	void lock();
	void unlock();
};

#endif
