#include "stdafx.h"
#include "Utility.h"


void SolidLine(bool fFill, double width, double dirX, double dirZ, double offset)
{
	double gradient = atan2(dirZ, dirX);
	double length = hypot(dirX, dirZ);

	glPushMatrix();
	glRotated(DEG(gradient), 0, -1, 0);

	glBegin(fFill ? GL_QUADS : GL_LINE_LOOP);
	glVertex3d(0, 0, offset - width / 2);
	glVertex3d(length, 0, offset - width / 2);
	glVertex3d(length, 0, offset + width / 2);
	glVertex3d(0, 0, offset + width / 2);
	glEnd();
	
	glPopMatrix();
}

void DashLine(bool fFill, double width, double dirX, double dirZ, double offset)
{
	double gradient = atan2(dirZ, dirX);
	double length = hypot(dirX, dirZ);
	double curLen = 0;
	double dashLen = 8;

	glPushMatrix();
	glRotated(DEG(gradient), 0, -1, 0);

	for (; curLen < (length - dashLen);
	curLen += (dashLen * 2))
	{
		glBegin(fFill ? GL_QUADS : GL_LINE_LOOP);
		glVertex3d(curLen, 0, offset - width / 2);
		glVertex3d(curLen + dashLen, 0, offset - width / 2);
		glVertex3d(curLen + dashLen, 0, offset + width / 2);
		glVertex3d(curLen, 0, offset + width / 2);
		glEnd();
	}

	glPopMatrix();
}

void SolidCurve(GLUquadric* pQuad, bool fFill, double width, double radius, double startAngle, double sweepAngle)
{
	double length = radius * RAD(abs(sweepAngle));

	gluQuadricDrawStyle(pQuad, fFill ? GLU_FILL : GLU_LINE);

	glPushMatrix();
	glRotated(90, -1, 0, 0);
	glRotated(90, 0, 0, -1);
	
	gluPartialDisk(pQuad,
		radius - width / 2,
		radius + width / 2,
		length / 4, 1,
		startAngle, sweepAngle);

	glPopMatrix();
}

void DashCurve(GLUquadric* pQuad, bool fFill, double width, double radius, double startAngle, double sweepAngle)
{
	double curSweep = 0;
	double dashSweep = DEG(8 / radius);

	gluQuadricDrawStyle(pQuad, fFill ? GLU_FILL : GLU_LINE);

	glPushMatrix();
	glRotated(90, -1, 0, 0);
	glRotated(90, 0, 0, -1);

	if (sweepAngle < 0)
		dashSweep *= -1;

	for (; abs(curSweep) < abs(sweepAngle - dashSweep);
	curSweep += (2 * dashSweep))
	{
		gluPartialDisk(pQuad,
			radius - width / 2,
			radius + width / 2,
			2, 1,
			startAngle + curSweep,
			dashSweep);
	}

	glPopMatrix();
}

void LinearLoad(CLine* pLine, bool fFill, int laneCnt, double laneWidth, double lineWidth)
{
	glPushMatrix();
	glTranslated(pLine->begin.x, 0.01, pLine->begin.y);

	// Paint asphalt (Gray)
	glColor3ub(108, 108, 108);
	SolidLine(
		fFill,
		laneWidth * laneCnt * 2,
		pLine->GetTangent(false).x,
		pLine->GetTangent(false).y, 0);

	glTranslated(0, 0.01, 0);
	// Paint center line (Yellow)
	glColor3ub(255, 255, 0);
	SolidLine(
		fFill,
		lineWidth,
		pLine->GetTangent(false).x,
		pLine->GetTangent(false).y, 0);

	// Paint normal lines (White)
	glColor3ub(255, 255, 255);
	for (int i = 1; i <= laneCnt; i++)
	{
		if (i < laneCnt)
		{
			DashLine(
				fFill,
				lineWidth,
				pLine->GetTangent(false).x,
				pLine->GetTangent(false).y,
				laneWidth * i);
			DashLine(
				fFill,
				lineWidth,
				pLine->GetTangent(false).x,
				pLine->GetTangent(false).y,
				-laneWidth * i);
		}
		else
		{
			SolidLine(
				fFill,
				lineWidth,
				pLine->GetTangent(false).x,
				pLine->GetTangent(false).y,
				laneWidth * i);
			SolidLine(
				fFill,
				lineWidth,
				pLine->GetTangent(false).x,
				pLine->GetTangent(false).y,
				-laneWidth * i);
		}
	}
	glPopMatrix();
}

void CurvedLoad(GLUquadric* pQuad, CCurve* pCurve, bool fFill, int laneCnt, double laneWidth, double lineWidth)
{
	glPushMatrix();
	glTranslated(pCurve->pivot.x, 0.01, pCurve->pivot.y);

	// Paint asphalt (Gray)
	glColor3ub(108, 108, 108);
	SolidCurve(
		pQuad, fFill,
		laneWidth * laneCnt * 2,
		pCurve->radius,
		pCurve->startAngle,
		pCurve->sweepAngle);

	glTranslated(0, 0.01, 0);
	// Paint center line (Yellow)
	glColor3ub(255, 255, 0);
	SolidCurve(
		pQuad, fFill,
		lineWidth,
		pCurve->radius,
		pCurve->startAngle,
		pCurve->sweepAngle);

	// Paint normal lines (White)
	glColor3ub(255, 255, 255);
	for (int i = 1; i <= laneCnt; i++)
	{
		if (i < laneCnt)
		{
			DashCurve(
				pQuad, fFill,
				lineWidth,
				pCurve->radius + laneWidth * i,
				pCurve->startAngle,
				pCurve->sweepAngle);
			DashCurve(
				pQuad, fFill,
				lineWidth,
				pCurve->radius - laneWidth * i,
				pCurve->startAngle,
				pCurve->sweepAngle);
		}
		else
		{
			SolidCurve(
				pQuad, fFill,
				lineWidth,
				pCurve->radius + laneWidth * i,
				pCurve->startAngle,
				pCurve->sweepAngle);
			SolidCurve(
				pQuad, fFill,
				lineWidth,
				pCurve->radius - laneWidth * i,
				pCurve->startAngle,
				pCurve->sweepAngle);
		}
	}
	glPopMatrix();
}


void PaintGauge(HDC hDC, double x, double y, double r, double maxAngle,
	double maxValue, double unitValue, double value)
{
	// Draw Pie
	BeginPath(hDC);
	MoveToEx(hDC, x, y, NULL);
	AngleArc(hDC, x, y, r, 180, -maxAngle);
	EndPath(hDC);
	StrokeAndFillPath(hDC);

	// Draw Hand
	double valueAngle = maxAngle * (value / maxValue);
	MoveToEx(hDC, x, y, NULL);
	LineTo(hDC, x + (r - 10) * cos(RAD(180 - valueAngle)),
		y - (r - 10) * sin(RAD(180 - valueAngle)));

	// Draw Grad
	double unitAngle = maxAngle * (unitValue / maxValue);
	for (double curAngle = 0; curAngle <= maxAngle; curAngle += unitAngle)
	{
		MoveToEx(hDC, x + r * cos(RAD(180 - curAngle)),
			y - r * sin(RAD(180 - curAngle)), NULL);
		LineTo(hDC, x + (r - 10) * cos(RAD(180 - curAngle)),
			y - (r - 10) * sin(RAD(180 - curAngle)));

		CString strValue;
		double curValue = unitValue * (curAngle / unitAngle);
		strValue.Format(L"%d", (int)curValue);
		SetTextAlign(hDC, TA_CENTER);
		TextOutW(hDC, x + (r + 20) * cos(RAD(180 - curAngle)),
			y - (r + 20) * sin(RAD(180 - curAngle)),
			strValue, strValue.GetLength());
	}

}

byte* GetBitmapRaster(CWnd* pWnd, CBitmap* pBitmap, CSize& size)
{
	// Get size of bitmap
	BITMAP bitmap;
	pBitmap->GetBitmap(&bitmap);
	size.cx = bitmap.bmWidth;
	size.cy = bitmap.bmHeight;

	// Read raster data from bitmap
	byte* raster; CDC mDC;
	raster = new byte[3 * size.cx * size.cy];
	mDC.CreateCompatibleDC(pWnd->GetDC());
	mDC.SelectObject(pBitmap);

	for (int i = 0, y = 0; y < size.cy; y++)
	{
		for (int x = 0; x < size.cx; x++, i += 3)
		{
			COLORREF c = mDC.GetPixel(x, y);
			raster[i] = GetRValue(c);
			raster[i + 1] = GetGValue(c);
			raster[i + 2] = GetBValue(c);
		}
	}
	mDC.DeleteDC();
	return raster;
}


