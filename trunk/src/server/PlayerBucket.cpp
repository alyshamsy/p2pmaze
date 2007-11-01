
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

#include "Server.h"

/***************************************************************************************************
*
* Constructor and destructor
*
***************************************************************************************************/

PlayerBucket::PlayerBucket()
{
	this->total_size = 0;
	lock = SDL_CreateMutex();
	if ( lock == NULL ) throw "Not enouth memory to create lock for bucket";

	first_pos = true;
	last_pos = false;
}

PlayerBucket::~PlayerBucket()
{
	SDL_DestroyMutex(lock);
}

/***************************************************************************************************
*
* Operations
*
***************************************************************************************************/

/*
* bool insert(Player *p)
* - inserts a player in the map
* - returns true if the player was inserted, false otherwise
*/

bool PlayerBucket::insert(Player *p)
{
	int old_size;

	if ( p == NULL ) return false;

	/* lock the bucket */
	SDL_LockMutex(lock);

	/* insert player in bucket */
	old_size = total_size;
	pair<PlayerDicIterator,bool> res = dic.insert( make_pair(p->address,p) );
	if ( res.second ) total_size++;

	/* unlock the bucket */
	SDL_UnlockMutex(lock);

	return old_size < total_size;
}

/*
* bool erase(IPaddress addr)
* - removes a player from the list
* - returns true if the player was in the list and has been removed
*/

bool PlayerBucket::erase(IPaddress addr)
{
	int old_size;

	/* lock the bucket */
	SDL_LockMutex(lock);

	/* remove player from bucket */
	old_size = total_size;
	PlayerDicIterator result = dic.find(addr);
	if ( result != dic.end() )
	{
		if ( it == result ) it++;
		dic.erase(result);
		total_size --;
	}

	/* unlock the bucket */
	SDL_UnlockMutex(lock);

	return old_size > total_size;
}

/*
* Player* find(IPaddress addr)
* - returns a pointer to the player with the given IP address
*/

Player* PlayerBucket::find(IPaddress addr)
{
	PlayerDicIterator result;
	Player* p = NULL;

	/* lock the bucket */
	SDL_LockMutex(lock);

	/* find player in bucket */
	result = dic.find(addr);
	if ( result != dic.end() ) p = result->second;

	/* unlock the bucket */
	SDL_UnlockMutex(lock);

	return p;
}

/*
* void clear()
* - removes all players from this bucket
*/

void PlayerBucket::clear()
{
	SDL_LockMutex(lock);
	dic.clear();
	total_size = 0;
	SDL_UnlockMutex(lock);
}

/*
* bool findValue(Player *p)
* - returns true if a player with the specified pointer is in this bucket
* - searching for the values p is done by iterating through the whole collection
* (this is O(n) and should be modified)
*/

bool PlayerBucket::findValue(Player *p)
{
	/* lock the bucket */
	SDL_LockMutex(lock);

	bool result = false;
	map<IPaddress,Player*,IpComparator>::iterator itb;
	for ( itb = dic.begin(); itb != dic.end(); itb++ )
		if ( itb->second == p )
		{
			result = true;
			break;
		}

	/* unlock the bucket */
	SDL_UnlockMutex(lock);

	return result;
}

/***************************************************************************************************
*
* Iteration
*
***************************************************************************************************/

void PlayerBucket::start()
{
	first_pos = true;
	last_pos = false;
}

Player* PlayerBucket::next()
{
	Player *p = NULL;

	if ( last_pos ) return NULL;
	SDL_LockMutex(lock);

	/* move iterator */
	if ( first_pos )
	{
		it = dic.begin();
		first_pos = false;
	}

	if ( it != dic.end() )
	{
		p = it->second;
		it++;
	} else { last_pos = true; }

	SDL_UnlockMutex(lock);

	return p;
}

/***************************************************************************************************
*
* Copy
*
***************************************************************************************************/

/*
* void copyPlayers(list<Player*> *copy_list)
* - copy all the pointers fron the bucket in the given list
*/

void PlayerBucket::copyPlayers(list<Player*> *copy_list)
{
	/* empty the list */
	if ( copy_list == NULL ) return;
	copy_list->clear();

	/* lock bucket */
	SDL_LockMutex(lock);

	/* copy all elements from this bucket to the list */
	map<IPaddress,Player*,IpComparator>::iterator itb;
	for ( itb = dic.begin(); itb != dic.end(); itb++ )
		copy_list->insert(copy_list->end(), itb->second);

	/* unlock bucket */
	SDL_UnlockMutex(lock);
}
