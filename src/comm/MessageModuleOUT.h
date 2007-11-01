
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

#ifndef __MESSAGEMODULEOUT_H
#define __MESSAGEMODULEOUT_H

#include "MessageQueue.h"
#include "../utils/RateMonitor.h"

class MessageModuleOUT : public Module
{
protected:
	/* communication data */
	bool finished;
	MessageQueue *mqueue;
	UDPsocket out_socket;

	/* statisctics */
	RateMonitor rm;

public:
	/* constructors/destructors */
	MessageModuleOUT();
	MessageModuleOUT(UDPsocket sock);
	virtual ~MessageModuleOUT();

	/* public methods */
	virtual void run();
	void finish();
	virtual MessageQueue *getQueue();
	UDPsocket getUDPsocket();
	/* statistics methods */
	float getBPS();
};

#endif
