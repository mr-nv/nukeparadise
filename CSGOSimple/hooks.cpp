//#define CRASH_HOOKS
#include "hooks.hpp"
#include <intrin.h>
#include "render.hpp"
#include "menu_helpers.hpp"
#include "config.hpp"
#include "features/ThirdpersonAngleHelper.h"
#include "features/AntiAim.h"
#include "features/Fakelag.h"
#include "features/Resolver.h"
#include "features/HitPossitionHelper.h"
#include "RuntimeSaver.h"
#include "features/Logger.h"
#include "features/ClantagChanger.h"
#include "features/Backtrack.h"
#include "features/Lbot.hpp"
#include "features/lBacktrack.h"
#include "Misc.h"
#include "ConsoleHelper.h"
#include "helpers/bfReader.hpp"
#include "features/BuyBot.h"
#include "features/Skinchanger.h"
#include "NoSmoke.h"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "features/LagCompensation.h"
#include "features/bhop.hpp"
#include "features/visuals.hpp"
#include "features/NightMode.h"
#include "features/glow.hpp"
#include "features/EnginePrediction.h"
#include "features/GrenadePrediction.h"
#include "features/Chams.h"
#include "MovementFix.h"
#include "features/Rbot.h"
#include "KeyLoop.h"
#include <time.h>
#pragma intrinsic(_ReturnAddress)
#define MAX_COORD_FLOAT ( 16384.0f )
#define MIN_COORD_FLOAT ( -MAX_COORD_FLOAT )
#define chk(expr) (expr) ? ((void)0) : check(false)
namespace Hooks
{
	vfunc_hook hlclient_hook;
	vfunc_hook direct3d_hook;
	vfunc_hook vguipanel_hook;
	vfunc_hook vguisurf_hook;
	vfunc_hook sound_hook;
	vfunc_hook mdlrender_hook;
	vfunc_hook clientmode_hook;
	recv_prop_hook* sequence_hook;
	vfunc_hook sv_cheats;
	vfunc_hook RenderView_hook;
	vfunc_hook ViewRender_hook;
	vfunc_hook gameevents_hook;
	vfunc_hook firebullets_hook;
	vfunc_hook clientstate_hook;
	vfunc_hook bsp_query_hook;
	TempEntities o_TempEntities = nullptr;

	void Initialize()
	{
		ConVar* blur = g_CVar->FindVar("@panorama_disable_blur"); blur->SetValue(1);

		hlclient_hook.setup(g_CHLClient, "client_panorama.dll");
		direct3d_hook.setup(g_D3DDevice9, "shaderapidx9.dll");
		vguipanel_hook.setup(g_VGuiPanel, "vgui2.dll");
		vguisurf_hook.setup(g_VGuiSurface, "vguimatsurface.dll");
		sound_hook.setup(g_EngineSound, "engine.dll");
		mdlrender_hook.setup(g_StudioRender);
		clientmode_hook.setup(g_ClientMode, "client_panorama.dll");
		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sequence_hook = new recv_prop_hook(C_BaseViewModel::m_nSequence(), hkRecvProxy);
		sv_cheats.setup(sv_cheats_con);
		RenderView_hook.setup(g_RenderView, "engine.dll");
		gameevents_hook.setup(g_GameEvents, "engine.dll");
		ViewRender_hook.setup(g_ViewRender, "client_panorama.dll");
		bsp_query_hook.setup(g_EngineClient->GetBSPTreeQuery());
		hlclient_hook.hook_index(index::DispatchUserMessage, hkDispatchUserMessage);
		bsp_query_hook.hook_index(index::ListLeavesInBox, hkListLeavesInBox);
#ifdef CRASH_HOOKS
		clientstate_hook.setup((uintptr_t*)((uintptr_t)g_ClientState + 0x8));
		clientstate_hook.hook_index(index::TempEntities, hkTempEntities);
		auto dwFireBullets = *(DWORD**)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "55 8B EC 51 53 56 8B F1 BB ? ? ? ? B8") + 0x131);
		firebullets_hook.setup(dwFireBullets, "client_panorama.dll");
#endif // CRASH_HOOKS
		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		hlclient_hook.hook_index(index::WriteUsercmdDeltaToBuffer, WriteUsercmdDeltaToBuffer_h);
		hlclient_hook.hook_index(index::CreateMove, hkCreateMove_Proxy);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		sound_hook.hook_index(index::EmitSound1, hkEmitSound1);
		vguisurf_hook.hook_index(index::LockCursor, hkLockCursor);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute); //thx dickware for chams
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);
		RenderView_hook.hook_index(index::SceneEnd, hkSceneEnd);
		ViewRender_hook.hook_index(index::SmokeOverlay, Hooked_RenderSmokeOverlay);
		gameevents_hook.hook_index(index::FireEvent, hkFireEvent);

#ifdef CRASH_HOOKS
		o_TempEntities = clientstate_hook.get_original<TempEntities>(index::TempEntities);
		firebullets_hook.hook_index(index::FireBullets, hkTEFireBulletsPostDataUpdate);
#endif // CRASH_HOOKS
		g_Logger.Debug("ENGINE", "Loaded.", Color(255, 50, 25));

		g_CVar->FindVar("cl_interpolate")->SetValue(1); //0
		g_CVar->FindVar("sv_showanimstate")->SetValue(1);
		g_CVar->FindVar("developer")->SetValue(0);
		g_CVar->FindVar("cl_interp_ratio")->SetValue(2);
	}
	void Shutdown()
	{
		Glow::Get().Shutdown();
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		sound_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		sv_cheats.unhook_all();
		sequence_hook->~recv_prop_hook();
		RenderView_hook.unhook_all();
		ViewRender_hook.unhook_all();
#ifdef CRASH_HOOKS
		firebullets_hook.unhook_all();
		clientstate_hook.unhook_all();
#endif // CRASH_HOOKS
		gameevents_hook.unhook_all();
	}
	bool HookedNetchan = false;
	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		auto oEndScene = direct3d_hook.get_original<EndScene>(index::EndScene);

		if (g_Unload) Utils::SetClantag("");

		if (g_Unload) clientstate_hook.unhook_all();

		if (g_Unload)
			oEndScene(pDevice);

		static uintptr_t gameoverlay_return_address = 0;
		if (!gameoverlay_return_address) {
			MEMORY_BASIC_INFORMATION info;
			VirtualQuery(_ReturnAddress(), &info, sizeof(MEMORY_BASIC_INFORMATION));

			char mod[MAX_PATH];
			GetModuleFileNameA((HMODULE)info.AllocationBase, mod, MAX_PATH);

			if (strstr(mod, "gameoverlay"))
				gameoverlay_return_address = (uintptr_t)(_ReturnAddress());
		}

		if (gameoverlay_return_address != (uintptr_t)(_ReturnAddress()))
			return oEndScene(pDevice);

		DWORD NetChannel = *(DWORD*)(*(DWORD*)g_ClientState + 0x9C);
		if (!g_EngineClient->IsInGame() || !g_EngineClient->IsConnected() || !NetChannel)
		{
			if (HookedNetchan) clientstate_hook.unhook_all();
			HookedNetchan = false;
		}

		//CVars
		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");

		static auto engine_no_focus_sleep_cvar = g_CVar->FindVar("engine_no_focus_sleep"); engine_no_focus_sleep_cvar->SetValue(0); //gold
		static auto fps_max = g_CVar->FindVar("fps_max"); fps_max->SetValue(0);
		static auto cl_lagcompensation = g_CVar->FindVar("cl_lagcompensation"); cl_lagcompensation->SetValue(1); //$
		AntiAim::Get().ResetLbyPrediction();

		viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;

		if (!g_Unload)
			viewmodel_fov->SetValue(g_Config.misc_viewmodel_fov);
		else
			viewmodel_fov->SetValue(60);

		if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
		{
			srand(time(0));

			int r;
			r = rand() % 20 + 1;

			int g;
			g = rand() % 20 + 1;

			int b;
			b = rand() % 20 + 1;

			int a;
			a = rand() % 20 + 1;

			NightMode::Get().Apply(false, r, g, b, a);
		}

		//modern problems require modern solutions

		if (g_Saver.NameTrigger)
		{
			g_Saver.NameTrigger = false;
			Utils::SetName(g_Saver.Name);
		}

		if (g_Saver.FakeNameMethod > 0)
		{
			if (!g_EngineClient->IsConnected())
				g_Saver.FakeNameMethod = 0;

			switch (g_Saver.FakeNameMethod) {
			case 1:
				Utils::SetName("\n\xAD\xAD\xAD");
				break;
			case 2:
				Utils::SetName("\n");
				break;
			case 3:
				Utils::SetName(Utils::Format(u8" \x01\x0B\x10 %s \x01has opened a container and found:\x02 ★ M9 Bayonet | Doppler", g_Saver.Fakename));
				break;
			case 4:
				Utils::SetName(Utils::Format(" \x01\x0B\x07 %s has been permanently banned from official CS:GO servers.", g_Saver.Fakename));
				break;
			case 5:
				Utils::SetName(Utils::Format("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n%s\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n", g_Saver.Fakename));
				break;
			default: break;
			}
			g_Saver.FakeNameMethod = 0;
		}

		if (g_Config.rbot_aa != 2 || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
		{
			g_Saver.AATimer = GetTickCount();
		}

		DWORD colorwrite, srgbwrite;
		IDirect3DVertexDeclaration9* vert_dec = nullptr;
		IDirect3DVertexShader9* vert_shader = nullptr;
		DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		//removes the source engine color correction
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->GetVertexDeclaration(&vert_dec);
		pDevice->GetVertexShader(&vert_shader);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		auto esp_drawlist = Render::Get().RenderScene();

		MenuHelper::Get().Render(pDevice);

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		pDevice->SetVertexDeclaration(vert_dec);
		pDevice->SetVertexShader(vert_shader);;

		return oEndScene(pDevice);
	}
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		auto oReset = direct3d_hook.get_original<Reset>(index::Reset);

		MenuHelper::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0)
			MenuHelper::Get().OnDeviceReset();

		return hr;
	}

	float AngleDiff(float destAngle, float srcAngle) {
		float delta;

		delta = fmodf(destAngle - srcAngle, 360.0f);
		if (destAngle > srcAngle) {
			if (delta >= 180)
				delta -= 360;
		}
		else {
			if (delta <= -180)
				delta += 360;
		}
		return delta;
	}
	static int max_choke_ticks = 14;

	static CCSGOPlayerAnimState g_AnimState;

	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		auto oCreateMove = hlclient_hook.get_original<CreateMove>(index::CreateMove);
		oCreateMove(g_CHLClient, sequence_number, input_sample_frametime, active);
		auto cmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);
		if (!cmd || !cmd->command_number || !bSendPacket || g_Unload || !g_EngineClient || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame()) return;
		KeyLoop::Get().OnCreateMove();
		if (MenuHelper::Get().IsVisible()) cmd->buttons &= ~(IN_ATTACK | IN_ATTACK2);
		DWORD NetChannel = *(DWORD*)(*(DWORD*)g_ClientState + 0x9C);
		if (NetChannel && g_EngineClient && g_ClientState && !HookedNetchan && g_LocalPlayer && g_EngineClient->IsInGame() && g_EngineClient->IsConnected())
		{
			auto netchan = *reinterpret_cast<INetChannel**>(reinterpret_cast<std::uintptr_t>(g_ClientState) + 0x9C);
			clientstate_hook.setup(netchan, "engine.dll");
			clientstate_hook.hook_index(46, SendDatagram_h);
			HookedNetchan = true;
		}

		static int latency_ticks = 0;
		float fl_latency = g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
		int latency = TIME_TO_TICKS(fl_latency);;
		if (g_EngineClient->GetNetChannel()->m_nChokedPackets <= 0) {
			latency_ticks = latency;
		}
		else {
			latency_ticks = std::max(latency, latency_ticks);
		}

		if (g_GameRules->m_bIsValveDS()) {
			if (fl_latency >= g_GlobalVars->interval_per_tick)
				max_choke_ticks = 11 - latency_ticks;
			else
				max_choke_ticks = 11;
		}
		else {
			max_choke_ticks = 13 - latency_ticks;
		}

		static float SpawnTime = 0.0f;
		if (g_LocalPlayer->m_flSpawnTime() != SpawnTime) {
			g_AnimState.pBaseEntity = g_LocalPlayer;
			g_LocalPlayer->ResetAnimationState(&g_AnimState);
			SpawnTime = g_LocalPlayer->m_flSpawnTime();
		}

		//	old flags
		g_Saver.OLDviewangles = cmd->viewangles;
		g_Saver.OLDforwardmove = cmd->forwardmove;
		g_Saver.OLDsidemove = cmd->sidemove;
		g_Saver.OLDflags = g_LocalPlayer->m_fFlags();
		g_Saver.OLDvelocity = g_LocalPlayer->m_vecVelocity();

		//	prediction
		prediction->RunPrediction(g_Saver.PredictionData, cmd);
		{
			if (g_Config.misc_franzjump && (g_Saver.OLDflags & FL_ONGROUND) && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND)) cmd->buttons |= IN_JUMP;

			if (g_Config.rbot_extrapolation)
				Misc::Get().LinearExtrapolationsrun();

			Fakelag::Get().OnCreateMove(cmd, bSendPacket);

			Misc::Get().OnCreateMove(cmd);

			if (g_Config.misc_bhop) BunnyHop::Get().OnCreateMove(cmd);
			if (g_Config.misc_autostrafe > 0 && g_LocalPlayer && g_LocalPlayer->IsAlive() && g_LocalPlayer->m_nMoveType() != MOVETYPE_NOCLIP && g_LocalPlayer->m_nMoveType() != MOVETYPE_LADDER)
			{
				if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
				{
					switch (g_Config.misc_autostrafe) {
					case 0:
						break;
					case 1:
						BunnyHop::Get().aw_AutoStrafe(cmd, g_Saver.OLDviewangles);
						break;
					case 2:
						BunnyHop::Get().glad_AutoStrafe(cmd);
						break;
					case 3:
						BunnyHop::Get().gideon_AutoStrafe(cmd);
						break;
					case 4:
						BunnyHop::Get().money_AutoStrafe(cmd);
						break;
					case 5:
						BunnyHop::Get().legit_AutoStrafe(cmd);
						break;
					case 6:
						BunnyHop::Get().rage_AutoStrafe(cmd);
						break;
					case 7:
						BunnyHop::Get().bruh_AutoStrafe(cmd, g_Saver.OLDviewangles);
						break;
					}
				}
			}

			if (g_Config.rbot_aa == 2) AntiAim::Get().OnCreateMove(cmd, bSendPacket);

			if (!InputSys::Get().IsKeyDown(g_Config.lbot_auto_fire_key) && g_Config.rbot_aa == 1 && std::fabsf(g_LocalPlayer->m_flSpawnTime() - g_GlobalVars->curtime) > 1.0f)
			{
				AntiAim::Get().LegitAA(g_Saver.OLDviewangles, cmd, bSendPacket);
				Math::FixAngles(cmd->viewangles);
				cmd->viewangles.yaw = std::remainderf(cmd->viewangles.yaw, 360.0f);
			}

			if (g_Config.rbot_aa == 1 && g_EngineClient->GetNetChannel()->m_nChokedPackets >= max_choke_ticks) {
				bSendPacket = true;
				cmd->viewangles = Math::VECTOR_TO_QANGLE(g_ClientState->viewangles);
			}

			if (g_Config.rbot_enable)
			{
				Backtrack::Get().OnCreateMove();
				Rbot::Get().PrecacheShit();
				Rbot::Get().AccuracyBoost(cmd);
				Rbot::Get().CreateMove(cmd, bSendPacket);
			}

			if (g_Config.rbot_slowwalk) Misc::Get().SlowWalkRun(cmd);

			CCSGrenadeHint::Get().Tick(cmd->buttons);

			if (!g_Config.rbot_enable)
			{
				if (g_Config.lbot_ijustflippedaswitch)
				{
					g_Lbot.OnMove(cmd);
					g_lBacktrack.OnMove(cmd);
				}

				Math::FixAngles(cmd->viewangles);
				cmd->viewangles.yaw = std::remainderf(cmd->viewangles.yaw, 360.0f);
			}

			if (g_Config.rbot_enable && g_Config.rbot_resolver) Resolver::Get().OnCreateMove(g_Saver.OLDviewangles);
			if (g_Config.misc_buybot) BuyBot::Get().OnCreateMove();

			if (g_Config.rbot_aa == 2) MovementFix::Get().Correct(g_Saver.OLDviewangles, cmd);
			Misc::Get().aimware_fix_mouse_dx(cmd);
			//MovementFix::Get().aaCorrect(g_Saver.OLDviewangles, cmd, g_Saver.OLDforwardmove, g_Saver.OLDsidemove);
			if (bSendPacket) {
				g_Saver.real_angle = g_AnimState.m_flGoalFeetYaw;
				g_Saver.view_angle = g_AnimState.m_flEyeYaw;
			}
		}
		prediction->EndPrediction(g_Saver.PredictionData);

		ClantagChanger::Get().OnCreateMove();

		if (g_LocalPlayer && g_LocalPlayer->IsAlive() && (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2))
			g_Saver.LastShotEyePos = g_LocalPlayer->GetEyePos();

		if (g_Saver.RbotDidLastShot)
		{
			g_Saver.RbotDidLastShot = false;

			if (bSendPacket)
				bSendPacket = false;
		}

		if (g_Config.rbot_enable && g_Config.rbot_lagcompensation)
		{
			for (int i = 1; i <= g_GlobalVars->maxClients; i++)
			{
				C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);
				if (player && player != g_LocalPlayer)
					LagCompensation::Get().UpdateAnimations(player);
			}
		}

		AntiAim::Get().LbyBreakerPrediction(cmd, bSendPacket);
		Math::NormalizeAngles(cmd->viewangles);

		if (g_Config.rbot_slidewalk) AntiAim::Get().SlideWalk(cmd);

		if (g_Config.misc_choke_limit) if (g_EngineClient->GetNetChannel()->m_nChokedPackets > 14) bSendPacket = true;

		Math::ClampAngles(cmd->viewangles);

		auto anim_state = g_LocalPlayer->GetPlayerAnimState();
		if (anim_state) {
			CCSGOPlayerAnimState anim_state_backup = *anim_state;
			*anim_state = g_AnimState;
			*g_LocalPlayer->GetVAngles() = cmd->viewangles;
			g_LocalPlayer->UpdateClientSideAnimation();

			if (anim_state->speed_2d > 0.1f || std::fabsf(anim_state->flUpVelocity)) {
				g_Saver.next_lby = g_GlobalVars->curtime + 0.22f;
			}
			else if (g_GlobalVars->curtime > g_Saver.next_lby) {
				if (std::fabsf(AngleDiff(anim_state->m_flGoalFeetYaw, anim_state->m_flEyeYaw)) > 35.0f) {
					g_Saver.next_lby = g_GlobalVars->curtime + 1.1f;
				}
			}

			g_AnimState = *anim_state;
			*anim_state = anim_state_backup;
		}

		g_Saver.OLDsendpacket = bSendPacket;

		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();

		static float updateTime = g_GlobalVars->curtime + .1f;
		static bool shouldForceUpdate = false;

		if (g_Saver.RequestForceUpdate)
		{
			g_Saver.RequestForceUpdate = false;
			updateTime = g_GlobalVars->curtime + .1f;
			shouldForceUpdate = true;
		}

		if (shouldForceUpdate)
		{
			if (g_GlobalVars->curtime > updateTime)
			{
				shouldForceUpdate = false;
				g_ClientState->ForceFullUpdate();
			}
		}
	}
	__declspec (naked) void __stdcall hkCreateMove_Proxy(int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx
			lea  ecx, [esp]
			push ecx
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}
	void __stdcall hkPaintTraverse(vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<PaintTraverse>(index::PaintTraverse);

		if (g_Config.vis_misc_noscope && !strcmp("HudZoom", g_VGuiPanel->GetName(panel))) return;

		oPaintTraverse(g_VGuiPanel, panel, forceRepaint, allowForce);

		if (g_Unload)
			return;

		if (!panelId)
		{
			const auto panelName = g_VGuiPanel->GetName(panel);

			if (!strcmp(panelName, "FocusOverlayPanel"))
				panelId = panel;
		}
		else if (panelId == panel)
		{
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;

			if (g_LocalPlayer && InputSys::Get().IsKeyDown(VK_TAB) && g_Config.misc_showranks)
				Utils::RankRevealAll();
			if (g_Config.vis_misc_screenshotreturner && g_EngineClient->IsTakingScreenshot()) return;
			Render::Get().BeginScene();
		}
	}
	void __stdcall hkEmitSound1(IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk)
	{
		static auto ofunc = sound_hook.get_original<EmitSound1>(index::EmitSound1);

		if (!strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep") && g_Config.misc_autoaccept) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			if (fnAccept) {

				fnAccept("");

				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}

		ofunc(g_EngineSound, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);
	}
	int __stdcall hkDoPostScreenEffects(int a1)
	{
		auto oDoPostScreenEffects = clientmode_hook.get_original<DoPostScreenEffects>(index::DoPostScreenSpaceEffects);

		if (g_LocalPlayer && (g_Config.glow_team_enable | g_Config.glow_enemy_enable) && !g_Unload && g_EngineClient->IsConnected() && g_EngineClient->IsInGame())
			Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, a1);
	}
	void __stdcall hkFrameStageNotify(ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<FrameStageNotify>(index::FrameStageNotify);

		g_Saver.OLDstage = stage;

		if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
			return ofunc(g_CHLClient, stage);

		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
		{
			if (g_Config.vis_misc_nightmode && !g_Unload)
			{
				float r = g_Config.vis_misc_clr_modificate[0];
				float g = g_Config.vis_misc_clr_modificate[1];
				float b = g_Config.vis_misc_clr_modificate[2];
				float a = g_Config.vis_misc_clr_modificate[3];
				NightMode::Get().Apply(false, r, g, b, a);
			}
			else
			{
				NightMode::Get().Revert();
			}

			if (g_Unload) return;

			if (g_Config.rbot_enable && g_Config.rbot_resolver) Resolver::Get().OnFramestageNotify();

			NoSmoke::Get().OnFrameStageNotify();

			for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
			{
				auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

				if (!entity || !g_LocalPlayer || !entity->IsPlayer() || entity->IsDormant()
					|| !entity->IsAlive())
					continue;

				VarMapping_t* map = entity->VarMapping();

				if (map)
				{
					for (int j = 0; j < map->m_nInterpolatedEntries; j++)
						map->m_Entries[j].m_bNeedsToInterpolate = false;
				}
			}
		}

		Skinchanger::Get().OnFrameStageNotify(stage);

		Misc::Get().OnFrameStageNotify(stage);

		if (stage == FRAME_START)
		{
			if (g_LocalPlayer->m_fFlags() & FL_ONGROUND && g_Config.rbot_fakeduck && InputSys::Get().IsKeyDown(g_Config.rbot_fakeduck_key) && g_Config.vis_misc_thirdperson)
				g_LocalPlayer->GetPlayerAnimState()->m_fDuckAmount = 1.f;

			g_LocalPlayer->GetPlayerAnimState()->m_vOrigin = g_LocalPlayer->GetPlayerAnimState()->m_vLastOrigin;
		}

		if (stage == FRAME_RENDER_START)
		{

			if (!g_Unload)
			{
				if ((g_Config.rbot_enable && g_Config.rbot_aa == 2) && g_Config.vis_misc_thirdperson)
				{
					ThirdpersonAngleHelper::Get().SetThirdpersonAngle();

					if (g_Config.vis_misc_localanimfix)
						ThirdpersonAngleHelper::Get().AnimFix();
				}
				else
				{
					if (g_LocalPlayer && g_LocalPlayer->IsAlive())
						g_LocalPlayer->m_bClientSideAnimation() = true;
				}

				auto old_curtime = g_GlobalVars->curtime;
				auto old_frametime = g_GlobalVars->frametime;

				for (int i = 1; i < g_EngineClient->GetMaxClients(); i++)
				{
					auto entity = static_cast<C_BasePlayer*> (g_EntityList->GetClientEntity(i));

					if (!entity || !g_LocalPlayer || !entity->IsPlayer() || entity->IsDormant() || !entity->IsAlive())
						continue;

					if (g_Config.rbot_enable && g_Config.rbot_guccigang)
					{
						//entity->InvalidateBoneCache();
						ThirdpersonAngleHelper::Get().EnemyAnimationFix(entity);
					}
					else
						entity->m_bClientSideAnimation() = true;
				}

				g_GlobalVars->curtime = old_curtime;
				g_GlobalVars->frametime = old_frametime;
			}

			static QAngle vecAngles; g_EngineClient->GetViewAngles(vecAngles);

			if (g_LocalPlayer)
			{
				if (g_LocalPlayer->IsAlive())
				{
					if (g_Config.vis_misc_thirdperson && !g_Unload)
					{
						if (!g_Input->m_fCameraInThirdPerson)
							g_Input->m_fCameraInThirdPerson = true;
					}
					else
						g_Input->m_fCameraInThirdPerson = false;
				}

				if (g_Config.vis_misc_thirdperson && !g_Unload)
					g_Input->m_vecCameraOffset = Vector(vecAngles.pitch, vecAngles.yaw, g_Config.vis_misc_thirdperson_dist); //gold fix
				else
					g_Input->m_vecCameraOffset = Vector(vecAngles.pitch, vecAngles.yaw, 0.f); //gold fix
			}
		}

		if (stage == FRAME_NET_UPDATE_END)
		{
			if (g_Config.rbot_lagcompensation)
				LagCompensation::Get().FrameUpdatePostEntityThink();
		}

		ofunc(g_CHLClient, stage);
	}
	void __stdcall hkOverrideView(CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<OverrideView>(index::OverrideView);

		if (g_Unload || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
			return ofunc(g_ClientMode, vsView);

		if (g_EngineClient->IsInGame() && vsView) {
			//Visuals::Get().ThirdPerson();
			CCSGrenadeHint::Get().View();

			if (g_Config.misc_no_recoil)
			{
				Vector viewPunch = g_LocalPlayer->GetViewPunchAngle();
				Vector aimPunch = g_LocalPlayer->GetAimPunchAngle();

				float RecoilScale = g_CVar->FindVar("weapon_recoil_scale")->GetFloat();

				vsView->angles -= (viewPunch + (aimPunch * RecoilScale * 0.4499999f)); // oof 1 line now ez
			}

		}
		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");

		if (g_LocalPlayer && g_LocalPlayer->m_bIsScoped() && !g_Config.vis_misc_disable_scope_zoom)
			return ofunc(g_ClientMode, vsView);

		vsView->fov = g_Config.misc_fov;

		ofunc(g_ClientMode, vsView);
	}
	void __stdcall hkLockCursor()
	{
		static auto ofunc = vguisurf_hook.get_original<LockCursor_t>(index::LockCursor);

		if (MenuHelper::Get().IsVisible())
		{
			g_VGuiSurface->UnlockCursor();
			return;
		}

		ofunc(g_VGuiSurface);

	}
	int __fastcall hkListLeavesInBox(void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax) {
		typedef int(__thiscall* ListLeavesInBox)(void*, const Vector&, const Vector&, unsigned short*, int);
		static auto ofunc = bsp_query_hook.get_original< ListLeavesInBox >(index::ListLeavesInBox);

		bool gate = g_Config.chams_backtrack || g_Config.chams_local || g_Config.chams_enemy || g_Config.chams_team;

		// occulusion getting updated on player movement/angle change,
		// in RecomputeRenderableLeaves ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L674 );
		// check for return in CClientLeafSystem::InsertIntoTree
		if (!gate || *(uint32_t*)_ReturnAddress() != 0x14244489) // 89 44 24 14 ( 0x14244489 ) - new / 8B 7D 08 8B ( 0x8B087D8B ) - old
			return ofunc(bsp, mins, maxs, pList, listMax);

		// get current renderable info from stack ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1470 )
		auto info = *(RenderableInfo_t**)((uintptr_t)_AddressOfReturnAddress() + 0x14);
		if (!info || !info->m_pRenderable)
			return ofunc(bsp, mins, maxs, pList, listMax);

		// check if disabling occulusion for players ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1491 )
		auto base_entity = info->m_pRenderable->GetIClientUnknown()->GetBaseEntity();
		if (!base_entity || !base_entity->IsPlayer())
			return ofunc(bsp, mins, maxs, pList, listMax);

		// fix render order, force translucent group ( https://www.unknowncheats.me/forum/2429206-post15.html )
		// AddRenderablesToRenderLists: https://i.imgur.com/hcg0NB5.png ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L2473 )
		info->m_Flags &= ~0x100;
		info->m_Flags2 |= 0xC0;

		// extend world space bounds to maximum ( https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L707 )
		static const Vector map_min = Vector(MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT);
		static const Vector map_max = Vector(MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT);
		auto count = ofunc(bsp, map_min, map_max, pList, listMax);
		return count;
	}
	void __fastcall hkDrawModelExecute(void* pEcx, void* pEdx, void* pResults, DrawModelInfo_t* pInfo, matrix3x4_t* pBoneToWorld, float* flpFlexWeights, float* flpFlexDelayedWeights, Vector& vrModelOrigin, int32_t iFlags)
	{
		static auto ofunc = mdlrender_hook.get_original<DrawModelExecute>(index::DrawModelExecute);
		bool forced_mat = !g_MdlRender->IsForcedMaterialOverride();

		if (forced_mat && !g_Saver.RequestForceUpdate)
		{
			Chams::Get().OnDrawModelExecute(pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);
		}

		ofunc(pEcx, pResults, pInfo, pBoneToWorld, flpFlexWeights, flpFlexDelayedWeights, vrModelOrigin, iFlags);

		if (forced_mat)
			g_MdlRender->ForcedMaterialOverride(nullptr);
	}
	auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "85 C0 75 30 38 86");
	typedef bool(__thiscall* svc_get_bool_t) (PVOID);
	bool __fastcall hkDispatchUserMessage(void* ecx, void* edx, int type, unsigned int a3, unsigned int length, const void* msg_data)
	{
		static auto oDispatchUserMessage = hlclient_hook.get_original<tDispatchUserMessage>(index::DispatchUserMessage);

		if (g_EngineClient->IsConnected()) Misc::Get().AntiKick(oDispatchUserMessage, ecx, type, a3, length, msg_data);

		return oDispatchUserMessage(ecx, type, a3, length, msg_data);
	}
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto ofunc = sv_cheats.get_original<svc_get_bool_t>(13);

		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD> (_ReturnAddress()) == reinterpret_cast<DWORD> (dwCAM_Think))
			return true;

		return ofunc(pConVar);
	}
	static auto random_sequence(const int low, const int high) -> int
	{
		return rand() % (high - low + 1) + low;
	}
	static auto fix_animation(const char* model, const int sequence) -> int
	{
		enum ESequence
		{
			SEQUENCE_DEFAULT_DRAW = 0,
			SEQUENCE_DEFAULT_IDLE1 = 1,
			SEQUENCE_DEFAULT_IDLE2 = 2,
			SEQUENCE_DEFAULT_LIGHT_MISS1 = 3,
			SEQUENCE_DEFAULT_LIGHT_MISS2 = 4,
			SEQUENCE_DEFAULT_HEAVY_MISS1 = 9,
			SEQUENCE_DEFAULT_HEAVY_HIT1 = 10,
			SEQUENCE_DEFAULT_HEAVY_BACKSTAB = 11,
			SEQUENCE_DEFAULT_LOOKAT01 = 12,
			SEQUENCE_BUTTERFLY_DRAW = 0,
			SEQUENCE_BUTTERFLY_DRAW2 = 1,
			SEQUENCE_BUTTERFLY_LOOKAT01 = 13,
			SEQUENCE_BUTTERFLY_LOOKAT03 = 15,
			SEQUENCE_FALCHION_IDLE1 = 1,
			SEQUENCE_FALCHION_HEAVY_MISS1 = 8,
			SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP = 9,
			SEQUENCE_FALCHION_LOOKAT01 = 12,
			SEQUENCE_FALCHION_LOOKAT02 = 13,
			SEQUENCE_DAGGERS_IDLE1 = 1,
			SEQUENCE_DAGGERS_LIGHT_MISS1 = 2,
			SEQUENCE_DAGGERS_LIGHT_MISS5 = 6,
			SEQUENCE_DAGGERS_HEAVY_MISS2 = 11,
			SEQUENCE_DAGGERS_HEAVY_MISS1 = 12,
			SEQUENCE_BOWIE_IDLE1 = 1,
		};
		if (strstr(model, "models/weapons/v_knife_butterfly.mdl")) {
			switch (sequence) {
			case SEQUENCE_DEFAULT_DRAW:
				return random_sequence(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03);
			default:
				return sequence + 1;
			}
		}
		else if (strstr(model, "models/weapons/v_knife_falchion_advanced.mdl")) {
			switch (sequence) {
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_FALCHION_IDLE1;
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
				return random_sequence(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP);
			case SEQUENCE_DEFAULT_LOOKAT01:
				return random_sequence(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02);
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			default:
				return sequence - 1;
			}
		}
		else if (strstr(model, "models/weapons/v_knife_push.mdl")) {
			switch (sequence) {
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_DAGGERS_IDLE1;
			case SEQUENCE_DEFAULT_LIGHT_MISS1:
			case SEQUENCE_DEFAULT_LIGHT_MISS2:
				return random_sequence(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5);
			case SEQUENCE_DEFAULT_HEAVY_MISS1:
				return random_sequence(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1);
			case SEQUENCE_DEFAULT_HEAVY_HIT1:
			case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
			case SEQUENCE_DEFAULT_LOOKAT01:
				return sequence + 3;
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			default:
				return sequence + 2;
			}
		}
		else if (strstr(model, "models/weapons/v_knife_survival_bowie.mdl")) {
			switch (sequence) {
			case SEQUENCE_DEFAULT_DRAW:
			case SEQUENCE_DEFAULT_IDLE1:
				return sequence;
			case SEQUENCE_DEFAULT_IDLE2:
				return SEQUENCE_BOWIE_IDLE1;
			default:
				return sequence - 1;
			}
		}
		else {
			return sequence;
		}
	}
	void hkRecvProxy(const CRecvProxyData* pData, void* entity, void* output)
	{
		static auto ofunc = sequence_hook->get_original_function();
		const auto local = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()));
		if (local && local->IsAlive())
		{
			const auto proxy_data = const_cast<CRecvProxyData*>(pData);
			const auto view_model = static_cast<C_BaseViewModel*>(entity);
			if (view_model && view_model->m_hOwner() && view_model->m_hOwner().IsValid())
			{
				const auto owner = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntityFromHandle(view_model->m_hOwner()));
				if (owner == g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer()))
				{
					const auto view_model_weapon_handle = view_model->m_hWeapon();
					if (view_model_weapon_handle.IsValid())
					{
						const auto view_model_weapon = static_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
						if (view_model_weapon)
						{
							if (k_weapons_info.count(view_model_weapon->m_Item().m_iItemDefinitionIndex()))
							{
								auto original_sequence = proxy_data->m_Value.m_Int;
								const auto override_model = k_weapons_info.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
								proxy_data->m_Value.m_Int = fix_animation(override_model, proxy_data->m_Value.m_Int);
							}
						}
					}
				}
			}
		}
		ofunc(pData, entity, output);
	}
	void __fastcall hkSceneEnd(void* pEcx, void* pEdx)
	{
		static auto ofunc = RenderView_hook.get_original<SceneEnd>(index::SceneEnd);

		if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
			return;

		if (g_Unload)
			return;

		//if (g_ClientState->m_nDeltaTick != -1) return;
		// |
		// v
		// code here

		//works
		/*
		if (g_Config.chams_local && g_LocalPlayer->IsAlive() && g_Config.vis_misc_thirdperson) {
			const auto clr_front = Color(g_Config.color_chams_local_visible);
			const auto flat = g_Config.chams_mode_local == 1;
			const auto metallic = g_Config.chams_mode_local == 4;
			const auto glow = g_Config.chams_mode_local == 5;
			const auto wireframe = g_Config.chams_mode_local == 2;
			const auto glass = g_Config.chams_mode_local == 3;

			if (g_Config.chams_local_scope && g_LocalPlayer->m_bIsScoped())
			{
				g_RenderView->SetColorModulation(g_Config.color_chams_scoped_visible[0], g_Config.color_chams_scoped_visible[1], g_Config.color_chams_scoped_visible[2]);
				g_RenderView->SetBlend(g_Config.color_chams_scoped_visible[3]);
			}
			else
			{
				g_RenderView->SetColorModulation(g_Config.color_chams_local_visible[0], g_Config.color_chams_local_visible[1], g_Config.color_chams_local_visible[2]);
				g_RenderView->SetBlend(g_Config.color_chams_local_visible[3]);
			}
			if (g_Config.chams_local_scope && g_LocalPlayer->m_bIsScoped())
			{
				const auto clr_front = Color(g_Config.color_chams_scoped_visible);
				MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
			}
			else
			{
				MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
			}

			g_LocalPlayer->DrawModel(0x00000001, 255);
			g_MdlRender->ForcedMaterialOverride(nullptr);
		}
		*/

		if (g_Config.rbot_aa > 0 && g_Config.chams_local_ghost && g_LocalPlayer->IsAlive() && g_Config.vis_misc_thirdperson) {
			{
				const auto clr_front = Color(g_Config.color_chams_local_ghost_visible);
				const auto flat = g_Config.chams_mode_local_ghost == 1;
				const auto metallic = g_Config.chams_mode_local_ghost == 4;
				const auto glow = g_Config.chams_mode_local_ghost == 5;
				const auto wireframe = g_Config.chams_mode_local_ghost == 2;
				const auto glass = g_Config.chams_mode_local_ghost == 3;

				//////////////////////////////90 degrees aa $$$$$$$$$$$$$$$$$$$$$////////////////////////////////////
				auto Abs = g_LocalPlayer->m_angAbsAngles();

				g_LocalPlayer->SetAbsAngles(QAngle(Abs.pitch, g_Saver.real_angle, Abs.roll));//switch

				g_RenderView->SetColorModulation(g_Config.color_chams_local_ghost_visible[0], g_Config.color_chams_local_ghost_visible[1], g_Config.color_chams_local_ghost_visible[2]);
				g_RenderView->SetBlend(g_Config.color_chams_local_ghost_visible[3]);

				MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
				g_LocalPlayer->DrawModel(0x1, 255);
				g_MdlRender->ForcedMaterialOverride(nullptr);
				g_LocalPlayer->SetAbsAngles(Abs);
			}
		}

		if (g_LocalPlayer->m_vecVelocity().Length2D() > 10 && g_Config.chams_local_fakelag && g_Config.misc_fakelag_enable && g_LocalPlayer->IsAlive() && g_Config.vis_misc_thirdperson) {

			const auto clr_front = Color(g_Config.color_chams_local_fakelag_visible);
			const auto flat = g_Config.chams_mode_local_fakelag == 1;
			const auto metallic = g_Config.chams_mode_local_fakelag == 4;
			const auto glow = g_Config.chams_mode_local_fakelag == 5;
			const auto wireframe = g_Config.chams_mode_local_fakelag == 2;
			const auto glass = g_Config.chams_mode_local_fakelag == 3;

			//////////////////////////////SHIT BRICKS////////////////////////////////////

			static auto last_networked_origin = Vector(0, 0, 0);
			if (g_Saver.OLDsendpacket)
			{
				last_networked_origin = g_LocalPlayer->m_vecOrigin();
			}

			g_LocalPlayer->SetAbsOriginal(last_networked_origin);

			g_RenderView->SetColorModulation(g_Config.color_chams_local_fakelag_visible[0], g_Config.color_chams_local_fakelag_visible[1], g_Config.color_chams_local_fakelag_visible[2]);
			g_RenderView->SetBlend(g_Config.color_chams_local_fakelag_visible[3]);

			MaterialManager::Get().OverrideMaterial(false, flat, wireframe, glass, metallic, glow, clr_front);
			g_LocalPlayer->DrawModel(0x1, 255);
			g_MdlRender->ForcedMaterialOverride(nullptr);

		}

		return ofunc(pEcx, pEdx);
	}
	bool __stdcall hkFireEvent(IGameEvent* pEvent)
	{
		static auto oFireEvent = gameevents_hook.get_original<FireEvent>(index::FireEvent);

		if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
			return oFireEvent(g_GameEvents, pEvent);

		// -->

		if (!strcmp(pEvent->GetName(), "player_death") && g_EngineClient->GetPlayerForUserID(pEvent->GetInt("attacker")) == g_EngineClient->GetLocalPlayer()) {
			auto& weapon = g_LocalPlayer->m_hActiveWeapon();
			if (weapon && weapon->IsWeapon()) {
				auto& skin_data = g_Config.skins.m_items[weapon->m_Item().m_iItemDefinitionIndex()];
				if (skin_data.enabled && skin_data.stat_trak) {
					skin_data.stat_trak++;
					weapon->m_nFallbackStatTrak() = skin_data.stat_trak;
					weapon->GetClientNetworkable()->PostDataUpdate(0);
					weapon->GetClientNetworkable()->OnDataChanged(0);
				}
			}
			const auto icon_override = g_Config.skins.get_icon_override(pEvent->GetString("weapon"));
			if (icon_override) {
				pEvent->SetString("weapon", icon_override);
			}
		}

		Rbot::Get().OnFireEvent(pEvent);
		Resolver::Get().OnFireEvent(pEvent);

		if (!strcmp(pEvent->GetName(), "round_start"))
			BuyBot::Get().OnRoundStart();

		HitPossitionHelper::Get().OnFireEvent(pEvent);

		return oFireEvent(g_GameEvents, pEvent);
	}
	void __stdcall Hooked_RenderSmokeOverlay(bool unk) { }
	int __fastcall SendDatagram_h(INetChannel* netchan, void*, bf_write* datagram)
	{
		static auto ofunc = clientstate_hook.get_original<SendDatagram_t>(index::SendDatagram);

		if (g_Config.misc_request_file && InputSys::Get().IsKeyDown(g_Config.misc_request_file_key) && netchan && g_LocalPlayer && !g_GameRules->m_bIsValveDS())
		{
			netchan->set_timeout(3600);
			for (int j = 0; j < 4000; j++) { netchan->request_file(".txt", false); }
			return ofunc(netchan, datagram);
		}

		if (!g_Config.misc_ping_spike)
			return ofunc(netchan, datagram);

		int instate = netchan->m_nInReliableState;
		int insequencenr = netchan->m_nInSequenceNr;

		if (g_LocalPlayer && g_LocalPlayer->IsAlive() && g_Config.misc_ping_spike)
		{
			netchan->m_nOutSequenceNr *= g_Config.misc_ping_spike_val; //ebat ya ohuenny
		}

		int ret = ofunc(netchan, datagram);

		netchan->m_nInReliableState = instate;
		netchan->m_nInSequenceNr = insequencenr;
		return ret;
	}
	bool __fastcall WriteUsercmdDeltaToBuffer_h(IBaseClientDLL* ECX, void* EDX, int nSlot, bf_write* buf, int from, int to, bool isNewCmd)
	{
		static auto ofunc = hlclient_hook.get_original<WriteUsercmdDeltaToBuffer_t>(index::WriteUsercmdDeltaToBuffer);

		//no.

		return ofunc(ECX, nSlot, buf, from, to, isNewCmd);
	}
	void __stdcall FireBullets_PostDataUpdate(C_TEFireBullets* thisptr, DataUpdateType_t updateType)
	{
		static auto ofunc = firebullets_hook.get_original<FireBullets>(index::FireBullets);

		if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || g_Unload)
			return ofunc(thisptr, updateType);

		if (g_Config.rbot_lagcompensation && thisptr)
		{
			int iPlayer = thisptr->m_iPlayer + 1;
			if (iPlayer < 64)
			{
				auto player = C_BasePlayer::GetPlayerByIndex(iPlayer);

				if (player && player != g_LocalPlayer && !player->IsDormant() && player->m_iTeamNum() != g_LocalPlayer->m_iTeamNum())
				{
					QAngle eyeAngles = QAngle(thisptr->m_vecAngles.pitch, thisptr->m_vecAngles.yaw, thisptr->m_vecAngles.roll);
					QAngle calcedAngle = Math::CalcAngle(player->GetEyePos(), g_LocalPlayer->GetEyePos());

					thisptr->m_vecAngles.pitch = calcedAngle.pitch;
					thisptr->m_vecAngles.yaw = calcedAngle.yaw;
					thisptr->m_vecAngles.roll = 0.f;

					float
						event_time = g_GlobalVars->tickcount,
						player_time = player->m_flSimulationTime();

					// Extrapolate tick to hit scouters etc
					auto lag_records = LagCompensation::Get().m_LagRecord[iPlayer];

					float shot_time = TICKS_TO_TIME(event_time);
					for (auto& record : lag_records)
					{
						if (record.m_iTickCount <= event_time)
						{
							shot_time = record.m_flSimulationTime + TICKS_TO_TIME(event_time - record.m_iTickCount); // also get choked from this
							char found1[256];
							sprintf(found1, "Found <<exact>> shot time: %f, ticks choked to get here: %d", shot_time, event_time - record.m_iTickCount);
							g_Logger.Backtrack("BACKTRACK", found1);
							//g_CVar->ConsoleColorPrintf(Color(0, 255, 0, 255), "Found <<exact>> shot time: %f, ticks choked to get here: %d\n", shot_time, event_time - record.m_iTickCount);
							break;
						}
						else
						{
							char found2[256];
							sprintf(found2, "Bad curtime difference, EVENT: %f, RECORD: %f", event_time, record.m_iTickCount);
							g_Logger.Backtrack("BACKTRACK", found2);
						}
					}

					g_CVar->ConsolePrintf("Calced angs: %f %f, Event angs: %f %f, CURTIME_TICKOUNT: %f, SIMTIME: %f, CALCED_TIME: %f\n", calcedAngle.pitch, calcedAngle.yaw, eyeAngles.pitch, eyeAngles.yaw, event_time, player_time, shot_time);

					if (!lag_records.empty())
					{
						int choked = floorf((event_time - player_time) / g_GlobalVars->interval_per_tick) + 0.5;
						choked = (choked > 14 ? 14 : choked < 1 ? 0 : choked);
						player->m_vecOrigin() = (lag_records.begin()->m_vecOrigin + (g_GlobalVars->interval_per_tick * lag_records.begin()->m_vecVelocity * choked));
					}

					LagCompensation::Get().SetOverwriteTick(player, calcedAngle, shot_time, 1);
				}
			}
		}

		ofunc(thisptr, updateType);
	}
	__declspec (naked) void __stdcall hkTEFireBulletsPostDataUpdate(DataUpdateType_t updateType)
	{
		__asm
		{
			push[esp + 4]
			push ecx
			call FireBullets_PostDataUpdate
			retn 4
		}
	}
	void CL_ParseEventDelta(void* RawData, void* pToData, RecvTable* pRecvTable)
	{
		// "RecvTable_DecodeZeros: table '%s' missing a decoder.", look at the function that calls it.
		static uintptr_t CL_ParseEventDeltaF = (uintptr_t)Utils::PatternScan(GetModuleHandle(L"engine.dll"), ("55 8B EC 83 E4 F8 53 57"));
		__asm
		{
			mov     ecx, RawData
			mov     edx, pToData
			push	pRecvTable
			call    CL_ParseEventDeltaF
			add     esp, 4
		}
	}
	bool __fastcall hkTempEntities(void* ECX, void* EDX, void* msg)
	{
		if (!g_LocalPlayer || !g_EngineClient->IsInGame() || !g_EngineClient->IsConnected())
			return o_TempEntities(ECX, msg);

		bool ret = o_TempEntities(ECX, msg);

		if (!g_Config.rbot_lagcompensation || !g_LocalPlayer->IsAlive() || g_Unload)
			return ret;

		CEventInfo* ei = g_ClientState->events;
		CEventInfo* next = NULL;

		if (!ei)
			return ret;

		// Filtering events
		do
		{
			next = *(CEventInfo**)((uintptr_t)ei + 0x38);

			uint16_t classID = ei->classID - 1;

			auto m_pCreateEventFn = ei->pClientClass->m_pCreateEventFn; // ei->pClientClass->m_pCreateEventFn ptr
			if (!m_pCreateEventFn)
				continue;

			IClientNetworkable* pCE = m_pCreateEventFn();
			if (!pCE)
				continue;

			if (classID == (int)ClassId::CTEFireBullets)
			{
				// set fire_delay to zero to send out event so its not here later.
				ei->fire_delay = 0.0f;

				auto pRecvTable = ei->pClientClass->m_pRecvTable;
				void* BasePtr = pCE->GetDataTableBasePtr();

				// Decode data into client event object and use the DTBasePtr to get the netvars
				CL_ParseEventDelta(ei->pData, BasePtr, pRecvTable);

				if (!BasePtr)
					continue;

				// This nigga right HERE just fired a BULLET MANE
				int EntityIndex = *(int*)((uintptr_t)BasePtr + 0x10) + 1;

				auto pEntity = (C_BasePlayer*)g_EntityList->GetClientEntity(EntityIndex);
				if (pEntity && pEntity->GetClientClass() && pEntity->GetClientClass()->m_ClassID == ClassId::CCSPlayer && !(pEntity->m_iTeamNum() == g_LocalPlayer->m_iTeamNum()))  //!pEntity->IsTeamMate())
				{
					QAngle EyeAngles = QAngle(*(float*)((uintptr_t)BasePtr + 0x24), *(float*)((uintptr_t)BasePtr + 0x28), 0.0f),
						CalcedAngle = Math::CalcAngle(pEntity->GetEyePos(), g_LocalPlayer->GetEyePos());

					*(float*)((uintptr_t)BasePtr + 0x24) = CalcedAngle.pitch;
					*(float*)((uintptr_t)BasePtr + 0x28) = CalcedAngle.yaw;
					*(float*)((uintptr_t)BasePtr + 0x2C) = 0;

					float
						event_time = TICKS_TO_TIME(g_GlobalVars->tickcount),
						player_time = pEntity->m_flSimulationTime();

					// Extrapolate tick to hit scouters etc
					auto lag_records = LagCompensation::Get().m_LagRecord[pEntity->EntIndex()];

					float shot_time = event_time;
					for (auto& record : lag_records)
					{
						if (TICKS_TO_TIME(record.m_iTickCount) <= event_time)
						{
							shot_time = record.m_flSimulationTime + (event_time - TICKS_TO_TIME(record.m_iTickCount)); // also get choked from this
							g_CVar->ConsoleColorPrintf(Color(0, 255, 0, 255), "Found exact shot time: %f, ticks choked to get here: %d\n", shot_time, TIME_TO_TICKS(event_time - TICKS_TO_TIME(record.m_iTickCount)));
							//#endif
							break;
						}
						else
							g_CVar->ConsolePrintf("Bad curtime difference, EVENT: %f, RECORD: %f\n", event_time, TICKS_TO_TIME(record.m_iTickCount));
						//#endif
					}
#ifdef _DEBUG
					g_CVar->ConsolePrintf("Calced angs: %f %f, Event angs: %f %f, CURTIME_TICKOUNT: %f, SIMTIME: %f, CALCED_TIME: %f\n", CalcedAngle.pitch, CalcedAngle.yaw, EyeAngles.pitch, EyeAngles.yaw, event_time, player_time, shot_time);
#endif
					if (!lag_records.empty())
					{
						int choked = floorf((event_time - player_time) / g_GlobalVars->interval_per_tick) + 0.5;
						choked = (choked > 14 ? 14 : choked < 1 ? 0 : choked);
						pEntity->m_vecOrigin() = (lag_records.begin()->m_vecOrigin + (g_GlobalVars->interval_per_tick * lag_records.begin()->m_vecVelocity * choked));
					}

					LagCompensation::Get().SetOverwriteTick(pEntity, CalcedAngle, shot_time, 1);
				}

			}
			ei = next;
		} while (next != NULL);

		return ret;
	}
}

/*
bool __fastcall WriteUsercmdDeltaToBuffer_h(IBaseClientDLL* ECX, void* EDX, int nSlot, bf_write* buf, int from, int to, bool isNewCmd)
	{
		static auto ofunc = hlclient_hook.get_original<WriteUsercmdDeltaToBuffer_t>(index::WriteUsercmdDeltaToBuffer);

		static DWORD WriteUsercmdDeltaToBufferReturn = (DWORD)Utils::PatternScan(GetModuleHandleA("engine.dll"), "84 C0 74 04 B0 01 EB 02 32 C0 8B FE 46 3B F3 7E C9 84 C0 0F 84 ? ? ? ?");

		if (tickbaseshift <= 0 || (DWORD)_ReturnAddress() != WriteUsercmdDeltaToBufferReturn)
			return ofunc(ECX, nSlot, buf, from, to, isNewCmd);

		if (from != -1) return true;

		auto CL_SendMove = []()
		{
			using CL_SendMove_t = void(__fastcall*)(void);
			static CL_SendMove_t CL_SendMoveF = (CL_SendMove_t)Utils::PatternScan(GetModuleHandleA("engine.dll"), "55 8B EC A1 ? ? ? ? 81 EC ? ? ? ? B9 ? ? ? ? 53 8B 98");
			CL_SendMoveF();
		};

		auto WriteUsercmd = [](bf_write* buf, CUserCmd* in, CUserCmd* out)
		{
			using WriteUsercmd_t = void(__fastcall*)(bf_write*, CUserCmd*, CUserCmd*);
			static WriteUsercmd_t WriteUsercmdF = (WriteUsercmd_t)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D");
			WriteUsercmdF(buf, in, out);
		};

		int* pNumBackupCommands = (int*)(reinterpret_cast<uintptr_t>(buf) - 0x30);
		int* pNumNewCommands = (int*)(reinterpret_cast<uintptr_t>(buf) - 0x2C);
		auto net_channel = g_ClientState->m_NetChannel;
		if (!net_channel) return ofunc(ECX, nSlot, buf, from, to, isNewCmd);
		int32_t new_commands = *pNumNewCommands;

		if (!insendmove)
		{
			if (!new_commands)
				return false;

			insendmove = true;
			firstsendmovepack = true;
			tickbaseshift += new_commands;

			while (tickbaseshift > 0)
			{
				CL_SendMove();
				net_channel->Transmit(false);
				firstsendmovepack = false;
			}

			insendmove = false;
			return false;
		}

		if (!firstsendmovepack)
		{
			int32_t loss = Math::Minimum(tickbaseshift, 10);

			tickbaseshift -= loss;
			net_channel->m_nOutSequenceNr += loss;
		}

		int32_t next_cmdnr = g_ClientState->lastoutgoingcommand + g_ClientState->chokedcommands + 1;
		int32_t total_new_commands = Math::Minimum(tickbaseshift, 62);
		tickbaseshift -= total_new_commands;

		from = -1;
		*pNumNewCommands = total_new_commands;
		*pNumBackupCommands = 0;

		for (to = next_cmdnr - new_commands + 1; to <= next_cmdnr; to++)
		{
			if (!ofunc(ECX, nSlot, buf, from, to, true))
				return false;

			from = to;
		}

		CUserCmd* last_realCmd = g_Input->pGetUserCmd(nSlot, from);
		CUserCmd fromCmd;

		if (last_realCmd)
			fromCmd = *last_realCmd;

		CUserCmd toCmd = fromCmd;
		toCmd.command_number = next_cmdnr++;
		toCmd.tick_count += (TIME_TO_TICKS(0.5f) + next_cmdnr);

		auto choked = g_EngineClient->GetNetChannel()->m_nChokedPackets;

		for (int i = new_commands; i <= total_new_commands; i++)
		{
			WriteUsercmd(buf, &toCmd, &fromCmd);
			fromCmd = toCmd;
			toCmd.command_number = next_cmdnr++;
			toCmd.tick_count += (TIME_TO_TICKS(0.5f) + i);
			if (total_new_commands > 1 && i != (total_new_commands - 1))
				choked++;
		}

		g_EngineClient->GetNetChannel()->m_nChokedPackets = choked;
		CL_SendMove();

		net_channel->Transmit(true);

		g_ClientState->lastoutgoingcommand = 1;
		g_EngineClient->GetNetChannel()->m_nChokedPackets = 0;

		return true;
	}
*/

/*
int __fastcall SendDatagram_h(INetChannel* netchan, void*, bf_write* datagram)
	{
		static auto ofunc = clientstate_hook.get_original<SendDatagram_t>(index::SendDatagram);

		if (g_Config.misc_request_file && InputSys::Get().IsKeyDown(g_Config.misc_request_file_key) && netchan && g_LocalPlayer && !g_GameRules->m_bIsValveDS())
		{
			netchan->set_timeout(3600);
			for (int j = 0; j < 4000; j++) { netchan->request_file(".txt", false); }
			return ofunc(netchan, datagram);
		}

		if (g_Config.misc_cripwalk && InputSys::Get().IsKeyDown(g_Config.misc_cripwalk_key) && netchan && g_LocalPlayer)
		{
			Misc::Get().CripWalk(netchan);
			return ofunc(netchan, datagram);
		}

		if (!g_Config.misc_ping_spike)
			return ofunc(netchan, datagram);

		int instate = netchan->m_nInReliableState;
		int insequencenr = netchan->m_nInSequenceNr;

		if (g_LocalPlayer && g_LocalPlayer->IsAlive() && g_Config.misc_ping_spike)
		{
			netchan->m_nOutSequenceNr *= g_Config.misc_ping_spike_val;
		}

		int ret = ofunc(netchan, datagram);

		netchan->m_nInReliableState = instate;
		netchan->m_nInSequenceNr = insequencenr;
		return ret;
	}
*/