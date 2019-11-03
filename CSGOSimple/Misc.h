
#include "singleton.hpp"
#include "valve_sdk\csgostructs.hpp"
#include "hooks.hpp"
#include "hooks.hpp"

#pragma once
class Misc : public Singleton<Misc>
{
public:
	void aimware_fix_mouse_dx(CUserCmd* cmd);
	void OnCreateMove(CUserCmd* cmd);
	void LegitResolver();
	void Kick(int userid);
	void LinearExtrapolationsrun();
    void OnFrameStageNotify(ClientFrameStage_t stage);
	void SlowWalk(CUserCmd* cmd, float speed);
	void SlowWalk(CUserCmd* cmd);
	void CripWalk(INetChannel* channel);
	void SlowWalkRun(CUserCmd* cmd);
	void AntiKick(Hooks::tDispatchUserMessage oDispatchUserMessage, PVOID ecx, int type, unsigned int a3, unsigned int length, const void* msg_data);
private:
	bool didSwitch = false;
	void ChatSpammer();
	void AutoWeapons(CUserCmd* cmd);
	void Airstuck(CUserCmd* cmd);
	void KnifeBot(CUserCmd* cmd);
	void NoFlash();
	void Steal(CUserCmd* cmd);
	QAngle m_aimPunchAngle[128];
};

