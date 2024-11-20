#include "stdafx.h"
#include "Observer.h"
#include <math.h>
#include <Windows.h>

CObserver::CObserver()
{
	x = y = z = 0;
	vf = vs = 0;
	lon = lat = 0;
}

CObserver::~CObserver() {}

void CObserver::GetPosition(double & x, double & y, double & z) const
{
	x = this->x;
	y = this->y;
	z = this->z;
}

void CObserver::SetPosition(double x, double y, double z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void CObserver::GetVelocity(double & x, double & y, double & z) const
{
	double a = (lon - 3.142 / 2);

	x = (vs * cos(a) - vf * sin(a));
	y = 0;
	z = (vs * sin(a) + vf * cos(a));
}

void CObserver::GetSpeed(int nDir, double & spd) const
{
	if (nDir == DIR_FORWARD)
		spd = vf;
	else if (nDir == DIR_BACKWARD)
		spd = -vf;
	else if (nDir == DIR_RIGHTWARD)
		spd = vs;
	else if (nDir == DIR_LEFTWARD)
		spd = -vs;
}

void CObserver::SetSpeed(int nDir, double spd)
{
	if (nDir & DIR_FORWARD)
		vf = spd;
	else if (nDir & DIR_BACKWARD)
		vf = -spd;

	if (nDir & DIR_LEFTWARD)
		vs = spd;
	else if (nDir & DIR_RIGHTWARD)
		vs = -spd;
}

void CObserver::GetDirection(double & lon, double & lat) const
{
	lon = DEG(this->lon);
	lat = DEG(this->lat);
}

void CObserver::SetDirection(double lon, double lat)
{
	this->lon = RAD(lon);
	this->lat = RAD(lat);
}

void CObserver::GetDirectionVector(double & x, double & y, double & z) const
{
	x = (cos(lat) * cos(lon));
	y = (sin(lat));
	z = (cos(lat) * sin(lon));
}

void CObserver::Turn(double lon, double lat)
{
	this->lon += RAD(lon);
	this->lat += RAD(lat);
}

void CObserver::Update(int nElapse)
{
	double elapse = nElapse/1000.;
	double vx, vy, vz;
	GetVelocity(vx, vy, vz);
	x += (vx * elapse);
	y += (vy * elapse);
	z += (vz * elapse);
}

