
// TrackRacerView.cpp : implementation of the CTrackRacerView class
//

#include "stdafx.h"
// SHARED_HANDLERS는 미리 보기, 축소판 그림 및 검색 필터 처리기를 구현하는 ATL 프로젝트에서 정의할 수 있으며
// 해당 프로젝트와 문서 코드를 공유하도록 해 줍니다.
#ifndef SHARED_HANDLERS
#include "TrackRacer.h"
#endif

#include "TrackRacerDoc.h"
#include "TrackRacerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTrackRacerView

IMPLEMENT_DYNCREATE(CTrackRacerView, CView)

BEGIN_MESSAGE_MAP(CTrackRacerView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_VIEW_FULLVIEW, &CTrackRacerView::OnViewFull)
	ON_COMMAND_RANGE(ID_BACKGROUND_GRASS, ID_BACKGROUND_SNOW, &CTrackRacerView::OnBackground)
	ON_UPDATE_COMMAND_UI_RANGE(ID_BACKGROUND_GRASS, ID_BACKGROUND_SNOW, &CTrackRacerView::OnUpdateBackground)
	ON_COMMAND(ID_BACKGROUND_DAY, &CTrackRacerView::OnBackgroundDay)
	ON_UPDATE_COMMAND_UI(ID_BACKGROUND_DAY, &CTrackRacerView::OnUpdateBackgroundDay)
	ON_COMMAND(ID_BACKGROUND_NIGHT, &CTrackRacerView::OnBackgroundNight)
	ON_UPDATE_COMMAND_UI(ID_BACKGROUND_NIGHT, &CTrackRacerView::OnUpdateBackgroundNight)
	ON_COMMAND(ID_HELP_INSTRUCTION, &CTrackRacerView::OnHelpInstruction)
END_MESSAGE_MAP()

// CTrackRacerView 생성/소멸

CTrackRacerView::CTrackRacerView()
{
}

CTrackRacerView::~CTrackRacerView()
{
}

BOOL CTrackRacerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

int CTrackRacerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_bkgnTexture = NULL;
	m_bFullView = false;
	m_formerWnd = this;
	m_nElapse = 30;
	m_mapScale = 3;

	// Prepare rendering
	m_hDC = ::GetDC(m_hWnd);
	GLPrepare();

	// Set background options
	OnBackground(ID_BACKGROUND_GRASS);
	OnBackgroundDay();

	// load surface data
	m_nCarSurface = LoadSurface(m_aCarSurface, L"surfaces\\car");
	m_nFlagSurface = LoadSurface(m_aFlagSurface, L"surfaces\\flag");

	return 0;
}

void CTrackRacerView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	m_eyeTort = 0;
	m_eyeTens = 0;

	// Initialize GL
	GLInit();

	// Set timers
	SetTimer(IDT_MAIN, m_nElapse, NULL);
	SetTimer(IDT_INITSOUND, 100, NULL);
}

void CTrackRacerView::OnDestroy()
{
	CView::OnDestroy();

	// Kill main timer
	KillTimer(IDT_MAIN);

	// Finish rendering
	GLUnprepare();
	::ReleaseDC(m_hWnd, m_hDC);

	// Delete backgroud texture data
	if (m_bkgnTexture != NULL)
		delete[] m_bkgnTexture;

	// Delete surface data
	for (int i = 0; i < m_nCarSurface; i++)
		DeleteSurface(m_aCarSurface[i]);
	for (int i = 0; i < m_nFlagSurface; i++)
		DeleteSurface(m_aFlagSurface[i]);

	// Cleanup all sound effects
	m_sndStart.Cleanup();
	m_sndMotor.Cleanup();
	m_sndAccel.Cleanup();
	m_sndDrive.Cleanup();
	m_sndHorn.Cleanup();

}


// CTrackRacerView 그리기

void CTrackRacerView::GLPrepare()
{
	PIXELFORMATDESCRIPTOR pfd;
	int nPixelFormat;

	// Set pixel format for openGL
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 0;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL;
	nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	SetPixelFormat(m_hDC, nPixelFormat, &pfd);

	// Create rendering context
	m_hRC = wglCreateContext(m_hDC);
	wglMakeCurrent(m_hDC, m_hRC);
}

void CTrackRacerView::GLUnprepare()
{
	wglMakeCurrent(m_hDC, NULL);
	wglDeleteContext(m_hRC);
}

void CTrackRacerView::GLInit()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

void CTrackRacerView::GLUpdate()
{
	CTrackRacerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	// Update viewing matrix
	CXYZ carPos, eyePos;
	double carLon;
	char carHan;
	pDoc->GetCarPosition(carPos);
	pDoc->GetCarDirection(carLon);
	carHan = pDoc->GetCarHandleState();
	eyePos.x = carPos.x - (7.0 + m_eyeTens) * cos(RAD(carLon-m_eyeTort));
	eyePos.y = 4;
	eyePos.z = carPos.z - (7.0 + m_eyeTens) * sin(RAD(carLon-m_eyeTort));
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyePos.x, eyePos.y, eyePos.z,
		carPos.x, carPos.y, carPos.z,
		0, 1, 0);
}

void CTrackRacerView::GLReshape(int cx, int cy)
{
	m_rtViewport = CRect(0, cy - 650, cx, cy - 200);

	// Reset viewport matrix
	glMatrixMode(GL_VIEWPORT);
	glLoadIdentity();
	glViewport(m_rtViewport.left, cy - m_rtViewport.bottom,
		m_rtViewport.Width(), m_rtViewport.Height());
	glScissor(m_rtViewport.left, cy - m_rtViewport.bottom,
		m_rtViewport.Width(), m_rtViewport.Height());

	// Reset projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, (float)m_rtViewport.Width() / m_rtViewport.Height(), 1, 1000);
}

void CTrackRacerView::GLDisplay(CTrackRacerDoc* pDoc)
{
	GLfloat ambient[4] = { 0.5, 0.5, 0.5, 1 };
	GLfloat diffuse[4] = { 0.3, 0.3, 0.3, 1 };
	GLfloat light[4] = { 0, 0, 0, 1 };
	GLUquadric* pQuad;

	// Clear rendering buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Prepare quadric object
	pQuad = gluNewQuadric();
	gluQuadricDrawStyle(pQuad, GL_FILL);

	// Set light
	CXYZ carPos, carDir;
	pDoc->GetCarPosition(carPos);
	pDoc->GetCarDirection(carDir);
	light[0] = carPos.x - 5*carDir.x;
	light[1] = 1;
	light[2] = carPos.z - 5*carDir.z;
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light);

	// Render a background texture
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
				 m_bkgnSize.cx, m_bkgnSize.cy,
				 0, GL_RGB, GL_UNSIGNED_BYTE, m_bkgnTexture);

	double texX, texZ, tileLen=10;
	texX = floor(carPos.x / tileLen) * tileLen;
	texZ = floor(carPos.z / tileLen) * tileLen;

	glPushMatrix();
	glTranslated(texX, 0, texZ);
	glBegin(GL_QUADS);
	for (int x = -100; x < 100; x++)
	{
		for (int z = -100; z < 100; z++)
		{
			glTexCoord2d(0, 0);
			glVertex3d(tileLen * x, 0, tileLen * z);
			glTexCoord2d(0, 1);
			glVertex3d(tileLen * x, 0, tileLen * (z + 1));
			glTexCoord2d(1, 1);
			glVertex3d(tileLen * (x + 1), 0, tileLen * (z + 1));
			glTexCoord2d(1, 0);
			glVertex3d(tileLen * (x + 1), 0, tileLen * z);
		}
	}
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);

	// Render a racing track
	CSection**	trackData;
	size_t		trackSize;
	int			trackLaneCount;
	double		trackLaneWidth;
	trackSize = pDoc->GetTrackSize();
	trackData = new CSection*[trackSize];
	pDoc->GetTrackData(trackData);
	trackLaneCount = pDoc->GetTrackLaneCount();
	trackLaneWidth = pDoc->GetTrackLaneWidth();

	for (int i = 0; i < trackSize; i++)
	{
		if (trackData[i]->IsKindOf(RUNTIME_CLASS(CLine)))
			LinearLoad(
				(CLine*)trackData[i], true, trackLaneCount, trackLaneWidth, 0.18);
		else if (trackData[i]->IsKindOf(RUNTIME_CLASS(CCurve)))
			CurvedLoad(
				pQuad, (CCurve*)trackData[i], true, trackLaneCount, trackLaneWidth, 0.18);
	}
	for (int i = 0; i < trackSize; i++)
	{
		if (trackData[i]->IsKindOf(RUNTIME_CLASS(CLine)))
			LinearLoad(
				(CLine*)trackData[i], false, trackLaneCount, trackLaneWidth, 0.18);
		else if (trackData[i]->IsKindOf(RUNTIME_CLASS(CCurve)))
			CurvedLoad(
				pQuad, (CCurve*)trackData[i], false, trackLaneCount, trackLaneWidth, 0.18);
	}

	// render flag
	POINTF ptBegin, uBegin;
	POINTF ptEnd, uEnd;
	ptBegin = trackData[0]->GetBegin();
	uBegin = trackData[0]->GetTangent(true);
	ptEnd = trackData[trackSize-1]->GetEnd();
	uEnd = trackData[trackSize - 1]->GetTangent(false);

	glPushMatrix();
	glTranslated(ptBegin.x, 0.05, ptBegin.y);
	glRotated(DEG(atan2(uBegin.y, uBegin.x)) - 90, 0, -1, 0);
	glScaled(2.3,2.3,1.5);
	for (int i = 0; i < m_nFlagSurface; i++)
		RenderSurface(m_aFlagSurface[i]);
	for (int i = 0; i < m_nFlagSurface; i++)
		BorderSurface(m_aFlagSurface[i]);
	glPopMatrix();

	glPushMatrix();
	glTranslated(ptEnd.x, 0.05, ptEnd.y);
	glRotated(DEG(atan2(uEnd.y, uEnd.x)) - 90, 0, -1, 0);
	glScaled(2.3, 2.3, 1.5);
	for (int i = 0; i < m_nFlagSurface; i++)
		RenderSurface(m_aFlagSurface[i]);
	glPopMatrix();

	delete[] trackData;

	// Render a racing car

	double carLon;
	pDoc->GetCarDirection(carLon);

	glPushMatrix();
	glTranslated(carPos.x, carPos.y+0.1, carPos.z);
	glRotated(carLon - 90, 0, -1, 0);
	glScaled(0.4, 0.4, 0.4);
	for (int i = 0; i < m_nCarSurface; i++)
		RenderSurface(m_aCarSurface[i]);
	for (int i = 0; i < m_nCarSurface; i++)
		BorderSurface(m_aCarSurface[i]);
	glPopMatrix();

	// render best car
	if (pDoc->IsBestRecordExist())
	{
		CTrackRacerDoc::RACERECORDFRAME recordFrame;
		pDoc->GetBestRecordFrame(recordFrame);

		glPushMatrix();
		glTranslated(recordFrame.x, 0.1, recordFrame.z);
		glRotated(recordFrame.lon - 90, 0, -1, 0);
		glScaled(0.4, 0.4, 0.4);
		for (int i = 0; i < m_nCarSurface; i++)
			RenderSurface(m_aCarSurface[i]);
		for (int i = 0; i < m_nCarSurface; i++)
			BorderSurface(m_aCarSurface[i]);
		glPopMatrix();
	}

	// Finish rendering
	glFinish();
	gluDeleteQuadric(pQuad);
}

void CTrackRacerView::OnDraw(CDC* pDC)
{
	CTrackRacerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	CPen myPen, *oldPen;
	CBrush myBrush, *oldBrush;
	CString text;

	// Display OpenGL part of screen
	GLDisplay(pDoc);

	// Fill margin area of screen
	pDC->SelectStockObject(BLACK_BRUSH);
	pDC->Rectangle(m_rtClient.left, 0,
		m_rtClient.right, m_rtViewport.top);
	pDC->Rectangle(m_rtClient.left, m_rtViewport.bottom,
		m_rtClient.right, m_rtClient.bottom);

	/* Paint dashboard of car */

	// Get dashboard data
	double spd, maxSpd, limSpd;
	pDoc->GetCarLimitSpeed(limSpd);
	pDoc->GetCarSpeed(spd);
	pDoc->GetCarMaxSpeed(maxSpd);

	// Set drawing options
	myPen.CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	oldPen = pDC->SelectObject(&myPen);
	oldBrush = (CBrush*)pDC->SelectStockObject(DKGRAY_BRUSH);
	pDC->SetTextColor(RGB(0, 255, 0));
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetViewportOrg(m_rtClient.right / 4,
		m_rtClient.bottom);

	// Paint RGM gauge
	PaintGauge(*pDC, -100, -30, 80, 120,
		6, 1, 6 * (spd / limSpd));
	// Paint Speed gauge
	PaintGauge(*pDC, 120, -30, 110, 150,
		KPH(maxSpd), 30, KPH(spd));

	// Restore drawing options
	pDC->SelectObject(oldPen);
	myPen.DeleteObject();
	pDC->SelectObject(oldBrush);
	pDC->SetViewportOrg(0, 0);

	/* Paint map of track */

	// Get track data
	CSection** trackData; size_t trackSize;
	trackSize = pDoc->GetTrackSize();
	trackData = new CSection*[trackSize];
	pDoc->GetTrackData(trackData);
	// Get car position
	CXYZ carPos;
	pDoc->GetCarPosition(carPos);

	// Prepare bitmap on which map is painted
	CDC mapDC; CBitmap mapBmp;
	CRect rtMap(m_rtViewport.right - 400, m_rtViewport.bottom + 30, 
				m_rtViewport.right - 100, m_rtClient.bottom - 20);
	mapDC.CreateCompatibleDC(pDC);
	mapBmp.CreateCompatibleBitmap(pDC, rtMap.Width(), rtMap.Height());
	mapDC.SelectObject(mapBmp);

	// Fill background
	oldBrush = (CBrush*)mapDC.SelectStockObject(DKGRAY_BRUSH);
	mapDC.RoundRect(0, 0, rtMap.Width(), rtMap.Height(), 20, 20);
	
	// Set origin point to center
	mapDC.SetViewportOrg(rtMap.Width()/2, rtMap.Height()/2);

	// Paint track (set of lines and curves)

	myPen.CreatePen(PS_SOLID, 5, RGB(128, 128, 128));
	oldPen = mapDC.SelectObject(&myPen);

	CLine* line; CCurve* curve;
	mapDC.MoveTo((trackData[0]->GetBegin().x - carPos.x) / m_mapScale,
		(trackData[0]->GetBegin().y - carPos.z) / m_mapScale);

	for (int i = 0; i < trackSize; i++)
	{
		if (trackData[i]->IsKindOf(RUNTIME_CLASS(CLine)))
		{
			line = (CLine*)trackData[i];
			mapDC.LineTo((line->end.x - carPos.x) / m_mapScale,
				(line->end.y - carPos.z) / m_mapScale);
		}
		else if (trackData[i]->IsKindOf(RUNTIME_CLASS(CCurve)))
		{
			curve = (CCurve*)trackData[i];
			mapDC.AngleArc((curve->pivot.x - carPos.x) / m_mapScale,
				(curve->pivot.y - carPos.z) / m_mapScale,
				curve->radius / m_mapScale,
				-curve->startAngle, -curve->sweepAngle);
		}
	}
	mapDC.SelectObject(oldPen);
	myPen.DeleteObject();

	// Display car position of best record
	if (pDoc->IsBestRecordExist())
	{
		CTrackRacerDoc::RACERECORDFRAME recordFrame;
		pDoc->GetBestRecordFrame(recordFrame);

		oldBrush = (CBrush*)mapDC.SelectStockObject(BLACK_BRUSH);
		mapDC.Ellipse(-5 + (recordFrame.x - carPos.x) / m_mapScale,
			-5 + (recordFrame.z - carPos.z) / m_mapScale,
			5 + (recordFrame.x - carPos.x) / m_mapScale,
			5 + (recordFrame.z - carPos.z) / m_mapScale);
		mapDC.SelectObject(oldBrush);
	}

	// Display car position
	oldBrush = (CBrush*)mapDC.SelectStockObject(DKGRAY_BRUSH);
	mapDC.Ellipse(-5, -5, 5, 5);
	mapDC.SelectObject(oldBrush);

	// Print bitmap on screen
	mapDC.SetViewportOrg(0, 0);
	pDC->BitBlt(rtMap.left, rtMap.top, rtMap.Width(), rtMap.Height(),
				&mapDC, 0, 0, SRCPAINT);
	mapBmp.DeleteObject();
	mapDC.DeleteDC();

	// Free memory
	delete[] trackData;

}


// CTrackRacerView 소리

void CTrackRacerView::InitSound()
{
	// Load all sound effects and init those volume
	wchar_t filePath[1024];
	if (!m_sndStart.IsReady())
	{
		GetFullPathName(L"sounds\\startup.mp3", 1024, filePath, NULL);
		m_sndStart.Load(filePath);
		m_sndStart.SetVolume(MILLIBEL(1.0));
	}
	if (!m_sndMotor.IsReady())
	{
		GetFullPathName(L"sounds\\motor.mp3", 1024, filePath, NULL);
		m_sndMotor.Load(filePath);
		m_sndMotor.SetVolume(MILLIBEL(1.0));
	}
	if (!m_sndAccel.IsReady())
	{
		GetFullPathName(L"sounds\\accel.mp3", 1024, filePath, NULL);
		m_sndAccel.Load(filePath);
		m_sndAccel.SetVolume(MILLIBEL(1.0));
	}
	if (!m_sndDrive.IsReady())
	{
		GetFullPathName(L"sounds\\drive.mp3", 1024, filePath, NULL);
		m_sndDrive.Load(filePath);
		m_sndDrive.SetVolume(MILLIBEL(1.0));
	}
	if (!m_sndHorn.IsReady())
	{
		GetFullPathName(L"sounds\\horn.mp3", 1024, filePath, NULL);
		m_sndHorn.Load(filePath);
		m_sndHorn.SetVolume(MILLIBEL(1.0));
	}

	// Stop all car sounds
	m_sndStart.Stop();
	m_sndMotor.Stop();
	m_sndDrive.Stop();
	m_sndHorn.Stop();

	// Play car startup sound
	m_sndStart.Play(true);
}

void CTrackRacerView::UpdateSound()
{
	CTrackRacerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	double carSpd; char carPedal;
	pDoc->GetCarSpeed(carSpd);
	carPedal = pDoc->GetCarPedalState();

	// Play motor sound of car
	if (m_sndStart.IsCompleted(2000000))
	{
		double volume = 1 - (carSpd / MPS(200));
		m_sndMotor.SetVolume(MILLIBEL(volume));
		m_sndMotor.Play();
	}

	// Play driving sound of car after accel is finished
	if (m_sndAccel.IsCompleted(2000000))
		m_sndDrive.Play();

	// Stop driving sound at low speed
	if (carSpd < MPS(30))
		m_sndDrive.Stop();

	// Preserve sounds
	if (m_sndMotor.IsCompleted())
		m_sndMotor.Rewind();

	if (m_sndDrive.IsCompleted())
		m_sndDrive.Rewind();
}


// CTrackRacerView 진단

#ifdef _DEBUG
void CTrackRacerView::AssertValid() const
{
	CView::AssertValid();
}

void CTrackRacerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTrackRacerDoc* CTrackRacerView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTrackRacerDoc)));
	return (CTrackRacerDoc*)m_pDocument;
}
#endif //_DEBUG


// CTrackRacerView 메시지 처리기

void CTrackRacerView::OnUpdate(CView *, LPARAM, CObject *)
{
	// Update GL
	GLUpdate();

	// Update screen
	Invalidate(false);

	// Update sound
	UpdateSound();
}

void CTrackRacerView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == IDT_INITSOUND)
	{
		KillTimer(IDT_INITSOUND);
		InitSound();
		return;
	}

	CTrackRacerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;
	
	// Update racing
	pDoc->UpdateRace(m_nElapse);

	// update eye's tortion
	char carHan = pDoc->GetCarHandleState();
	if (carHan == HANDLE_CW)
		m_eyeTort += 1.5 * (12 - m_eyeTort) * (m_nElapse / 1000.0);
	else if (carHan == HANDLE_CCW)
		m_eyeTort += 1.5 * (-12 - m_eyeTort) * (m_nElapse / 1000.0);
	else if (carHan == HANDLE_RELEASE)
		m_eyeTort += 3.0 * (0 - m_eyeTort) * (m_nElapse / 1000.0);
	
	// update eye's tension
	char carPed = pDoc->GetCarPedalState();
	if (carPed == PEDAL_ACCEL)
		m_eyeTens += (2.0 - m_eyeTens) * (m_nElapse / 1000.0);
	else if (carPed == PEDAL_BREAK)
		m_eyeTens += (-0.5 - m_eyeTens) * (m_nElapse / 1000.0);
	else if (carPed == PEDAL_RELEASE)
		m_eyeTens += (0 - m_eyeTens) * (m_nElapse / 1000.0);

	// Cheke if race is finished
	if (pDoc->IsRaceFinished())
	{
		// Pause game
		KillTimer(IDT_MAIN);
		// Escape full mode
		if (m_bFullView) OnViewFull();

		// Record this race
		pDoc->SaveBestRecord();

		// Restart race
		pDoc->OnNewDocument();
		OnInitialUpdate();
	}

	// Update all views
	pDoc->UpdateAllViews(NULL, 0, NULL);

	CView::OnTimer(nIDEvent);
}

void CTrackRacerView::OnSize(UINT nType, int cx, int cy)
{
	// Record client rect
	m_rtClient.right = cx;
	m_rtClient.bottom = cy;

	// Reshape OpenGL screen
	GLReshape(cx, cy);

	CView::OnSize(nType, cx, cy);
}

void CTrackRacerView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CTrackRacerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	if ((nFlags & 0x4000) == true)
		return;

	switch (nChar) {
	case VK_UP:
		if (m_sndStart.IsCompleted())
		{
			// Push accel pedal of car
			pDoc->PushCarAccelPedal();
			// Play accel sound of car
			if (!m_sndDrive.IsRunning())
				m_sndAccel.Play();
		}
		break;
	case VK_DOWN:
		// Push break pedal of car
		pDoc->PushCarBreakPedal();
		break;
	case VK_RIGHT:
		// Turn handle of car
		pDoc->TurnCarHandle(HANDLE_CW);
		break;
	case VK_LEFT:
		// Turn handle of car
		pDoc->TurnCarHandle(HANDLE_CCW);
		break;
	case VK_X:
		// Up gear stick of car
		pDoc->UpGearStick();
		break;
	case VK_Z:
		// Down gear stick of car
		pDoc->DownGearStick();
		break;
	case VK_CONTROL:
		// Play horn soudn of car
		m_sndHorn.Play(true);
		break;
	case VK_SPACE:
		// Restart race
		pDoc->OnNewDocument();
		OnInitialUpdate();
		break;
	case VK_ESCAPE:
		// Escape full view mode
		if (m_bFullView)
			OnViewFull();
		break;
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CTrackRacerView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CTrackRacerDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;

	switch (nChar) {
	case VK_UP:
		// Release pedal of car
		pDoc->ReleaseCarPedal();
		// Stop accel sound of car
		m_sndAccel.Stop();
		break;
	case VK_DOWN:
		// Release pedal of car
		pDoc->ReleaseCarPedal();
		break;
	case VK_RIGHT:
	case VK_LEFT:
		// Release handle of car
		pDoc->ReleaseCarHandle();
		break;
	}

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL CTrackRacerView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// Change scale of track map
	if (zDelta > 0)
		m_mapScale *= 0.8;
	else if (zDelta < 0)
		m_mapScale *= 1.2;

	// Update this view
	OnUpdate(NULL, 0, NULL);

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}


void CTrackRacerView::OnViewFull()
{
	if (!m_bFullView)
	{
		// Start full view mode
		m_bFullView = true;

		// Set parent to desktop window
		m_formerWnd = GetParent();
		SetParent(GetDesktopWindow());

		// Resize this view fit to desktop window
		CRect rect;
		GetDesktopWindow()->GetClientRect(rect);
		MoveWindow(rect);
	}
	else
	{
		// Escape full view mode
		m_bFullView = false;

		// Set parent to former window
		this->SetParent(m_formerWnd);

		// Resize this view fit to former window
		CRect rect;
		GetParent()->GetClientRect(rect);
		this->MoveWindow(rect);
	}

	// Show/Hide cursor
	ShowCursor(!m_bFullView);
}

void CTrackRacerView::OnBackground(UINT nID)
{
	m_nBkgnID = nID;

	// Load background texture data
	CBitmap bmpBkgn;
	bmpBkgn.LoadBitmapW(IDB_GRASS + (nID - ID_BACKGROUND_GRASS));

	if (m_bkgnTexture != NULL)
		delete[] m_bkgnTexture;

	m_bkgnTexture = GetBitmapRaster(this, &bmpBkgn, m_bkgnSize);
	bmpBkgn.DeleteObject();
}

void CTrackRacerView::OnUpdateBackground(CCmdUI* pCmd)
{
	pCmd->SetCheck(pCmd->m_nID == m_nBkgnID);
}

void CTrackRacerView::OnBackgroundDay()
{
	m_bBkgnDay = true;
	glClearColor(135 / 255.0, 206 / 255.0, 235 / 255.0, 0);
}

void CTrackRacerView::OnUpdateBackgroundDay(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bBkgnDay);
}

void CTrackRacerView::OnBackgroundNight()
{
	m_bBkgnDay = false;
	glClearColor(0, 24 / 255.0, 72 / 255.0, 0);
}

void CTrackRacerView::OnUpdateBackgroundNight(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(!m_bBkgnDay);
}

void CTrackRacerView::OnHelpInstruction()
{
	CDialog dialog(IDD_INSTRUCTION);
	dialog.DoModal();
}


