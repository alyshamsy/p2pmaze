
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
#include "MessageModuleOUT.h"

/***************************************************************************************************
*
* Constructors and destructor
*
***************************************************************************************************/

MessageModuleOUT::MessageModuleOUT()
{
	/* queue for storing messages */
	mqueue = new MessageQueue();
	if ( mqueue == NULL )
		throw "MessageModuleOUT constructor failed (not enough memory)";

	/* run method finish condition */
	finished = false;

	/* open a socket for sending packets */
	Uint16 i;
	Uint16 start_port = 1024 + rand()%(0xFFFF-1024);
	for ( i = start_port; i < 0xFFFF; i++ )
	{
		out_socket = SDLNet_UDP_Open( i );
		if ( out_socket ) return;
	}
	for ( i = 1024; i < start_port; i++ )
{
		out_socket = SDLNet_UDP_Open( i );
		if ( out_socket ) return;
	}
	throw "Cannot open UDP socket (SDLNet_UDP_Open)";
}

MessageModuleOUT::MessageModuleOUT(UDPsocket sock)
{
	/* queue for storing received messages */
	mqueue = new MessageQueue();
	if ( mqueue == NULL )
		throw "MessageModuleIN constructor failed (not enough memory)";

	/* run method finish condition */
	finished = false;

	/* set socket */
	out_socket = sock;
}

MessageModuleOUT::~MessageModuleOUT()
{
	if ( mqueue != NULL ) delete mqueue;
}

/***************************************************************************************************
*
* Main loop
*
***************************************************************************************************/

void MessageModuleOUT::run()
{
	Message *m;
	UDPpacket *p;

	printf("MessageModuleOUT started\n");

	/* main loop */
	while ( !finished )
	{
		#ifndef __MESSAGE_BUFFER__

		m = mqueue->getMessage();	/* get message */
		p = m->getUDPpacket();		/* create packet from message */
		if ( SDLNet_UDP_Send(out_socket, -1, p) == 0 )	/* send packet */
			printf("[WARNING] Error sending UDP packet\n");
		rm.addValue( p->status > 0 ? p->status : 0 );
		delete m;			/* delete message */

		#else

		list<Message*> lm;
		mqueue->getMessages(&lm);

		while ( !lm.empty() )
		{
			m = lm.front();
			lm.pop_front();

			p = m->getUDPpacket();		/* create packet from message */
			if ( SDLNet_UDP_Send(out_socket, -1, p) == 0 )	/* send packet */
				printf("[WARNING] Error sending UDP packet\n");
			rm.addValue( p->status > 0 ? p->status : 0 );
			delete m;			/* delete message */
		}

		#endif
	}
}

/***************************************************************************************************
*
* More methods
*
***************************************************************************************************/

void MessageModuleOUT::finish()
{
	finished = true;
}

MessageQueue *MessageModuleOUT::getQueue()
{
	return mqueue;
}

UDPsocket MessageModuleOUT::getUDPsocket()
{
	return out_socket;
}

/***************************************************************************************************
*
* Statistics related methods
*
***************************************************************************************************/

float MessageModuleOUT::getBPS()
{
	return rm.getAverage();
}
