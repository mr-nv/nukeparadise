//ne rabotaet =)
#include "Resolver.h"
#include "AntiAim.h"
#include "Autowall.h"
#include "Logger.h"
#include "../ConsoleHelper.h"
#include "../valve_sdk\csgostructs.hpp"
#include "../helpers\math.hpp"
#include "Backtrack.h"
#include "../RuntimeSaver.h"
#include "../options.hpp"

/*
is fakelag --> set first tick angle
*/

void Resolver::OnCreateMove(QAngle OrgViewang)
{
	return;

	/*
	if (!g_LocalPlayer) return;

	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(i));
		if (!entity || !g_LocalPlayer || !entity->IsPlayer() || entity == g_LocalPlayer || entity->IsDormant()
			|| !entity->IsAlive() || !entity->IsEnemy()) continue;

		float FreestandAng = 0.f;
		bool CanFreestand = FreestandingSim(entity, FreestandAng);//AntiAim::Get().Freestanding(entity, FreestandAng);
		//if (CanFreestand) CanFreestand = false;

		float Edge		   = 0.f;
		bool CanEdge = false;//AntiAim::Get().GetEdgeDetectAngle(entity, Edge);

		float Distance     = 0.f;
		bool CanDistance   = GetWallDistance(entity, Distance);

		float BackwardsAng = 0.f;
		bool CanUseBackwards = AtTargetSim(entity, BackwardsAng);

		SimulatedAAs[i] = { CanFreestand, FreestandAng, CanEdge, Edge, CanDistance, Distance, CanUseBackwards, BackwardsAng };
	}*/
}


void Resolver::OnFramestageNotify()
{
	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		typedef unsigned long LODWORD;
		auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!entity || !g_LocalPlayer || !entity->IsPlayer() || entity == g_LocalPlayer || entity->IsDormant()
			|| !entity->IsAlive() || !entity->IsEnemy())
		{
			g_Resolver.StandAAData[i].UsedEdge = false;
			SavedResolverData[i].WasDormantBetweenMoving = true;
			SavedResolverData[i].CanUseMoveStandDelta = false;
			continue;
		}

		bool InFakeLag = false;

		if (TIME_TO_TICKS(fmax(0, (entity->m_flOldSimulationTime() - entity->m_flSimulationTime()))))
			InFakeLag = true;

		if (SavedResolverData[i].LastSimtime != entity->m_flSimulationTime())
			InFakeLag = false;

		// Ghetto desync detector
		g_Resolver.GResolverData[i].Fake = InFakeLag;
		SavedResolverData[i].LastPos = entity->m_vecOrigin();

		if (InFakeLag)
			g_Resolver.GResolverData[i].BreakingLC = (entity->m_vecOrigin() - SavedResolverData[i].LastPos).LengthSqr() > 4096.f;

		SavedResolverData[i].LastSimtime = entity->m_flSimulationTime();

		if (!g_Resolver.GResolverData[i].Fake)
			continue;

		SavedResolverData[i].UsingAA;

		entity->SetAbsOriginal(entity->m_vecOrigin()); //xD

		entity->SetAbsOriginal(entity->m_vecOrigin()); //fix possition

		g_Resolver.GResolverData[i].InFakelag = InFakeLag;

		ResolverAnimDataStorage AnimationStorage;
		AnimationStorage.UpdateAnimationLayers(entity);

		bool Moving = entity->m_vecVelocity().Length2D() > 0.1f;
		bool SlowWalk = Moving && entity->m_vecVelocity().Length2D() < 52.f && fabs(SavedResolverData[i].LastVel - entity->m_vecVelocity().Length2D()) < 4.f;
		bool InAir = !(entity->m_fFlags() & FL_ONGROUND);
		bool Standing = !Moving && !InAir;

		SavedResolverData[i].LastVel = entity->m_vecVelocity().Length2D();

		if (!InFakeLag)
			AddCurrentYaw(entity, i);

		float avgang = 0.f;
		bool b = GetAverageYaw(i, avgang);

		auto animstate = entity->AnimStatev2();
		if (animstate)
		{
			// missed shot <= 2
			if (g_Resolver.GResolverData[entity->EntIndex()].Shots <= 2)
			{
				if (animstate->m_fDuckAmount > 0.0)
				{
					auto v29 = 0.0;
					if (animstate->m_flFeetSpeedUnknownForwardOrSideways < 0.0)
						v29 = 0.0;
					else
						v29 = std::fminf(LODWORD(animstate->m_flFeetSpeedUnknownForwardOrSideways), 0x3F800000);
				}

				float speed;
				if (*(float*)(animstate + 0xF8) < 0.f)
				{
					speed = 0.0;
				}
				else
				{
					speed = fminf(*(DWORD*)(animstate + 0xF8), 1.0f);
				}

				float flYawModifier = (*(float*)(animstate + 0x11C) * -0.30000001 - 0.19999999) * speed;
				flYawModifier += 1.0f;

				if (*(float*)(animstate + 0xA4) > 0.0 && *(float*)(animstate + 0xFC) >= 0.0)
					flYawModifier = fminf(*(float*)(uintptr_t(animstate) + 0xFC), 1.0f);

				float m_flMaxBodyYaw = *(float*)(uintptr_t(animstate) + 0x334) * flYawModifier;
				float m_flMinBodyYaw = *(float*)(uintptr_t(animstate) + 0x330) * flYawModifier;

				float ResolvedYaw = animstate->m_flEyeYaw;
				float delta = std::abs(animstate->m_flEyeYaw - animstate->m_flGoalFeetYaw);
				if (m_flMaxBodyYaw < delta)
				{
					ResolvedYaw = animstate->m_flEyeYaw - std::abs(m_flMaxBodyYaw);
				}
				else if (m_flMinBodyYaw > delta)
				{
					ResolvedYaw = animstate->m_flEyeYaw + std::abs(m_flMinBodyYaw);
				}
				animstate->m_flGoalFeetYaw = Math::NormalizeYaw(ResolvedYaw);
			}
			else
			{
				switch (g_Resolver.GResolverData[entity->EntIndex()].Shots % 7)
				{
				case 0:
					animstate->m_flGoalFeetYaw += 59.0f;
					break;
				case 1:
					animstate->m_flGoalFeetYaw -= 59.0f;
					break;
				case 2:
					animstate->m_flGoalFeetYaw -= 78.0f;
					break;
				case 3:
					animstate->m_flGoalFeetYaw += 78.0f;
					break;
				case 4:
					animstate->m_flGoalFeetYaw -= animstate->m_flGoalFeetYaw ? -180 : 180;
					break;
				case 5:
					animstate->m_flGoalFeetYaw += animstate->m_flGoalFeetYaw ? -180 : 180;
					break;
				case 6:
					animstate->m_flGoalFeetYaw -= animstate->m_flGoalFeetYaw ? -58 : 58; // desync
					break;
				default:
					break;
				}
			}
		}
		if (Moving && !SlowWalk && !InAir)
			g_Resolver.GResolverData[i].Resolved = true;
		else if (Moving && SlowWalk && !InAir)
		{
			if (IsStaticYaw(i) && b)
			{
				if (g_Resolver.GResolverData[i].Shots >= 1)
				{
					switch (g_Resolver.GResolverData[i].Shots % 4)
					{
					case 0:
						entity->m_angEyeAngles().yaw += 58.f;
						break;

					case 1:
						entity->m_angEyeAngles().yaw -= 58.f;
						break;

					case 2:
						entity->m_angEyeAngles().yaw += 29.f;
						break;

					case 3:
						entity->m_angEyeAngles().yaw -= 29.f;
						break;
					}
				}
			}

			g_Resolver.GResolverData[i].Resolved = false;

		}
		else if (InAir)
			g_Resolver.GResolverData[i].Resolved = true;
		else
		{
			g_Resolver.GResolverData[i].Resolved = true;
			float fl_ang = fabs(fabs(entity->m_flLowerBodyYawTarget()) - fabs(entity->m_angEyeAngles().yaw));
			bool fake = (fl_ang >= 45.f && fl_ang <= 85.f) || !IsStaticYaw(i);
			float Yaw = entity->m_angEyeAngles().yaw;

			if (fake)
			{
				g_Resolver.GResolverData[i].Resolved = false;

				if (!IsStaticYaw(i) && b)
					Yaw = avgang;
			}

			if (fake && g_Resolver.GResolverData[i].Shots >= 1)
			{
				switch (g_Resolver.GResolverData[i].Shots % 2)
				{
				case 0:
					entity->m_angEyeAngles().yaw = Yaw + 58.f;
					entity->GetPlayerAnimState()->m_flGoalFeetYaw += entity->GetMaxDesyncAngle();
					break;

				case 1:
					entity->m_angEyeAngles().yaw = Yaw - 58.f;
					entity->GetPlayerAnimState()->m_flGoalFeetYaw -= entity->GetMaxDesyncAngle();
					break;

				case 2:
					entity->m_angEyeAngles().yaw = Yaw + 29;
					break;

				case 3:
					entity->m_angEyeAngles().yaw = Yaw - 29;
					break;
				}
			}
		}

		g_Resolver.GResolverData[i].Fake = !g_Resolver.GResolverData[i].Resolved;
	}
}


void Resolver::OnFireEvent(IGameEvent* event)
{
	if (!g_LocalPlayer)
		return;

	static float LastPlayerHurt = 0.f;

	if (!strcmp(event->GetName(), "player_hurt"))
	{
		if (g_GlobalVars->curtime == LastPlayerHurt)
			return;

		LastPlayerHurt = g_GlobalVars->curtime;

		int i = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		int attacker = g_EngineClient->GetPlayerForUserID(event->GetInt("attacker"));

		if (attacker != g_LocalPlayer->EntIndex())
			return;

		C_BasePlayer* user = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!user)
			return;

		std::string hitgroup = "";

		switch (event->GetInt("hitgroup"))
		{
		case HITBOX_HEAD:
			hitgroup = "HEAD";
			break;

		case HITBOX_NECK:
			hitgroup = "NECK";
			break;

		case HITBOX_PELVIS:
			hitgroup = "PELVIS";
			break;

		case HITBOX_STOMACH:
			hitgroup = "STOMACH";
			break;

		case HITBOX_LOWER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST:
			hitgroup = "CHEST";
			break;

		case HITBOX_RIGHT_THIGH:
		case HITBOX_LEFT_THIGH:
			hitgroup = "THIGH";
			break;

		case HITBOX_RIGHT_CALF:
		case HITBOX_LEFT_CALF:
			hitgroup = "CALF";
			break;

		case HITBOX_RIGHT_FOOT:
		case HITBOX_LEFT_FOOT:
			hitgroup = "FOOT";
			break;

		case HITBOX_RIGHT_HAND:
		case HITBOX_LEFT_HAND:
			hitgroup = "HAND";
			break;

		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
			hitgroup = "ARM";
			break;
		}

		//std::string InLbyUpdate = g_Saver.RbotShotInfo.InLbyUpdate ? "TRUE" : "FALSE";
		std::string InLC = g_Saver.RbotShotInfo.InLc ? "TRUE" : "FALSE";
		std::string InMove = g_Saver.RbotShotInfo.Moving ? "TRUE" : "FALSE";
		g_Logger.Damage(
			"DAMAGE", "-" + std::to_string(event->GetInt("dmg_health")) + " in " + hitgroup + " to " + std::string(user->GetName(true))
			+ " [ LC: " + InLC + " MOVING: " + InMove + " ]"
		);

		if (!g_Saver.RbotShotInfo.InLbyUpdate)
		{
			g_Resolver.GResolverData[i].ShotsAtMode[(int)g_Resolver.GResolverData[i].mode]--;
			g_Resolver.GResolverData[i].Shots--;

			if (g_Resolver.GResolverData[i].Shots < 0)
				g_Resolver.GResolverData[i].Shots = 0;
		}
	}

	if (!strcmp(event->GetName(), "player_death"))
	{
		int i = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		g_Resolver.GResolverData[i].Shots = 0;
		SavedResolverData[i].MoveStandDelta = 0.f;

		for (int i2 = 0; i2 < (int)ResolverModes::MAX; i2++)
			g_Resolver.GResolverData[i].ShotsAtMode[i2] = 0;
	}

	static std::string lastName = "";
	static std::string lastWeapon = "";

	if (!strcmp(event->GetName(), "item_purchase"))
	{
		int i = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));
		C_BasePlayer* user = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!user)
			return;

		if (lastName == user->GetPlayerInfo().szName && lastWeapon == event->GetString("weapon"))
			return;

		lastName = user->GetPlayerInfo().szName;
		lastWeapon = event->GetString("weapon");

		g_Logger.Info("PURCHASE", lastName + " purchased " + lastWeapon);
	}

	if (!strcmp(event->GetName(), "round_end"))
	{
		for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
		{
			g_Resolver.GResolverData[i].Shots = 0;
			SavedResolverData[i].MoveStandDelta = 0.f;

			for (int i2 = 0; i2 < (int)ResolverModes::MAX; i2++)
				g_Resolver.GResolverData[i].ShotsAtMode[i2] = 0;
		}
	}

	if (!strcmp(event->GetName(), "player_disconnect"))
	{
		int i = g_EngineClient->GetPlayerForUserID(event->GetInt("userid"));

		if (i == g_EngineClient->GetLocalPlayer())
			return;

		g_Resolver.GResolverData[i].Shots = 0;
		SavedResolverData[i].MoveStandDelta = 0.f;

		for (int i2 = 0; i2 < (int)ResolverModes::MAX; i2++)
			g_Resolver.GResolverData[i].ShotsAtMode[i2] = 0;
	}

	if (!strcmp(event->GetName(), "bomb_pickup"))
	{
		auto userid = event->GetInt("userid");

		if (!userid)
			return;

		auto userid_id = g_EngineClient->GetPlayerForUserID(userid);

		player_info_t userid_info;
		if (!g_EngineClient->GetPlayerInfo(userid_id, &userid_info))
			return;

		std::string string(Utils::Format("%s has pickup bomb.", userid_info.szName));

		g_Logger.GameEvent("PICKUP", string);
	}


	if (!strcmp(event->GetName(), "bomb_beginplant"))
	{
		auto userid = event->GetInt("userid");

		if (!userid)
			return;

		auto userid_id = g_EngineClient->GetPlayerForUserID(userid);

		player_info_t userid_info;
		if (!g_EngineClient->GetPlayerInfo(userid_id, &userid_info))
			return;

		std::string string(Utils::Format("%s has began planting the bomb at site %d.", userid_info.szName, static_cast<int>(event->GetInt("site")))); //o_0

		g_Logger.GameEvent("PLANT", string);
	}

	if (!strcmp(event->GetName(), "bomb_begindefuse"))
	{
		auto userid = event->GetInt("userid");

		if (!userid)
			return;

		auto userid_id = g_EngineClient->GetPlayerForUserID(userid);

		player_info_t userid_info;
		if (!g_EngineClient->GetPlayerInfo(userid_id, &userid_info))
			return;

		std::string string;

		if (event->GetBool("haskit") == true)
			string = (Utils::Format("%s has began defusing the bomb with kit.", userid_info.szName));
		else
			string = (Utils::Format("%s has began defusing the bomb without kit.", userid_info.szName));

		g_Logger.GameEvent("DEFUSE", string);
	}
}

void Resolver::AddCurrentYaw(C_BasePlayer* pl, int i)
{
	LastYaws[i].push_back(pl->m_angEyeAngles().yaw);

	if (LastYaws[i].size() > 8)
		LastYaws[i].erase(LastYaws[i].begin());
}
bool Resolver::IsStaticYaw(int i)
{
	if (LastYaws[i].size() < 3)
		return true;

	//float LastYaw = 0.f;
	//float LastYaw2 = 0.f;
	//bool ReturnVal = true;

	float HighestDifference = 0.f;

	for (size_t p = 0; p < LastYaws[i].size(); p++)
	{
		for (size_t p2 = 0; p2 < LastYaws[i].size(); p2++)
		{
			float c = fabs(fabs(LastYaws[i].at(p)) - fabs(LastYaws[i].at(p2)));

			if (c > HighestDifference)
				HighestDifference = c;
		}
	}

	if (HighestDifference > 15.f)
		return false;
	else
		return true;
}
bool Resolver::GetAverageYaw(int i, float& ang)
{
	if (LastYaws[i].size() < 3)
		return true;

	float add = 0.f;

	for (size_t p = 0; p < LastYaws[i].size(); p++)
		add += LastYaws[i].at(p);

	ang = add / LastYaws[i].size();
	return true;
}

Resolver g_Resolver;