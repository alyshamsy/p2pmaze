
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

#ifndef __MESSAGE_REQUEST_CLIENT_UPDATE_H
#define __MESSAGE_REQUEST_CLIENT_UPDATE_H

class MessageRequestClientUpdate : public Message
{
public:
	int x,y;
	IPaddress addr;

public:
	MessageRequestClientUpdate();
	MessageRequestClientUpdate(int x, int y, IPaddress a);
	MessageRequestClientUpdate(UDPpacket *p);

	void setParam(int x, int y, IPaddress a);

	UDPpacket *getUDPpacket();
};

#endif
