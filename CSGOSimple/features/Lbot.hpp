#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
class Aimbot {
public:
	void OnMove(CUserCmd* pCmd);
	bool IsEnabled(CUserCmd* pCmd);
	float GetFovToPlayer(QAngle viewAngle, QAngle aimAngle);
	lbot_settings settings;
private:
	bool IsRcs();
	void RCS(QAngle& angle, C_BasePlayer* target);
	bool IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos);
	void Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle);
	bool IsNotSilent(float fov);
	C_BasePlayer* GetClosestPlayer(CUserCmd* cmd, int& bestBone);
	float shot_delay_time;
	bool shot_delay = false;
	bool silent_enabled = false;
	QAngle CurrentPunch = { 0,0,0 };
	QAngle RCSLastPunch = { 0,0,0 };
	bool is_delayed = false;
	int kill_delay_time;
	bool kill_delay = false;
	float GetSmooth();
	float GetFov();
	C_BasePlayer* target = NULL;
};
extern Aimbot g_Lbot;