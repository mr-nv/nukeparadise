
#include "../singleton.hpp"
#include "../valve_sdk/csgostructs.hpp"

enum class BuyBotPistols : int
{
    NONE,
    GLOCK_18__USP_S__P2000,
    DUAL_BERETTAS,
    P250,
    TEC_9__FIVE_SEVEN,
    DESERT_EAGLE__R8
};

enum class BuyBotHvHWeapons : int
{
    NONE,
    SG_AUG,
    SSG,
    AUTO,
    MAC_10,
    P90,
    PP_BIZON,
    AK_47,
    AWP
};

#pragma once
class BuyBot : public Singleton<BuyBot>
{
public:
    void OnRoundStart(); //round_start
    void OnCreateMove();
private:
    float LastRoundStartTime = -1.f;
    bool ShouldBuy = false;
};