
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

MessageWithIP::MessageWithIP(int t) : Message(t)
{
	memset(&address, 0, sizeof(address));
}

MessageWithIP::MessageWithIP(int t, IPaddress a) : Message(t)
{
	memset(&address, 0, sizeof(address));
	address.host = a.host;
	address.port = a.port;
}

MessageWithIP::MessageWithIP(UDPpacket *p) : Message(0)
{
	if ( p == NULL ) return;

	/* get basic info */
	packet = p;
	ip = packet->address.host;
	port = packet->address.port;

	/* get MessageOkJoin specific information */
	message_type = ((int*)packet->data)[0];
	memcpy(&address, packet->data + sizeof(int), sizeof(address));
}

void MessageWithIP::setParam(IPaddress a)
{
	address.host = a.host;
	address.port = a.port;
}

UDPpacket *MessageWithIP::getUDPpacket()
{
	/* packet already created */
	if ( packet != NULL ) return packet;

	/* alocate space for a new packet */
	packet = SDLNet_AllocPacket(MAX_UPD_PACKET_SIZE);
	if ( packet == NULL ) return NULL;

	/* copy data to new packet */
	packet->len = 3 * sizeof(int);
	packet->address.host = ip;
	packet->address.port = port;
	((int*)packet->data)[0] = message_type;
	memcpy(packet->data + sizeof(int), &address, sizeof(address));

	return packet;
}

