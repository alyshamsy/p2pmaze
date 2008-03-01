
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

#ifndef __SENDER_MODULE_H
#define __SENDER_MODULE_H

class SenderModule : public Module
{
protected:
  MessageQueue *message_queue;

public:
  SenderModule(MessageQueue *message_queue);

  /* main loop */
  void run();
  void sendMessage(Message *m);
  int SDLNet_TCP_Send2(TCPsocket sock, void *data, int len);
};

#endif
