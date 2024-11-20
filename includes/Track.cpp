#include "stdafx.h"
#include "Track.h"

POINTF GetSolution(double m11,double m12,double m21,double m22, double c1, double c2)
{
	POINTF sol;
	sol.x = (c1*m22 - c2*m12) / (m11*m22 - m12*m21);
	sol.y = (c1*m21 - c2*m11) / (m12*m21 - m11*m22);
	return sol;
}

void RotatePt(double& x, double& y, double a)
{
	double X = x, Y = y;
	x = (X * cos(a) - Y * sin(a));
	y = (X * sin(a) + Y * cos(a));
}

bool IsClockwise(double rx, double ry, double vx, double vy)
{
	return (rx*vy - ry*vx < 0);
}


// Implementation of CLine

IMPLEMENT_SERIAL(CLine, CObject, 1)

CLine::CLine(double beginX, double beginY, double endX, double endY)
{
	begin.x = beginX;
	begin.y = beginY;
	end.x = endX;
	end.y = endY;
}

void CLine::Extend(CSection * prev, double x, double y)
{
	this->begin = prev->GetEnd();
	this->end.x = x;
	this->end.y = y;
}

void CLine::Serialize(CArchive & ar)
{
	if (ar.IsStoring())
	{
		ar << begin.x << begin.y << end.x << end.y;
	}
	else
	{
		ar >> begin.x >> begin.y >> end.x >> end.y;
	}
}

POINTF CLine::GetBegin() const
{
	return begin;
}

POINTF CLine::GetEnd() const
{
	return end;
}

POINTF CLine::GetTangent(bool bBeginEnd) const
{
	POINTF direc;
	direc.x = (end.x - begin.x);
	direc.y = (end.y - begin.y);
	return direc;
}

double CLine::GetLength() const
{
	return hypot(end.x - begin.x, end.y - begin.y);
}

double CLine::GetCurvature() const
{
	return 0;
}

bool CLine::Contains(double dist, double x, double y) const
{
	// Convert to relative coordinates
	x -= begin.x;
	y -= begin.y;
	RotatePt(x, y, -atan2(end.y - begin.y, end.x - begin.x));

	return (x > 0 && x < GetLength() && abs(y) < dist);
}

double CLine::ContainsAt(double dist, double x, double y) const
{
	if (!Contains(dist, x, y))
		return 0;

	// Convert to relative coordinates
	x -= begin.x;
	y -= begin.y;
	RotatePt(x, y, -atan2(end.y - begin.y, end.x - begin.x));

	return x;
}

void CLine::Expand(double ratio)
{
	begin.x *= ratio;
	begin.y *= ratio;
	end.x *= ratio;
	end.y *= ratio;
}


// Implementation of CCurve

IMPLEMENT_SERIAL(CCurve, CObject, 1)

CCurve::CCurve(double pivotX, double pivotY, double radius, double startAngle, double sweepAngle)
{
	pivot.x = pivotX;
	pivot.y = pivotY;
	this->radius = radius;
	this->startAngle = startAngle;
	this->sweepAngle = sweepAngle;
}

void CCurve::Extend(CSection * prev, double x, double y)
{
	POINTF begin, end, tangent;

	begin = prev->GetEnd();
	end.x = x; end.y = y;
	tangent = prev->GetTangent(false);

	// Calculate pivot point
	pivot = 
	GetSolution(
		2 * (end.x - begin.x), 
		2 * (end.y - begin.y),
		tangent.x, tangent.y,
		pow(end.x, 2) + pow(end.y, 2) - 
		pow(begin.x, 2) - pow(begin.y, 2),
		tangent.x * begin.x + tangent.y * begin.y);

	POINTF rbegin, rend;
	rbegin.x = (begin.x - pivot.x);
	rbegin.y = (begin.y - pivot.y);
	rend.x = (end.x - pivot.x);
	rend.y = (end.y - pivot.y);

	// Calculate radius
	radius = hypot(rbegin.x, rbegin.y);

	// Calculate start angle
	startAngle = atan2(rbegin.y, rbegin.x);
	
	// Calculate sweep angle
	bool bClockwise = 
		IsClockwise(rbegin.x, rbegin.y, tangent.x, tangent.y);

	sweepAngle = atan2(rend.y, rend.x) - startAngle;

	if (bClockwise && sweepAngle > 0)
		sweepAngle -= 6.28;
	else if (!bClockwise && sweepAngle < 0)
		sweepAngle += 6.28;

	// Convert angle to degree
	startAngle = DEG(startAngle);
	sweepAngle = DEG(sweepAngle);
}

void CCurve::Serialize(CArchive & ar)
{
	if (ar.IsStoring())
	{
		ar << pivot.x << pivot.y << radius << startAngle << sweepAngle;
	}
	else
	{
		ar >> pivot.x >> pivot.y >> radius >> startAngle >> sweepAngle;
	}
}

POINTF CCurve::GetBegin() const
{
	POINTF end;
	end.x = pivot.x + radius * cos(RAD(startAngle));
	end.y = pivot.y + radius * sin(RAD(startAngle));
	return end;
}

POINTF CCurve::GetEnd() const
{
	POINTF end;
	end.x = pivot.x + radius * cos(RAD(startAngle + sweepAngle));
	end.y = pivot.y + radius * sin(RAD(startAngle + sweepAngle));
	return end;
}

POINTF CCurve::GetTangent(bool bBeginEnd) const
{
	POINTF direc; 
	double angle;
	angle = (bBeginEnd ? startAngle : startAngle + sweepAngle);
	direc.x = -sin(RAD(angle));
	direc.y = cos(RAD(angle));
	if (sweepAngle < 0)
	{
		direc.x *= -1;
		direc.y *= -1;
	}
	return direc;
}

double CCurve::GetLength() const
{
	return (radius * abs(RAD(sweepAngle)));
}

double CCurve::GetCurvature() const
{
	return (1 / radius);
}

bool CCurve::Contains(double dist, double x, double y) const
{
	// Convert to relative coordinates
	x -= pivot.x;
	y -= pivot.y;
	RotatePt(x, y, -RAD(startAngle));

	// Convert to angular coordinates
	double r, a;
	r = hypot(x, y);
	a = atan2(y, x);
	if (sweepAngle > 0 && a < 0)
		a += 6.28;
	else if (sweepAngle < 0 && a > 0)
		a -= 6.28;

	return (abs(r - radius) < dist) &&
			(abs(a) < abs(RAD(sweepAngle)));
}

double CCurve::ContainsAt(double dist, double x, double y) const
{
	if (!Contains(dist, x, y))
		return 0;

	// Convert to relative coordinates
	x -= pivot.x;
	y -= pivot.y;
	RotatePt(x, y, -RAD(startAngle));

	// Convert to angular coordinates
	double r, a;
	r = hypot(x, y);
	a = atan2(y, x);
	if (sweepAngle > 0 && a < 0)
		a += 6.28;
	else if (sweepAngle < 0 && a > 0)
		a -= 6.28;

	return (radius * abs(a));
}

void CCurve::Expand(double ratio)
{
	pivot.x *= ratio;
	pivot.y *= ratio;
	radius *= ratio;
}


// Implementation of CTrack

IMPLEMENT_SERIAL(CTrack, CObject, 1)

CTrack::~CTrack()
{
	Clear();
}

void CTrack::Serialize(CArchive & ar)
{
	if (ar.IsStoring())
	{
		// Store array size
		ar << m_array.GetSize();

		if (m_array.IsEmpty())
			return;

		// Store array data
		for (int i = 0; i < m_array.GetSize(); i++)
			ar << m_array[i];
	}
	else
	{
		// Load array size
		int nSize;
		ar >> nSize;

		// Load array data
		Clear();
		for (int i = 0; i < nSize; i++)
		{
			CObject* pNew;
			ar >> pNew;
			m_array.Add((CSection*)pNew);
		}
	}
}

void CTrack::Start(CSection * begin)
{
	if (m_array.IsEmpty())
		m_array.Add(begin);
}

void CTrack::Extend(CRuntimeClass * type, double x, double y)
{
	CSection *pNew, *pTop;
	if (!m_array.IsEmpty())
	{
		pNew = (CSection*)type->CreateObject();
		pTop = m_array[m_array.GetUpperBound()];
		pNew->Extend(pTop, x, y);
		m_array.Add(pNew);
	}
}

void CTrack::Reduce()
{
	if (!m_array.IsEmpty())
	{
		delete m_array[m_array.GetUpperBound()];
		m_array.RemoveAt(m_array.GetUpperBound());
	}
}

void CTrack::Clear()
{
	while (!m_array.IsEmpty())
		Reduce();
}

CSection * CTrack::GetBegin()
{
	if (!m_array.IsEmpty())
		return m_array[0];
}

CSection * CTrack::GetLast()
{
	if (!m_array.IsEmpty())
		return m_array[m_array.GetUpperBound()];
}

void CTrack::GetData(CSection ** buffer)
{
	for (int i = 0; i < m_array.GetSize(); i++)
		buffer[i] = m_array[i];
}

int CTrack::GetSize() const
{
	return m_array.GetSize();
}

double CTrack::GetLength() const
{
	double sum = 0;
	for (int i = 0; i < m_array.GetSize(); i++)
		sum += m_array[i]->GetLength();
	return sum;
}

bool CTrack::Contains(double dist, double x, double y) const
{
	for (int i = 0; i < m_array.GetSize(); i++)
	{
		if (m_array[i]->Contains(dist, x, y))
			return true;
	}
	return false;
}

double CTrack::ContainsAt(double dist, double x, double y) const
{
	if (!Contains(dist, x, y))
		return 0;

	double sum = 0;
	int i = 0;

	while (!m_array[i]->Contains(dist, x, y))
		sum += m_array[i++]->GetLength();

	sum += m_array[i]->ContainsAt(dist, x, y);

	return sum;
}

CSection* CTrack::ContainsIn(double dist, double x, double y) const
{
	if (!Contains(dist, x, y))
		return NULL;

	for (int i = 0; i < m_array.GetSize(); i++)
	{
		if (m_array[i]->Contains(dist, x, y))
			return m_array[i];
	}
	return NULL;
}

void CTrack::Expand(double ratio)
{
	for (int i = 0; i < m_array.GetSize(); i++)
		m_array[i]->Expand(ratio);
}

