
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

#ifndef __PERIODIC_EVENTS_MODULE_H
#define __PERIODIC_EVENTS_MODULE_H

#include "Server.h"

class PeriodicEventsModule : public Module
{
protected:
	ServerData *server_data;
	MessageQueue *out;

public:
	/* constructors and setup */
	PeriodicEventsModule(ServerData *sd);
	void setOutQueue(MessageQueue *out);

	/* main loop */
	void run();

	void checkQuests();
	void bonusToPlayers( int x, int y, int bonus );
	void regenerateResources();
	void cleanRegion(Region *r);
};

#endif
