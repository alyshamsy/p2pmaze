
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
#include "Message.h"

/***************************************************************************************************
*
* Constructors / destructor
*
***************************************************************************************************/

Message::Message()
{
	message_type = MESSAGE_DEFAULT;
	ip = 0;
	port = 0;
	packet = NULL;
}

Message::Message(int t)
{
	message_type = t;
	ip = 0;
	port = 0;
	packet = NULL;
}

Message::Message(UDPpacket *p)
{
	packet = p;
	if ( packet != NULL )
	{
		ip = packet->address.host;
		port = packet->address.port;
		message_type = getPacketType(packet);
	} else {
		ip = 0;
		port = 0;
		message_type = 0;
	}
}

Message::~Message()
{
	if ( packet != NULL ) SDLNet_FreePacket((UDPpacket*)packet);
}

/***************************************************************************************************
*
* Message management
*
***************************************************************************************************/

int Message::getType()
{
	return message_type;
}

void Message::setAddress(Uint32 ip, Uint16 port)
{
	this->ip = ip;
	this->port = port;
	if ( packet != NULL )
	{
		packet->address.host = ip;
		packet->address.port = port;
	}
}

void Message::setAddress(IPaddress adr)
{
	ip = adr.host;
	port = adr.port;
	if ( packet != NULL )
	{
		packet->address.host = ip;
		packet->address.port = port;
	}
}

IPaddress Message::getAddress()
{
	if ( packet != NULL ) return packet->address;

	IPaddress addr;
	addr.host = ip;
	addr.port = port;
	return addr;
}

Uint32 Message::getIP()
{
	return ip;
}

Uint16 Message::getPort()
{
	return port;
}

/***************************************************************************************************
*
* Serialization
*
***************************************************************************************************/

UDPpacket *Message::getUDPpacket()
{
	/* packet already created */
	if ( packet != NULL ) return packet;

	/* alocate space for a new packet */
	packet = SDLNet_AllocPacket(sizeof(int)); /* or MAX_UPD_PACKET_SIZE */
	if ( packet == NULL ) return NULL;

	/* copy data to new packet */
	*((int*)packet->data) = message_type;
	packet->len = sizeof(int);
	packet->address.host = ip;
	packet->address.port = port;

	return packet;
}

/***************************************************************************************************
*
* Returns the type of the message (always the first int in the data)
*
***************************************************************************************************/

int getPacketType(UDPpacket *p)
{
	return *((int*)p->data);
}
