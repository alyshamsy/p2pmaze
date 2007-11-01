
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

#ifndef __MASTER_MESSAGES_H
#define __MASTER_MESSAGES_H

/***************************************************************************************************
*
* Master Message Classes
* - Unlike the other classes that extend Message, objects of this types are used inside the master
*   not between server and clients like the other messages.
* - I implemented these classes to extend Message just to be able to use the MessageQueue class
*
***************************************************************************************************/

#include "Message.h"

/*
	MasterMessage - basic message with just its type and socket
	***********************************************************
*/

class MasterMessage : public Message
{
public:
	TCPsocket sock;

public:
	MasterMessage(int mtype, TCPsocket sock)
	{

		this->message_type = mtype;
		this->sock = sock;
	}
};

/*
	MasterMessageWithBuffer - Message with a buffer
	***********************************************
*/

class MasterMessageWithBuffer : public Message
{
public:
	char *buffer;
	Uint32 len;
	TCPsocket sock;

public:
	MasterMessageWithBuffer(int mtype, char *buffer, Uint32 len, TCPsocket sock)
	{
		this->message_type = mtype;
		this->buffer = buffer;
		this->len = len;
		this->sock = sock;
	}

	~MasterMessageWithBuffer()
	{
		if ( buffer != NULL ) delete buffer;
	}
};

/*
	MasterMessageXY - Message containing x,y coordinates
	****************************************************
*/

class MasterMessageXY : public Message
{
public:
	int x,y;
	TCPsocket sock;

public:
	MasterMessageXY(int mtype, int x, int y, TCPsocket sock)
	{

		this->message_type = mtype;
		this->x = x;
		this->y = y;
		this->sock = sock;
	}
};

/*
	MasterMessageXY - Message containing x,y coordinates and an IP address
	**********************************************************************
*/

class MasterMessageXYServer : public Message
{
public:
	IPaddress addr;
	int x,y;
	TCPsocket sock;

public:
	MasterMessageXYServer(int mtype, int x, int y, IPaddress a, TCPsocket sock)
	{
		this->message_type = mtype;
		this->x = x;
		this->y = y;
		this->addr = a;
		this->sock = sock;
	}
};

#endif
