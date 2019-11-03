#include "NightMode.h"
#include <string>
#include "..\\RuntimeSaver.h"
#include "..\valve_sdk\csgostructs.hpp"
#include "../options.hpp"
#include "Logger.h"

std::string fallback_skybox = "";

float pr;
float pg;
float pb;
float pa;

void NightMode::Apply(bool ForceUpdate, float r, float g, float b, float a)
{
	//kakoi je ya govnopaster ubeite menya
	static bool perfomed = false, bLastSetting, checkcolors = false;
	bool optMode = false;
	static ConVar* sv_skyname = g_CVar->FindVar("sv_skyname");
	sv_skyname->m_nFlags &= ~FCVAR_CHEAT;
	static ConVar* r_3dsky = g_CVar->FindVar("r_3dsky");

	if (pr != r || pg != g || pb != b || pa != a || !g_Saver.Colorama)
	{
		pr = r;
		pg = g;
		pb = b;
		pa = a;
		checkcolors = true;
		g_Saver.Colorama = true;
	}
	else
	{
		checkcolors = false;
	}

	if (!g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !checkcolors)
	{
		Active = false;
		return;
	}

	if (!Active || ForceUpdate)
	{
		for (MaterialHandle_t i = g_MatSystem->pFirstMaterial(); i != g_MatSystem->pInvalidMaterial(); i = g_MatSystem->pNextMaterial(i))
		{
			IMaterial* pMaterial = g_MatSystem->pGetMaterial(i);

			if (!pMaterial)
				continue;
			if (!pMaterial || pMaterial->IsErrorMaterial())
				continue;

			if (strstr(pMaterial->GetTextureGroupName(), ("SkyBox")))
			{
				pMaterial->ColorModulate(r, g, b);
			}
			if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls
			{
				pMaterial->ColorModulate(r, g, b);
			}
			if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls
			{
				pMaterial->AlphaModulate(a);
				pMaterial->ColorModulate(r, g, b);
			}
			if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
			{
				pMaterial->AlphaModulate(a);
				pMaterial->ColorModulate(r, g, b);
			}
		}
		Active = true;
	}

	if (bLastSetting != g_Config.vis_misc_nightmode)
		bLastSetting = g_Config.vis_misc_nightmode;
}
void NightMode::Revert()
{
	if (Active || g_Saver.Colorama || g_Unload)
	{
		for (MaterialHandle_t i = g_MatSystem->pFirstMaterial(); i != g_MatSystem->pInvalidMaterial(); i = g_MatSystem->pNextMaterial(i))
		{
			IMaterial* pMaterial = g_MatSystem->pGetMaterial(i);

			if (!pMaterial)
				continue;
			if (!pMaterial || pMaterial->IsErrorMaterial())
				continue;

			if (strstr(pMaterial->GetTextureGroupName(), ("SkyBox")))
			{
				pMaterial->ColorModulate(1, 1, 1);
			}
			if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls
			{
				pMaterial->ColorModulate(1, 1, 1);
			}
			if (!strcmp(pMaterial->GetTextureGroupName(), "World textures"))  // walls
			{
				pMaterial->AlphaModulate(1);
				pMaterial->ColorModulate(1, 1, 1);
			}
			if (!strcmp(pMaterial->GetTextureGroupName(), "StaticProp textures"))
			{
				pMaterial->AlphaModulate(1);
				pMaterial->ColorModulate(1, 1, 1);
			}
			g_Saver.Colorama = false;
		}

		g_Saver.RequestForceUpdate = true;
		Active = false;
	}
}
