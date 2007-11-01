
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

/***************************************************************************************************
*
* Sending regions
*
***************************************************************************************************/

/*
	packRegionCell
	- uses the Serializator s to put terrain, player and game object information in
	the send buffer
	- if trere is no player in the requested cell, this methods inserts 0 as the
	player data size; the same for game objects
*/

void MapManagModule::packRegionCell(Region *r, int i, int j, Serializator &s)
{
	/* pack terrain*/
	s << r->terrain[i][j];

	/* pack player */
	Player *p = r->grid[i][j].player;
	if ( p != NULL && server_data->player_list.findValue(p) )
	{
		/*
		- if player is in the map but he is not in the list then there must be a duplicate
		*/

		/* remove player from list */
		server_data->player_list.erase(p->address);
		server_data->migrating_players.erase(p->address);

		/* send a migrate message to the player */
		Message *m = new MessageWithIP(MESSAGE_SC_MIGRATE, server_data->own_ip);
		if ( m != NULL )
		{
			m->setAddress(p->address);
			out->putPriorityMessage(m);
		} else printf("[WARNING] Cannot send MIGRATE message to player\n");

		/* add player to buffer */
		char *pbuff = p->playerData();
		int psize = p->playerDataSize();
		if ( pbuff == NULL )
			throw "Cannot transfer region because not enough memory";
		s << psize;
		s.putBytes(pbuff, psize);
		delete[] pbuff;

		/* delete player */
		server_data->trash->add(p);
	} else s << (int)0;

	/* pack object */
	GameObject *o = r->grid[i][j].object;
	if ( o != NULL )
	{
		/* add object to buffer */
		char *obuff = o->objectData();
		int osize = o->objectDataSize();
		if ( obuff == NULL )
			throw "Cannot transfer region because not enough memory";
		s << osize;
		s.putBytes(obuff, osize);
		delete[] obuff;

		server_data->trash->add(o);
	} else s << (int)0;
}

/*
	handle_GIVE_REGION
	- this method receives a give region request from the master, removes the
	selected region from its data, packs the region and sends it to the master
*/

bool MapManagModule::handle_GIVE_REGION()
/* returns false only if there was a socket error */
{
	int x,y;		/* region coordinates */
	IPaddress new_server;	/* address of new server */
	Region *r;		/* region to be moved */
	int i,j;

	/* get region details from master */
	if ( SDLNet_TCP_Recv2( sock, &x, sizeof(int) ) <= 0
		|| SDLNet_TCP_Recv2( sock, &y, sizeof(int) ) <= 0
		|| SDLNet_TCP_Recv2( sock, &new_server, sizeof(IPaddress) ) <= 0 )
		return false;

	/* Display message */
	if ( server_data->display_migrations )
		printf("[MIGRATION] Sending region %d,%d to %X:%u\n",
		x,y, new_server.host, new_server.port);

	/* verify coordinates */
	if ( x < 0 || y < 0 || x >= server_data->nregx || y >= server_data->nregy )
	{
		printf("[WARNING] Give region message is invalid\n");
		return true;
	}
	if ( server_data->region[x][y] == NULL )
	{
		printf("[WARNING] Requested region does not belong to current server\n");
		return true;
	}

	/* get region pointer */
	r = server_data->region[x][y];

	/* lock region */
	r->lock();

	/* replace region */
	server_data->region[x][y] = NULL;
	server_data->layout[x][y] = new_server;

	/* pack region */
	Serializator s;
	s << r->sizex; s << r->sizey;
	s << r->px; s << r->py;
	for ( i = 0; i < r->sizex; i++ )
		for ( j = 0; j < r->sizey; j++ )
			packRegionCell(r, i,j, s);

	/* unlock region */
	r->unlock();

	/* sent this region */
	SDL_LockMutex(send_mutex);
	Uint32 message_type = SM_MOVE_REGION;
	Uint32 buf_size = s.getSize();
	char *buffer = s.getBuffer();
	if ( SDLNet_TCP_Send2( sock, (void*)&message_type, sizeof(Uint32) ) < (int)sizeof(Uint32)
		|| SDLNet_TCP_Send2( sock, (void*)&buf_size, sizeof(Uint32) ) < (int)sizeof(Uint32)
		|| SDLNet_TCP_Send2( sock, (void*)buffer, buf_size ) < (int)buf_size
		|| SDLNet_TCP_Send2( sock, (void*)&x, sizeof(int) ) < (int)sizeof(int)
		|| SDLNet_TCP_Send2( sock, (void*)&y, sizeof(int) ) < (int)sizeof(int) )
			throw "Failed to send region to master";
	SDL_UnlockMutex(send_mutex);

	/* delete region */
	server_data->trash->add(r);

	return true;
}

/***************************************************************************************************
*
* Receiving regions
*
***************************************************************************************************/

/*
	handle_TAKE_REGION
	- receives a region from the master
*/

bool MapManagModule::handle_TAKE_REGION()
{
	Uint32 buf_size;
	char *buffer;
	int i,j;
	Region *r;
	Player *p;
	GameObject *o;
	list<Player*> pl;
	list<Player*>::iterator plit;

	/* retrieve region from master */
	if ( SDLNet_TCP_Recv2( sock, &buf_size, sizeof(Uint32) ) <= 0
		|| ( buffer = new char[buf_size] ) == NULL
		|| SDLNet_TCP_Recv2( sock, buffer, buf_size ) <= 0 )
		return false;

	/* create region from buffer */
	Serializator s(buffer, buf_size);

	/* general region info */
	int sx,sy,px,py;
	s >> sx; s >> sy;
	s >> px; s >> py;
	r = new Region(sx,sy);
	if ( r == NULL ) throw "Not enough memory to create region";
	r->setPosition(px,py);

	/* get terrain and player data */
	for ( i = 0; i < sx; i++ )
		for ( j = 0; j < sy; j++ )
		{
			char terrain;
			int pbsize,obsize;
			char *pbuff,*obuff;

			/* get terrain */
			s >> terrain;
			r->terrain[i][j] = terrain;

			/* get player */
			s >> pbsize;
			if ( pbsize > 0 )
			{
				pbuff = s.getBytesPtr(pbsize);

				/* add player to map */
				p = new Player(pbuff);
				if ( p == NULL ) throw "Not enough memory to add new player";
				p->x = px * server_data->regx + i;
				p->y = py * server_data->regy + j;
				r->addPlayerNoSync(p, i,j);
				pl.insert(pl.end(), p);
			}

			/* get object */
			s >> obsize;
			if ( obsize > 0 )
			{
				obuff = s.getBytesPtr(obsize);

				/* add object to map */
				o = new GameObject(obuff);
				if ( o == NULL ) throw "Not enough memory to add new object";
				r->grid[i][j].object = o;
				r->number_of_objects++;
			}
		}

	/* display message */
	if ( server_data->display_migrations )
		printf("[MIGRATION] Received region %d,%d (%d bytes)\n", px,py, buf_size);

	/* add this region */
	server_data->layout[px][py] = server_data->own_ip;
	server_data->region[px][py] = r;

	for ( plit = pl.begin(); plit != pl.end(); plit++ )
	{
		p = *plit;

		/* add player to list */
		server_data->migrating_players.insert(p);

		/* send MESSAGE_SC_OK_MIGRATE to client */
		server_data->notifyClientOfMigration(p,out);
	}

	/* free memory and exit */
	delete[] buffer;
	return true;
}

/***************************************************************************************************
*
* Moving regions
*
***************************************************************************************************/

/*
	handle_MOVING_REGION
	- a region is being moved from another server
*/

bool MapManagModule::handle_MOVING_REGION()
{
	int x,y;		/* region coordinates */
	IPaddress new_server;	/* address of new server */

	/* get region details from master */
	if ( SDLNet_TCP_Recv2( sock, &x, sizeof(int) ) <= 0
		|| SDLNet_TCP_Recv2( sock, &y, sizeof(int) ) <= 0
		|| SDLNet_TCP_Recv2( sock, &new_server, sizeof(IPaddress) ) <= 0 )
		return false;

	/* display message */
	if ( server_data->display_migrations )
		printf("[MIGRATION] Region %d,%d moved from server %X:%u to %X:%u\n",
		x,y, server_data->layout[x][y].host, server_data->layout[x][y].port,
		new_server.host, new_server.port);

	/* verify coordinates */
	if ( x < 0 || y < 0 || x >= server_data->nregx || y >= server_data->nregy )
	{
		printf("[WARNING] Give region message is invalid\n");
		return true;
	}

	/* replace region */
	server_data->layout[x][y] = new_server;

	return true;
}
