
// TrackRacerDoc.cpp : implementation of the CTrackRacerDoc class
//

#include "stdafx.h"
// SHARED_HANDLERS�� �̸� ����, ����� �׸� �� �˻� ���� ó���⸦ �����ϴ� ATL ������Ʈ���� ������ �� ������
// �ش� ������Ʈ�� ���� �ڵ带 �����ϵ��� �� �ݴϴ�.
#ifndef SHARED_HANDLERS
#include "TrackRacer.h"
#endif

#include "TrackRacerDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTrackRacerDoc

IMPLEMENT_DYNCREATE(CTrackRacerDoc, CDocument)

BEGIN_MESSAGE_MAP(CTrackRacerDoc, CDocument)
	ON_COMMAND_RANGE(ID_ROAD_1Lane, ID_ROAD_4Lane, &CTrackRacerDoc::OnRoadLane)
	ON_UPDATE_COMMAND_UI_RANGE(ID_ROAD_1Lane, ID_ROAD_4Lane, &CTrackRacerDoc::OnUpdateRoadLane)
END_MESSAGE_MAP()


// CTrackRacerDoc ����/�Ҹ�

CTrackRacerDoc::CTrackRacerDoc() 
	: m_nLaneCount(3), m_lfLaneWidth(3.0)
{
	m_nowRecord.lpFrames = NULL;
	m_bestRecord.lpFrames = NULL;

	// Allocate car object
	double arGear[4] = { 50/3.6, 100/3.6, 200/3.6, 300/3.6 };
	m_car = new CCar(CPowerSystem(10000, 50/3.6, arGear, true));

	// Allocate track object
	m_track = new CTrack();
}

CTrackRacerDoc::~CTrackRacerDoc() {}

void CTrackRacerDoc::DeleteContents()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	// Clear track object
	if (m_track != NULL)
		m_track->Clear();

	// Clear race records
	if (m_nowRecord.lpFrames != NULL)
	{
		free(m_nowRecord.lpFrames);
		m_nowRecord.lpFrames = NULL;
	}
	if (m_bestRecord.lpFrames != NULL)
	{
		free(m_bestRecord.lpFrames);
		m_bestRecord.lpFrames = NULL;
	}

	CDocument::DeleteContents();
}

BOOL CTrackRacerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	SetTitle(L"Untitled");

	// Make default track
	m_track->Start(new CLine(100, 100, 200, 200));

	// Initialize race
	InitializeRace();

	return TRUE;
}

BOOL CTrackRacerDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	SetTitle(wcsrchr(lpszPathName + 1, '\\'));

	// Initialize race
	InitializeRace();

	return TRUE;
}

void CTrackRacerDoc::OnCloseDocument()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	// de-allocate all objects

	if (m_car != NULL)
	{
		delete m_car;
		m_car = NULL;
	}

	if (m_track != NULL)
	{
		delete m_track;
		m_track = NULL;
	}

	CDocument::OnCloseDocument();
}


// CTrackRacerDoc serialization

void CTrackRacerDoc::Serialize(CArchive& ar)
{
	int nDummy;

	// Load track object
	if (ar.IsLoading())
	{
		// Load track informations
		double length;
		ar >> nDummy >> length;
		m_track->Serialize(ar);

		// Magnificate track size
		double ratio;
		ratio = length / m_track->GetLength();
		m_track->Expand(ratio);
	}
}

#ifdef SHARED_HANDLERS

// ����� �׸��� �����մϴ�.
void CTrackRacerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// ������ �����͸� �׸����� �� �ڵ带 �����Ͻʽÿ�.
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// �˻� ó���⸦ �����մϴ�.
void CTrackRacerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ������ �����Ϳ��� �˻� �������� �����մϴ�.
	// ������ �κ��� ";"�� ���еǾ�� �մϴ�.

	// ��: strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CTrackRacerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CTrackRacerDoc ����

#ifdef _DEBUG
void CTrackRacerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTrackRacerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CTrackRacerDoc operations

void CTrackRacerDoc::InitializeRace()
{

	// load the best record
	LoadBestRecord();

	// Initialize race record
	m_nowRecord.header.nTotalTime = 0;
	m_nowRecord.header.nTotalFrame = 0;
	if (m_fBestRecordExist)
	{
		m_nowRecord.header.nStartLane = rand() % (2 * m_nLaneCount - 1);
		if (m_nowRecord.header.nStartLane >= m_bestRecord.header.nStartLane)
			m_nowRecord.header.nStartLane++;
	}
	else m_nowRecord.header.nStartLane = rand() % (2 * m_nLaneCount);

	// Bring car back to begin of track
	POINTF ptBegin;
	POINTF uBegin;
	ptBegin = m_track->GetBegin()->GetBegin();
	uBegin = m_track->GetBegin()->GetTangent(true);
	uBegin.x = uBegin.x / m_track->GetBegin()->GetLength();
	uBegin.y = uBegin.y / m_track->GetBegin()->GetLength();

	m_car->Stop();
	m_car->ReleasePedal();
	m_car->ReleaseHandle();
	m_car->SetPosition(
		ptBegin.x - uBegin.y * m_lfLaneWidth * (-m_nLaneCount + 1/2.0 + m_nowRecord.header.nStartLane), 0,
		ptBegin.y + uBegin.x * m_lfLaneWidth * (-m_nLaneCount + 1/2.0 + m_nowRecord.header.nStartLane));
	m_car->SetDirection(DEG(atan2(uBegin.y, uBegin.x)), 0);
}

void CTrackRacerDoc::UpdateRace(int nElapse)
{
	// Update car
	m_car->Update(nElapse);

	double x, y, z, lon, lat;
	m_car->GetPosition(x, y, z);
	if (!m_track->Contains(m_lfLaneWidth * m_nLaneCount, x, z))
		m_car->SetFriction(5);
	else m_car->SetFriction(0);

	// Update record of race
	m_nowRecord.header.nTotalTime += nElapse;
	if (m_nowRecord.header.nTotalFrame == 0 ||
		m_nowRecord.header.nTotalFrame % 100 == 0)
	{
		m_nowRecord.lpFrames = (RACERECORDFRAME*)realloc(
			m_nowRecord.lpFrames,
			sizeof(RACERECORDFRAME) * (m_nowRecord.header.nTotalFrame + 100));
	}

	m_car->GetDirection(lon, lat);
	m_nowRecord.lpFrames[m_nowRecord.header.nTotalFrame].x = x;
	m_nowRecord.lpFrames[m_nowRecord.header.nTotalFrame].z = z;
	m_nowRecord.lpFrames[m_nowRecord.header.nTotalFrame].lon = lon;
	m_nowRecord.header.nTotalFrame++;

	if (m_fBestRecordExist && m_iBestRecordFrame < m_bestRecord.header.nTotalFrame - 1)
		m_iBestRecordFrame++;
}


// CTrackRacerDoc commands

void CTrackRacerDoc::OnRoadLane(UINT nID)
{
	//m_nLaneCount = (nID - ID_ROAD_1Lane) + 1;
}

void CTrackRacerDoc::OnUpdateRoadLane(CCmdUI * pCmdUI)
{
	//int nLane = (pCmdUI->m_nID - ID_ROAD_1Lane) + 1;
	//pCmdUI->SetCheck(m_nTrackLane == nLane);
}


