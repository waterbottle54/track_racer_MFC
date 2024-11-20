#pragma once
class CXYZ
{
public:
	double x, y, z;

	CXYZ();
	CXYZ(double x, double y, double z);
	~CXYZ();

	CXYZ& operator + (CXYZ& pt) const
	{
		return CXYZ(x + pt.x, y + pt.y, z + pt.z);
	}
	CXYZ& operator += (CXYZ& pt)
	{
		return (*this = *this + pt);
	}
};

