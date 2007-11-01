
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
#include "GameObject.h"

GameObject::GameObject()
{
	x = y = 0;
	attr = 0;
	quantity = 0;
}

GameObject::GameObject(char *data)
{
	x	= *( (int*)data + 0 );
	y	= *( (int*)data + 1 );
	attr	= *( (int*)data + 2 );
	quantity= *( (int*)data + 3 );
}

int GameObject::objectDataSize()
{
	return 4 * sizeof(int);
}

char *GameObject::objectData()
{
	char *buffer;

	/* allocate memory */
	buffer = new char[objectDataSize()];
	if ( buffer == NULL ) return NULL;

	/* copy data to buffer */
	*( (int*)buffer + 0 ) = x;
	*( (int*)buffer + 1 ) = y;
	*( (int*)buffer + 2 ) = attr;
	*( (int*)buffer + 3 ) = quantity;

	return buffer;
}
