
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

#include "General.h"
#include "Region.h"

/***************************************************************************************************
*
* Constructors / Destructor
*
***************************************************************************************************/

Region::Region()
{
	sizex = sizey = 0;
	px = py = 0;
	grid = NULL;
	terrain = NULL;
	number_of_players = 0;
	number_of_objects = 0;
	mutex = SDL_CreateMutex();
}

Region::Region(int x, int y)
{
	int i,j;

	/* allocate matrices */
	grid = new GridElement*[x];
	terrain = new char*[x];
	if ( grid == NULL || terrain == NULL )
		throw "Cannot create region (not enough memory)";

	/* allocate each row in matrices */
	for ( i = 0; i < x; i++ )
	{
		grid[i] = new GridElement[y];
		terrain[i] = new char[y];
		if ( grid[i] == NULL || terrain[i] == NULL )
			throw "Cannot create region (not enough memory)";

		/* fill default data */
		for ( j = 0; j < y; j++ )
		{
			terrain[i][j] = 0;
			grid[i][j].object = NULL;
			grid[i][j].player = NULL;
		}
	}

	/* set region parameters */
	sizex = x;
	sizey = y;
	px = py = 0;
	number_of_objects = 0;
	number_of_players = 0;
	mutex = SDL_CreateMutex();
}

Region::~Region()
{
	if ( isEmpty() ) return;

	/* free the terrain */
	for ( int i = 0; i < sizex; i++ ) delete[] terrain[i];
	delete[] terrain;

	/* free the grid */
	for ( int i = 0; i < sizex; i++ ) delete[] grid[i];
	delete[] grid;

	/* destroy mutex */
	if ( mutex != NULL ) SDL_DestroyMutex(mutex);
}

/***************************************************************************************************
*
* Parameters get/set
*
***************************************************************************************************/

void Region::setPosition(int x, int y)
{
	px = x;
	py = y;
}

int Region::getWidth()
{
	return sizex;
}

int Region::getHeight()
{
	return sizey;
}

int Region::getX()
{
	return py;
}

int Region::getY()
{
	return px;
}

/***************************************************************************************************
*
* Generate region data
*
***************************************************************************************************/

void Region::generate(int blocks, int resources, int min_res, int max_res)
/*
	blocks - the number of blocked cells from 1000 cells
	resources - the number of resources in a region
	min_res, max_res - the minimun and maximum quantity a resource can have
*/
{
	int i,j;

	/* generate terrain */
	for ( i = 0; i < sizex; i++ )
		for ( j = 0; j < sizey; j++ )
			if ( rand() % 1000 < blocks )
				terrain[i][j] = (char)(rand() % 100);
			else
				terrain[i][j] = 0;

	/* generate objects */
	for ( i = 0; i < resources; i++ )
	{
		GameObject *o = new GameObject();
		if ( o == NULL ) continue;
		do
		{
			o->x = rand() % sizex;
			o->y = rand() % sizey;
		} while ( terrain[o->x][o->y] != 0 ||
			grid[o->x][o->y].object != NULL );
		o->attr = rand() % 256;
		o->quantity = min_res + rand() % (max_res - min_res + 1);
		grid[o->x][o->y].object = o;
		number_of_objects++;
	}
}

/***************************************************************************************************
*
* Serialization
*
***************************************************************************************************/

char *Region::getSerializedTerrain()
{
	int i;
	char *buffer,*ptr;

	/* allocate the buffer to hold the terrain data */
	buffer = new char[sizex * sizey];
	if ( buffer == NULL ) throw "Cannot serialize terrain (not enough memory)";

	/* copy terrain information in the buffer */
	for ( ptr = buffer, i = 0; i < sizex; ptr += sizey, i++ )
		memcpy(ptr, terrain[i], sizey);

	return buffer;
}

void Region::deserializeTerrain(char *buffer)
/* the buffer must contain all data, no checks performed */
{
	int i;
	char *ptr;

	for ( ptr = buffer, i = 0; i < sizex; ptr += sizey, i++ )
		memcpy(terrain[i], ptr, sizey);
}

/***************************************************************************************************
*
* Player and object management
*
***************************************************************************************************/

void Region::addPlayer(Player *p, int x, int y)
{
	lock();
	grid[x][y].player = p;
	unlock();
}

void Region::removePlayer(int x, int y)
{
	lock();
	grid[x][y].player = NULL;
	unlock();
}

void Region::addPlayerNoSync(Player *p, int x, int y)
{
	grid[x][y].player = p;
}

void Region::removePlayerNoSync(int x, int y)
{
	grid[x][y].player = NULL;
}

bool Region::movePlayer(int x, int y, int xd, int yd)
{
	bool result;

	lock();
	if ( grid[xd][yd].player != NULL )
	{
		grid[xd][yd].player = grid[x][y].player;
		grid[x][y].player = NULL;
		result = true;
	} else result = false;
	unlock();

	return result;
}

int Region::getNumberOfPlayers()
{
	lock();
	int np = 0;
	for ( int i = 0; i < sizex; i++ )
		for ( int j = 0; j < sizey; j++ )
			if ( grid[i][j].player != NULL )
				np++;
	unlock();
	return np;

}

int Region::getNumberOfObjects()
{
	return number_of_objects;
}

/***************************************************************************************************
*
* Region synchronization
*
***************************************************************************************************/

void Region::lock()
{
	SDL_LockMutex(mutex);
}

void Region::unlock()
{
	SDL_UnlockMutex(mutex);
}
