
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

MessageRequestClientUpdate::MessageRequestClientUpdate()
	: Message(MESSAGE_SS_REQUEST_CLIENT_UPDATE)
{
	x = y = 0;
}

MessageRequestClientUpdate::MessageRequestClientUpdate(int x, int y, IPaddress a)
	: Message(MESSAGE_SS_REQUEST_CLIENT_UPDATE)
{
	this->x = x;
	this->y = y;
	memcpy(&addr, &a, sizeof(IPaddress));
}

MessageRequestClientUpdate::MessageRequestClientUpdate(UDPpacket *p)
	: Message(0)
{
	if ( p == NULL ) return;

	/* get basic info */
	packet = p;
	ip = packet->address.host;
	port = packet->address.port;

	/* get MessageOkJoin specific information */
	message_type = ((int*)packet->data)[0];
	x = ((int*)packet->data)[1];
	y = ((int*)packet->data)[2];
	memcpy(&addr, (IPaddress*)(packet->data + 3*sizeof(int)), sizeof(IPaddress));
}

void MessageRequestClientUpdate::setParam(int x, int y, IPaddress a)
{
	this->x = x;
	this->y = y;
	addr = a;
}

UDPpacket *MessageRequestClientUpdate::getUDPpacket()
{
	/* packet already created */
	if ( packet != NULL ) return packet;

	/* alocate space for a new packet */
	packet = SDLNet_AllocPacket(MAX_UPD_PACKET_SIZE);
	if ( packet == NULL ) return NULL;

	/* copy data to new packet */
	packet->len = 3 * sizeof(int) + sizeof(IPaddress);
	packet->address.host = ip;
	packet->address.port = port;
	((int*)packet->data)[0] = message_type;
	((int*)packet->data)[1] = x;
	((int*)packet->data)[2] = y;

	/* copy IP address */
	IPaddress *addr_m = (IPaddress*)(packet->data + 3*sizeof(int));
	memset(addr_m, 0, sizeof(IPaddress));
	addr_m->host = addr.host;
	addr_m->port = addr.port;

	return packet;
}

