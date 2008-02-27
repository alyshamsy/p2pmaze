
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

#include "Master.h"
#include "SenderModule.h"

/***************************************************************************************************
*
* Constructor
*
***************************************************************************************************/

SenderModule::SenderModule(MessageQueue *message_queue)
{
	this->message_queue = message_queue;
}

/***************************************************************************************************
*
* Main loop
* - Takes a message from the queue, sends it and deletes it.
*
***************************************************************************************************/

void SenderModule::run()
{
	while ( true )
	{
		Message *m = message_queue->getMessage();
		if ( m == NULL ) continue;
		sendMessage(m);
		delete m;
	}
}

/***************************************************************************************************
*
* Message for sending messages
*
***************************************************************************************************/

void SenderModule::sendMessage(Message *m)
{
	Uint32 message_type = (Uint32)(m->getType());
	MasterMessageWithBuffer *mb = NULL;
	MasterMessageXY *mxy = NULL;
	MasterMessageXYServer *mxys = NULL;
	MasterMessage *mm = NULL;
	IPaddress *ip = NULL;

	switch ( message_type )
	{
		case MS_TAKE_PLAYER:
		case MS_TAKE_REGION:
			mb = (MasterMessageWithBuffer*)m;
			ip = SDLNet_TCP_GetPeerAddress(mb->sock);
			SDLNet_TCP_Send2(mb->sock, &message_type, sizeof(Uint32));
			SDLNet_TCP_Send2(mb->sock, &(mb->len), sizeof(Uint32));
			SDLNet_TCP_Send2(mb->sock, mb->buffer, mb->len);
			break;

		case MS_START_QUEST:
			mxy = (MasterMessageXY*)m;
			SDLNet_TCP_Send2(mxy->sock, &message_type, sizeof(Uint32));
			SDLNet_TCP_Send2(mxy->sock, &(mxy->x), sizeof(int));
			SDLNet_TCP_Send2(mxy->sock, &(mxy->y), sizeof(int));
			break;

		case MS_GIVE_REGION:
		case MS_MOVING_REGION:
			mxys = (MasterMessageXYServer*)m;
			SDLNet_TCP_Send2(mxys->sock, &message_type, sizeof(Uint32));
			SDLNet_TCP_Send2(mxys->sock, &(mxys->x), sizeof(int));
			SDLNet_TCP_Send2(mxys->sock, &(mxys->y), sizeof(int));
			SDLNet_TCP_Send2(mxys->sock, &(mxys->addr), sizeof(IPaddress));
			break;

		default:
			mm = (MasterMessage*)m;
			SDLNet_TCP_Send2(mm->sock, &message_type, sizeof(Uint32));
			break;
	}
}

/***************************************************************************************************
*
* Sending data over a TCP connection with error checking
*
***************************************************************************************************/

inline int SenderModule::SDLNet_TCP_Send2(TCPsocket sock, void *data, int len)
{
	int result = SDLNet_TCP_Send(sock, data, len);
	if ( result < len )
	{
		printf("Sending %d bytes on TCP connection\n", len);
		throw "Error sending message to server";
	}
	return result;
}
