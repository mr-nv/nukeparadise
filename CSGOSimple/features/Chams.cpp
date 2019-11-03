#include "lBacktrack.h"
#include "Lbot.hpp"

#include "Chams.h"
#include "../RuntimeSaver.h"
#include "AntiAim.h"
#include "../hooks.hpp"
#include "../valve_sdk/sdk.hpp"
#include "../options.hpp"

/*
bool is_grenade_smoke = strstr(mdl->szName, "models/weapons/w_eq_smokegrenade_thrown.mdl") != nullptr;
bool is_grenade_flash = strstr(mdl->szName, "models/weapons/w_eq_flashbang_dropped.mdl") != nullptr;
bool is_grenade_grenade = strstr(mdl->szName, "models/weapons/w_eq_fraggrenade_dropped.mdl") != nullptr;
bool is_grenade_molotov = strstr(mdl->szName, "models/weapons/w_eq_molotov_dropped.mdl") != nullptr;
bool is_grenade_inc = strstr(mdl->szName, "models/weapons/w_eq_incendiarygrenade_dropped.mdl") != nullptr;
bool is_grenade_fire = (is_grenade_molotov || is_grenade_inc);
bool is_grenade_decoy = strstr(mdl->szName, "models/weapons/w_eq_decoy_dropped.mdl") != nullptr;
*/

//bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;

void Chams::OnDrawModelExecute(void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld,
	float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int iFlags)
{
	static auto fnDME = Hooks::mdlrender_hook.get_original<Hooks::DrawModelExecute>(index::DrawModelExecute);

	if (!pInfo->m_pClientEntity || !g_LocalPlayer || g_Unload)
		return;

	const auto mdl = pInfo->m_pClientEntity->GetModel();

	bool chamsEnabled = g_Config.chams_local || g_Config.chams_backtrack || g_Config.chams_enemy || g_Config.chams_team;

	bool is_arm = strstr(mdl->szName, "models/weapons/v_models/arms") != nullptr;
	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	bool is_weapon = strstr(mdl->szName, "weapons/v_") != nullptr;

	if (chamsEnabled && is_player)
	{
		auto ent = (C_BasePlayer*)(pInfo->m_pClientEntity->GetIClientUnknown()->GetBaseEntity());

		if (ent == g_LocalPlayer && g_Config.chams_local) {

			const auto clr_front = Color(g_Config.color_chams_local_visible);
			const auto clr_back = Color(g_Config.color_chams_local_invisible);
			const auto flat = g_Config.chams_mode_local == 1 || g_Config.chams_mode_local == 8;
			const auto metallic = g_Config.chams_mode_local == 4 || g_Config.chams_mode_local == 7;
			const auto glow = g_Config.chams_mode_local == 5 || g_Config.chams_mode_local == 9;
			const auto wireframe = g_Config.chams_mode_local == 2;
			const auto glass = g_Config.chams_mode_local == 3;
			bool ignoreZ = g_Config.chams_mode_local >= 6;

			if (ignoreZ)
			{
				MaterialManager::Get().OverrideMaterial(true, flat, wireframe, false, metallic, glow, clr_back);
				fnDME(g_StudioRender, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

				MaterialManager::Get().OverrideMaterial(false, flat, wireframe, false, metallic, glow, clr_front);
			}
			else
			{
				if (g_Config.chams_local_scope && g_LocalPlayer->m_bIsScoped())
				{
					const auto clr_front = Color(g_Config.color_chams_scoped_visible);
					MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
				}
				else
				{
					MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
				}

			}
		}

		if (g_Config.chams_backtrack && g_Lbot.settings.enabled && g_Lbot.settings.backtrack.enabled && g_lBacktrack.data.count(ent->EntIndex()) > 0) {
			auto& data = g_lBacktrack.data.at(ent->EntIndex());
			int validRecordsCount = 0;

			if (data.size() > 0) {

				const auto clr_front = Color(g_Config.color_chams_backtrack_visible);
				const auto clr_back = Color(g_Config.color_chams_backtrack_invisible);
				const auto flat = g_Config.chams_mode_backtrack == 1 || g_Config.chams_mode_backtrack == 8;
				const auto metallic = g_Config.chams_mode_backtrack == 4 || g_Config.chams_mode_backtrack == 7;
				const auto glow = g_Config.chams_mode_backtrack == 5 || g_Config.chams_mode_backtrack == 9;
				const auto wireframe = g_Config.chams_mode_backtrack == 2;
				const auto glass = g_Config.chams_mode_backtrack == 3;
				bool ignoreZ = g_Config.chams_mode_backtrack >= 6;

				if (g_Config.chams_type_backtrack == 1) {
					for (auto& record : data) {
						if (ignoreZ)
						{
							MaterialManager::Get().OverrideMaterial(true, flat, wireframe, false, metallic, glow, clr_back);


							MaterialManager::Get().OverrideMaterial(false, flat, wireframe, false, metallic, glow, clr_front);
							fnDME(g_StudioRender, pResults, pInfo, record.boneMatrix, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);
						}
						else
						{
							MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
							fnDME(g_StudioRender, pResults, pInfo, record.boneMatrix, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);
						}
					}
				}
				else if (g_Config.chams_type_backtrack == 0) {
					auto& back = data.back();
					if (ignoreZ)
					{
						MaterialManager::Get().OverrideMaterial(true, flat, wireframe, false, metallic, glow, clr_back);


						MaterialManager::Get().OverrideMaterial(false, flat, wireframe, false, metallic, glow, clr_front);
						fnDME(g_StudioRender, pResults, pInfo, back.boneMatrix, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);
					}
					else
					{
						MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
						fnDME(g_StudioRender, pResults, pInfo, back.boneMatrix, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);
					}
				}
			}
		}

		// Other Chams
		if (ent && ent->IsPlayer() && ent != g_LocalPlayer && ent->IsAlive())
		{
			const auto enemy = ent->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();

			if (!enemy && !g_Config.chams_team)
				return;

			const auto clr_front = enemy ? Color(g_Config.color_chams_enemy_visible) : Color(g_Config.color_chams_team_visible);
			const auto clr_back = enemy ? Color(g_Config.color_chams_enemy_invisible) : Color(g_Config.color_chams_team_invisible);
			const auto flat = enemy ? (g_Config.chams_mode_enemy == 1 || g_Config.chams_mode_enemy == 8)
				: (g_Config.chams_mode_team == 1 || g_Config.chams_mode_team == 7);

			const auto glow = enemy ? (g_Config.chams_mode_enemy == 5 || g_Config.chams_mode_enemy == 9)
				: (g_Config.chams_mode_team == 5 || g_Config.chams_mode_team == 8);

			const auto metallic = enemy ? (g_Config.chams_mode_enemy == 4 || g_Config.chams_mode_enemy == 7)
				: (g_Config.chams_mode_team == 4 || g_Config.chams_mode_team == 6);
			const auto wireframe = enemy ? (g_Config.chams_mode_enemy == 2)
				: (g_Config.chams_mode_team == 2);
			const auto glass = enemy ? (g_Config.chams_mode_enemy == 3)
				: (g_Config.chams_mode_team == 3);
			bool ignoreZ = g_Config.chams_mode_enemy >= 6 || g_Config.chams_mode_team >= 6;

			if (ignoreZ)
			{
				MaterialManager::Get().OverrideMaterial(true, flat, wireframe, false, metallic, glow, clr_back);
				fnDME(g_StudioRender, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

				MaterialManager::Get().OverrideMaterial(false, flat, wireframe, false, metallic, glow, clr_front);
			}
			else
			{
				MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
			}
		}
	}
	else if ((g_Config.chams_local_arms || g_Config.misc_no_hands) && is_arm) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;

		if (g_LocalPlayer->m_bIsScoped()) return;

		if (g_Config.misc_no_hands) {
			material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			g_MdlRender->ForcedMaterialOverride(material);
		}

		else if (g_Config.chams_local_arms) {

			const auto clr_front = Color(g_Config.color_chams_local_arms_visible);
			const auto clr_back = Color(g_Config.color_chams_local_arms_invisible);
			const auto flat = g_Config.chams_mode_local_arms == 1 || g_Config.chams_mode_local_arms == 8;
			const auto metallic = g_Config.chams_mode_local_arms == 4 || g_Config.chams_mode_local_arms == 7;
			const auto glow = g_Config.chams_mode_local_weapon == 5 || g_Config.chams_mode_local_weapon == 9;
			const auto wireframe = g_Config.chams_mode_local_arms == 2;
			const auto glass = g_Config.chams_mode_local_arms == 3;
			bool ignoreZ = g_Config.chams_mode_local_arms >= 6;

			if (ignoreZ)
			{
				MaterialManager::Get().OverrideMaterial(true, flat, wireframe, false, metallic, glow, clr_back);
				fnDME(g_StudioRender, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

				MaterialManager::Get().OverrideMaterial(false, flat, wireframe, false, metallic, glow, clr_front);
			}
			else
			{
				MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
			}

		}
	}
	else if (g_Config.chams_local_weapon && is_weapon) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;

		if (g_LocalPlayer->m_bIsScoped()) return;

		const auto clr_front = Color(g_Config.color_chams_local_weapon_visible);
		const auto clr_back = Color(g_Config.color_chams_local_weapon_invisible);
		const auto flat = g_Config.chams_mode_local_weapon == 1 || g_Config.chams_mode_local_weapon == 8;
		const auto metallic = g_Config.chams_mode_local_weapon == 4 || g_Config.chams_mode_local_weapon == 7;
		const auto glow = g_Config.chams_mode_local_weapon == 5 || g_Config.chams_mode_local_weapon == 9;
		const auto wireframe = g_Config.chams_mode_local_weapon == 2;
		const auto glass = g_Config.chams_mode_local_weapon == 3;
		bool ignoreZ = g_Config.chams_mode_local_weapon >= 6;

		if (ignoreZ)
		{
			MaterialManager::Get().OverrideMaterial(true, flat, wireframe, false, metallic, glow, clr_back);
			fnDME(g_StudioRender, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

			MaterialManager::Get().OverrideMaterial(false, flat, wireframe, false, metallic, glow, clr_front);
		}
		else
		{
			MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
		}

	}
}

FORCEINLINE float DotProduct(const float* v1, const float* v2) {
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void Chams::zzVectorRotate(const float* in1, const matrix3x4_t& in2, float* out)
{
	out[0] = DotProduct(in1, in2[0]);
	out[1] = DotProduct(in1, in2[1]);
	out[2] = DotProduct(in1, in2[2]);
}

void Chams::zVectorRotate(const Vector& in1, const matrix3x4_t& in2, Vector& out)
{
	zzVectorRotate(&in1.x, in2, &out.x);
}

void Chams::VectorRotate(const Vector& in1, const QAngle& in2, Vector& out)
{
	matrix3x4_t matRotate;
	zAngleMatrix(Vector(in2.pitch, in2.yaw, in2.roll), matRotate);
	zVectorRotate(in1, matRotate, out);
}

void Chams::MatrixCopy(const matrix3x4_t& source, matrix3x4_t& target)
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 4; j++) {
			target[i][j] = source[i][j];
		}
	}
}

void Chams::MatrixMultiply(matrix3x4_t& in1, const matrix3x4_t& in2)
{
	matrix3x4_t out;
	if (&in1 == &out)
	{
		matrix3x4_t in1b;
		MatrixCopy(in1, in1b);
		MatrixMultiply(in1b, in2);
		return;
	}
	if (&in2 == &out)
	{
		matrix3x4_t in2b;
		MatrixCopy(in2, in2b);
		MatrixMultiply(in1, in2b);
		return;
	}
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
		in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
		in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
		in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
		in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
		in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
		in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
		in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
		in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
		in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
		in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
		in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
		in1[2][2] * in2[2][3] + in1[2][3];

	in1 = out;
}

void Chams::zAngleMatrix(const Vector angles, matrix3x4_t& matrix)
{
	float sr, sp, sy, cr, cp, cy;

	sy = sin(DEG2RAD(angles[1]));
	cy = cos(DEG2RAD(angles[1]));

	sp = sin(DEG2RAD(angles[0]));
	cp = cos(DEG2RAD(angles[0]));

	sr = sin(DEG2RAD(angles[2]));
	cr = cos(DEG2RAD(angles[2]));

	//matrix = (YAW * PITCH) * ROLL
	matrix[0][0] = cp * cy;
	matrix[1][0] = cp * sy;
	matrix[2][0] = -sp;

	float crcy = cr * cy;
	float crsy = cr * sy;
	float srcy = sr * cy;
	float srsy = sr * sy;

	matrix[0][1] = sp * srcy - crsy;
	matrix[1][1] = sp * srsy + crcy;
	matrix[2][1] = sr * cp;

	matrix[0][2] = (sp * crcy + srsy);
	matrix[1][2] = (sp * crsy - srcy);
	matrix[2][2] = cr * cp;

	matrix[0][3] = 0.0f;
	matrix[1][3] = 0.0f;
	matrix[2][3] = 0.0f;
}