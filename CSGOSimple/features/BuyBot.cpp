
#include "BuyBot.h"
#include "../options.hpp"
#include "Logger.h"
#include "../ConsoleHelper.h"

void BuyBot::OnRoundStart()
{
    //static float LastBuy = 0.f;
    if (!g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_Config.misc_buybot)
    {
        return;
    }

    LastRoundStartTime = g_GlobalVars->curtime;
    ShouldBuy = true;
}

void BuyBot::OnCreateMove()
{
    if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || !ShouldBuy)
    {
        return;
    }
    ShouldBuy = false;

    std::string buycommand = "";
    BuyBotHvHWeapons weapon = (BuyBotHvHWeapons)g_Config.misc_buybot_weapon;

	if (g_Config.misc_buybot_grenade[0])
	{
		buycommand += "buy molotov; ";
	}
	if (g_Config.misc_buybot_grenade[1])
	{
		buycommand += "buy hegrenade; ";
	}
	if (g_Config.misc_buybot_grenade[2])
	{
		buycommand += "buy smokegrenade; ";
	}
	if (g_Config.misc_buybot_grenade[3])
	{
		buycommand += "buy flashbang; ";
	}
	if (g_Config.misc_buybot_grenade[4])
	{
		buycommand += "buy decoy; ";
	}
	if (g_Config.misc_buybot_grenade[5])
	{
		buycommand += "buy vest; buy vesthelm; ";
	}
	if (g_Config.misc_buybot_grenade[6])
	{
		buycommand += "buy taser; ";
	}
	if (g_Config.misc_buybot_grenade[7])
	{
		buycommand += "buy defuser; ";
	}

    switch (weapon)
    {
        case BuyBotHvHWeapons::SG_AUG:
            buycommand += "buy sg556; buy aug; ";
            break;
        case BuyBotHvHWeapons::SSG:
            buycommand += "buy ssg08; ";
            break;
        case BuyBotHvHWeapons::AUTO:
            buycommand += "buy scar20; buy g3sg1; ";
            break;
        case BuyBotHvHWeapons::MAC_10:
            buycommand += "buy mac10; ";
            break;
        case BuyBotHvHWeapons::P90:
            buycommand += "buy p90; ";
            break;
        case BuyBotHvHWeapons::PP_BIZON:
            buycommand += "buy bizon; ";
            break;
        case BuyBotHvHWeapons::AK_47:
            buycommand += "buy ak47; buy m4a4; buy m4a1_silencer; ";
            break;
        case BuyBotHvHWeapons::AWP:
            buycommand += "buy awp; ";
            break;
    }


    BuyBotPistols pistol = (BuyBotPistols)g_Config.misc_buybot_pistol;
    switch (pistol)
    {
        case BuyBotPistols::GLOCK_18__USP_S__P2000:
            buycommand += "buy glock; buy usp_silencer; buy hkp2000; ";
            break;
        case BuyBotPistols::DUAL_BERETTAS:
            buycommand += "buy elite; ";
            break;
        case BuyBotPistols::P250: //p250
            buycommand += "buy p250; ";
            break;
        case BuyBotPistols::TEC_9__FIVE_SEVEN:
            buycommand += "buy tec9; buy fiveseven; ";
            break;
        case BuyBotPistols::DESERT_EAGLE__R8:
            buycommand += "buy deagle; buy revolver; ";
            break;
    }

    g_Logger.Info("BUYBOT", buycommand.c_str());

    g_EngineClient->ExecuteClientCmd(buycommand.c_str());
}
