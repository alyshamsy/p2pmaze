
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

#ifndef __MESSAGE_WITH_IP_H
#define __MESSAGE_WITH_IP_H

class MessageWithIP : public Message
{
public:
	IPaddress address;

public:
	MessageWithIP(int t);
	MessageWithIP(int t, IPaddress a);
	MessageWithIP(UDPpacket *p);

	void setParam(IPaddress a);

	UDPpacket *getUDPpacket();
};

#endif

