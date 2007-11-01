
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
#include "MessageModuleIN.h"

/***************************************************************************************************
*
* Constructor
*
***************************************************************************************************/

MessageModuleIN::MessageModuleIN(int port)
{
	/* the port the server listens for messages */
	this->port = port;

	/* queue for storing received messages */
	mqueue = new MessageQueue();
	if ( mqueue == NULL )
		throw "MessageModuleIN constructor failed (not enough memory)";

	/* run method finish condition */
	finished = false;

	/* open a new UDP socket */
	if ( !(server_socket = SDLNet_UDP_Open(port)) )
		throw "Cannot open UDP socket (SDLNet_UDP_Open)";

	/* create a socket set */
	/*	- only one socket will be added to the socket set (server_socket)
		- we need socket sets because SDLNet_UDP_Recv is nonblocking and
		we don't want the server to do busy waiting
	*/
	sset = SDLNet_AllocSocketSet(1);	/* one entry */
	if ( SDLNet_UDP_AddSocket(sset, server_socket) < 0 )
		printf("[WARNING] UDP AddSocket error (busy waiting for receiving UPD packets)\n");
}

/***************************************************************************************************
*
* Constructor if a port is already open
*
***************************************************************************************************/

MessageModuleIN::MessageModuleIN(UDPsocket sock)
{
	/* the port the server listens for messages */
	this->port = 0;

	/* queue for storing received messages */
	mqueue = new MessageQueue();
	if ( mqueue == NULL )
		throw "MessageModuleIN constructor failed (not enough memory)";

	/* run method finish condition */
	finished = false;

	/* set the socket from the parameter as the input socket */
	server_socket = sock;

	/* create a socket set */
	/*	- only one socket will be added to the socket set (server_socket)
		- we need socket sets because SDLNet_UDP_Recv is nonblocking and
		we don't want the server to do busy waiting
	*/
	sset = SDLNet_AllocSocketSet(1);	/* one entry */
	if ( SDLNet_UDP_AddSocket(sset, server_socket) < 0 )
		printf("[WARNING]UPD AddSocket error (busy waiting for receiving UPD packets)\n");
}

/***************************************************************************************************
*
* Destructor
*
***************************************************************************************************/

MessageModuleIN::~MessageModuleIN()
{
	if ( mqueue != NULL ) delete mqueue;
}

/***************************************************************************************************
*
* Main loop for this module
*
***************************************************************************************************/

void MessageModuleIN::run()
{
	if ( port > 0 )
		printf("MessageModuleIN started on port %d\n", port);
	else printf("MessageModuleIN started on a random port\n");

	/* main loop */
	UDPpacket *p = NULL;
	while ( !finished )
	{
		/* make space for the packet */
		if ( p == NULL ) p = SDLNet_AllocPacket((int)MAX_UPD_PACKET_SIZE);
		if ( p == NULL ) throw "SDLNet_AllocPacket failed (not enough memory)";

		/* wait to receive at least one packet */
		SDLNet_CheckSockets(sset, (Uint32)UDP_TIMEOUT);	/* only one socket was added */

		/* read packet contents */
		int result = SDLNet_UDP_Recv(server_socket,p);
		if ( result > 0 )
		{
			/* get the user defined type from the package */
			int mtype = getPacketType(p);

			/* update statistics */
			rm.addValue(p->len);

			/* create the message object */
			Message *m = NULL;
			switch ( mtype )
			{
				case MESSAGE_SC_OK_JOIN:
					m = new MessageOkJoin(p);
					p = NULL;
					break;
				case MESSAGE_SC_REGULAR_UPDATE:
					m = new MessageWithSerializator(p);
					p = NULL;
					break;
				case MESSAGE_SC_NEW_QUEST:
					m = new MessageXY(p);
					p = NULL;
					break;
				case MESSAGE_SS_REQUEST_CLIENT_UPDATE:
					m = new MessageRequestClientUpdate(p);
					p = NULL;
					break;
				case MESSAGE_SS_ANSWER_CLIENT_UPDATE:
					m = new MessageWithSerializator(p);
					p = NULL;
					break;
				case MESSAGE_SC_MIGRATE:
					m = new MessageWithIP(p);
					p = NULL;
					break;
				default: m = new Message(p); p = NULL;
			}

			/* put the message in the queue */
			mqueue->putMessage(m);
		} else if ( result < 0 ) printf("[WARNING] Error receiving UDP packet");
	}

	/* free packet if needed */
	if ( p != NULL ) SDLNet_FreePacket(p), p = NULL;
}

/***************************************************************************************************
*
* More methods
*
***************************************************************************************************/

void MessageModuleIN::finish()
{
	finished = true;
}

MessageQueue *MessageModuleIN::getQueue()
{
	return mqueue;
}

UDPsocket MessageModuleIN::getUDPsocket()
{
	return server_socket;
}

/***************************************************************************************************
*
* Statistics related methods
*
***************************************************************************************************/

float MessageModuleIN::getBPS()
{
	return rm.getAverage();
}
