
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

MessageXY::MessageXY(int t) : Message(t)
{
	x = y = 0;
}

MessageXY::MessageXY(int t, int x, int y) : Message(t)
{
	this->x = x;
	this->y = y;
}

MessageXY::MessageXY(UDPpacket *p) : Message(0)
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
}

void MessageXY::setParam(int x, int y)
{
	this->x = x;
	this->y = y;
}

UDPpacket *MessageXY::getUDPpacket()
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
	((int*)packet->data)[1] = x;
	((int*)packet->data)[2] = y;

	return packet;
}

