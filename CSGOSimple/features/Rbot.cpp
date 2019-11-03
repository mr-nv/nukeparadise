#include "Backtrack.h"
#include "Rbot.h"
#include "../Misc.h"
#include "../config.hpp"
#include "../helpers/math.hpp"
#include "../helpers/input.hpp"
#include "Autowall.h"
#include "Logger.h"
#include "Resolver.h"
#include "../helpers/utils.hpp"
#include "../features/LagCompensation.h"
#include "../RuntimeSaver.h"
#include "../ConsoleHelper.h"
#include "../options.hpp"
#include "../KeyLoop.h"
RbotMatrixData matrixData[128];

static int MinimumVelocity = 2.1f;

void Rbot::CreateMove(CUserCmd* cmd, bool& bSendPacket)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;
	if (g_Config.rbot_fakeduck && InputSys::Get().IsKeyDown(g_Config.rbot_fakeduck_key)) FakeDuck(cmd, bSendPacket);
	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon) return;
	if (weapon->IsKnife() || weapon->IsGrenade() || weapon->IsPlantedC4() || weapon->IsDefuseKit()) return;
	if (weapon->Is_no_aim() || weapon->m_iClip1() < 1) return;
	//if (!weapon->CanFire()) return;
	//if ((g_LocalPlayer->m_flNextAttack() > Utils::ServerTime(cmd))) return;

	if (weapon->IsZeus()) { ZeusBot(cmd, weapon); return; }

	if (Utils::pWeaponType() == 10) return;

	CurrentCmd = cmd;

	if (!this->Revolver(cmd, weapon))
		return;

	Vector hitpos = Vector(0, 0, 0);

	UpdateConfigData();

	int BestEntity = FindBestEntity(cmd, weapon, hitpos, g_Config.rbot_lagcompensation);

	if (hitpos == Vector(0, 0, 0)) return;

	if (BestEntity == -1) return;

	C_BasePlayer* entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(BestEntity));

	QAngle newAng = Math::CalcAngle(g_LocalPlayer->GetEyePos(), hitpos);

	float recoil_scale = g_CVar->FindVar("weapon_recoil_scale")->GetFloat();
	QAngle a = (g_LocalPlayer->m_aimPunchAngle() * recoil_scale);
	newAng -= a;
	Math::NormalizeAngles(a);

	Vector v; Math::AngleVectors(a, v);
	float tdmg = Autowall::Get().CanHit(hitpos - v);
	if (tdmg == -1.f) return;

	if (weapon->IsSniper() && !g_LocalPlayer->m_bIsScoped() && (g_LocalPlayer->m_fFlags() & FL_ONGROUND) && g_Config.rbot_autoscope)
	{
		if (!(cmd->buttons & IN_ZOOM))
			cmd->buttons |= IN_ZOOM;
		/*
		if (g_Config.rbot_autostop)
		{
			auto ShootState = this->IsAbleToShoot();
			if ((g_LocalPlayer->m_fFlags() & FL_ONGROUND) && ShootState && g_LocalPlayer->m_vecVelocity().Length() >= MinimumVelocity && !GetAsyncKeyState(VK_SPACE))
				AutoStop(cmd, entity);
		}
		if (!(cmd->buttons & IN_DUCK) && g_Config.rbot_autocrouch)
			AutoCrouch(cmd);
			*/
		return;
	}

	if (!HitChance(newAng, entity, HitChina))
	{
		if (g_Config.rbot_autostop)
		{
			auto ShootState = this->IsAbleToShoot();
			if ((g_LocalPlayer->m_fFlags() & FL_ONGROUND) && ShootState && /*g_LocalPlayer->m_vecVelocity().Length2D() >= MinimumVelocity && */!GetAsyncKeyState(VK_SPACE))
				AutoStop(cmd, entity);
		}

		if (!(cmd->buttons & IN_DUCK) && g_Config.rbot_autocrouch)
			AutoCrouch(cmd);

		return;
	}

	if (!weapon->CanFire() && g_Saver.curtime <= weapon->m_flNextPrimaryAttack()) return;

	LastRbotEnemyIndex = BestEntity;

	Math::NormalizeAngles(newAng);
	Math::ClampAngles(newAng);

	int tick = TIME_TO_TICKS(entity->m_flSimulationTime() + LagCompensation::Get().GetLerpTime());

	if (!LagCompensation::Get().IsTickValid(tick))
		return;

	if (g_Config.rbot_aimstep)
	{
		if (newAng.Length() > 25.f)
		{
			Math::QANGLE_Normalize(newAng, newAng);
			newAng *= 25;
			cmd->viewangles = newAng;
			//Math::QANGLE_NormaliseViewAngle(cmd->viewangles);
		}
		else
			cmd->viewangles = newAng;
	}
	else
		cmd->viewangles = newAng;

	cmd->buttons |= IN_ATTACK;

	g_Saver.RbotShotInfo.InLbyUpdate = g_Resolver.GResolverData[BestEntity].mode == ResolverModes::LBY_BREAK;
	g_Saver.RbotShotInfo.InLc = g_Resolver.GResolverData[BestEntity].BreakingLC;
	g_Saver.RbotShotInfo.Moving = g_Resolver.GResolverData[BestEntity].Moving;
	g_Saver.AARealAngle = cmd->viewangles;

	//if (g_Config.rbot_lagcompensation)
		//cmd->tick_count = tick;

}

void Rbot::OnFireEvent(IGameEvent* event)
{
	if (!g_LocalPlayer)
		return;

	//Utils::ConsolePrint(std::string(std::string(event->GetName()) + std::string("\n")).c_str());
	static int LastEventShotLastEntIndex = -1;

	if (!strcmp(event->GetName(), "weapon_fire"))
	{
		if (g_EngineClient->GetPlayerForUserID(event->GetInt("userid")) == g_LocalPlayer->EntIndex())
		{
			//g_Logger.Add("onfire entity", std::to_string(LastRbotEnemyIndex), Color::Blue);
			if (LastRbotEnemyIndex != -1)
			{
				LastEventShotLastEntIndex = LastRbotEnemyIndex;
				//g_Resolver.GResolverData[LastRbotEnemyIndex].Shots++;
				//g_Resolver.GResolverData[LastRbotEnemyIndex].ShotsAtMode[(int)g_Resolver.GResolverData[LastRbotEnemyIndex].mode]++;
				LastRbotEnemyIndex = -1;
			}
		}
	}

	static float lastEventTime = 0.f;

	if (!strcmp(event->GetName(), "bullet_impact"))
	{
		C_BasePlayer* shooter = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(g_EngineClient->GetPlayerForUserID(event->GetInt("userid"))));

		if (!shooter || shooter != g_LocalPlayer)
			return;

		if (LastEventShotLastEntIndex > 128)
			LastEventShotLastEntIndex = -1;

		if (lastEventTime == g_GlobalVars->curtime || LastEventShotLastEntIndex == -1)
			return;

		lastEventTime = g_GlobalVars->curtime;

		Vector p = Vector(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));

		float Dmg = -1.f;

		if (g_Saver.LastShotEyePos == Vector(0.f, 0.f, 0.f))
			Dmg = Autowall::Get().CanHit(p);
		else
			Dmg = Autowall::Get().CanHit(g_Saver.LastShotEyePos, p);

		if (Dmg == -1.f)
		{
			g_Logger.Error("MISS", "missed due to spread");
			//LastMissedDueToSpread = true;
		}
		else
		{

			if (!g_Saver.RbotShotInfo.InLbyUpdate)
			{
				g_Resolver.GResolverData[LastEventShotLastEntIndex].Shots++;
				g_Resolver.GResolverData[LastEventShotLastEntIndex].ShotsAtMode[(int)g_Resolver.GResolverData[LastEventShotLastEntIndex].mode]++;
				LastEventShotLastEntIndex = -1;
			}

			//LastMissedDueToSpread = false;
		}

		g_Resolver.GResolverData[LastEventShotLastEntIndex].Shots++;
		g_Resolver.GResolverData[LastEventShotLastEntIndex].ShotsAtMode[(int)g_Resolver.GResolverData[LastEventShotLastEntIndex].mode]++;
		LastEventShotLastEntIndex = -1;
	}
}

bool Rbot::InFakeLag(C_BasePlayer* player)
{
	bool rBool = true;
	float CurrentSimtime = player->m_flSimulationTime();
	int i = player->EntIndex();

	if (Simtimes[i] != CurrentSimtime)
		rBool = false;

	Simtimes[i] = CurrentSimtime;
	return rBool;
}

void Rbot::AutoStop(CUserCmd* cmd, C_BasePlayer* target)
{
	static auto nospread = g_CVar->FindVar("weapon_accuracy_nospread");
	if (nospread->GetInt() == 1) return;

	if (cmd->buttons & IN_JUMP)
		return;

	if (!g_LocalPlayer->m_hActiveWeapon())
		return;
	Vector local_position = g_LocalPlayer->m_vecOrigin() + g_LocalPlayer->m_vecViewOffset();
	if (target->m_bGunGameImmunity()) return;
	if (target->IsDormant()) return;
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon) return;
	if (weapon->Is_no_aim() || weapon->m_iItemDefinitionIndex() == WEAPON_TASER) return;

	QAngle dir;
	Math::VectorAngles(g_Saver.OLDvelocity, dir);
	QAngle view;
	g_EngineClient->GetViewAngles(view);
	dir.yaw = view.yaw - dir.yaw;

	Vector move;
	Math::AngleVectors(dir, move);

	if (g_Saver.OLDvelocity.Length2D() > .1f)
		move *= -450.f / std::max(std::abs(move.x), std::abs(move.y));

	cmd->forwardmove = move.x;
	cmd->sidemove = move.y;

	const auto backup = cmd->viewangles;
	cmd->viewangles = view;
	cmd->viewangles = backup;

	if (g_LocalPlayer->m_vecVelocity().Length2D() > g_Saver.OLDvelocity.Length2D())
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;
	}
}

void Rbot::AutoCrouch(CUserCmd* cmd)
{
	cmd->buttons |= IN_DUCK;
}

void Rbot::ZeusBot(CUserCmd* cmd, C_BaseCombatWeapon* weapon)
{
	cmd->buttons &= ~IN_ATTACK;

	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	C_BasePlayer* local = g_LocalPlayer;

	static bool DidShotLast = false;

	float range = weapon->GetCSWeaponData()->flRange;

	Vector BestPos;
	float BestDmg = 0.f;
	bool Found = false;

	if (DidShotLast)
	{
		DidShotLast = false;
		return;
	}

	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!entity || !g_LocalPlayer || !entity->IsPlayer() || entity == g_LocalPlayer || entity->IsDormant()
			|| !entity->IsAlive() || !entity->IsEnemy())
			continue;

		if (local->m_vecOrigin().DistTo(entity->m_vecOrigin()) >= range)
			continue;

		for (int hitboxn = 0; hitboxn < 2; hitboxn++)
		{
			Vector pos = Vector(0, 0, 0);

			switch (hitboxn)
			{
			case 0: //pelvis
				pos = entity->GetHitboxPos(HITBOX_PELVIS);
				break;

			case 1: //chest
				pos = entity->GetHitboxPos(HITBOX_CHEST);
				break;
			}

			if (local->GetEyePos().DistTo(pos) >= range)
				continue;

			float Dmg = Autowall::Get().CanHit(pos);

			if (Dmg >= entity->m_iHealth() && Dmg > BestDmg)
			{
				BestDmg = Dmg;
				Found = true;
				BestPos = pos;
			}
		}
	}

	if (!Found)
		return;

	QAngle newAng = Math::CalcAngle(g_LocalPlayer->GetEyePos(), BestPos);

	Math::NormalizeAngles(newAng);
	Math::ClampAngles(newAng);

	cmd->viewangles = newAng;
	cmd->buttons |= IN_ATTACK;
	DidShotLast = true;
}

bool Rbot::HitChance(QAngle angles, C_BasePlayer* ent, float chance)
{
	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)
		return false;

	Vector forward, right, up;
	Vector src = g_LocalPlayer->GetEyePos();
	Math::AngleVectors(angles, forward, right, up);

	int cHits = 0;
	int cNeededHits = static_cast<int> (256.f * (chance / 100.f));

	weapon->UpdateAccuracyPenalty();
	float weap_spread = weapon->GetSpread();
	float weap_inaccuracy = weapon->GetInaccuracy();

	for (int i = 0; i < 255; i++)
	{
		Math::RandomSeed(i + 1);
		float a = Math::RandomFloat(0.f, 1.f);
		float b = Math::RandomFloat(0.f, 2.f * PI_F);
		float c = Math::RandomFloat(0.f, 1.f);
		float d = Math::RandomFloat(0.f, 2.f * PI_F);

		float inaccuracy = a * weap_inaccuracy;
		float spread = c * weap_spread;

		if (weapon->m_Item().m_iItemDefinitionIndex() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
		direction.Normalized();

		QAngle viewAnglesSpread;
		Math::VectorAngles(direction, up, viewAnglesSpread);
		viewAnglesSpread.Normalize();

		Vector viewForward;
		Math::AngleVectors(viewAnglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = src + (viewForward * weapon->GetCSWeaponData()->flRange);
		trace_t tr;
		Ray_t ray;

		ray.Init(src, viewForward);
		g_EngineTrace->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, ent, &tr);

		if (tr.hit_entity == ent)
			++cHits;

		if (static_cast<int> ((static_cast<float> (cHits) / 256.f) * 100.f) >= chance)
			return true;

		if ((256 - i + cHits) < cNeededHits)
			return false;
	}

	return false;
}

void Rbot::FakeDuck(CUserCmd* cmd, bool& bSendPackets)
{
	cmd->buttons |= IN_BULLRUSH;

	static int cnt = 0;
	static bool do_ = false;

	if (cmd->buttons) {
		bSendPackets = false;

		if (cnt % 14 == 0)
			do_ = true;
		else if (cnt % 14 == 6)
			bSendPackets = true;
		else if (cnt % 14 == 7)
			do_ = false;

		if (do_)
			cmd->buttons |= IN_DUCK;
		else
			cmd->buttons &= ~IN_DUCK;

		cnt++;
	}
	else {
		do_ = false;
		cnt = 0;
	}
}

int Rbot::FindBestEntity(CUserCmd* cmd, C_BaseCombatWeapon* weapon, Vector& hitpos, bool& bBacktrack)
{
	int BestEntityIndex = -1;
	float WeaponRange = weapon->GetCSWeaponData()->flRange;

	float BestDamage = 0.f;
	Vector BestHitpoint;

	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		bool LagComp_Hitchanced = false;
		bool WillKillEntity = false;

		auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!entity || !g_LocalPlayer || entity->m_bGunGameImmunity() || !entity->IsPlayer() || entity == g_LocalPlayer || entity->IsDormant()
			|| !entity->IsAlive() || !entity->IsEnemy())
			continue;

		if (!matrixData[i].gotMatrix)
			continue;

		if (g_Config.rbot_force_unlage && entity->m_vecVelocity().Length2D() > 0.1f && g_Resolver.GResolverData[i].InFakelag)
			continue;

		float Distance = Math::VectorDistance(g_LocalPlayer->GetEyePos(), entity->GetEyePos());

		if (Distance > WeaponRange)
			continue;

		float CDamage = 0.f;
		Vector CHitpos;
		TickRecord CBacktrackRecord;
		bool CUsingBacktrack = false;

		BaimMode baim = g_Resolver.GResolverData[i].Resolved ? BaimMode::NONE : BaimMode::BAIM;

		if (!g_Config.rbot_resolver)
			baim = BaimMode::NONE;

		if (g_Resolver.GResolverData[i].Shots > BaimMax&& g_Config.rbot_baim_after_shots != 0) //ebat
			baim = BaimMode::BAIM;			//baim = BaimMode::FORCE_BAIM;

		if (g_Config.rbot_baim_while_moving && g_LocalPlayer->m_vecVelocity().Length() > 0.1f)
			baim = BaimMode::FORCE_BAIM;

		if (!g_Config.rbot_baimmode) baim = BaimMode::NONE;

		if (g_Resolver.GResolverData[i].ForceBaim) { baim = BaimMode::FORCE_BAIM; }

		if (!g_Config.rbot_lagcompensation)
		{
			if (!GetBestHitboxPoint(entity, CDamage, CHitpos, baim, WillKillEntity))
				continue;
		}
		else
		{
			LagCompensation::Get().RageBacktrack(entity, CurrentCmd, CHitpos, CDamage, LagComp_Hitchanced); //ya zapreshchayu vam strelyat
		}

		BestDamage = CDamage;
		BestHitpoint = CHitpos;
		BestEntityIndex = entity->EntIndex();

	}

	hitpos = BestHitpoint;
	return BestEntityIndex;
}

bool Rbot::IsAbleToShoot() {
	if (!g_LocalPlayer->m_hActiveWeapon())
		return false;

	return !(g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() > g_LocalPlayer->m_nTickBase()* g_GlobalVars->interval_per_tick&& g_LocalPlayer->m_hActiveWeapon()->m_iClip1() > 0);
}

BaimMode* Rbot::GetBAimStatus()
{
	return &baim;
}

void Rbot::AccuracyBoost(CUserCmd* cmd)
{
	if (!g_LocalPlayer || !cmd)// || !G::me->has_attackable_weapon())
		return;

	auto no_spread = [](CUserCmd* _cmd)
	{
		C_BaseCombatWeapon* weapon;

		if ((weapon = g_LocalPlayer->m_hActiveWeapon()) == nullptr || g_Saver.PredictionData.prediction_random_seed == nullptr)
			return;

		((*g_Saver.PredictionData.prediction_random_seed & 255) + 1);

		auto s1 = Math::RandomFloat(0.f, 1.f), s2 = Math::RandomFloat(0.f, 2.f * DirectX::XM_PI), s3 = Math::RandomFloat(0.f, 1.f), s4 = Math::RandomFloat(0.f, 2.f * DirectX::XM_PI);

		if (weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && _cmd->buttons & IN_ATTACK2)
		{
			s1 = 1.f - s1 * s1;
			s3 = 1.f - s3 * s3;
		}
		else if (weapon->m_iItemDefinitionIndex() == WEAPON_NEGEV && weapon->m_flRecoilIndex() < 3.f)
		{
			for (auto i = 3; i > weapon->m_flRecoilIndex(); i--)
			{
				s1 *= s1;
				s3 *= s3;
			}

			s1 = 1.f - s1;
			s3 = 1.f - s3;
		}

		auto random_spread = s1 * weapon->GetSpread();
		auto cone = s3 * weapon->GetCone();

		auto spread = Vector(sin(s2) * random_spread + sin(s4) * cone, cos(s2) * random_spread + cos(s4) * cone, 0.f);

		_cmd->viewangles.pitch += DirectX::XMConvertToDegrees(atan(spread.Length2D()));
		_cmd->viewangles.roll = -DirectX::XMConvertToDegrees(atan2(spread.y, spread.x));
	};

	auto no_recoil = [](CUserCmd* _cmd)
	{
		_cmd->viewangles -= g_LocalPlayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();
	};

	//no_recoil(cmd);

	//no_spread(cmd); //nice

	Math::NormalizeAngles(cmd->viewangles);
}

bool Rbot::GetBestHitboxPoint(C_BasePlayer* entity, float& damage, Vector& hitbox, BaimMode baim, bool& WillKill, matrix3x4_t matrix[MAXSTUDIOBONES], mstudiohitboxset_t* StudioSet, bool NoPointscale)
{
	matrix3x4_t cmatrix[MAXSTUDIOBONES];

	if (!matrix)
	{
		//PrecachePointers(entity);
		matrix = matrixData[entity->EntIndex()].matrix;
	}

	if (!StudioSet)
		StudioSet = matrixData[entity->EntIndex()].StudioSet;

	WillKill = false;

	float BestDamage = 0.f;
	Vector BestHitpoint;
	bool FoundHitableEntity = false;

	CanHitStruct CanHitHead;
	CanHitStruct CanBaimKill;

	for (int hitbox = 0; hitbox < Hitboxes::HITBOX_MAX; hitbox++)
	{
		//if (CanKillWithBaim) continue;

		if ((hitbox == HITBOX_HEAD || hitbox == HITBOX_NECK || hitbox >= HITBOX_RIGHT_THIGH) && baim == BaimMode::FORCE_BAIM)
			continue;

		float pointscale = 0.f;

		switch (hitbox)
		{
		case HITBOX_HEAD:
			if (!HitboxHead)
				continue;

			pointscale = HitboxHeadScale;
			break;

		case HITBOX_NECK:
			if (!HitboxNeck)
				continue;

			pointscale = HitboxNeckScale;
			break;

		case HITBOX_PELVIS:
			if (!HitboxPelvis)
				continue;

			pointscale = HitboxPelvisScale;
			break;

		case HITBOX_STOMACH:
			if (!HitboxStomach)
				continue;

			pointscale = HitboxStomachScale;
			break;

		case HITBOX_LOWER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_UPPER_CHEST:
			if (!HitboxChest)
				continue;

			pointscale = HitboxChestScale;
			break;

		case HITBOX_RIGHT_THIGH:
		case HITBOX_LEFT_THIGH:
		case HITBOX_RIGHT_CALF:
		case HITBOX_LEFT_CALF:
			if (!HitboxLeg)
				continue;

			pointscale = HitboxLegScale;
			break;

		case HITBOX_RIGHT_FOOT:
		case HITBOX_LEFT_FOOT:
			if (!HitboxFoot)
				continue;

			pointscale = HitboxFootScale;
			break;

		case HITBOX_RIGHT_HAND:
		case HITBOX_LEFT_HAND:
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_RIGHT_FOREARM:
		case HITBOX_LEFT_UPPER_ARM:
		case HITBOX_LEFT_FOREARM:
			if (!HitboxArm)
				continue;

			pointscale = HitboxArmScale;
			break;

		default:
			continue;
		}

		//float pointscale = hitbox == HITBOX_HEAD ? rbot_head_scale : rbot_body_scale;

		//if (pointscale != 0.f) {

		if (NoPointscale)
			pointscale = 0.f;

		std::vector<PointScanStruct> Points = GetPointsForScan(entity, hitbox, StudioSet, matrix, pointscale);

		//Utils::ConsolePrint("got pints\n");

		float CBestDamage = 0.f;
		Vector CBestPoint;
		Vector CCenter;
		bool CCanHitCenter = false;
		float CCenterDamage = -1.f;

		for (size_t p = 0; p < Points.size(); p++)
		{
			/*
			if (g_LocalPlayer->m_hActiveWeapon()->IsSniper() && !g_LocalPlayer->m_bIsScoped() && (g_LocalPlayer->m_fFlags() & FL_ONGROUND) && g_Config.GetBool("rbot_autoscope"))
			{
				if (!(CurrentCmd->buttons & IN_ZOOM)) CurrentCmd->buttons |= IN_ZOOM;
				if (g_Config.GetBool("rbot_autostop")) AutoStop(CurrentCmd);
				if (!(CurrentCmd->buttons & IN_DUCK) && g_Config.GetBool("rbot_autocrouch")) AutoCrouch(CurrentCmd);
				continue;
			}
			if (!HitChance(Math::CalcAngle(g_LocalPlayer->GetEyePos(), Points[p].pos), entity, g_Config.GetFloat("rbot_min_hitchance")))
			{
				if (g_Config.GetBool("rbot_autostop")) AutoStop(CurrentCmd);
				if (!(CurrentCmd->buttons & IN_DUCK) && g_Config.GetBool("rbot_autocrouch")) AutoCrouch(CurrentCmd);
				continue;
			}
			*/
			float CDamage = 0.f;
			CDamage = Autowall::Get().CanHit(Points[p].pos);

			if ((hitbox == HITBOX_HEAD || hitbox == HITBOX_NECK) && baim == BaimMode::BAIM)
			{
				if (CDamage >= MinDmg && CDamage > CanHitHead.damage)
				{
					CanHitHead.CanHit = true;
					CanHitHead.damage = CDamage;
					CanHitHead.pos = Points[p].pos;
				}

				continue;
			}

			if (baim == BaimMode::BAIM && (hitbox >= HITBOX_RIGHT_HAND || hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_LEFT_THIGH || hitbox == HITBOX_RIGHT_CALF || hitbox == HITBOX_LEFT_CALF))
				continue;

			if (Points[p].center && CDamage >= MinDmg)
			{
				CCanHitCenter = true;
				CCenter = Points[p].pos;
				CCenterDamage = CDamage;
			}


			if (CDamage >= MinDmg && CDamage > CBestDamage)
			{
				CBestDamage = CDamage;
				CBestPoint = Points[p].pos;
			}
		}

		if (CCanHitCenter && CCenterDamage >= entity->m_iHealth() / 2.f)
		{
			CBestDamage = CCenterDamage;
			CBestPoint = CCenter;
		}

		if (CBestDamage >= MinDmg && CanBaimKill.damage < CBestDamage && (hitbox == HITBOX_CHEST || hitbox == HITBOX_LOWER_CHEST || hitbox == HITBOX_PELVIS || hitbox == HITBOX_STOMACH))
		{
			CanBaimKill.CanHit = true;
			CanBaimKill.damage = CBestDamage;
			CanBaimKill.pos = CBestPoint;
		}


		if (CBestDamage >= MinDmg && CBestDamage > BestDamage)
		{
			BestDamage = CBestDamage;
			BestHitpoint = CBestPoint;
			FoundHitableEntity = true;
		}

	}

	if (!FoundHitableEntity && CanHitHead.CanHit)
	{
		FoundHitableEntity = true;
		BestDamage = CanHitHead.damage;
		BestHitpoint = CanHitHead.pos;
	}

	if (CanBaimKill.damage >= entity->m_iHealth())
	{
		FoundHitableEntity = true;
		BestDamage = CanBaimKill.damage;
		BestHitpoint = CanBaimKill.pos;
	}

	if (BestDamage >= entity->m_iHealth())
		WillKill = true;

	damage = BestDamage;
	hitbox = BestHitpoint;

	return FoundHitableEntity;
}

void Rbot::PrecacheShit() //need fix
{
	for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
	{
		auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

		if (!entity || !g_LocalPlayer || !entity->IsPlayer() || entity == g_LocalPlayer || entity->IsDormant()
			|| !entity->IsAlive() || !entity->IsEnemy())
			continue;


		model_t* pModel = entity->GetModel();

		if (!pModel)
		{
			matrixData[i].gotMatrix = false;
			continue;
		}

		matrixData[i].StudioHdr = g_MdlInfo->GetStudiomodel(pModel);

		if (!matrixData[i].StudioHdr)
		{
			matrixData[i].gotMatrix = false;
			continue;
		}

		matrixData[i].StudioSet = matrixData[i].StudioHdr->GetHitboxSet(0);

		if (!matrixData[i].StudioSet)
		{
			matrixData[i].gotMatrix = false;
			continue;
		}

		matrixData[i].gotMatrix = entity->SetupBones(matrixData[i].matrix, 128, 256, entity->m_flSimulationTime());

		//if (matrixData[i].gotMatrix) Console.WriteLine("GotMatrix");
	}
}

std::vector<PointScanStruct> Rbot::GetPointsForScan(C_BasePlayer* entity, int hitbox, mstudiohitboxset_t* hitset, matrix3x4_t matrix[MAXSTUDIOBONES], float pointscale)
{
	std::vector<PointScanStruct> pointsToScan;

	if (!matrix)
		return pointsToScan;

	if (!hitset)
		return pointsToScan;

	mstudiobbox_t* bbox = hitset->GetHitbox(hitbox);

	if (!bbox)
		return pointsToScan;

	float mod = bbox->m_flRadius != -1.f ? bbox->m_flRadius : 0.f;

	//Utils::ConsolePrint("mod r\n");

	Vector max;
	Vector min;

	Vector in1 = bbox->bbmax + mod;
	Vector in2 = bbox->bbmin - mod;


	Math::VectorTransform(in1, matrix[bbox->bone], max);
	Math::VectorTransform(in2, matrix[bbox->bone], min);


	Vector center = (min + max) * 0.5f;

	QAngle curAngles = Math::CalcAngle(center, g_LocalPlayer->GetEyePos());

	Vector forward;
	Math::AngleVectors(curAngles, forward);

	Vector right = forward.Cross(Vector(0, 0, 1));
	Vector left = Vector(-right.x, -right.y, right.z);

	Vector top = Vector(0, 0, 1);
	Vector bot = Vector(0, 0, -1);

	//Utils::ConsolePrint("t\n");

	//float POINT_SCALE = pointscale;//hitbox == HITBOX_HEAD ? g_Options.rbot_head_pointscale : g_Options.rbot_pointscale;

	if (pointscale == 0.f)
	{
		pointsToScan.emplace_back(PointScanStruct{ center, true });
		return pointsToScan;
	}

	if (hitbox == HITBOX_HEAD)
	{
		for (auto i = 0; i < 5; ++i)
		{
			pointsToScan.emplace_back(PointScanStruct{ center });
		}

		pointsToScan[1].pos += top * (bbox->m_flRadius * pointscale);
		pointsToScan[2].pos += right * (bbox->m_flRadius * pointscale);
		pointsToScan[3].pos += left * (bbox->m_flRadius * pointscale);
		pointsToScan[4].pos = center;
		pointsToScan[4].center = true;

		//pointsToScan[4] = center;
		//pointsToScan[4].center = true;
	}
	else
	{
		for (auto i = 0; i < 3; ++i)
		{
			pointsToScan.emplace_back(PointScanStruct{ center });
		}

		pointsToScan[0].pos += right * (bbox->m_flRadius * pointscale);
		pointsToScan[1].pos += left * (bbox->m_flRadius * pointscale);
		pointsToScan[2].pos = center;
		pointsToScan[2].center = true;
		//pointsToScan[2] = center;
		//pointsToScan[2].center = true;
	}

	//}
	//else
	//	pointsToScan.emplace_back(center);

	return pointsToScan;

	//return std::vector<Vector>();
}

void Rbot::UpdateConfigData()
{
	int activeweapon = Utils::pWeaponType();

	if (activeweapon == 10)
	{
		HitboxHead = false;

		HitboxHeadScale = 0;

		HitboxNeck = false;

		HitboxHeadScale = 0;

		HitboxChest = false;

		HitboxHeadScale = 0;

		HitboxPelvis = false;

		HitboxHeadScale = 0;

		HitboxStomach = false;

		HitboxHeadScale = 0;

		HitboxArm = false;

		HitboxHeadScale = 0;

		HitboxLeg = false;

		HitboxHeadScale = 0;

		HitboxFoot = false;

		HitboxHeadScale = 0;

		MinDmg = 0;

		HitChina = 0;

		BaimMax = 0;

		return;
	}

	HitboxHead = g_Config.rbot_hitbox_head[activeweapon];

	if (HitboxHead)
		HitboxHeadScale = g_Config.rbot_hitbox_head_scale[activeweapon];

	HitboxNeck = g_Config.rbot_hitbox_neck[activeweapon];

	if (HitboxNeck)
		HitboxHeadScale = g_Config.rbot_hitbox_neck_scale[activeweapon];

	HitboxChest = g_Config.rbot_hitbox_chest[activeweapon];

	if (HitboxChest)
		HitboxHeadScale = g_Config.rbot_hitbox_chest_scale[activeweapon];

	HitboxPelvis = g_Config.rbot_hitbox_pelvis[activeweapon];

	if (HitboxPelvis)
		HitboxHeadScale = g_Config.rbot_hitbox_pelvis_scale[activeweapon];

	HitboxStomach = g_Config.rbot_hitbox_stomach[activeweapon];

	if (HitboxStomach)
		HitboxHeadScale = g_Config.rbot_hitbox_stomach_scale[activeweapon];

	HitboxArm = g_Config.rbot_hitbox_arm[activeweapon];

	if (HitboxArm)
		HitboxHeadScale = g_Config.rbot_hitbox_arm_scale[activeweapon];

	HitboxLeg = g_Config.rbot_hitbox_leg[activeweapon];

	if (HitboxLeg)
		HitboxHeadScale = g_Config.rbot_hitbox_leg_scale[activeweapon];

	HitboxFoot = g_Config.rbot_hitbox_foot[activeweapon];

	if (HitboxFoot)
		HitboxHeadScale = g_Config.rbot_hitbox_foot_scale[activeweapon];

	MinDmg = g_Config.rbot_mindamage[activeweapon];

	HitChina = g_Config.rbot_min_hitchance[activeweapon];

	BaimMax = g_Config.rbot_baim_after_shots[activeweapon];
}

bool Rbot::Revolver(CUserCmd* cmd, C_BaseCombatWeapon* weapon)
{
	constexpr float REVOLVER_COCK_TIME = 0.2421875f;
	const int count_needed = floor(REVOLVER_COCK_TIME / g_GlobalVars->interval_per_tick);
	static int cocks_done = 0;

	if (!weapon ||
		weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER ||
		g_LocalPlayer->m_flNextAttack() > g_GlobalVars->curtime ||
		weapon->IsReloading())
	{
		if (weapon && weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER)
			cmd->buttons &= ~IN_ATTACK;
		cocks_done = 0;
		return true;
	}

	if (cocks_done < count_needed)
	{
		cmd->buttons |= IN_ATTACK;
		++cocks_done;
		return false;
	}
	else
	{
		cmd->buttons &= ~IN_ATTACK;
		cocks_done = 0;
		return true;
	}

	cmd->buttons |= IN_ATTACK;

	float curtime = TICKS_TO_TIME(g_LocalPlayer->m_nTickBase());
	static float next_shoot_time = 0.f;

	bool ret = false;

	if (next_shoot_time - curtime < -0.5f)
		next_shoot_time = curtime + 0.2f - g_GlobalVars->interval_per_tick;

	if (next_shoot_time - curtime - g_GlobalVars->interval_per_tick <= 0.f) {
		next_shoot_time = curtime + 0.2f;
		ret = true;
	}

	return ret;
}

//blya face vot ti fleksish!

float Rbot::get_MinDmg() {
	return this->MinDmg;
}

float Rbot::get_HitChina() {
	return this->HitChina;
}

int Rbot::get_BaimMax() {
	return this->BaimMax;
}