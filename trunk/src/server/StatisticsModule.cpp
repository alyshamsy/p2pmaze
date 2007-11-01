
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

#include "Server.h"
#include "../comm/MessageModuleIN.h"
#include "../comm/MessageModuleOUT.h"
#include "MapManagModule.h"
#include "RegularUpdateModule.h"
#include "StatisticsModule.h"

/***************************************************************************************************
*
* Constructor and destructor
*
***************************************************************************************************/

StatisticsModule::StatisticsModule(ServerData *sd,
	MapManagModule *mm, RegularUpdateModule **rum,
	MessageModuleIN *in, MessageModuleOUT *out)
{
	if ( sd == NULL || mm == NULL || in == NULL || out == NULL )
		throw "Cannot start statistics module";
	server_data = sd;
	mm_module = mm;
	in_module = in;
	out_module = out;
	ru_module = rum;

	ss.tcp_total = 0.0;
	ss.udp_total = 0.0;
	ss.number_of_statistics = 0;

	#ifdef __linux
	cp1 = cp2 = cp3 = cp4 = 0;
	#endif
}

StatisticsModule::~StatisticsModule()
{
	/* nothing */
}

/***************************************************************************************************
*
* Main loop
*
***************************************************************************************************/

void StatisticsModule::run()
{
	printf("StatisticsModule started\n");
	Garbage *old_ptr = NULL;

	while ( true )
	{
		gatherStatistics();
		mm_module->sendStatistics(&ss,&se);

		server_data->trash->acceptIt(old_ptr);
		SDL_Delay( server_data->stats_interval * 1000 );
	}
}

void StatisticsModule::gatherStatistics()
{
	int i,j;

	se.clear();

	/* get region data */
	ss.number_of_regions = 0;
	ss.number_of_players = 0;
	ss.players_in_most_crowded_region = 0;
	for ( i = 0; i < server_data->nregx; i++ )
		for ( j = 0; j < server_data->nregy; j++ )
		{
			Region *r = server_data->region[i][j];
			if ( r == NULL ) continue;
			int ppr = r->getNumberOfPlayers();

			if ( ss.players_in_most_crowded_region < ppr )
				ss.players_in_most_crowded_region = ppr;

			ss.number_of_regions++;
			ss.number_of_players += ppr;
			se << i;
			se << j;
			se << ppr;
		}
	ss.number_of_players = server_data->player_list.size()
		+ server_data->migrating_players.size();

	/* get process and machine data */
	ss.machine_cpu_usage = getCpuUsage();
	ss.machine_mem_usage = getMemUsage();
	ss.process_cpu_usage = getCpuForThisProcess();
	ss.process_mem_usage = getMemForThisProcess();
	ss.number_of_threads = getNumberOfThreads();

	/* get network data */
	ss.bps_tcp_recv = mm_module->getBPS_recv();
	ss.bps_tcp_sent = mm_module->getBPS_sent();
	ss.bps_udp_recv = in_module->getBPS();
	ss.bps_udp_sent = out_module->getBPS();
	ss.average_regular_update_interval = 0;
	for ( i = 0; i < server_data->regular_update_threads; i++ )
		ss.average_regular_update_interval += ru_module[i]->average_regular_update_interval;
	ss.average_regular_update_interval /= server_data->regular_update_threads;
	ss.average_real_regular_update_interval = 0;
	for ( i = 0; i < server_data->regular_update_threads; i++ )
		if ( ru_module[i]->average_real_regular_update_interval >
			ss.average_real_regular_update_interval )
			ss.average_real_regular_update_interval =
				ru_module[i]->average_real_regular_update_interval;

	ss.tcp_total += ss.bps_tcp_recv + ss.bps_tcp_sent;
	ss.udp_total += ss.bps_udp_recv + ss.bps_udp_sent;
	ss.number_of_statistics ++;
}

#ifndef __linux

int StatisticsModule::getCpuUsage() { return 0; }
int StatisticsModule::getMemUsage() { return 0; }
int StatisticsModule::getCpuForThisProcess() { return 0; }
int StatisticsModule::getMemForThisProcess() { return 0; }
int StatisticsModule::getNumberOfThreads() { return 0; }

#else

/* get information from /proc */

char *getNumberStart(char *s)
{
	for ( char *p = s; *p != 0; p++ )
		if ( *p >= '0' && *p < '9' ) return p;
	return NULL;
}

int StatisticsModule::getCpuUsage()
{
	unsigned int c1,c2,c3,c4;
	float r,v1,rc;

	/* read values */
	FILE *f = fopen("/proc/stat", "r");
	if (f == NULL ) return 0;
	rc = fscanf(f, "cpu %u %u %u %u", &c1,&c2,&c3,&c4);
	fclose(f);
	if ( rc == 0 ) return 0;

	/* compute CPU usage */
	if ( c1 == 0 )
	{
		v1 = c1 + c2 + c3 + c4;
		r = ( v1 - c4 ) / v1 * 100.0;
	} else {
		v1 = c1 + c2 + c3 + c4 - cp1 - cp2 - cp3 - cp4;
		r = ( v1 - ( c4 - cp4 ) ) / v1 * 100.0;
	}
	cp1 = c1;
	cp2 = c2;
	cp3 = c3;
	cp4 = c4;

	return (int)r;
}

int StatisticsModule::getMemUsage()
{
	int total,free_mem,buffers,cached;
	int app = 0,n = 0;
	char line[MAX_FILE_READ_BUFFER],*p;

	/* open file */
	FILE *f = fopen("/proc/meminfo", "r");
	if ( f == NULL ) return 0;

	/* read the required 4 lines */
	while ( n < 4 )
	{
		if ( fgets(line, MAX_FILE_READ_BUFFER, f) == NULL ) goto end_get_mem;

		if ( !strncmp("MemTotal:", line, 9) )
		{
			if ( ( p = getNumberStart(line) ) == NULL ) goto end_get_mem;
			sscanf(p, "%d", &total);
			n++;
		}
		if ( !strncmp("MemFree:", line, 8) )
		{
			if ( ( p = getNumberStart(line) ) == NULL ) goto end_get_mem;
			sscanf(p, "%d", &free_mem);
			n++;
		}
		if ( !strncmp("Buffers:", line, 8) )
		{
			if ( ( p = getNumberStart(line) ) == NULL ) goto end_get_mem;
			sscanf(p, "%d", &buffers);
			n++;
		}
		if ( !strncmp("Cached:", line, 7) )
		{
			if ( ( p = getNumberStart(line) ) == NULL ) goto end_get_mem;
			sscanf(p, "%d", &cached);
			n++;
		}
	}

	app = total - free_mem - buffers - cached;

end_get_mem:
	fclose(f);
	return app;
}

int StatisticsModule::getCpuForThisProcess()
{
	/* TODO */
	return 0;
}

int StatisticsModule::getMemForThisProcess()
{
	int mem = 0;
	char line[MAX_FILE_READ_BUFFER],*p;
	char file_name[256];

	/* get file name */
	pid_t pid = getpid();
	sprintf(file_name, "/proc/%d/status", pid);

	/* open file */
	FILE *f = fopen(file_name, "r");
	if ( f == NULL ) return 0;

	/* read lines until the requested value is found */
	while ( mem == 0 )
	{
		if ( fgets(line, MAX_FILE_READ_BUFFER, f) == NULL ) break;

		if ( !strncmp("VmRSS:", line, 6) )
		{
			if ( ( p = getNumberStart(line) ) == NULL ) break;
			sscanf(p, "%d", &mem);
		}
	}

	/* close file and return value */
	fclose(f);
	return mem;
}

int StatisticsModule::getNumberOfThreads()
{
	int th = 0;
	char line[MAX_FILE_READ_BUFFER],*p;
	char file_name[256];

	/* get file name */
	pid_t pid = getpid();
	sprintf(file_name, "/proc/%d/status", pid);

	/* open file */
	FILE *f = fopen(file_name, "r");
	if ( f == NULL ) return 0;

	/* read lines until the requested value is found */
	while ( th == 0 )
	{
		if ( fgets(line, MAX_FILE_READ_BUFFER, f) == NULL ) break;

		if ( !strncmp("Threads:", line, 8) )
		{
			if ( ( p = getNumberStart(line) ) == NULL ) break;
			sscanf(p, "%d", &th);
		}
	}

	/* close file and return value */
	fclose(f);
	return th;
}

#endif
