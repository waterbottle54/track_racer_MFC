
// TrackRacerView.h : interface of the CTrackRacerView class
//

#pragma once
#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")
#pragma comment(lib, "winmm")
#include "resource.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include <mmsystem.h>
#include "includes\Utility.h"
#include "includes\Mp3.h"
#include "includes\surface.h"

#define IDT_MAIN		0
#define IDT_INITSOUND	1
#define VK_X			0x58
#define VK_Z			0x5A

class CTrackRacerView : public CView
{
	DECLARE_DYNCREATE(CTrackRacerView)
	// Contructor/Destructors
protected:
	CTrackRacerView();

	// Attributes
public:
	HDC		m_hDC;			// Device context
	HGLRC	m_hRC;			// Rendering context
	CRect	m_rtClient;		// Client rect
	CRect	m_rtViewport;	// Viewport rect
	double	m_eyeTort;
	double	m_eyeTens;

	byte*	m_bkgnTexture;	// Background texture
	CSize	m_bkgnSize;		// Background size
	UINT	m_nBkgnID;		// Background ID
	bool	m_bBkgnDay;		// Is background night

	HSURFACE m_aCarSurface[256]; // car surfaces
	int		 m_nCarSurface;		 // number of car surfaces
	HSURFACE m_aFlagSurface[256]; // flag surfaces
	int		 m_nFlagSurface;	  // number of '' 

	bool	m_bFullView;	// Is full view mode
	CWnd*	m_formerWnd;	// Saved former window

	int		m_nElapse;		// Timer period
	double	m_mapScale;		// Track scale

	Mp3		m_sndStart;
	Mp3		m_sndMotor;
	Mp3		m_sndAccel;
	Mp3		m_sndDrive;
	Mp3		m_sndHorn;

	CTrackRacerDoc* GetDocument() const;

	// Operations
public:
	void GLPrepare();
	void GLUnprepare();
	void GLInit();
	void GLUpdate();
	void GLReshape(int cx, int cy);
	void GLDisplay(CTrackRacerDoc* pDoc);
	void InitSound();
	void UpdateSound();

	// Overidables
public:
	virtual void OnDraw(CDC* pDC);  // Overriden for redraw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
protected:

	// Implements
public:
	virtual ~CTrackRacerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Created message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnViewFull();
	afx_msg void OnBackground(UINT nID);
	afx_msg void OnUpdateBackground(CCmdUI* pCmd);
	afx_msg void OnBackgroundDay();
	afx_msg void OnUpdateBackgroundDay(CCmdUI *pCmdUI);
	afx_msg void OnBackgroundNight();
	afx_msg void OnUpdateBackgroundNight(CCmdUI *pCmdUI);
	afx_msg void OnHelpInstruction();
};

#ifndef _DEBUG
inline CTrackRacerDoc* CTrackRacerView::GetDocument() const
{
	return reinterpret_cast<CTrackRacerDoc*>(m_pDocument);
}
#endif

