
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
#include "Player.h"

Player::Player(IPaddress adr)
{
	int i;
	memset(name, 0, MAX_PLAYER_NAME);
	for ( i = 0; i < 8; i++ ) name[i] = 'a' + rand() % ('z'-'a');
	sprintf( name + i, "_%X_%d", adr.host, adr.port);
	memset(&address, 0, sizeof(address));
	address.host = adr.host;
	address.port = adr.port;

	x = y = 0;
	life = 0;
	attr = 0;
	dir = 0;
	count = 0;
	mutex = SDL_CreateMutex();
}

Player::Player(char *data)
{
	char *ptr;
	mutex = SDL_CreateMutex();
	count = 0;

	/* copy player name */
	memset(name, 0, MAX_PLAYER_NAME);
	memcpy(name, data, MAX_PLAYER_NAME);

	/* copy player address */
	ptr = data + MAX_PLAYER_NAME;
	memset(&address, 0, sizeof(address));
	address.host = ((IPaddress*)ptr)->host;
	address.port = ((IPaddress*)ptr)->port;

	/* copy attributes */
	ptr += sizeof(IPaddress);
	x	= *( (int*)ptr + 0 );
	y	= *( (int*)ptr + 1 );
	life	= *( (int*)ptr + 2 );
	attr	= *( (int*)ptr + 3 );
	dir	= *( (int*)ptr + 4 );
}

Player::~Player()
{
	if ( mutex != NULL ) SDL_DestroyMutex(mutex);
}

int Player::playerDataSize()
{
	return MAX_PLAYER_NAME + sizeof(IPaddress) + sizeof(int)*5;
}

char *Player::playerData()
{
	char *buffer,*ptr;

	/* alocate buffer */
	buffer = new char[playerDataSize()];
	if ( buffer == NULL ) return NULL;

	/* copy data into the buffer */
	memcpy(buffer, name, MAX_PLAYER_NAME);
	ptr = buffer + MAX_PLAYER_NAME;
	memcpy( (IPaddress*)ptr, &address, sizeof(IPaddress) );
	ptr += sizeof(IPaddress);
	*( (int*)ptr + 0 ) = x;
	*( (int*)ptr + 1 ) = y;
	*( (int*)ptr + 2 ) = life;
	*( (int*)ptr + 3 ) = attr;
	*( (int*)ptr + 4 ) = dir;

	return buffer;
}

void Player::setName(char *name)
{
	strncpy(this->name, name, MAX_PLAYER_NAME);
}

void Player::lock()
{
	SDL_LockMutex(mutex);
}

void Player::unlock()
{
	SDL_UnlockMutex(mutex);
}
