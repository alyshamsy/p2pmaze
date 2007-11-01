
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

#ifndef __STATISTICS_MODULE_H
#define __STATISTICS_MODULE_H

class StatisticsModule : public Module
{
private:
	ServerData *server_data;
	MapManagModule *mm_module;
	MessageModuleIN *in_module;
	MessageModuleOUT *out_module;
	RegularUpdateModule **ru_module;
	Serializator se;
	ServerStatistics ss;

#ifdef __linux
	unsigned int cp1,cp2,cp3,cp4;
#endif

public:
	StatisticsModule(ServerData *sd,
		MapManagModule *mmm, RegularUpdateModule **rum,
		MessageModuleIN *in, MessageModuleOUT *out);
	~StatisticsModule();

	void run();
	void gatherStatistics();

private:
	int getCpuUsage();
	int getMemUsage();
	int getCpuForThisProcess();
	int getMemForThisProcess();
	int getNumberOfThreads();
};

#endif
