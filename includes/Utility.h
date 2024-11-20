#pragma once
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Track.h"


void SolidLine(bool fFill, double width, double dirX, double dirZ, double offset);

void DashLine(bool fFill, double width, double dirX, double dirZ, double offset);

void SolidCurve(GLUquadric* pQuad, bool fFill, double width, double radius, double startAngle, double sweepAngle);

void DashCurve(GLUquadric* pQuad, bool fFill, double width, double radius, double startAngle, double sweepAngle);

void LinearLoad(CLine* pLine, bool fFill, int laneCnt, double laneWidth, double lineWidth);

void CurvedLoad(GLUquadric* pQuad, CCurve* pCurve, bool fFill, int laneCnt, double laneWidth, double lineWidth);


void PaintGauge(HDC hDC, double x, double y, double r, double maxAngle,
	double maxValue, double unitValue, double value);

byte* GetBitmapRaster(CWnd* pWnd, CBitmap* pBitmap, CSize& size);

