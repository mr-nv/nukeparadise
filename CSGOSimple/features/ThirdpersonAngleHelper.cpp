
#include "ThirdpersonAngleHelper.h"
#include "../RuntimeSaver.h"
#include "../ConsoleHelper.h"
#include "Resolver.h"
#include "../config.hpp"
#include  "../helpers/math.hpp"

#include "backtrack.h"
#include "resolver.h"

#define TICK_INTERVAL			(g_GlobalVars->interval_per_tick)
#define TICKS_TO_TIME(t) (g_GlobalVars->interval_per_tick * (t) )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

void ThirdpersonAngleHelper::SetThirdpersonAngle()
{
    if ( !g_LocalPlayer || !g_LocalPlayer->IsAlive() )
        return;

    g_LocalPlayer->SetVAngles ( g_Saver.FakelagData.ang );
    LastAngle = g_Saver.FakelagData.ang;

    g_Saver.HasChokedLast = g_ClientState->chokedcommands >= 1;
    g_Saver.LastRealAnglesBChoke = g_Saver.AARealAngle;
}

void ThirdpersonAngleHelper::EnemyAnimationFix ( C_BasePlayer* player )
{
	if (!g_LocalPlayer)
		return;

	if (!player || player == nullptr)
		return;

	if (player == g_LocalPlayer)
		return;

	if (player->TeamMate())
		return;

	if (player->IsDormant())
		return;

	if (player->m_bGunGameImmunity())
		return;

	if (!player->IsAlive())
		return;

    AnimFix2 ( player ); //xD
}

void ThirdpersonAngleHelper::update_animations(C_BasePlayer* entity)
{

}

void ThirdpersonAngleHelper::AnimFix()
{//hot
	
		if (g_EngineClient->IsConnected() && g_EngineClient->IsInGame())
		{
			auto animations = g_LocalPlayer->GetPlayerAnimState();

			if (!animations)
				return;
			if (!g_LocalPlayer)
				return;

					g_LocalPlayer->client_side_animation() = true;
			
					auto old_curtime = g_GlobalVars->curtime;
					auto old_frametime = g_GlobalVars->frametime;
					auto old_ragpos = g_LocalPlayer->m_ragPos();
			
					g_GlobalVars->curtime = g_LocalPlayer->m_flSimulationTime();
					g_GlobalVars->frametime = g_GlobalVars->interval_per_tick;
			
					auto player_animation_state = g_LocalPlayer->GetPlayerAnimState();
					auto player_model_time = reinterpret_cast<int*>(player_animation_state + 112);
					if (player_animation_state != nullptr && player_model_time != nullptr)
						if (*player_model_time == g_GlobalVars->framecount)
							*player_model_time = g_GlobalVars->framecount - 1;
			
					g_LocalPlayer->m_ragPos() = old_ragpos;
					g_LocalPlayer->UpdateClientSideAnimation();
			
					g_GlobalVars->curtime = old_curtime;
					g_GlobalVars->frametime = old_frametime;
			
					g_LocalPlayer->SetAbsAngles(QAngle(0.f, g_LocalPlayer->GetPlayerAnimState()->m_flGoalFeetYaw, 0.f));
			
					g_LocalPlayer->m_bClientSideAnimation() = false;
		}
}

void ThirdpersonAngleHelper::AnimFix2 ( C_BasePlayer* entity )
{
	//dengi
	static float sim_time;
	if (sim_time != entity->m_flSimulationTime())
	{
		auto state = entity->GetPlayerAnimState(); if (!state) return;

		const float curtime = g_GlobalVars->curtime;
		const float frametime = g_GlobalVars->frametime;
		const float realtime = g_GlobalVars->realtime;
		const float absoluteframetime = g_GlobalVars->absoluteframetime;
		const float absoluteframestarttimestddev = g_GlobalVars->absoluteframestarttimestddev;
		const float interpolation_amount = g_GlobalVars->interpolation_amount;
		const float framecount = g_GlobalVars->framecount;
		const float tickcount = g_GlobalVars->tickcount;

		static auto host_timescale = g_CVar->FindVar(("host_timescale"));

		g_GlobalVars->curtime = entity->m_flSimulationTime();
		g_GlobalVars->realtime = entity->m_flSimulationTime();
		g_GlobalVars->frametime = g_GlobalVars->interval_per_tick * host_timescale->GetFloat();
		g_GlobalVars->absoluteframetime = g_GlobalVars->interval_per_tick * host_timescale->GetFloat();
		g_GlobalVars->absoluteframestarttimestddev = entity->m_flSimulationTime() - g_GlobalVars->interval_per_tick * host_timescale->GetFloat();
		g_GlobalVars->interpolation_amount = 0;
		g_GlobalVars->framecount = TIME_TO_TICKS(entity->m_flSimulationTime());
		g_GlobalVars->tickcount = TIME_TO_TICKS(entity->m_flSimulationTime());

		int backup_flags = entity->m_fFlags();
		int backup_eflags = entity->m_iEFlags();

		AnimationLayer backup_layers[15];
		std::memcpy(backup_layers, entity->GetAnimOverlays(), (sizeof(AnimationLayer) * 15));

		if (state->m_iLastClientSideAnimationUpdateFramecount == g_GlobalVars->framecount)
			state->m_iLastClientSideAnimationUpdateFramecount = g_GlobalVars->framecount - 1;

		entity->client_side_animation() = true;
		entity->UpdateClientSideAnimation();
		entity->client_side_animation() = false;

		float lby_delta = entity->m_flLowerBodyYawTarget();
		lby_delta = std::remainderf(lby_delta, 360.f);
		lby_delta = Math::clamp(lby_delta, -60.f, 60.f);

		float feet_yaw = std::remainderf(lby_delta, 360.f);

		if (feet_yaw < 0.f) {
			feet_yaw += 360.f;
		}

		std::memcpy(entity->GetAnimOverlays(), backup_layers, (sizeof(AnimationLayer) * 15));

		entity->m_fFlags() = backup_flags;
		entity->m_iEFlags() = backup_eflags;

		g_GlobalVars->curtime = curtime;
		g_GlobalVars->realtime = realtime;
		g_GlobalVars->frametime = frametime;
		g_GlobalVars->absoluteframetime = absoluteframetime;
		g_GlobalVars->absoluteframestarttimestddev = absoluteframestarttimestddev;
		g_GlobalVars->interpolation_amount = interpolation_amount;
		g_GlobalVars->framecount = framecount;
		g_GlobalVars->tickcount = tickcount;
		sim_time = entity->m_flSimulationTime();
	}
	entity->InvalidateBoneCache();
	entity->SetupBones(nullptr, -1, 0x7FF00, g_GlobalVars->curtime);
}