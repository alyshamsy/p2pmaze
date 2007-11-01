
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

MessageOkJoin::MessageOkJoin() : Message(MESSAGE_SC_OK_JOIN)
{
	name[0] = 0;
	x = y = 0;
	mapx = mapy = 0;
}

MessageOkJoin::MessageOkJoin(UDPpacket *p) : Message(MESSAGE_SC_OK_JOIN)
{
	if ( p == NULL ) return;

	/* get basic info */
	packet = p;
	ip = packet->address.host;
	port = packet->address.port;

	/* get MessageOkJoin specific information */
	message_type = ((int*)packet->data)[0];
	mapx	= ((int*)packet->data)[1];
	mapy	= ((int*)packet->data)[2];
	x	= ((int*)packet->data)[3];
	y	= ((int*)packet->data)[4];
	strncpy(name, (char*)packet->data + 5 * sizeof(int), MAX_PLAYER_NAME);
}

void MessageOkJoin::setParam(char *name, int x, int y)
{
	strncpy(this->name, name, MAX_PLAYER_NAME);
	this->x = x;
	this->y = y;
}

void MessageOkJoin::setMapSize(int x, int y)
{
	mapx = x;
	mapy = y;
}

UDPpacket *MessageOkJoin::getUDPpacket()
{
	/* packet already created */
	if ( packet != NULL ) return packet;

	/* alocate space for a new packet */
	packet = SDLNet_AllocPacket(MAX_UPD_PACKET_SIZE);
	if ( packet == NULL ) return NULL;

	/* copy data to new packet */
	packet->len = 5 * sizeof(int) + MAX_PLAYER_NAME;
	packet->address.host = ip;
	packet->address.port = port;
	((int*)packet->data)[0] = message_type;
	((int*)packet->data)[1] = mapx;
	((int*)packet->data)[2] = mapy;
	((int*)packet->data)[3] = x;
	((int*)packet->data)[4] = y;
	memcpy((char*)packet->data + 5 * sizeof(int), name, MAX_PLAYER_NAME);

	return packet;
}
