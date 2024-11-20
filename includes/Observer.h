#pragma once
#define DIR_FORWARD		0x01
#define DIR_BACKWARD	0x02
#define DIR_LEFTWARD	0x04
#define DIR_RIGHTWARD	0X08
#define RAD(deg)		((deg) * 3.142/180)
#define DEG(rad)		((rad) * 180/3.142)

class CObserver
{
public:

	double x, y, z;
	// forward, sideward speed
	double vf, vs;
	// longitude, latitude of direction
	double lon, lat;

public:

	CObserver();
	~CObserver();

	// Access functions
	void GetPosition(double& x, double& y, double& z) const;
	void SetPosition(double x, double y, double z);
	void GetVelocity(double& x, double& y, double& z) const;
	void GetSpeed(int nDir, double& spd) const;
	void SetSpeed(int nDir, double spd);
	void GetDirection(double& lon, double& lat) const;
	void SetDirection(double lon, double lat);
	void GetDirectionVector(double& x, double& y, double& z) const;

	void Turn(double lon, double lat);
	void Update(int nElapse);
	
};

