
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

#ifndef __SERVER_H
#define __SERVER_H

#include "General.h"
#include "../utils/Configurator.h"
#include "../utils/ThreadSafeQueue.h"
#include "../utils/IPUtils.h"
#include "../utils/RateMonitor.h"
#include "../utils/SDL_barrier.h"

#include "../game/Player.h"
#include "../game/Quest.h"
#include "../game/GameObject.h"
#include "../game/Region.h"

#include "../comm/Message.h"
#include "../comm/MessageQueue.h"
#include "../comm/ServerMasterProtocol.h"

#include "PlayerBucket.h"
#include "CustomPlayerList.h"
#include "ServerData.h"

#endif
