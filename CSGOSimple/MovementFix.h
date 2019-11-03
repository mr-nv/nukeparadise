
#include "singleton.hpp"
#include "valve_sdk/csgostructs.hpp"

#pragma once
class MovementFix : public Singleton<MovementFix>
{
public:
	void aaCorrect(QAngle vOldAngles, CUserCmd* pCmd, float fOldForward, float fOldSidemove);
    void Correct(QAngle& wish_angle, CUserCmd* m_pcmd);
};

