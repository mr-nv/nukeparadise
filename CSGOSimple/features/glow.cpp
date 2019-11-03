
#include "glow.hpp"

#include "../valve_sdk/csgostructs.hpp"
#include "../config.hpp"
#include "../options.hpp"


Glow::Glow()
{
}

Glow::~Glow()
{
	// We cannot call shutdown here unfortunately.
	// Reason is not very straightforward but anyways:
	// - This destructor will be called when the dll unloads
	//   but it cannot distinguish between manual unload
	//   (pressing the Unload button or calling FreeLibrary)
	//   or unload due to game exit.
	//   What that means is that this destructor will be called
	//   when the game exits.
	// - When the game is exiting, other dlls might already
	//   have been unloaded before us, so it is not safe to
	//   access intermodular variables or functions.
	//
	//   Trying to call Shutdown here will crash CSGO when it is
	//   exiting (because we try to access g_GlowObjManager).
	//
}

void Glow::Shutdown()
{
	// Remove glow from all entities
	for (auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++)
	{
		auto& glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
		auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

		if (glowObject.IsUnused())
		{
			continue;
		}

		if (!entity || entity->IsDormant())
		{
			continue;
		}

		glowObject.m_flAlpha = 0.0f;
	}
}

void Glow::Run()
{
	if (!g_LocalPlayer)
		return;

	for (auto i = 0; i < g_GlowObjManager->m_GlowObjectDefinitions.Count(); i++)
	{
		auto& glowObject = g_GlowObjManager->m_GlowObjectDefinitions[i];
		auto entity = reinterpret_cast<C_BasePlayer*>(glowObject.m_pEntity);

		if (glowObject.IsUnused())
			continue;

		auto class_id = entity->GetClientClass()->m_ClassID;
		auto color = Color{};

		switch (class_id)
		{
		case ClassId::CCSPlayer:
		{
			auto is_enemy = entity->IsEnemy();
			bool glow_enabled = g_Config.glow_team_enable || g_Config.glow_enemy_enable;

			if (!entity || entity->IsDormant())
				continue;

			if(entity->HasC4() && is_enemy && g_Config.glow_c4_carrier)
			{
				color = Color(g_Config.color_glow_c4_carrier);
				break;
			}

			if (!glow_enabled || !entity->IsAlive())
				continue;

			if (!is_enemy && !g_Config.glow_team_enable)
				continue;

			color = is_enemy ? Color(g_Config.color_glow_enemy) : Color(g_Config.color_glow_team);

			glowObject.m_nGlowStyle = is_enemy ? g_Config.glow_enemy_type : g_Config.glow_team_type;
			break;
		}
		case ClassId::CChicken:
			if(!g_Config.glow_chickens)
			{
				continue;
			}
			entity->m_bShouldGlow() = true;
			color = Color(g_Config.color_glow_chickens);
			glowObject.m_nGlowStyle = g_Config.glow_chickens_type;
			break;
		case ClassId::CBaseAnimating:
			if(!g_Config.glow_defuse_kits)
			{
				continue;
			}
			color = Color(g_Config.color_glow_defuse);
			glowObject.m_nGlowStyle = g_Config.glow_defuse_type;
			break;
		case ClassId::CBaseCSGrenadeProjectile:
		case ClassId::CDecoyProjectile:
		case ClassId::CMolotovProjectile:
		case ClassId::CSmokeGrenadeProjectile:
			if (!g_Config.glow_grenade)
			{
				continue;
			}
			color = Color(g_Config.color_glow_grenade);
			glowObject.m_nGlowStyle = g_Config.glow_grenade_type;
			break;
		case ClassId::CPlantedC4:
			if (!entity->m_bBombDefused())
			{
				if (!g_Config.glow_planted_c4)
				{
					continue;
				}
				if (g_Config.glow_planted_warning)
				{
					float flblow = entity->m_flC4Blow();
					float explode = flblow - (g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick);
					if (explode >= 10.f) color = Color(0, 255, 0);
					else if (explode < 10.f && explode >= 5.f) color =Color(255, 255, 0);
					else color = Color(255, 0, 0);
				}
				else
				{
					color = Color(g_Config.color_glow_planted_c4);
				}
				glowObject.m_nGlowStyle = g_Config.glow_planted_c4_type;
			}
			break;
		default:
		{
			if(entity->IsWeapon())
			{
				if(!g_Config.glow_weapons)
				{
					continue;
				}
				color = Color(g_Config.color_glow_weapons);
				glowObject.m_nGlowStyle = g_Config.glow_weapons_type;
			}
		}
		}

		glowObject.m_flRed = color.r() / 255.0f;
		glowObject.m_flGreen = color.g() / 255.0f;
		glowObject.m_flBlue = color.b() / 255.0f;
		glowObject.m_flAlpha = color.a() / 255.0f;
		glowObject.m_bRenderWhenOccluded = true;
		glowObject.m_bRenderWhenUnoccluded = false;

	}
}
