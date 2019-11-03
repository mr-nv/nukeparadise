#include "lAutowall.h"
#include "Math.h"
#include "../valve_sdk/interfaces/IEngineTrace.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_GEAR 10
float lAutowall::GetHitgroupDamageMultiplier(int iHitGroup)
{
	switch (iHitGroup)
	{
	case HITGROUP_GENERIC:
		return 0.5f;
	case HITGROUP_HEAD:
		return 2.0f;
	case HITGROUP_CHEST:
		return 0.5f;
	case HITGROUP_STOMACH:
		return 0.75f;
	case HITGROUP_LEFTARM:
		return 0.5f;
	case HITGROUP_RIGHTARM:
		return 0.5f;
	case HITGROUP_LEFTLEG:
		return 0.375f;
	case HITGROUP_RIGHTLEG:
		return 0.375f;
	case HITGROUP_GEAR:
		return 0.5f;
	default:
		return 1.0f;
	}
	return 1.0f;
}
void lAutowall::ScaleDamage(int hitbox, C_BasePlayer* enemy, float weapon_armor_ratio, float& current_damage)
{
	current_damage *= lAutowall::GetHitgroupDamageMultiplier(hitbox);
	if (enemy->m_ArmorValue() > 0)
	{
		if (hitbox == 8)
		{
			if (enemy->m_bHasHelmet())
				current_damage *= weapon_armor_ratio * 0.5f;
		}
		else
			current_damage *= weapon_armor_ratio * 0.5f;
	}
}
bool lAutowall::TraceToExit(Vector& end, trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace)
{
	float distance = 0.0f;
	while (distance <= 90.0f)
	{
		distance += 4.0f;
		end = start + dir * distance;
		auto point_contents = g_EngineTrace->GetPointContents(end, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL);
		if (point_contents & MASK_SHOT_HULL && !(point_contents & CONTENTS_HITBOX))
			continue;
		auto new_end = end - (dir * 4.0f);
		Ray_t ray;
		ray.Init(end, new_end);
		g_EngineTrace->TraceRay(ray, MASK_SHOT, 0, exit_trace);
		if (exit_trace->startsolid && exit_trace->surface.flags & SURF_HITBOX)
		{
			ray.Init(end, start);
			CTraceFilter filter;
			filter.pSkip = exit_trace->hit_entity;
			g_EngineTrace->TraceRay(ray, 0x600400B, &filter, exit_trace);
			if ((exit_trace->fraction < 1.0f || exit_trace->allsolid) && !exit_trace->startsolid)
			{
				end = exit_trace->endpos;
				return true;
			}
			continue;
		}
		if (!(exit_trace->fraction < 1.0 || exit_trace->allsolid || exit_trace->startsolid) || exit_trace->startsolid)
		{
			if (exit_trace->hit_entity)
			{
				if (enter_trace->hit_entity && enter_trace->hit_entity == g_EntityList->GetClientEntity(0))
					return true;
			}
			continue;
		}
		if (exit_trace->surface.flags >> 7 & 1 && !(enter_trace->surface.flags >> 7 & 1))
			continue;
		if (exit_trace->plane.normal.Dot(dir) <= 1.0f)
		{
			auto fraction = exit_trace->fraction * 4.0f;
			end = end - (dir * fraction);
			return true;
		}
	}
	return false;
}
bool lAutowall::HandleBulletPenetration(CCSWeaponInfo* weaponInfo, FireBulletData& data)
{
	surfacedata_t* enter_surface_data = g_PhysSurface->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;
	float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;
	data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
	data.current_damage *= powf(weaponInfo->flRangeModifier, data.trace_length * 0.002f);
	if (data.trace_length > 3000.f || enter_surf_penetration_mod < 0.1f)
		data.penetrate_count = 0;
	if (data.penetrate_count <= 0)
		return false;
	Vector dummy;
	trace_t trace_exit;
	if (!TraceToExit(dummy, &data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit))
		return false;
	surfacedata_t* exit_surface_data = g_PhysSurface->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_mod = *(float*)((uint8_t*)exit_surface_data + 76);
	float final_damage_modifier = 0.16f;
	float combined_penetration_modifier = 0.0f;
	if ((data.enter_trace.contents & CONTENTS_GRATE) != 0 || enter_material == 89 || enter_material == 71)
	{
		combined_penetration_modifier = 3.0f;
		final_damage_modifier = 0.05f;
	}
	else
		combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;
	if (enter_material == exit_material)
	{
		if (exit_material == 87 || exit_material == 85)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == 76)
			combined_penetration_modifier = 2.0f;
	}
	float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
	float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / weaponInfo->flPenetration) * 1.25f);
	float thickness = (trace_exit.endpos - data.enter_trace.endpos).Length();
	thickness *= thickness;
	thickness *= v34;
	thickness /= 24.0f;
	float lost_damage = fmaxf(0.0f, v35 + thickness);
	if (lost_damage > data.current_damage)
		return false;
	if (lost_damage >= 0.0f)
		data.current_damage -= lost_damage;
	if (data.current_damage < 1.0f)
		return false;
	data.src = trace_exit.endpos;
	data.penetrate_count--;
	return true;
}
void TraceLine(Vector vecAbsStart, Vector vecAbsEnd, unsigned int mask, C_BasePlayer* ignore, trace_t* ptr)
{
	Ray_t ray;
	ray.Init(vecAbsStart, vecAbsEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;
	g_EngineTrace->TraceRay(ray, mask, &filter, ptr);
}
bool lAutowall::SimulateFireBullet(C_BaseCombatWeapon* pWeapon, FireBulletData& data)
{
	CCSWeaponInfo* weaponInfo = pWeapon->GetCSWeaponData();
	data.penetrate_count = 4;
	data.trace_length = 0.0f;
	data.current_damage = (float)weaponInfo->iDamage;
	while (data.penetrate_count > 0 && data.current_damage >= 1.0f)
	{
		data.trace_length_remaining = weaponInfo->flRange - data.trace_length;
		Vector end = data.src + data.direction * data.trace_length_remaining;
		// data.enter_trace
		TraceLine(data.src, end, MASK_SHOT, g_LocalPlayer, &data.enter_trace);
		Ray_t ray;
		ray.Init(data.src, end + data.direction * 40.f);
		g_EngineTrace->TraceRay(ray, MASK_SHOT, &data.filter, &data.enter_trace);
		TraceLine(data.src, end + data.direction * 40.f, MASK_SHOT, g_LocalPlayer, &data.enter_trace);
		if (data.enter_trace.fraction == 1.0f)
			break;
		if (data.enter_trace.hitgroup <= HITGROUP_RIGHTLEG && data.enter_trace.hitgroup > HITGROUP_GENERIC)
		{
			data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
			data.current_damage *= powf(weaponInfo->flRangeModifier, data.trace_length * 0.002f);
			C_BasePlayer* player = (C_BasePlayer*)data.enter_trace.hit_entity;
			lAutowall::ScaleDamage(data.enter_trace.hitgroup, player, weaponInfo->flArmorRatio, data.current_damage);
			return true;
		}
		if (!HandleBulletPenetration(weaponInfo, data))
			break;
	}
	return false;
}
float lAutowall::GetDamage(const Vector& point)
{
	float damage = 0.f;
	Vector dst = point;
	FireBulletData data;
	data.src = g_LocalPlayer->GetEyePos();
	data.filter.pSkip = g_LocalPlayer;
	data.direction = dst - data.src;
	data.direction.NormalizeInPlace();
	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*)g_EntityList->GetClientEntityFromHandle(g_LocalPlayer->m_hActiveWeapon());
	if (!activeWeapon)
		return -1.0f;
	if (SimulateFireBullet(activeWeapon, data))
		damage = data.current_damage;
	return damage;
}