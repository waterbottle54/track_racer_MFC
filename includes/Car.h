#pragma once
#include "Observer.h"
#define PEDAL_ACCEL		0x01
#define PEDAL_BREAK		0x02
#define PEDAL_RELEASE	0x04
#define HANDLE_CW		0x08
#define HANDLE_CCW		0x10
#define HANDLE_RELEASE	0x20
#define MPS(kph)		((kph) * 1000 / 3600)
#define KPH(mps)		((mps) * 3600 / 1000)

class CCar;
class CPowerSystem
{
	friend class CCar;
private:

	// properties
	double maxRPM, maxToq;
	// RPMs
	double engineRPM, wheelRPM;
	// Break torques
	double breakToq, fricToq;
	// Transmission Gear
	double arShift[4]; int nShift; bool bAuto;

public:

	CPowerSystem() {}
	CPowerSystem(double maxRPM, double maxToq, double arShift[4], bool bAuto);
	~CPowerSystem();

	// On/Off Engine
	void OnEngine();
	void OffEngine();
	void StopGear()
	{
		OffEngine();
		wheelRPM = 0;
		nShift = 0;
	}
	// Set Break torque
	void SetBreak(double torque);
	void SetFriction(double torque);
	// Up/Down Gear shift
	void UpGearShift();
	void DownGearShift();
	void SetGearMode(bool bAuto)
	{
		StopGear();
		this->bAuto = bAuto;
	}
	// Update Power system
	void UpdateSystem(int nElapse);

};

class CCar : public CObserver
{
private:
	CPowerSystem m_psy;
	double m_steer;

public:

	CCar(CPowerSystem& ps);
	~CCar();

	// About Pedal
	void PushPedal(int nEnum);
	void ReleasePedal();
	char GetPedalState() const;

	// About Handle
	void TurnHandle(int nEnum, double steer);
	void ReleaseHandle();
	char GetHandleState() const;

	// About Gear Stick
	void UpGearStick();
	void DownGearStick();
	char GetGearShift() const;
	bool IsGearAuto() const;
	void SetGearMode(bool bAuto)
	{
		m_psy.SetGearMode(bAuto);
	}

	// About RPM
	double GetSpeed() const 
	{ 
		return m_psy.wheelRPM; 
	}
	double GetLimitSpeed() const 
	{ 
		return m_psy.maxRPM / m_psy.arShift[m_psy.nShift]; 
	}
	double GetMaxSpeed() const 
	{
		return (m_psy.maxRPM / m_psy.arShift[3]);
	}
	void Stop()
	{
		m_psy.StopGear();
	}

	void SetFriction(double torque)
	{
		m_psy.SetFriction(torque);
	}
	void Update(int nElapse);

};

