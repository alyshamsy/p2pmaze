
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

#ifndef __WORLD_UPDATE_MODULE
#define __WORLD_UPDATE_MODULE

#include "Server.h"
#include "MapManagModule.h"

class WorldUpdateModule : public Module
{
protected:
	/* general data */
	ServerData *server_data;
	MessageQueue *in,*out;
	MapManagModule *mm_module;

	/* data for generating new player */
	int min_life,max_life;
	int min_attr,max_attr;

public:
	/* Constructor and setup methods */
	WorldUpdateModule(ServerData *sd);
	void setInQueue(MessageQueue *in);
	void setOutQueue(MessageQueue *out);
	void setMapManagModule(MapManagModule *m);

	/* main loop */
	void run();

	/* message handlers */
	void handleClientJoinRequest(Message *m);
	void handleClientLeaveRequest(Message *m);
	void handle_ACK_OK_MIGRATE(Message *m);
	void handle_REQUEST_CLIENT_UPDATE(Message *m);
	void handle_ANSWER_CLIENT_UPDATE(Message *m);

	/* handler for client actions */
protected:
	void handle_move(Player *p, int new_x, int new_y);
public:
	void handle_MOVE_UP(Message *m);
	void handle_MOVE_DOWN(Message *m);
	void handle_MOVE_LEFT(Message *m);
	void handle_MOVE_RIGHT(Message *m);
	void handle_USE(Message *m);
	void handle_ATTACK(Message *m);
};

#endif
