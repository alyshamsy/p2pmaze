
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
#include <time.h>
#include <wx/wx.h>

#include "SDL_replace.h"
#include "Constants.h"
#include "Structs.h"

#include "SlowReader.h"
#include "MainWindow.h"
#include "SliderDialog.h"
#include "ReadThread.h"

/***************************************************************************************************
*
* Global variables
*
***************************************************************************************************/

SlowReader sr;
MainWindow *frame = NULL;
SliderDialog *sdiag = NULL;

MapData map_data;
MasterStatistics master_stats;
Uint32 timestamp;
ConnectedServerInfo ss[MAX_SERVERS];
int **layout;
int **players_per_region;
int quest_active,quest_x,quest_y;
wxMutex global_lock;

/***************************************************************************************************
*
* Application class
*
***************************************************************************************************/

class MiniApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
};

IMPLEMENT_APP(MiniApp)

bool MiniApp::OnInit()
{
	bool offline = false;
	wxString filename;

	if ( argc < 2 )
	{
		wxMessageBox(L"Program must receive the name of the input file",
		L"Error", wxOK | wxCENTRE | wxICON_ERROR);
		printf("Error: Program must receive the name of the input file\n");
		return false;
	}

	filename = argv[1];
	if ( argc >= 3 && !strcmp((char*)argv[1], "--offline") )
	{
		offline = true;
		filename = argv[2];
	}

	/* open file for reading */
	if ( !sr.open(filename.mb_str()) )
	{
		wxMessageBox(L"Cannot open input file",
		L"Error", wxOK | wxCENTRE | wxICON_ERROR);
		printf("Error: Cannot open input file\n");
		return false;
	}
	sr.read(&map_data, sizeof(MapData));

	srand(time(NULL));

	/* create a new thread for reading */
	ReadThread *rt = new ReadThread(&sr);
	layout = rt->new_int_matrix(map_data.nregx, map_data.nregy);
	players_per_region = rt->new_int_matrix(map_data.nregx, map_data.nregy);

	/* create and display main window */
	frame = new MainWindow(offline);
	SetTopWindow(frame);
	frame->Show(true);

	if ( offline )
	{
		int dx,dy,px,py;
		frame->GetScreenPosition(&px,&py);
		frame->GetSize(&dx,&dy);
		sdiag = new SliderDialog(frame,
			wxPoint(px+dx-320,py+50), wxSize(320,80), &sr);
		sdiag->Show();
	} else {
		rt->Run();
	}

	return true;
}

int MiniApp::OnExit()
{
	//sr.close();
	return 0;
}
