
#include "../singleton.hpp"
#include "../valve_sdk/csgostructs.hpp"

#pragma once
class HitPossitionHelper : public Singleton<HitPossitionHelper>
{
public:
    void OnFireEvent(IGameEvent* event);
private:
    void PlayerHurt(IGameEvent* event);
    void BulletImpact(IGameEvent* event);
    C_BasePlayer* GetPlayer(int i);
};