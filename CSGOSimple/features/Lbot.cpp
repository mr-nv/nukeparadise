#include "Lbot.hpp"
#include "lAutowall.h"
#include "../helpers/math.hpp"
#include "../valve_sdk/misc/Studio.hpp"
#include "Backtrack.h"
#include "../helpers/input.hpp"

#define HITGROUP_GENERIC    0
#define HITGROUP_HEAD        1
#define HITGROUP_CHEST        2
#define HITGROUP_STOMACH    3
#define HITGROUP_LEFTARM    4
#define HITGROUP_RIGHTARM    5
#define HITGROUP_LEFTLEG    6
#define HITGROUP_RIGHTLEG    7
#define HITGROUP_GEAR        10

float RandomFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

bool C_BasePlayer::IsNotTarget()
{
	return !this || !IsPlayer() || this == g_LocalPlayer || m_iHealth() <= 0 || !IsAlive() || m_bGunGameImmunity() || (m_fFlags() & FL_FROZEN) || GetClientClass()->m_ClassID != CCSPlayer;
}
//--------------------------------------------------------------------------------
bool Aimbot::IsRcs()
{
	return g_LocalPlayer->m_iShotsFired() >= settings.rcs_start;
}
//--------------------------------------------------------------------------------
float GetRealDistanceFOV(float distance, QAngle angle, CUserCmd* cmd)
{
	Vector aimingAt;
	Math::AngleVectors(cmd->viewangles, aimingAt);
	aimingAt *= distance;
	Vector aimAt;
	Math::AngleVectors(angle, aimAt);
	aimAt *= distance;
	return aimingAt.DistTo(aimAt) / 5;
}
//--------------------------------------------------------------------------------
float Aimbot::GetFovToPlayer(QAngle viewAngle, QAngle aimAngle)
{
	QAngle delta = aimAngle - viewAngle;
	Math::FixAngles(delta);
	return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
}
//--------------------------------------------------------------------------------
bool Aimbot::IsLineGoesThroughSmoke(Vector vStartPos, Vector vEndPos)
{
	static auto LineGoesThroughSmokeFn = (bool(*)(Vector vStartPos, Vector vEndPos))Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
	return LineGoesThroughSmokeFn(vStartPos, vEndPos);
}
//--------------------------------------------------------------------------------
bool Aimbot::IsEnabled(CUserCmd* pCmd)
{
	if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) {
		return false;
	}
	auto pWeapon = g_LocalPlayer->m_hActiveWeapon();
	if (!pWeapon || !(pWeapon->IsSniper() || pWeapon->IsPistol() || pWeapon->IsRifle())) {
		return false;
	}
	auto weaponData = pWeapon->GetCSWeaponData();
	auto weapontype = weaponData->WeaponType;
	settings = g_Config.lbot_items[pWeapon->m_Item().m_iItemDefinitionIndex()];
	if (settings.check_jump && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND)) {
		return false;
	}

	if ((pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08) && settings.only_in_zoom && !g_LocalPlayer->m_bIsScoped()) {
		return false;
	}
	if (settings.fov == 0 && settings.silent_fov == 0 && !settings.rcs) {
		return false;
	}
	if (!pWeapon->HasBullets() || pWeapon->IsReloading()) {
		return false;
	}
	if (pWeapon->IsWeaponSwitching())
		return false;

	if (settings.enabled) {
		if (pWeapon->CanFire() && settings.autofire && (InputSys::Get().IsKeyDown(g_Config.lbot_auto_fire_key)))
		{
			return true;
		}
		else
		{
			if (!(pCmd->buttons & IN_ATTACK))
				return false;
		}
	}
	else
		return false;

	if (settings.on_key) {
		if (InputSys::Get().IsKeyDown(g_Config.lbot_on_fire_key))
			return true;
		else
			return false;
	}
	else
		return true;
}
//--------------------------------------------------------------------------------
float Aimbot::GetSmooth()
{
	float smooth = IsRcs() && settings.rcs_smooth_enabled ? settings.rcs_smooth : settings.smooth;
	if (settings.humanize) {
		smooth += RandomFloat(-1, 4);
	}
	return smooth;
}
//--------------------------------------------------------------------------------
void Aimbot::Smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle)
{
	if (GetSmooth() <= 1) {
		return;
	}
	Vector vAimAngle;
	Math::AngleVectors(aimAngle, vAimAngle);
	Vector vCurrentAngle;
	Math::AngleVectors(currentAngle, vCurrentAngle);
	Vector delta = vAimAngle - vCurrentAngle;
	Vector smoothed = vCurrentAngle + delta / GetSmooth();
	Math::VectorAngles(smoothed, angle);
}
//--------------------------------------------------------------------------------
void Aimbot::RCS(QAngle& angle, C_BasePlayer* target)
{
	if (!settings.rcs || !IsRcs()) {
		return;
	}
	if (settings.rcs_x == 0 && settings.rcs_y == 0) {
		return;
	}
	if (target) {
		QAngle punch = g_LocalPlayer->m_aimPunchAngle();
		angle.pitch -= punch.pitch * (settings.rcs_x / 50.f);
		angle.yaw -= punch.yaw * (settings.rcs_y / 50.f);
	}
	else if (settings.rcs_type == 0) {
		QAngle NewPunch = { CurrentPunch.pitch - RCSLastPunch.pitch, CurrentPunch.yaw - RCSLastPunch.yaw, 0 };
		angle.pitch -= NewPunch.pitch * (settings.rcs_x / 50.f);
		angle.yaw -= NewPunch.yaw * (settings.rcs_y / 50.f);
	}
	Math::FixAngles(angle);
}
//--------------------------------------------------------------------------------
float Aimbot::GetFov()
{
	if (IsRcs() && settings.rcs && settings.rcs_fov_enabled) return settings.rcs_fov;
	if (!silent_enabled) return settings.fov;
	return settings.silent_fov > settings.fov ? settings.silent_fov : settings.fov;
}
//--------------------------------------------------------------------------------
C_BasePlayer* Aimbot::GetClosestPlayer(CUserCmd* cmd, int& bestBone)
{
	QAngle ang;
	Vector eVecTarget;
	Vector pVecTarget = g_LocalPlayer->GetEyePos();
	if (target && !kill_delay && settings.kill_delay > 0 && target->IsNotTarget()) {
		target = NULL;
		shot_delay = false;
		kill_delay = true;
		kill_delay_time = (int)GetTickCount() + settings.kill_delay;
	}
	if (kill_delay) {
		if (kill_delay_time <= (int)GetTickCount()) kill_delay = false;
		else return NULL;
	}
	C_BasePlayer* player;
	target = NULL;
	int bestHealth = 100.f;
	float bestFov = 9999.f;
	float bestDamage = 0.f;
	float bestBoneFov = 9999.f;
	float bestDistance = 9999.f;
	int health;
	float fov;
	float damage;
	float distance;
	int fromBone = settings.aim_type == 1 ? 0 : settings.hitbox;
	int toBone = settings.aim_type == 1 ? 7 : settings.hitbox;
	for (int i = 1; i < g_EngineClient->GetMaxClients(); ++i)
	{
		damage = 0.f;
		player = (C_BasePlayer*)g_EntityList->GetClientEntity(i);
		if (player->IsNotTarget()) {
			continue;
		}
		if (!settings.deathmatch && player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()) {
			continue;
		}
		for (int bone = fromBone; bone <= toBone; bone++) {
			eVecTarget = player->GetHitboxPos(bone);
			Math::VectorAngles(eVecTarget - pVecTarget, ang);
			Math::FixAngles(ang);
			distance = pVecTarget.DistTo(eVecTarget);
			if (settings.fov_type == 1)
				fov = GetRealDistanceFOV(distance, ang, cmd);
			else
				fov = GetFovToPlayer(cmd->viewangles, ang);
			if (fov > GetFov()) {
				continue;
			}
			if (!g_LocalPlayer->CanSeePlayer(player, eVecTarget)) {
				if (!settings.autowall) {
					continue;
				}
				damage = lAutowall::GetDamage(eVecTarget);
				if (damage < settings.min_damage) {
					continue;
				}
			}
			if ((settings.priority == 1 || settings.priority == 2) && damage == 0.f) {
				damage = lAutowall::GetDamage(eVecTarget);
			}
			health = player->m_iHealth() - damage;
			if (settings.check_smoke && IsLineGoesThroughSmoke(pVecTarget, eVecTarget)) {
				continue;
			}
			if (settings.aim_type == 1 && bestBoneFov < fov) {
				continue;
			}
			bestBoneFov = fov;
			if (
				(settings.priority == 0 && bestFov > fov) ||
				(settings.priority == 1 && bestHealth > health) ||
				(settings.priority == 2 && bestDamage < damage) ||
				(settings.priority == 3 && distance < bestDistance)
				) {
				bestBone = bone;
				target = player;
				bestFov = fov;
				bestHealth = health;
				bestDamage = damage;
				bestDistance = distance;
			}
		}
	}
	return target;
}
//--------------------------------------------------------------------------------
bool Aimbot::IsNotSilent(float fov)
{
	return IsRcs() || !silent_enabled || (silent_enabled && fov > settings.silent_fov);
}
//--------------------------------------------------------------------------------
void Aimbot::OnMove(CUserCmd* pCmd)
{
	if (!IsEnabled(pCmd)) {
		RCSLastPunch = { 0, 0, 0 };
		is_delayed = false;
		shot_delay = false;
		kill_delay = false;
		silent_enabled = settings.silent && settings.silent_fov > 0;
		target = NULL;
		return;
	}
	auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
	if (!weapon)
		return;

	auto weapon_data = weapon->GetCSWeaponData();
	if (!weapon_data)
		return;
	QAngle angles = pCmd->viewangles;
	QAngle current = angles;
	float fov = 180.f;
	if (!(settings.check_flash && g_LocalPlayer->IsFlashed())) {
		int bestBone = -1;
		if (GetClosestPlayer(pCmd, bestBone)) {
			Math::VectorAngles(target->GetHitboxPos(bestBone) - g_LocalPlayer->GetEyePos(), angles);
			Math::FixAngles(angles);
			if (settings.fov_type == 1)
				fov = GetRealDistanceFOV(g_LocalPlayer->GetEyePos().DistTo(target->GetHitboxPos(bestBone)), angles, pCmd);
			else
				fov = GetFovToPlayer(pCmd->viewangles, angles);
			if (!settings.silent && !is_delayed && !shot_delay && settings.shot_delay > 0) {
				is_delayed = true;
				shot_delay = true;
				shot_delay_time = GetTickCount() + settings.shot_delay;
			}
			if (shot_delay && shot_delay_time <= GetTickCount()) {
				shot_delay = false;
			}
			if (shot_delay) {
				pCmd->buttons &= ~IN_ATTACK;
			}
			if (settings.autostop) {
				pCmd->forwardmove = pCmd->sidemove = 0;
			}
		}
	}
	CurrentPunch = g_LocalPlayer->m_aimPunchAngle();

	if (IsNotSilent(fov)) {
		RCS(angles, target);
	}
	RCSLastPunch = CurrentPunch;
	if (target && IsNotSilent(fov)) {
		Smooth(current, angles, angles);
	}
	Math::FixAngles(angles);
	pCmd->viewangles = angles;
	if (IsNotSilent(fov)) {
		g_EngineClient->SetViewAngles(angles);
	}
	silent_enabled = false;

	if (settings.autofire && weapon->CanFire()) { //glad
		//if (!g_Config.lbot_auto_fire_key || InputSys::Get().IsKeyDown(g_Config.lbot_auto_fire_key)) {

			Vector rem, forward,
				src = g_LocalPlayer->GetEyePos();

			trace_t tr;
			Ray_t ray;
			CTraceFilter filter;
			filter.pSkip = g_LocalPlayer;

			QAngle viewangles = pCmd->viewangles;

			viewangles += g_LocalPlayer->m_aimPunchAngle() * 2.f;

			Math::AngleVectors(viewangles, forward);

			forward *= g_LocalPlayer->m_hActiveWeapon().Get()->GetCSWeaponData()->flRange;

			rem = src + forward;

			ray.Init(src, rem);
			g_EngineTrace->TraceRay(ray, 0x46004003, &filter, &tr);

			if (!tr.hit_entity)
				return;

			bool dh = false;

			if (tr.hitgroup == HITGROUP_HEAD || tr.hitgroup == HITGROUP_CHEST || tr.hitgroup == HITGROUP_STOMACH || (tr.hitgroup == HITGROUP_LEFTARM || tr.hitgroup == HITGROUP_RIGHTARM) || (tr.hitgroup == HITGROUP_LEFTLEG || tr.hitgroup == HITGROUP_RIGHTLEG))
				dh = true;

			auto player = reinterpret_cast<C_BasePlayer*>(tr.hit_entity);
			if (player && !player->IsDormant() && !player->m_bGunGameImmunity() && player->m_iHealth() > 0 && player->IsPlayer())
			{
				if (player->m_iTeamNum() != g_LocalPlayer->m_iTeamNum())
				{
					if (g_Config.lbot_auto_fire_key) //o_0
					{
						if (dh && (!(pCmd->buttons & IN_ATTACK) && (!g_Config.lbot_auto_fire_key || InputSys::Get().IsKeyDown(g_Config.lbot_auto_fire_key)))) // if you don't attack currently
							pCmd->buttons |= IN_ATTACK;

						static bool already_shot = false;
						if (g_LocalPlayer->m_hActiveWeapon().Get()->IsPistol())
						{
							if (pCmd->buttons & IN_ATTACK)
								if (already_shot)
									pCmd->buttons &= ~IN_ATTACK;

							already_shot = pCmd->buttons & IN_ATTACK ? true : false;
						}
					}
					else
					{
						if (dh && (!(pCmd->buttons & IN_ATTACK) && (!(pCmd->buttons & IN_ATTACK2)))) // if you don't attack currently
							pCmd->buttons |= IN_ATTACK;

						static bool already_shot = false;
						if (g_LocalPlayer->m_hActiveWeapon().Get()->IsPistol())
						{
							if (pCmd->buttons & IN_ATTACK)
								if (already_shot)
									pCmd->buttons &= ~IN_ATTACK;

							already_shot = pCmd->buttons & IN_ATTACK ? true : false;
						}
					}
				}
			}

		//}
	}
	/*
	if (!weapon_data->bFullAuto) {
		if (pCmd->command_number % 2 == 0) {
			pCmd->buttons &= ~IN_ATTACK;
		}
		else {
			pCmd->buttons |= IN_ATTACK;
		}
	}
	else {
		pCmd->buttons |= IN_ATTACK;
	}
	*/
	if (g_LocalPlayer->m_hActiveWeapon()->IsPistol() && settings.autopistol) {
		float server_time = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
		float next_shot = g_LocalPlayer->m_hActiveWeapon()->m_flNextPrimaryAttack() - server_time;
		if (next_shot > 0) {
			pCmd->buttons &= ~IN_ATTACK;
		}
	}
}
Aimbot g_Lbot;