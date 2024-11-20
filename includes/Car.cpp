#include "stdafx.h"
#include "Car.h"


// Implementation of PowerSystem

CPowerSystem::CPowerSystem(double maxRPM, double maxToq, double arLimit[4], bool bAuto)
{
	this->maxRPM = maxRPM;
	this->maxToq = maxToq;
	this->engineRPM = 0;
	this->wheelRPM = 0;
	this->breakToq = 0;
	this->fricToq = 0;

	for (int i = 0; i < 4; i++)
		arShift[i] = maxRPM / arLimit[i];
	this->nShift = 0;
	this->bAuto = bAuto;
}

CPowerSystem::~CPowerSystem()
{
}

void CPowerSystem::OnEngine()
{
	engineRPM = maxRPM;
}

void CPowerSystem::OffEngine()
{
	engineRPM = 0;
}

void CPowerSystem::SetBreak(double torque)
{
	breakToq = torque;
}

void CPowerSystem::SetFriction(double torque)
{
	fricToq = torque;
}

void CPowerSystem::UpGearShift()
{
	if (nShift < 3)
		nShift++;
}

void CPowerSystem::DownGearShift()
{
	if (nShift > 0 && wheelRPM < (maxRPM / arShift[nShift-1]))
		nShift--;
}

void CPowerSystem::UpdateSystem(int nElapse)
{
	// Apply torque on wheel gear
	if (engineRPM > 0)
		wheelRPM += maxToq * (1 - arShift[nShift] * (wheelRPM / engineRPM)) * nElapse / 1000;
	else if (engineRPM == 0)
		wheelRPM += -maxToq * arShift[nShift] * (wheelRPM / maxRPM) * nElapse / 1000;

	// Apply break/friction torque on wheel gear
	if (wheelRPM > 0)
		wheelRPM -= ((breakToq + fricToq) * nElapse / 1000);
	else if (wheelRPM < 0)
		wheelRPM += ((breakToq + fricToq) * nElapse / 1000);

	// Shift transmission gear
	if (bAuto)
	{
		static double prevRPM = 0;

		if (wheelRPM > (maxRPM / arShift[nShift]) * 0.9
			&& prevRPM < (maxRPM / arShift[nShift]) * 0.9)
			UpGearShift();

		else if (wheelRPM < (maxRPM / arShift[nShift-1]) * 0.9
			&& prevRPM >(maxRPM / arShift[nShift-1]) * 0.9)
			DownGearShift();

		prevRPM = wheelRPM;
	}
}


// Implementation of CCar

CCar::CCar(CPowerSystem& psy)
{
	m_psy = psy;
	m_steer = 0;
}

CCar::~CCar() {}

void CCar::PushPedal(int nEnum)
{
	if (nEnum == PEDAL_ACCEL)
		m_psy.OnEngine();
	else if (nEnum == PEDAL_BREAK)
		m_psy.SetBreak(10);
}

void CCar::ReleasePedal()
{
	if (GetPedalState() == PEDAL_ACCEL)
		m_psy.OffEngine();
	else if (GetPedalState() == PEDAL_BREAK)
		m_psy.SetBreak(0);
}

char CCar::GetPedalState() const
{
	if (m_psy.engineRPM > 0)
		return PEDAL_ACCEL;
	else if (m_psy.breakToq > 0)
		return PEDAL_BREAK;
	else if (m_psy.engineRPM == 0)
		return PEDAL_RELEASE;
}

void CCar::TurnHandle(int nEnum, double steer)
{
	if (nEnum == HANDLE_CW)
		m_steer = RAD(steer);
	else if (nEnum == HANDLE_CCW)
		m_steer = RAD(-steer);
}

void CCar::ReleaseHandle()
{
	m_steer = 0;
}

char CCar::GetHandleState() const
{
	if (m_steer > 0)
		return HANDLE_CW;
	else if (m_steer < 0)
		return HANDLE_CCW;
	else if (m_steer == 0)
		return HANDLE_RELEASE;
}

void CCar::UpGearStick()
{
	m_psy.UpGearShift();
}

void CCar::DownGearStick()
{
	m_psy.DownGearShift();
}

char CCar::GetGearShift() const
{
	return m_psy.nShift;
}

bool CCar::IsGearAuto() const

{
	return m_psy.bAuto;
}

void CCar::Update(int nElapse)
{
	double elapse = nElapse / 1000.;

	m_psy.UpdateSystem(nElapse);
	vf = m_psy.wheelRPM;

	lon += (m_steer * elapse);
	CObserver::Update(nElapse);
}

