#include "stdafx.h"
#include "XYZ.h"


CXYZ::CXYZ()
{
	x = y = z = 0;
}

CXYZ::CXYZ(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

CXYZ::~CXYZ()
{
}
