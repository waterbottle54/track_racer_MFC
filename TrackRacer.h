
// TrackRacer.h : main header file for the TrackRacer application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CTrackRacerApp:
// See TrackRacer.cpp for the implementation of this class
//

class CTrackRacerApp : public CWinApp
{
public:
	CTrackRacerApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CTrackRacerApp theApp;
