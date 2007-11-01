
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

#ifndef __GAMEOBJECT_H
#define __GAMEOBJECT_H

#include "../utils/Garbage.h"

class GameObject : public Garbage
{
public:
	int quantity;
	int attr;
	int x,y;

public:
	/* constructors */
	GameObject();
	GameObject(char *data);

	/* public methods */
	int objectDataSize();
	char *objectData();	/* serialize player data */
};

#endif
