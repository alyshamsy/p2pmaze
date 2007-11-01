
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

#ifndef __MESSAGEMODULEIN_H
#define __MESSAGEMODULEIN_H

#include "MessageQueue.h"
#include "../utils/RateMonitor.h"

class MessageModuleIN : public Module
{
private:
	/* communication data */
	int port;
	volatile bool finished;
	MessageQueue *mqueue;
	SDLNet_SocketSet sset;
	UDPsocket server_socket;

	/* statisctics */
	RateMonitor rm;

public:
	/* constructors/destructors */
	MessageModuleIN(int port);
	MessageModuleIN(UDPsocket sock);
	~MessageModuleIN();

	/* public methods */
	void run();
	void finish();
	MessageQueue *getQueue();
	UDPsocket getUDPsocket();

	/* statistics methods */
	float getBPS();
};

#endif
