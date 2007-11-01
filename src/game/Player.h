
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

#ifndef __PLAYER_H
#define __PLAYER_H

#include "../utils/Garbage.h"

class Player : public Garbage
{
public:
	/* player identification */
	char name[MAX_PLAYER_NAME];
	IPaddress address;

	/* player game state */
	int x,y;	/* position */
	int life,attr;	/* attributes */
	int dir;	/* direction (0,1,2,3) = (up,right,down,left) */

	/* synchronization */
	SDL_mutex *mutex;

	/* parameter user in operations requiring retries (??english!) */
	int count;

public: /* constructors / destructor */
	Player(IPaddress adr);
	Player(char *data);
	~Player();

public: /* public methods */
	char *playerData();	/* serialize player data */
	int playerDataSize();
	void setName(char *name);

	void lock();
	void unlock();
};

#endif
