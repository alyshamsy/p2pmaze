
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

#include "ClientActionModule.h"

#define CLIENT_DEBUG 0

#if CLIENT_DEBUG
#define cdprintf(frmt, ...) printf(frmt,##__VA_ARGS__)
#else
#define cdprintf(frmt, ...)
#endif

/***************************************************************************************************
*
* Constructor and setup methods
*
***************************************************************************************************/

ClientActionModule::ClientActionModule(ClientData *cd)
{
	client_data = cd;
	in = out = NULL;
	leave_retry_cont = 0;
	migration_counter = 0;
	time_of_last_update = SDL_GetTicks();
	ai = new PlayerAI(cd);
	if ( ai == NULL ) throw "Cannot create AI object";
}

void ClientActionModule::setInQueue(MessageQueue *in)
{
	this->in = in;
}

void ClientActionModule::setOutQueue(MessageQueue *out)
{
	this->out = out;
}

/***************************************************************************************************
*
* Main loop
*
***************************************************************************************************/

void ClientActionModule::run()
{
	Message *m;
	Uint32 time_of_last_message;
	Uint32 current_time;
	Uint32 start_time;
	Uint32 interval, interval2;

	/* check queues */
	if ( in == NULL ) throw "Input message queue is NULL";
	if ( out == NULL ) throw "Output message queue is NULL";
	printf("ClientActionModule started\n");

	/* main loop */
	time_of_last_message = SDL_GetTicks();
	interval = CLIENT_AI_DELAY;
	while ( client_data->state != GONE )
	{
		start_time = SDL_GetTicks();
		m = in->getMessage(interval);

		if ( m != NULL ) /* new message */
		{
			/* interpret message */
			time_of_last_message = SDL_GetTicks();
			switch ( m->getType() )
			{
				case MESSAGE_SC_OK_JOIN: handle_OK_JOIN(m); break;
				case MESSAGE_SC_NOK_JOIN: handle_NOK_JOIN(); break;
				case MESSAGE_SC_OK_LEAVE: handle_OK_LEAVE(); break;
				case MESSAGE_SC_REGULAR_UPDATE: handle_REGULAR_UPDATE(m); break;
				case MESSAGE_SC_NEW_QUEST: handle_NEW_QUEST(m); break;
				case MESSAGE_SC_QUEST_OVER: handle_QUEST_OVER(); break;
				case MESSAGE_SS_ANSWER_CLIENT_UPDATE: handle_ANSWER_CLIENT_UPDATE(m);
				break;

				case MESSAGE_SC_MIGRATE: handle_MIGRATE(m); break;
				case MESSAGE_SC_OK_MIGRATE: handle_OK_MIGRATE(m); break;

				default:
					printf("Received unknown message (%d) from server\n", m->getType());
			}

			/* shrink timeout interval */
			interval2 = SDL_GetTicks() - start_time;
			if ( interval2 > interval ) interval = 0;
			else interval = interval - interval2;

			delete m;

		} else {	/* timeout */

			/* check if no message has been received for a long time */
			/* (that could mean the server is down) */
			current_time = SDL_GetTicks();
			interval = current_time - time_of_last_message;
			if ( interval > client_data->disconnect_timeout
				&& client_data->state != MIGRATING )
			{
				client_data->state = GONE;
					printf("Timeout receiving from server ");
					printAddress(client_data->server_address);
					printf(" (%d ms)\n", interval);
				throw "Server is down";
			}

			/* take a new action */
			switch ( client_data->state )
			{
				case INITIAL: action_INITIAL(); break;
				case WAITING_JOIN: action_WAITING_JOIN(); break;
				case PLAYING: action_PLAYING(); break;
				case WAITING_LEAVE: action_WAITING_LEAVE(); break;
				case MIGRATING: action_MIGRATING(); break;
			}

			/* reset timeout interval */
			interval = CLIENT_AI_DELAY;
		}
	}
}

/***************************************************************************************************
*
* Action taken depending on client state
*
***************************************************************************************************/

void ClientActionModule::action_INITIAL()
/* send a join request to the server */
{
	Message *m = new Message(MESSAGE_CS_JOIN);
	if ( m == NULL ) throw "Cannot send initial message";
	m->setAddress(client_data->server_address);
	client_data->state = WAITING_JOIN;
	connect_retry_count = 0;
	out->putMessage(m);
}

void ClientActionModule::action_WAITING_JOIN()
/* maybe send RETRY_COUNT join request to the server */
{
	connect_retry_count++;
	if ( connect_retry_count == RETRY_COUNT )
		throw "Cannot connect to server (retry limit reach)";

	Message *m = new Message(MESSAGE_CS_JOIN);
	if ( m == NULL ) throw "Cannot send initial message";
	m->setAddress(client_data->server_address);
	out->putMessage(m);
}

void ClientActionModule::action_PLAYING()
/* play */
{
	PlayerAI_Actions action = ai->takeAction();
	int message_type = 0;

	if ( client_data->debug_AI) printf("Action: %s\n", AI_actions[(int)action]);

	switch ( action )
	{
	case ATTACK: message_type = MESSAGE_CS_ATTACK; cdprintf("Attack\n"); break;
	case USE: message_type = MESSAGE_CS_USE; cdprintf("Use\n"); break;
	case MOVE_UP: message_type = MESSAGE_CS_MOVE_UP; cdprintf("Up\n"); break;
	case MOVE_DOWN: message_type = MESSAGE_CS_MOVE_DOWN; cdprintf("Down\n"); break;
	case MOVE_LEFT: message_type = MESSAGE_CS_MOVE_LEFT; cdprintf("Left\n");break;
	case MOVE_RIGHT: message_type = MESSAGE_CS_MOVE_RIGHT; cdprintf("Right\n");break;
		default:
			return;
			};
	//message_type = MESSAGE_CS_MOVE_UP;
	Message *m = new Message(message_type);
	if ( m == NULL ) throw "Cannot send initial message";
	m->setAddress(client_data->server_address);
	out->putMessage(m);
}

void ClientActionModule::action_WAITING_LEAVE()
/* retry sending the leave message to the server */
{
	leave_retry_cont++;
	if ( leave_retry_cont == RETRY_COUNT )
	{
		printf("[WARNING] Server not responding to leave messages\n");
		client_data->state = GONE;
		SDL_CondSignal(client_data->term_cond);
	}

	Message *m = new Message(MESSAGE_CS_LEAVE);
	if ( m == NULL ) throw "Cannot send exit message";
	m->setAddress(client_data->server_address);
	out->putMessage(m);
}

void ClientActionModule::action_MIGRATING()
/* timeout if migration is taking too long */
{
	Uint32 current_time = SDL_GetTicks();
	if ( current_time - migration_start_time > client_data->migration_timeout )
	{
		printf("Migration timeout (%d ms)\n", current_time - migration_start_time);
		throw "Player migration is taking too long";
	}
}

/***************************************************************************************************
*
* Action taken when a message is received
*
***************************************************************************************************/

void ClientActionModule::handle_OK_JOIN(Message *message)
/* switch state to PLAYING */
{
	MessageOkJoin *m = (MessageOkJoin*)message;
	printf("Server approved join. Client name '%s' (%d,%d)\n", m->name, m->x, m->y);

	/* change player state */
	client_data->state = PLAYING;

	/* copy data from the message */
	memcpy(client_data->name, m->name, MAX_PLAYER_NAME);
	client_data->xpos = m->x;
	client_data->ypos = m->y;
	client_data->mapx = m->mapx;
	client_data->mapy = m->mapy;

	/* verify values */
	if ( client_data->xpos < 0 || client_data->ypos < 0
		|| client_data->xpos >= client_data->mapx
		|| client_data->ypos >= client_data->mapy )
	{
		printf("Map size not valid. Stopping client\n");
		client_data->state = WAITING_LEAVE;
		return;
	}
	if ( client_data->mapx <= 0 || client_data->mapy <= 0 )
	{
		printf("Map size not valid. Stopping client\n");
		client_data->state = WAITING_LEAVE;
		return;
	}

	/* allocating space for the terrain
	  (client keeps the terrain from all the map for pathfinding) */
	client_data->terrain = new char*[client_data->mapx];
	if ( client_data->terrain == NULL )
	{
		printf("Not enough memory for terrain. Stopping client\n");
		client_data->state = WAITING_LEAVE;
		return;
	}
	for ( int i = 0; i < client_data->mapx; i++ )
	{
		client_data->terrain[i] = new char[client_data->mapy];
		if ( client_data->terrain[i] == NULL )
		{
			printf("Not enough memory for terrain. Stopping client\n");
			client_data->state = WAITING_LEAVE;
			return;
		}

		for ( int j = 0; j < client_data->mapy; j++ )
			client_data->terrain[i][j] = 0;
	}
}

void ClientActionModule::handle_NOK_JOIN()
/* throw an error */
{
	throw "Client not allowed to join";
}

void ClientActionModule::handle_OK_LEAVE()
/* terminate */
{
	printf("Client is clear to leave\n");
	client_data->state = GONE;
	SDL_CondSignal(client_data->term_cond);
}

void ClientActionModule::handle_NEW_QUEST(Message *message)
/* set the location of the new quest */
{
	MessageXY *m = (MessageXY*)message;

	client_data->quest_active = true;
	client_data->questx = m->x;
	client_data->questy = m->y;
	printf("New quest %d,%d\n", m->x, m->y);
}

void ClientActionModule::handle_QUEST_OVER()
/* erase the location of the quest */
{
	printf("Quest over\n");
	client_data->quest_active = false;
}

void ClientActionModule::handle_REGULAR_UPDATE(Message *message)
/* update game world */
{
	int i,j;
	int xp,yp;
	char terrain,cell_type;

	if ( client_data->state != PLAYING ) return;

	/* update average */
	Uint32 current_time = SDL_GetTicks();
	client_data->last_update_interval = current_time - time_of_last_update;
	if ( client_data->average_update_interval < 0 )
		client_data->average_update_interval = client_data->last_update_interval;
	else
		client_data->average_update_interval
			= client_data->average_update_interval * 0.95
			+ (double)client_data->last_update_interval * 0.05;
	time_of_last_update = current_time;

	/* cast message */
	MessageWithSerializator *m = (MessageWithSerializator*)message;
	Serializator *s = m->getSerializator();

	SDL_LockMutex(client_data->game_mutex);

	/* get player and view position */
	*s >> client_data->xpos; *s >> client_data->ypos;
	*s >> client_data->x1; *s >> client_data->y1;
	*s >> client_data->x2; *s >> client_data->y2;

	cdprintf("v1 (%d,%d) v2 (%d, %d) pos (%d, %d)\n", client_data->x1, client_data->y1, client_data->x2, 
	       client_data->y2, client_data->xpos, client_data->ypos);

	/* get player attributes */
	*s >> client_data->life;
	*s >> client_data->attr;
	*s >> client_data->dir;

	/* get map (sorrounding region only) */
	for ( i = client_data->x1; i < client_data->x2; i++ )
	{
		for ( j = client_data->y1; j < client_data->y2; j++ )
		{
			/* get coordinates */
			xp = i - client_data->xpos + MAX_CLIENT_VIEW;
			yp = j - client_data->ypos + MAX_CLIENT_VIEW;

			/* get terrain */
			*s >> terrain;
			*s >> cell_type;
			if ( cell_type != CELL_NONE )
			{
				/* use the values provided in the message */
				client_data->map[xp][yp].terrain = terrain;
				client_data->map[xp][yp].type = cell_type;

				/* update the terrain from the matrix with all the map */
				client_data->terrain[i][j] = client_data->map[xp][yp].terrain;

			} else {
				/* use last known values */
				client_data->map[xp][yp].terrain = client_data->terrain[i][j];
				client_data->map[xp][yp].type = CELL_EMPTY;
			}

			/* get player or object */
			if ( client_data->map[xp][yp].type == CELL_OBJECT )
			{
				*s >> client_data->map[xp][yp].attr;
				*s >> client_data->map[xp][yp].quantity;
			}
			if ( client_data->map[xp][yp].type == CELL_PLAYER )
			{
				*s >> client_data->map[xp][yp].life;
				*s >> client_data->map[xp][yp].attr;
				*s >> client_data->map[xp][yp].dir;
				s->getBytes((char*)&client_data->map[xp][yp].id, sizeof(IPaddress));
				client_data->dynamic_objects[xp][yp] = 1;
			} else client_data->dynamic_objects[xp][yp] = 0;
		}
	}

	SDL_UnlockMutex(client_data->game_mutex);
}

void ClientActionModule::handle_ANSWER_CLIENT_UPDATE(Message *m)
{
	if ( client_data->state != PLAYING ) return;

	MessageWithSerializator *ans = (MessageWithSerializator*)m;
	Serializator *s = ans->getSerializator();

	updateMapFromSerializator(s);
}

void ClientActionModule::handle_MIGRATE(Message *m)
{
	MessageWithIP* mip = (MessageWithIP*)m;

	if ( client_data->state != PLAYING ) return;
	if ( !equalIP(mip->address, client_data->server_address) ) return;
	migration_start_time = SDL_GetTicks();
	client_data->state = MIGRATING;
	printf("Begin migrating ...\n");
}

void ClientActionModule::handle_OK_MIGRATE(Message *m)
{
	client_data->state = PLAYING;

	if ( !equalIP(client_data->server_address, m->getAddress()) )
	{
		client_data->server_address = m->getAddress();
		if ( migration_start_time != 0 )
		{
			printf("Finished migrating to ");
			printAddress(m->getAddress());
			printf(" in %d seconds\n", (SDL_GetTicks() - migration_start_time) / 1000 );
		} else {
			printf("Finished migrating to ");
			printAddress(m->getAddress());
			printf("\n");
		}
		migration_start_time = 0;
	}

	Message *rm = new Message(MESSAGE_CS_ACK_OK_MIGRATE);
	if ( rm == NULL )
	{
		printf("Cannot answer migrate message\n");
		return;
	}
	rm->setAddress(client_data->server_address);
	out->putMessage(rm);
}

/***************************************************************************************************
*
* Other methods
*
***************************************************************************************************/

#define in_matrix(xp,yp) ( xp >= 0 && xp < CLIENT_MATRIX_SIZE && yp >= 0 && yp < CLIENT_MATRIX_SIZE )

void ClientActionModule::updateMapFromSerializator(Serializator *s)
{
	int i,j;
	int playerx,playery;	/* players position in the received message (not used) */
	int x1,x2,y1,y2;	/* location of the map region received */
	char terrain,cell_type;	/* cell type */
	int xp,yp;		/* cell position */
	int attr,life,dir;	/* player attributes */
	int quantity;
	IPaddress plid;		/* player ID */

	SDL_LockMutex(client_data->game_mutex);

	/* get player and view position */
	s->rewind();
	s->jump( sizeof(int) + sizeof(IPaddress) );
	*s >> playerx; *s >> playery;
	*s >> x1; *s >> y1;
	*s >> x2; *s >> y2;

	/* don't parse message if player is too far */
	if ( abs( playerx - client_data->xpos ) > MAX_CLIENT_VIEW
		|| abs( playery - client_data->ypos ) > MAX_CLIENT_VIEW )
	{
		SDL_UnlockMutex(client_data->game_mutex);
		return;
	}

	/* get map (sorrounding region only) */
	for ( i = x1; i < x2; i++ )
		for ( j = y1; j < y2; j++ )
		{
			/* get coordinates */
			xp = i - client_data->xpos + MAX_CLIENT_VIEW;
			yp = j - client_data->ypos + MAX_CLIENT_VIEW;

			/* get terrain */
			*s >> terrain;
			*s >> cell_type;
			if ( cell_type != CELL_NONE )
			{
				/* use the values provided in the message */
				if ( in_matrix(xp,yp) )
				{
					client_data->map[xp][yp].terrain = terrain;
					client_data->map[xp][yp].type = cell_type;

					/* update the terrain from the matrix with all the map */
					client_data->terrain[i][j] = client_data->map[xp][yp].terrain;
				}

				/* get player or object */
				if ( cell_type == CELL_OBJECT )
				{
					*s >> attr;
					*s >> quantity;
					if ( in_matrix(xp,yp) ) client_data->map[xp][yp].attr = attr;
				}
				if ( cell_type == CELL_PLAYER )
				{
					*s >> life;
					*s >> attr;
					*s >> dir;
					s->getBytes((char*)&plid, sizeof(IPaddress));
					if ( in_matrix(xp,yp) )
					{
						client_data->map[xp][yp].life = life;
						client_data->map[xp][yp].attr = attr;
						client_data->map[xp][yp].dir = dir;
						client_data->map[xp][yp].quantity = quantity;
						client_data->map[xp][yp].id.host = plid.host;
						client_data->map[xp][yp].id.port = plid.port;
						client_data->dynamic_objects[xp][yp] = 1;
					}
				}

			}
		}

	SDL_UnlockMutex(client_data->game_mutex);
}
