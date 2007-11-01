
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

#ifndef __REGULAR_UPDATE_MODULE
#define __REGULAR_UPDATE_MODULE

#include "Server.h"

class RegularUpdateModule : public Module
{
protected:
	ServerData *server_data;
	PlayerBucket *bucket;
	MessageQueue *out;
	int rum_id;	/* RegularUpdateModule identifier */
	list<Message*> temp_message_list;

public:
	SDL_barrier *barrier;
	double average_regular_update_interval;
	double average_real_regular_update_interval;

public:
	/* constructors and setup */
	RegularUpdateModule(ServerData *sd, int id);
	void setOutQueue(MessageQueue *out);

	/* main loop */
	void run();
	void updatePlayer(Player *p);
};

#endif

