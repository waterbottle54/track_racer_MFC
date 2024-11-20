#pragma once
#include <stack>
#define RAD(deg)	((deg) * 3.142 / 180)
#define DEG(rad)	((rad) * 180 / 3.142)

class CSection : public CObject
{
public:
	virtual void Extend(CSection* prev, double x, double y) = 0;
	virtual void Serialize(CArchive& ar) = 0;

	// Geomatrical Properties
	virtual POINTF GetBegin() const = 0;
	virtual POINTF GetEnd() const = 0;
	virtual POINTF GetTangent(bool bBeginEnd) const = 0;
	virtual double GetLength() const = 0;
	virtual double GetCurvature() const = 0;
	virtual bool Contains(double dist, double x, double y) const = 0;
	virtual double ContainsAt(double dist, double x, double y) const = 0;

	// Simple Transformations
	virtual void Expand(double ratio) = 0;
};
class CLine : public CSection
{
public:
	POINTF begin, end;

public:
	CLine() {}
	CLine(double beginX, double beginY, double endX, double endY);
	virtual void Extend(CSection* prev, double x, double y);
	virtual void Serialize(CArchive& ar);

	// Geomatrical Properties
	virtual POINTF GetBegin() const;
	virtual POINTF GetEnd() const;
	virtual POINTF GetTangent(bool bBeginEnd) const;
	virtual double GetLength() const;
	virtual double GetCurvature() const;
	virtual bool Contains(double dist, double x, double y) const;
	virtual double ContainsAt(double dist, double x, double y) const;
	
	// Simple Transformations
	virtual void Expand(double ratio);

	DECLARE_SERIAL(CLine)
};
class CCurve : public CSection
{
public:
	POINTF pivot;
	double radius;
	double startAngle;
	double sweepAngle;

public:
	CCurve() {}
	CCurve(double pivotX, double pivotY, double radius, double startAngle, double sweepAngle);
	virtual void Extend(CSection* prev, double x, double y);
	virtual void Serialize(CArchive& ar);

	// Geomatrical Properties
	virtual POINTF GetBegin() const;
	virtual POINTF GetEnd() const;
	virtual POINTF GetTangent(bool bBeginEnd) const;
	virtual double GetLength() const;
	virtual double GetCurvature() const;
	virtual bool Contains(double dist, double x, double y) const;
	virtual double ContainsAt(double dist, double x, double y) const;
	
	// Simple Transformations
	virtual void Expand(double ratio);

	DECLARE_SERIAL(CCurve)
};

class CTrack : public CObject
{
protected:
	CArray<CSection*> m_array;

public:
	CTrack() {}
	~CTrack();
	virtual void Serialize(CArchive& ar);

	// Operators
	void Start(CSection* begin);
	void Extend(CRuntimeClass* type, double x, double y);
	void Reduce();
	void Clear();
	
	// Attributes
	CSection* GetBegin();
	CSection* GetLast();
	void GetData(CSection** buffer);
	int GetSize() const;

	// Geometrical Properties
	double GetLength() const;
	bool Contains(double dist, double x, double y) const;
	double ContainsAt(double dist, double x, double y) const;
	CSection* ContainsIn(double dist, double x, double y) const;

	// Simple Transformations
	void Expand(double ratio);

	DECLARE_SERIAL(CTrack)
};
