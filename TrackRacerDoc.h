
// TrackRacerDoc.h : interface of the CTrackRacerDoc class
//


#pragma once
#include "includes\Car.h"
#include "includes\XYZ.h"
#include "includes\Track.h"
#include <algorithm>


class CTrackRacerDoc : public CDocument
{
	DECLARE_DYNCREATE(CTrackRacerDoc)

	// Contructor/Destructors
protected:
	CTrackRacerDoc();

	// Attributes
public:
	CCar*			m_car;			// car object of race
	CTrack*			m_track;		// track object of race
	const int		m_nLaneCount;	// count of track lanes
	const double	m_lfLaneWidth;	// width of track lane

	struct RACERECORDHEADER
	{
		int		nStartLane;
		int		nTotalFrame;
		int		nTotalTime;
	};
	struct RACERECORDFRAME
	{
		double x, z;
		double lon;
	};
	struct RACERECORD
	{
		RACERECORDHEADER header;
		RACERECORDFRAME* lpFrames;
	};

	RACERECORD  m_nowRecord;
	RACERECORD	m_bestRecord;
	bool		m_fBestRecordExist;
	int			m_iBestRecordFrame;
	
	void SaveBestRecord()
	{
		HANDLE hFile;
		WCHAR strPath[64];
		DWORD dwBytes;

		// this race is not the best record
		if (m_fBestRecordExist &&
			m_nowRecord.header.nTotalTime > m_bestRecord.header.nTotalTime)
			return;
		
		// open best record file for current track
		wcscpy_s(strPath, L"best\\");
		wcscat_s(strPath, GetTitle());
		hFile = CreateFile(strPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		
		// write best record to the file
		WriteFile(hFile, &m_nowRecord.header, sizeof(RACERECORDHEADER), NULL, NULL);
		WriteFile(hFile, m_nowRecord.lpFrames, 
					sizeof(RACERECORDFRAME) * m_nowRecord.header.nTotalFrame, NULL, NULL);

		// close the file
		CloseHandle(hFile);
	}
	void LoadBestRecord()
	{
		HANDLE hFile;
		WCHAR strPath[64];
		DWORD dwBytes;
		RACERECORDHEADER header;

		m_fBestRecordExist = false;
		m_iBestRecordFrame = 0;

		// open best record file for current track
		wcscpy_s(strPath, L"best\\");
		wcscat_s(strPath, GetTitle());
		hFile = CreateFile(strPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
			return;
		
		// read best record from the file
		ReadFile(hFile, &header, sizeof(RACERECORDHEADER), &dwBytes, NULL);
		if (dwBytes > 0)
		{
			m_bestRecord.header = header;
			m_bestRecord.lpFrames = (RACERECORDFRAME*)malloc(sizeof(RACERECORDFRAME) * header.nTotalFrame);
			ReadFile(hFile, m_bestRecord.lpFrames, sizeof(RACERECORDFRAME) * header.nTotalFrame, NULL, NULL);
			m_fBestRecordExist = true;
		}

		// close the file
		CloseHandle(hFile);
	}
	void GetBestRecordFrame(RACERECORDFRAME& recordFrame)
	{
		if (m_fBestRecordExist)
			recordFrame = m_bestRecord.lpFrames[m_iBestRecordFrame];
	}
	bool IsBestRecordExist()
	{
		return m_fBestRecordExist;
	}
	
	int GetRaceTime() const
	{
		return m_nowRecord.header.nTotalTime;
	}
	double GetRaceLength() const
	{
		double x, y, z;
		m_car->GetPosition(x, y, z);
		return m_track->ContainsAt(m_lfLaneWidth * m_nLaneCount, x, z);
	}
	bool IsRaceFinished() const
	{
		return (GetRaceLength() > GetTrackLength() - 10);
	}

	// About m_car
	void GetCarPosition(CXYZ& pos) const
	{
		m_car->GetPosition(pos.x, pos.y, pos.z);
	}
	void GetCarDirection(CXYZ& dir) const
	{
		m_car->GetDirectionVector(dir.x, dir.y, dir.z);
	}
	void GetCarDirection(double& dir) const
	{
		double n;
		m_car->GetDirection(dir, n);
	}
	void GetCarSpeed(double& spd) const
	{
		spd = m_car->GetSpeed();
	}
	void GetCarLimitSpeed(double& spd) const
	{
		spd = m_car->GetLimitSpeed();
	}
	void GetCarMaxSpeed(double& spd) const
	{
		spd = m_car->GetMaxSpeed();
	}
	char GetCarPedalState() const
	{
		return m_car->GetPedalState();
	}
	char GetCarHandleState() const
	{
		return m_car->GetHandleState();
	}
	char GetCarGearShift() const
	{
		return m_car->GetGearShift();
	}
	bool isCarGearAuto() const
	{
		return m_car->IsGearAuto();
	}

	// About m_track and track lane
	void GetTrackData(CSection** buffer)
	{
		m_track->GetData(buffer);
	}
	size_t GetTrackSize() const
	{
		return m_track->GetSize();
	}
	double GetTrackLength() const
	{
		return m_track->GetLength();
	}
	int GetTrackLaneCount() const
	{
		return m_nLaneCount;
	}
	double GetTrackLaneWidth() const
	{
		return m_lfLaneWidth;
	}

	// Operators
public:
	void InitializeRace();
	void UpdateRace(int nElapse);

	// About m_car
	void PushCarAccelPedal()
	{
		m_car->PushPedal(PEDAL_ACCEL);
	}
	void PushCarBreakPedal()
	{
		m_car->PushPedal(PEDAL_BREAK);
	}
	void ReleaseCarPedal()
	{
		m_car->ReleasePedal();
	}
	void TurnCarHandle(int nEnum)
	{
		double speed;
		GetCarSpeed(speed);
		if (speed < MPS(5))
			return;

		// Retrieve current section of track
		double x, y, z;
		CSection* pSection;
		m_car->GetPosition(x, y, z);
		pSection = m_track->ContainsIn(m_lfLaneWidth * m_nLaneCount, x, z);
		
		// Caculate appropriate angle to turn car
		double turns;
		if (pSection != NULL)
		{
			double speed, curv;
			GetCarSpeed(speed);
			curv = pSection->GetCurvature();
			turns = ((curv != 0) ? DEG(curv * speed) : 10);
		}
		else turns = 10;

		// Turn handle with the angle
		m_car->TurnHandle(nEnum, turns);
	}
	void ReleaseCarHandle()
	{
		m_car->ReleaseHandle();
	}
	void UpGearStick()
	{
		m_car->UpGearStick();
	}
	void DownGearStick()
	{
		m_car->DownGearStick();
	}

	// Overidables
public:
	virtual void DeleteContents();
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual void Serialize(CArchive& ar);

#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

	// Impelements
public:
	virtual ~CTrackRacerDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Created message map functions
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 검색 처리기에 대한 검색 콘텐츠를 설정하는 도우미 함수
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
	afx_msg void OnRoadLane(UINT nID);
	afx_msg void OnUpdateRoadLane(CCmdUI *pCmdUI);
};
