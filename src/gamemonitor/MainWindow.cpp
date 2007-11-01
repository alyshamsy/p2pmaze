
/***************************************************************************************************
*
* SUBJECT:
*    A monitoring application for the Massive Multiplayer Online Game benchmark
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

#include <stdio.h>
#include <wx/wx.h>

#include "SDL_replace.h"
#include "Constants.h"
#include "Structs.h"

#include "Utils.h"
#include "SlowReader.h"
#include "MainWindow.h"
#include "SliderDialog.h"
#include "help.html.h"
#include "icon.xpm"

#define RIGHT_REGION_WIDTH	200

/***************************************************************************************************
*
* Global variables (extern)
*
***************************************************************************************************/

extern SliderDialog *sdiag;

extern Uint32 timestamp;
extern MapData map_data;
extern MasterStatistics master_stats;
extern ConnectedServerInfo ss[MAX_SERVERS];
extern int **layout;
extern int **players_per_region;
extern int quest_active,quest_x,quest_y;
extern wxMutex global_lock;
int number_of_players;

int server_colors;
wxColor table_color[MAX_SERVERS];

/***************************************************************************************************
*
* Constants
*
***************************************************************************************************/

#define TIMER_ID 1000

enum
{
	NOTEBOOK = 1,
	MENU_EXIT = wxID_EXIT,
	MENU_REFRESH = wxID_REFRESH,
	MENU_ABOUT = wxID_ABOUT,
	MENU_SAVE = wxID_SAVE,
	MENU_SAVE_REDUCED = 1000,
	MENU_SELECT_COLUMNS
};

/***************************************************************************************************
*
* Event table
*
***************************************************************************************************/

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(MENU_EXIT, MainWindow::OnExit)
	EVT_MENU(MENU_REFRESH, MainWindow::OnRefresh)
	EVT_MENU(MENU_SELECT_COLUMNS, MainWindow::OnSelectColumns)
	EVT_MENU(MENU_SAVE, MainWindow::OnSave)
	EVT_MENU(MENU_SAVE_REDUCED, MainWindow::OnSaveReduced)
	EVT_MENU(MENU_ABOUT, MainWindow::OnAbout)
	EVT_TIMER(TIMER_ID, MainWindow::OnTimer)
END_EVENT_TABLE()

/***************************************************************************************************
*
* Constructor
*
***************************************************************************************************/

MainWindow::MainWindow(bool offline) : wxFrame(NULL, -1, wxString(L"GameMonitor"),
	wxPoint(100,100), wxSize(640,480),
	wxDEFAULT_FRAME_STYLE )
{
	this->offline = offline;
	this->server_columns = 2;

	/* set size, icon and colors */
	SetMinSize(wxSize(400,320));
	SetIcon(wxIcon(icon_xpm));

	/* generate colors */
	randomColors(MAX_SERVERS);

	/* set tabs with HTML controls */
	nb = new wxNotebook(this, NOTEBOOK);
	wh_general = new wxHtmlWindow(nb, -1);
	nb->AddPage(wh_general, _("General"));
	wh_servers = new wxHtmlWindow(nb, -1);
	nb->AddPage(wh_servers, _("Servers"));
	wh_regions = new wxHtmlWindow(nb, -1);
	nb->AddPage(wh_regions, _("Regions"));
	wh_load = new wxHtmlWindow(nb, -1);
	nb->AddPage(wh_load, _("Load"));
	wh_help = new wxHtmlWindow(nb, -1);
	nb->AddPage(wh_help, _("Help"));
	wh_help->SetPage(help_text);

	/* set menu */
	wxMenu *menu1 = new wxMenu();
	menu1->Append(MENU_REFRESH, _("&Refresh"));
	menu1->Append(MENU_SELECT_COLUMNS, _("Select number of &columns"));
	menu1->AppendSeparator();
	menu1->Append(MENU_EXIT, _("E&xit"));

	wxMenu *menu2 = new wxMenu();
	menu2->Append(MENU_SAVE, _("&Save CSV ..."));
	menu2->Append(MENU_SAVE_REDUCED, _("Save CSV &reduced ..."));

	wxMenu *menu3 = new wxMenu();
	menu3->Append(MENU_ABOUT, _("&About"));

	wxMenuBar *menuBar = new wxMenuBar();
	menuBar->Append(menu1, _("&Menu"));
	if ( offline)
		menuBar->Append(menu2, _("&Save"));
	menuBar->Append(menu3, _("&Help"));
	SetMenuBar(menuBar);

	/* set update timer */
	wxTimer *timer = new wxTimer(this, TIMER_ID);
	if ( !offline )
		timer->Start(500, wxTIMER_CONTINUOUS);
}

/***************************************************************************************************
*
* Evemt handlers
*
***************************************************************************************************/

void MainWindow::OnExit(wxCommandEvent& WXUNUSED(event))
{
	exit(0);
}

void MainWindow::OnRefresh(wxCommandEvent& WXUNUSED(event))
{
	UpdateHTMLControls();
}

void MainWindow::OnSelectColumns(wxCommandEvent& WXUNUSED(event))
{
	/* get ratio */
	int res = (int)wxGetNumberFromUser( _T("Select the number of columns\n")
		_T("for the server tab."),
		_T("Columns: "), _T("Number of columns"),
		server_columns, 1, 10, this );
	if ( res == -1 ) return;
	server_columns = res;
	UpdateServers();
}

void MainWindow::OnSave(wxCommandEvent& WXUNUSED(event))
{

  //FIXME
#if 0
	int k;
	if ( sdiag == NULL ) return;

	/* get file name */
	wxFileDialog dialog(this,
		_T("Save CSV file"),
		wxEmptyString,
		_T("statistics.csv"),
		_T("Text CSV (*.csv)|*.csv|Text files (*.txt)|*.txt"),
		wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	dialog.SetFilterIndex(0);
	if (dialog.ShowModal() != wxID_OK) return;
	//printf("%s (%d)\n", dialog.GetPath().c_str(), dialog.GetFilterIndex());

	/* get current position */
	int pos = sdiag->getPosition();

	/* open file */
	FILE *f = fopen(dialog.GetPath().c_str(), "w");
	if ( f == NULL )
	{
		printf("Cannot save file %s\n", dialog.GetPath().c_str());
		wxMessageBox("Cannot save file", "ERROR", wxICON_ERROR);
		return;
	}
	fprintf(f, "Timestamp;Number of players;Number of regions;Quest active;");
	for ( k = 0; k < map_data.num_servers; k++ )
		fprintf(f, "Players S%d;Regions S%d;Density S%d;Crowded %d;CPU S%d;MEM S%d (MB);PMEM S%d (MB);Update interval S%d (ms);"
			"TCP send S%d (kbps);TCP recv S%d (kbps);"
			"UDP send S%d (kbps);UDP recv S%d(kbps);",
			k,k,k,k,k,k,k,k,k,k,k,k);
	fprintf(f,"\n");

	/* save data */
	int n = sdiag->getNumberOfRecords();
	for ( int i = 0; i < n; i++ )
	{
		sdiag->loadPosition(i);
		number_of_players = 0;
		for ( int j = 0; j < map_data.num_servers; j++ )
			number_of_players += ss[j].statistics.number_of_players;
		fprintf(f, "%d;%d;%d;%d;",
			timestamp,number_of_players,
			(map_data.nregx * map_data.nregy),quest_active);
		for ( k = 0; k < map_data.num_servers; k++ )
			fprintf(f, "%d;%d;%.2f;%d;%d;%.2f;%.2f;%d;%.2f;%.2f;%.2f;%.2f;",
				ss[k].statistics.number_of_players,
				ss[k].statistics.number_of_regions,
				((ss[k].statistics.number_of_regions == 0 ) ? 0.0 :
					( (float)ss[k].statistics.number_of_players /
					(float)ss[k].statistics.number_of_regions ) ),
				ss[k].statistics.players_in_most_crowded_region,
				ss[k].statistics.machine_cpu_usage,
				ss[k].statistics.machine_mem_usage / 1024.0,
				ss[k].statistics.process_mem_usage / 1024.0,
				(int)ss[k].statistics.average_regular_update_interval,
				ss[k].statistics.bps_tcp_sent / 1000.0,
				ss[k].statistics.bps_tcp_recv / 1000.0,
				ss[k].statistics.bps_udp_sent / 1000.0,
				ss[k].statistics.bps_udp_recv / 1000.0);
		fprintf(f, "\n");
	}

	/* close file */
	fclose(f);

	/* restore last position */
	sdiag->setPosition(pos);
#endif
}

void MainWindow::OnSaveReduced(wxCommandEvent& WXUNUSED(event))
{
  //FIXME
#if 0
	int k;
	if ( sdiag == NULL ) return;

	/* get ratio */
	int res = (int)wxGetNumberFromUser( _T("Enter the ratio to reduce the data.\n")
		_T("from the original file."),
		_T("1 /"), _T("Save CSV reduced ..."),
		10, 1, 10000, this );
	if ( res == -1 ) return;

	/* get file name */
	wxFileDialog dialog(this,
		_T("Save CSV reduced file"),
		wxEmptyString,
		_T("statistics.csv"),
		_T("Text CSV (*.csv)|*.csv|Text files (*.txt)|*.txt"),
		wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
	dialog.SetFilterIndex(0);
	if (dialog.ShowModal() != wxID_OK) return;
	printf("%s (%d) - ratio 1 / %d\n", dialog.GetPath().c_str(), dialog.GetFilterIndex(), res);

	/* get current position */
	int pos = sdiag->getPosition();

	/* open file */
	FILE *f = fopen(dialog.GetPath().c_str(), "w");
	if ( f == NULL )
	{
		printf("Cannot save file %s\n", dialog.GetPath().c_str());
		wxMessageBox("Cannot save file", "ERROR", wxICON_ERROR);
		return;
	}
	fprintf(f, "Timestamp;Number of players;Number of regions;Quest active;");
	for ( k = 0; k < map_data.num_servers; k++ )
		fprintf(f, "Players S%d;Regions S%d;CPU S%d;Update interval S%d (ms);"
			"TCP send S%d (kbps);TCP recv S%d (kbps);"
			"UDP send S%d (kbps);UDP recv S%d(kbps);",
			k,k,k,k,k,k,k,k);
	fprintf(f,"\n");

	/* save data */
	int n = sdiag->getNumberOfRecords();
	for ( int i = 0; i < n; i++ )
	{
		if ( i % res != 0 ) continue;
		sdiag->loadPosition(i);
		number_of_players = 0;
		for ( int j = 0; j < map_data.num_servers; j++ )
			number_of_players += ss[j].statistics.number_of_players;
		fprintf(f, "%d;%d;%d;%d;",
			timestamp,number_of_players,
			(map_data.nregx * map_data.nregy),quest_active);
		for ( k = 0; k < map_data.num_servers; k++ )
			fprintf(f, "%d;%d;%d;%d;%.2f;%.2f;%.2f;%.2f;",
				ss[k].statistics.number_of_players,
				ss[k].statistics.number_of_regions,
				ss[k].statistics.machine_cpu_usage,
				(int)ss[k].statistics.average_regular_update_interval,
				ss[k].statistics.bps_tcp_sent / 1000.0,
				ss[k].statistics.bps_tcp_recv / 1000.0,
				ss[k].statistics.bps_udp_sent / 1000.0,
				ss[k].statistics.bps_udp_recv / 1000.0);
		fprintf(f, "\n");
	}

	/* close file */
	fclose(f);

	/* restore last position */
	sdiag->setPosition(pos);
#endif
}

void MainWindow::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	wxAboutDialogInfo info;
	info.SetName(_T("GameMonitor"));
	info.SetVersion(wxVERSION_NUM_DOT_STRING_T);
	info.SetDescription(_T("This application is used to monitor the game servers"));
	info.SetCopyright(_T("(C) 2007 Mihai Paslariu"));
	info.AddDeveloper(_T("Mihai Paslariu"));
	wxAboutBox(info);
}

void MainWindow::OnTimer(wxTimerEvent& event)
{
	UpdateHTMLControls();
}

/***************************************************************************************************
*
* Formatting methods
*
***************************************************************************************************/

void MainWindow::randomColors(int n)
{
	if ( n > MAX_SERVERS ) n = MAX_SERVERS;

		int base = rand() % 360;
	int dif = 360 / n;
	for ( int i = 0; i < n; i++ )
	{
		int rr,gg,bb;
		hsv2rgb( (base +  dif * i) % 360 , 110,255, &rr,&gg,&bb);
		table_color[i] = wxColor(rr,gg,bb);
	}
	server_colors = n;
}

wxString MainWindow::printAddress(IPaddress a)
{
	wxString s;
	Uint8 b1,b2,b3,b4;
	Uint16 port;

	port = ( ( a.port & 0xFF ) << 8 ) | ( a.port >> 8 );
	b1 = a.host & 0xFF;
	b2 = ( a.host >> 8 )& 0xFF;
	b3 = ( a.host >> 16 )& 0xFF;
	b4 = ( a.host >> 24 )& 0xFF;

	s.Printf(L"%u.%u.%u.%u:%d", b1,b2,b3,b4, port);
	return s;
}

wxString MainWindow::printColor(wxColor c)
{
	wxString s;
	s.Printf(L"#%X%X%X%X%X%X",
		c.Red() >> 4, c.Red() & 0xF,
		c.Green() >> 4, c.Green() & 0xF,
		c.Blue() >> 4, c.Blue() & 0xF);
	return s;
}

wxString MainWindow::printDouble(double lf)
{
	wxString s;
	s.Printf(L"%.2lf", lf);
	return s;
}

/***************************************************************************************************
*
* Update Methods
*
***************************************************************************************************/

void MainWindow::UpdateHTMLControls()
{
	global_lock.Lock();

	if ( map_data.num_servers < server_colors )
		randomColors(map_data.num_servers);

	UpdateGeneral();
	UpdateServers();
	UpdateRegions();
	UpdateLoad();

	global_lock.Unlock();
}

void MainWindow::UpdateGeneral()
{
	wxString c;
	c << L"<html><head><title>Game Monitor</title></head><body>";
	c << L"<b>Last timestamp: </b>" << timestamp << L"<br />";
	c << L"<b>Number of servers: </b>" << map_data.num_servers << L"<br />";
	number_of_players = 0;
	for ( int i = 0; i < map_data.num_servers; i++ )
		number_of_players += ss[i].statistics.number_of_players;
	c << L"<b>Number of players: </b>" << number_of_players << L"<br />";
	c << L"<b>Regular update interval (SLA): </b>"
		<< map_data.regular_update_interval << L"<br />";
	c << L"<b>Partitioning algorithm: </b>"
	  << wxString(wxConvUTF8.cMB2WC(map_data.algorithm_name)) << L"<br />";
	c << L"<br />";

	c << L"<b>Map size: </b>" << map_data.mapx << L" x "
		<< map_data.mapy << L"<br />";
	c << L"<b>Region size: </b>" << map_data.regx << L" x "
		<< map_data.regy << L"<br />";
	c << L"<b>Number of regions: </b>" << map_data.nregx << L" x " << map_data.nregy
		<< L" (" << (map_data.nregx * map_data.nregy) << L")<br />";
	c << L"<br />";


	c << L"<b>Min/Max quest interval: </b>" << map_data.quest_min << L"/"
		<< map_data.quest_max << L" seconds<br />";
	c << L"<b>Maximum interval between quests: </b>" << map_data.quest_between << L" seconds<br />";
	if ( quest_active )
		c << L"<b>Current quest:</b> " << quest_x << L"," << quest_y << L"<br />";
	else
		c << L"<b>Current quest:</b> no quest active<br />";
	c << L"<b>Quest bonus: </b>" << map_data.quest_bonus << L"<br />";
	c << L"<br />";

	c << L"<b>Number of player migrations: </b>" << master_stats.player_migrations << L"<br />";
	c << L"<b>Number of region migrations: </b>" << master_stats.region_migrations << L"<br />";
	c << L"<br />";

	c << wxString(L"</body></html>");
	wh_general->SetPage(c);
}

void MainWindow::UpdateServers()
{
	wxString c;
	double x;
	c << L"<html><head><title>Game Monitor</title></head><body>";

	c << L"<table border=\"0\" width=\"100%\">";
	for ( int i = 0; i < map_data.num_servers; i++ )
	{
		if ( i % server_columns == 0 ) c << L"<tr>";
		c << L"<td bgcolor=\"" << printColor(table_color[i]) << L"\">";
		c << L"<h5><b>Server " << i << L" - ";
		c << L"<font color=\"#0000ff\"> " << printAddress(ss[i].udp_connection) << L"</font>";
		c << L"</b></h5><br/><br />";
		c << L"<font size=\"2\">";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Number of regions: </b>" <<
			ss[i].statistics.number_of_regions << L"<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Number of players: </b>" <<
			ss[i].statistics.number_of_players << L"<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Player density: </b>" <<
			(( ss[i].statistics.number_of_regions == 0 ) ? 0 :
			( ss[i].statistics.number_of_players / ss[i].statistics.number_of_regions ))
			<< L"<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Players in most crowded region: </b>" <<
			ss[i].statistics.players_in_most_crowded_region << L"<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>CPU usage (machine): </b>" <<
			ss[i].statistics.machine_cpu_usage << L"%<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Memory usage (machine): </b>"
			<< (ss[i].statistics.machine_mem_usage/1024) << L"MB<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>CPU usage (process): </b>" <<
			ss[i].statistics.process_cpu_usage << L"%<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Memory usage (process): </b>"
			<< printDouble(ss[i].statistics.process_mem_usage/1024.0) << L"MB<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Number of threads: </b>" <<
			ss[i].statistics.number_of_threads << L"<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Average regular update interval: </b>"
			<< (int)ss[i].statistics.average_regular_update_interval << L" ms<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Real last regular update interval: </b>"
			<< (int)ss[i].statistics.average_real_regular_update_interval << L" ms<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Master TCP connection (sent): </b>"
			<< printDouble(ss[i].statistics.bps_tcp_sent / 1000.0 ) << L" kb/s<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Master TCP connection (recv): </b>"
			<< printDouble(ss[i].statistics.bps_tcp_recv / 1000.0 ) << L" kb/s<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Clients UDP connection (sent): </b>"
			<< printDouble(ss[i].statistics.bps_udp_sent / 1000.0 ) << L" kb/s<br />";
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Clients UDP connection (recv): </b>"
			<< printDouble(ss[i].statistics.bps_udp_recv / 1000.0 ) << L" kb/s<br />";
		x = ( ss[i].statistics.number_of_statistics == 0 ) ? 0 :
			( ss[i].statistics.tcp_total / ss[i].statistics.number_of_statistics );
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Average TCP transfer: </b>"
			<< printDouble( x / 1000.0 ) << L" kb/s<br />";
		x = ( ss[i].statistics.number_of_statistics == 0 ) ? 0 :
			( ss[i].statistics.udp_total / ss[i].statistics.number_of_statistics );
		c << L"&nbsp;&nbsp;&nbsp;&nbsp;<b>Average UDP transfer: </b>"
			<< printDouble( x / 1000.0 ) << L" kb/s<br />";
		c << L"</font>";
		c << L"</td>";
		if ( i % server_columns == server_columns - 1 ) c << L"</tr>";
	}
	c << L"</table>";
	c << L"</body></html>";
	wh_servers->SetPage(c);
}

void MainWindow::UpdateRegions()
{
	wxString c;
	int i,j,np;

	np = 0;
	for ( i = 0; i < map_data.nregx; i++ )
		for ( j = 0; j < map_data.nregy; j++ )
			np += players_per_region[i][j];


	c << L"<html><head><title>Game Monitor</title></head><body>";
	c << L"<font size=\"2\">";

	c << L"<table border=\"0\"><tr>";
	for ( int i = 0; i < map_data.num_servers; i++ )
	{
		c << L"<td bgcolor=\"" << printColor(table_color[i]) << L"\">";
		c << L"<b>S" << i << L"</b>: " << ss[i].statistics.number_of_players;
		c << L"</td>";
	}
	c << L"<td bgcolor=\"#FFFFFF\"><b>Total</b>: " << number_of_players << L"</td>";
	c << L"<td bgcolor=\"#FFFFFF\">(from map: " << np << L")</td>";
	c << L"</tr></table>";

	c << L"<table width=\"100%\" border=\"0\">";
	for ( i = 0; i < map_data.nregx; i++ )
	{
		c << L"<tr valign=\"center\">";
		for ( j = 0; j < map_data.nregy; j++ )
		{
			c << L"<td align=\"center\" bgcolor=\"";
			if ( layout[i][j] >= 0 && layout[i][j] < MAX_SERVERS )
				c << printColor(table_color[layout[i][j]]) << L"\" >";
			else
				c << L"white" << L"\" >";
			c << L"<b>S" << layout[i][j] << L" </b> ";
			c << L"(" << i << L"," << j << L") ";
			if ( quest_x / map_data.regx == i && quest_y / map_data.regy == j
				&& quest_active )
				c << L"<font color=\"red\"><b>Q</b></font>";
			c << L"<br />";
			c << players_per_region[i][j] << L" players<br />";
			c << (i * map_data.regx) << L"," << (j * map_data.regy);
			c << L"</td>";
		}
		c << L"</tr>";
	}
	c << L"</table>";

	c << L"</font>";
	c << L"</body></html>";
	wh_regions->SetPage(c);
}

void MainWindow::UpdateLoad()
{
	wxString c;
	wxColor color;
	double rv;
	int v;

	c << L"<html><head><title>Game Monitor</title></head><body>";
	c << L"<font size=\"2\">";
	c << L"<table width=\"100%\" border=\"0\">";
	for ( int i = 0; i < map_data.nregx; i++ )
	{
		c << L"<tr valign=\"center\">";
		for ( int j = 0; j < map_data.nregy; j++ )
		{
			/* get color */
			if ( number_of_players == 0 )
			{
				color = wxColor(0,255,0);
			} else {
				rv = players_per_region[i][j] / (double)number_of_players;
				v = (int)(255 * rv);
				color = wxColor(128 + v/2,255-v,0);
			}

			/* display page */
			c << L"<td align=\"center\" bgcolor=\"";
			if ( layout[i][j] >= 0 && layout[i][j] < MAX_SERVERS )
				c << printColor(color) << L"\" >";
			else
				c << L"white" << L"\" >";
			c << L"<b>S" << layout[i][j] << L" </b> ";
			c << L"(" << i << L"," << j << L") ";
			if ( quest_x / map_data.regx == i && quest_y / map_data.regy == j
				&& quest_active )
				c << L"<font color=\"red\"><b>Q</b></font>";
			c << L"<br />";
			c << players_per_region[i][j] << L" players<br />";
			c << (i * map_data.regx) << L"," << (j * map_data.regy);
			c << L"</td>";
		}
		c << L"</tr>";
	}
	c << L"</table>";
	c << L"</font>";
	c << L"</body></html>";
	wh_load->SetPage(c);
}
