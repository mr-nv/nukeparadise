#include "../singleton.hpp"
#include "../valve_sdk/csgostructs.hpp"

#pragma once
class BunnyHop : public Singleton<BunnyHop>
{
public:
    void OnCreateMove(CUserCmd* cmd);
	void aw_AutoStrafe(CUserCmd* cmd, QAngle va);
	void glad_AutoStrafe(CUserCmd* cmd);
	void gideon_AutoStrafe(CUserCmd* cmd);
	void money_AutoStrafe(CUserCmd* cmd);
	void legit_AutoStrafe(CUserCmd* cmd);
	void rage_AutoStrafe(CUserCmd* cmd);
	void bruh_AutoStrafe(CUserCmd* cmd, QAngle va);
};