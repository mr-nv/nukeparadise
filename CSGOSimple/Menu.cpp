#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING //for new vs
#include "Menu.h"
#include "Hooks.hpp"
#include "features/Lobby.hpp"
#include "features/ClantagChanger.h"
#include "config.hpp"
#include "Misc.h"
#include "ConsoleHelper.h"
#include "options.hpp"
#include <experimental/filesystem>
#include "ui.hpp"
#include "RuntimeSaver.h"
#include "valve_sdk/sdk.hpp"
#include "render.hpp"
#include "features/EventLogger.h"
#include "pic.h"
namespace fs = std::experimental::filesystem;
ImFont* IconsFont;
Menu::Menu() {  }
float window_alpha = 0.f;
void DrawKzWindow()
{
	if (g_EngineClient->IsInGame() && g_LocalPlayer)
	{
		if (!g_LocalPlayer->IsAlive()) return;
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.25f));
		ImGui::SetNextWindowPos(ImVec2(((ImGui::GetIO().DisplaySize.x / 2) - ImGui::GetIO().DisplaySize.x / 20 * 1), (ImGui::GetIO().DisplaySize.y / 4) * 3), ImGuiCond_Once);
		ImGui::Begin("kz-show-kz", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
		{
			auto netchannel = g_EngineClient->GetNetChannelInfo();

			if (netchannel)
			{
				int incoming = (int)(netchannel->GetLatency(FLOW_INCOMING) * 1000);
				int outgoing = (int)(netchannel->GetLatency(FLOW_OUTGOING) * 1000);

				static float fMaxPspeed = 0.f;
				int choked = g_EngineClient->GetNetChannel()->m_nChokedPackets;
				if (g_LocalPlayer->m_vecVelocity().Length2D() == 0)
					fMaxPspeed = 0.0;
				if (g_LocalPlayer->m_vecVelocity().Length2D() > fMaxPspeed)
					fMaxPspeed = g_LocalPlayer->m_vecVelocity().Length2D();

				ImGui::Text("Speed: %.3f", g_LocalPlayer->m_vecVelocity().Length2D());
				ImGui::Text("SpeedMax: %.3f", fMaxPspeed);
				ImGui::Text("Tickbase: %d", g_LocalPlayer->m_nTickBase());
				ImGui::Text("FlowIn: %d", incoming);
				ImGui::Text("FlowOut: %d", outgoing);
				ImGui::Text("LBY: %.3f", g_LocalPlayer->m_flLowerBodyYawTarget());
				if (choked <= 0)
				{
					ImGui::TextColored(ImVec4(255, 255, 255, 255), "Choked: %d", choked);
				}
				else if (!(choked > 14))
				{
					ImGui::TextColored(ImVec4(0, 255, 0, 255), "Choked: %d", choked);
				}
				else
				{
					ImGui::TextColored(ImVec4(255, 0, 0, 255), "Choked: %d", choked);
				}
				int xd = Utils::pWeaponType();
				ImGui::Text("WeaponType: %d", xd);
			}
			ImGui::End();
		}
		ImGui::PopStyleColor();
	}
}
void background()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once); ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Once);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.09f, 0.09f, 0.2f / 1.f * window_alpha)); ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.09f, 0.09f, 0.09f, 0.40f / 1.f * window_alpha));
	static const auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus;
	ImGui::Begin("##background", nullptr, flags);

	//ImGui::Image(Menu::Get().GetTexture(), ImVec2(700, 700));

	ImGui::End(); ImGui::PopStyleColor();
}
void Menu::Initialize() { ImGuiIO& io = ImGui::GetIO(); Loaded = true; }
IDirect3DTexture9* Menu::GetTexture() {
	return this->Texture;
}
void Menu::TextureInit(IDirect3DDevice9* pDevice) {
	//D3D9EX GOLD FIX
	D3DXCreateTextureFromFileInMemoryEx(pDevice
		, &pic, sizeof(pic),
		700, 700, D3DUSAGE_DYNAMIC, 0, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &Texture);
	g_Saver.LoadTexture = true;
}
void Menu::Render()
{
	if (!Loaded || g_Unload || !g_Saver.LoadTexture) return;

	MenuHelper::Get().CreateStyle();

	if (g_Config.vis_misc_kzwindow) DrawKzWindow();

	if (MenuHelper::Get()._visible) { if (window_alpha < 1.f) window_alpha += 0.02f; }
	else { window_alpha -= 0.02f; if (window_alpha <= 0.f) return; }
	if (window_alpha <= 0.f) window_alpha = 0.02f; else if (window_alpha > 1.f) window_alpha = 1.f;

	background();

	const auto old_alpha = ImGui::GetStyle().Alpha; ImGui::GetStyle().Alpha = window_alpha;

	ImGui::SetNextWindowSize(ImVec2{ 650, 469 }, ImGuiSetCond_Once);

	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.30f, 0.30f, 0.30f, 0.61f)); ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.06f, 0.06f, 0.94f));

	if (ImGui::Begin(Utils::Format("nukeparadise - built on %s %s", __DATE__, __TIME__),
		NULL,
		ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {

		if (ImGui::BeginTabBar("Main", ImGuiTabBarFlags_NoTooltip)) {
			if (ImGui::BeginTabItem("Ragebot")) { RenderRagebot(); ImGui::EndTabItem(); }
			if (ImGui::BeginTabItem("Legitbot")) { RenderLegitbot(); ImGui::EndTabItem(); }
			if (ImGui::BeginTabItem("Visuals")) { RenderVisuals(); ImGui::EndTabItem(); }
			if (ImGui::BeginTabItem("Skin Changer")) { RenderSkins(); ImGui::EndTabItem(); }
			if (ImGui::BeginTabItem("Player List")) { RenderList(); ImGui::EndTabItem(); }
			if (ImGui::BeginTabItem("Miscellaneous")) { RenderMisc(); ImGui::EndTabItem();  ImGui::EndTabBar(); }
		}

	}

	ImGui::GetStyle().Alpha = old_alpha;
}
static int weapon_index = 7;
static auto definition_vector_index = 0;
static std::map<int, const char*> k_weapon_names =
{
{ 7, "AK-47" },
{ 8, "AUG" },
{ 9, "AWP" },
{ 63, "CZ75 Auto" },
{ 1, "Desert Eagle" },
{ 2, "Dual Berettas" },
{ 10, "FAMAS" },
{ 3, "Five-SeveN" },
{ 11, "G3SG1" },
{ 13, "Galil AR" },
{ 4, "Glock-18" },
{ 14, "M249" },
{ 60, "M4A1-S" },
{ 16, "M4A4" },
{ 17, "MAC-10" },
{ 27, "MAG-7" },
{ 23, "MP5-SD" },
{ 33, "MP7" },
{ 34, "MP9" },
{ 28, "Negev" },
{ 35, "Nova" },
{ 32, "P2000" },
{ 36, "P250" },
{ 19, "P90" },
{ 26, "PP-Bizon" },
{ 64, "R8 Revolver" },
{ 29, "Sawed-Off" },
{ 38, "SCAR-20" },
{ 40, "SSG 08" },
{ 39, "SG 553" },
{ 30, "Tec-9" },
{ 24, "UMP-45" },
{ 61, "USP-S" },
{ 25, "XM1014" },
};
void RenderCurrentWeaponButton()
{
	if (g_Config.lbot_set_automatically_weapon || !g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
	{
		return;
	}
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon || !weapon->IsWeapon()) {
		return;
	}
	if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_KNIFE) return;
	if (ImGui::Button("Current")) {
		weapon_index = weapon->m_Item().m_iItemDefinitionIndex();
	}
}
void SetWeapon()
{
	if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
	{
		return;
	}
	auto weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon || !weapon->IsWeapon()) {
		return;
	}
	if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_KNIFE) return;
	if (weapon->m_Item().m_iItemDefinitionIndex() != weapon_index)
	{
		weapon_index = weapon->m_Item().m_iItemDefinitionIndex();
	}
}
void Menu::RenderLegitbot()
{
	ImGui::Columns(3, NULL, false);
	ImGui::BeginChild("##aimbot.weapons", ImVec2(0, 87.f), true);
	{
		ImGui::Text("Weapons");
		ImGui::Separator();

		ImGui::Checkbox("Set automatically", &g_Config.lbot_set_automatically_weapon);

		const char* item_current = k_weapon_names[weapon_index];
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
		ImGui::PushItemWidth(127);
		if (g_Config.lbot_set_automatically_weapon)
		{
			SetWeapon();
		}
		else if (ImGui::BeginCombo("##weaponslist", item_current, ImGuiComboFlags_HeightLarge))
		{
			for (auto weapon : k_weapon_names)
			{
				bool is_selected = (item_current = k_weapon_names[weapon_index]);

				if (is_selected)
					ImGui::SetItemDefaultFocus();

				if (ImGui::Selectable(weapon.second, weapon_index == weapon.first))
				{
					weapon_index = weapon.first;
					item_current = k_weapon_names[weapon_index];
				}
				// ne rabotaet PIDR !! suukAa
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
		ImGui::SameLine();
		RenderCurrentWeaponButton();
		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
	auto settings = &g_Config.lbot_items[weapon_index];
	ImGui::BeginChild("##aimbot.general", ImVec2(0, 316.f), true);
	{
		ImGui::Text("General");
		ImGui::Separator();
		ImGui::PushItemWidth(-1);
		ImGui::Checkbox("Master switch", &g_Config.lbot_ijustflippedaswitch);
		ImGui::Checkbox("Enabled", &settings->enabled);
		ImGui::Checkbox("Deathmatch", &settings->deathmatch);
		if (weapon_index == WEAPON_P250 ||
			weapon_index == WEAPON_USP_SILENCER ||
			weapon_index == WEAPON_GLOCK ||
			weapon_index == WEAPON_FIVESEVEN ||
			weapon_index == WEAPON_TEC9 ||
			weapon_index == WEAPON_DEAGLE ||
			weapon_index == WEAPON_ELITE ||
			weapon_index == WEAPON_HKP2000) {
			ImGui::Checkbox("Autopistol", &settings->autopistol);
		}
		ImGui::Checkbox("Autowall", &settings->autowall);
		ImGui::Checkbox("On Key", &settings->on_key);
		//ImGui::PushItemWidth(-1);
		if (settings->on_key) { ImGui::SameLine(); ImGui::Hotkey("##on_kero_kero", &g_Config.lbot_on_fire_key); }
		//ImGui::PopItemWidth();
		ImGui::Checkbox("Autofire", &settings->autofire);
		//ImGui::PushItemWidth(-1);
		if (settings->autofire) { ImGui::SameLine(); ImGui::Hotkey("##autofire_key", &g_Config.lbot_auto_fire_key); }
		//ImGui::PopItemWidth();

		ImGui::Checkbox("Auto Stop", &settings->autostop);
		ImGui::Checkbox("Check Smoke", &settings->check_smoke);
		ImGui::Checkbox("Check Flash", &settings->check_flash);
		ImGui::Checkbox("Check Jump", &settings->check_jump);
		ImGui::Checkbox("Backtrack", &settings->backtrack.enabled);
		ImGui::Checkbox("Silent", &settings->silent);
		ImGui::Checkbox("Humanize", &settings->humanize);
		if (weapon_index == WEAPON_AWP || weapon_index == WEAPON_SSG08) {
			ImGui::Checkbox("Only In Zoom", &settings->only_in_zoom);
		}
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("##aimbot.misc", ImVec2(0, 0), true);
	{
		ImGui::Text("Misc");
		ImGui::Separator();
		ImGui::PushItemWidth(-1);
		static char* priorities[] = {
		"Fov",
		"Health",
		"Damage",
		"Distance"
		};
		static char* aim_types[] = {
		"Hitbox",
		"Nearest"
		};
		static char* fov_types[] = {
		"Static",
		"Dynamic"
		};
		static char* hitbox_list[] = {
		"Head",
		"Neck",
		"Lower Neck",
		"Body",
		"Thorax",
		"Chest",
		"Right Thing",
		"Left Thing", // 7
		};
		ImGui::Text("Aim Type:");
		ImGui::Combo("##aimbot.aim_type", &settings->aim_type, aim_types, IM_ARRAYSIZE(aim_types));
		if (settings->aim_type == 0) {
			ImGui::Text("Hitbox:");
			ImGui::Combo("##aimbot.hitbox", &settings->hitbox, hitbox_list, IM_ARRAYSIZE(hitbox_list));
		}
		ImGui::Text("Priority:");
		ImGui::Combo("##aimbot.priority", &settings->priority, priorities, IM_ARRAYSIZE(priorities));
		ImGui::Text("Fov Type:");
		ImGui::Combo("##aimbot.fov_type", &settings->fov_type, fov_types, IM_ARRAYSIZE(fov_types));
		ImGui::SliderFloat("##aimbot.fov", &settings->fov, 0, 20, "Fov: %.2f");
		if (settings->silent) {
			ImGui::SliderFloat("##aimbot.silent_fov", &settings->silent_fov, 0, 20, "Silent Fov: %.2f");
		}
		ImGui::SliderFloat("##aimbot.smooth", &settings->smooth, 1, 15, "Smooth: %.2f");
		if (!settings->silent) {
			ImGui::SliderInt("##aimbot.shot_delay", &settings->shot_delay, 0, 100, "Shot Delay: %.0f");
		}
		ImGui::SliderInt("##aimbot.kill_delay", &settings->kill_delay, 0, 1000, "Kill Delay: %.0f");
		if (settings->backtrack.enabled) {			
			ImGui::SliderFloat("##aimbot_backtrack_time", &settings->backtrack.time, 0.f, 0.2f, "BackTrack Time: %.3f");		
		}
		if (settings->autowall) {
			ImGui::SliderInt("##aimbot.min_damage", &settings->min_damage, 1, 100, "Min Damage: %.0f");
		}
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("##aimbot.rcs", ImVec2(0, 0), true);
	{
		ImGui::Text("Recoil Control System");
		ImGui::Separator();
		ImGui::PushItemWidth(-1);
		ImGui::Checkbox("Enabled##aimbot.rcs", &settings->rcs);
		ImGui::Text("RCS Type:");
		static char* rcs_types[] = {
		"Standalone",
		"Aim"
		};
		ImGui::Combo("##aimbot.rcs_type", &settings->rcs_type, rcs_types, IM_ARRAYSIZE(rcs_types));
		ImGui::Checkbox("RCS Custom Fov", &settings->rcs_fov_enabled);
		if (settings->rcs_fov_enabled) {
			ImGui::SliderFloat("##aimbot.rcs_fov", &settings->rcs_fov, 0, 20, "RCS Fov: %.2f");
		}
		ImGui::Checkbox("RCS Custom Smooth", &settings->rcs_smooth_enabled);
		if (settings->rcs_smooth_enabled) {
			ImGui::SliderFloat("##aimbot.rcs_smooth", &settings->rcs_smooth, 1, 15, "RCS Smooth: %.2f");
		}
		ImGui::SliderInt("##aimbot.rcs_x", &settings->rcs_x, 0, 100, "RCS X: %.0f");
		ImGui::SliderInt("##aimbot.rcs_y", &settings->rcs_y, 0, 100, "RCS Y: %.0f");
		ImGui::SliderInt("##aimbot.rcs_start", &settings->rcs_start, 1, 30, "RCS Start: %.0f");
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
	ImGui::Columns(1, NULL, false);
}
void Menu::RenderSkins()
{
	if (k_skins.size() == 0)
		initialize_kits();

	auto& entries = g_Config.skins.m_items;
	ImGui::Columns(2, nullptr, false);
	ImGui::BeginChild("##skins.list", ImVec2(0, 0), true);
	{
		auto& selected_entry = entries[k_weapons_names[definition_vector_index].definition_index];
		selected_entry.definition_index = k_weapons_names[definition_vector_index].definition_index;
		selected_entry.definition_vector_index = definition_vector_index;
		ImGui::Checkbox("Enabled", &selected_entry.enabled);
		ImGui::InputInt("Seed", &selected_entry.seed);
		ImGui::InputInt("StatTrak", &selected_entry.stat_trak);
		ImGui::SliderFloat("Wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);
		if (selected_entry.definition_index != GLOVE_T_SIDE)
		{
			ImGui::Combo("Paint Kit", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_skins[idx].name.c_str();
					return true;
				}, nullptr, k_skins.size(), 10);
			selected_entry.paint_kit_index = k_skins[selected_entry.paint_kit_vector_index].id;
		}
		else
		{
			ImGui::Combo("Paint Kit", &selected_entry.paint_kit_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_gloves[idx].name.c_str();
					return true;
				}, nullptr, k_gloves.size(), 10);
			selected_entry.paint_kit_index = k_gloves[selected_entry.paint_kit_vector_index].id;
		}
		if (selected_entry.definition_index == WEAPON_KNIFE)
		{
			ImGui::Combo("Knife", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_knifes_names.at(idx).name;
					return true;
				}, nullptr, k_knifes_names.size(), 5);

			selected_entry.definition_override_index = k_knifes_names.at(selected_entry.definition_override_vector_index).definition_index;
		}
		else if (selected_entry.definition_index == GLOVE_T_SIDE)
		{
			ImGui::Combo("Glove", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
				{
					*out_text = k_gloves_names.at(idx).name;
					return true;
				}, nullptr, k_gloves_names.size(), 5);
			selected_entry.definition_override_index = k_gloves_names.at(selected_entry.definition_override_vector_index).definition_index;
		}
		else
		{
			static auto unused_value = 0;
			selected_entry.definition_override_vector_index = 0;
			ImGui::Combo("Unavailable", &unused_value, "For knives or gloves\0");
		}
		ImGui::InputText("Name Tag", selected_entry.custom_name, 32);
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	ImGui::BeginChild("##skins.misc", ImVec2(0, 0), true);
	{
		ImGui::PushItemWidth(-1);
		ImGui::ListBoxHeader("###config");
		{
			for (size_t w = 0; w < k_weapons_names.size(); w++) {
				if (ImGui::Selectable(k_weapons_names[w].name, definition_vector_index == w))
				{
					definition_vector_index = w;
				}
			}
		}
		ImGui::ListBoxFooter();
		if (ImGui::Button("Update"))
			g_Saver.RequestForceUpdate = true;
		ImGui::PopItemWidth();
	}
	ImGui::EndChild();
	ImGui::Columns(1, nullptr, false);
}
void WeaponMenu(int chidawaredotcc)
{
	ImGui::PushItemWidth(-1);
	ImGui::SliderFloat("##hitchance", &g_Config.rbot_min_hitchance[chidawaredotcc], 0.f, 100.f, "Hit chance: %.2f");
	ImGui::SliderFloat("##Mindamage", &g_Config.rbot_mindamage[chidawaredotcc], 0.f, 100.f, "Mininal damage: %.2f");
	ImGui::SliderInt("##baim", &g_Config.rbot_baim_after_shots[chidawaredotcc], 0, 10, "Baim after shots: %.f");
	ImGui::Separator();
	ImGui::Checkbox("Head:", &g_Config.rbot_hitbox_head[chidawaredotcc]); if (g_Config.rbot_hitbox_head[chidawaredotcc]) { ImGui::SameLine(); ImGui::SliderFloat(" ", &g_Config.rbot_hitbox_head_scale[chidawaredotcc], 0.f, 1.f, "Scale: %.2f"); }
	ImGui::Checkbox("Neck:", &g_Config.rbot_hitbox_neck[chidawaredotcc]); if (g_Config.rbot_hitbox_neck[chidawaredotcc]) { ImGui::SameLine(); ImGui::SliderFloat("  ", &g_Config.rbot_hitbox_neck_scale[chidawaredotcc], 0.f, 1.f, "Scale: %.2f"); }
	ImGui::Checkbox("Chest:", &g_Config.rbot_hitbox_chest[chidawaredotcc]); if (g_Config.rbot_hitbox_chest[chidawaredotcc]) { ImGui::SameLine(); ImGui::SliderFloat("   ", &g_Config.rbot_hitbox_chest_scale[chidawaredotcc], 0.f, 1.f, "Scale: %.2f"); }
	ImGui::Checkbox("Pelvis:", &g_Config.rbot_hitbox_pelvis[chidawaredotcc]); if (g_Config.rbot_hitbox_pelvis[chidawaredotcc]) { ImGui::SameLine(); ImGui::SliderFloat("    ", &g_Config.rbot_hitbox_pelvis_scale[chidawaredotcc], 0.f, 1.f, "Scale: %.2f"); }
	ImGui::Checkbox("Stomach:", &g_Config.rbot_hitbox_stomach[chidawaredotcc]); if (g_Config.rbot_hitbox_stomach[chidawaredotcc]) { ImGui::SameLine(); ImGui::SliderFloat("     ", &g_Config.rbot_hitbox_stomach_scale[chidawaredotcc], 0.f, 1.f, "Scale: %.2f"); }
	ImGui::Checkbox("Arm:", &g_Config.rbot_hitbox_arm[chidawaredotcc]); if (g_Config.rbot_hitbox_arm[chidawaredotcc]) { ImGui::SameLine(); ImGui::SliderFloat("      ", &g_Config.rbot_hitbox_arm_scale[chidawaredotcc], 0.f, 1.f, "Scale: %.2f"); }
	ImGui::Checkbox("Leg:", &g_Config.rbot_hitbox_leg[chidawaredotcc]); if (g_Config.rbot_hitbox_leg[chidawaredotcc]) { ImGui::SameLine();  ImGui::SliderFloat("       ", &g_Config.rbot_hitbox_leg_scale[chidawaredotcc], 0.f, 1.f, "Scale: %.2f"); }
	ImGui::Checkbox("Foot:", &g_Config.rbot_hitbox_foot[chidawaredotcc]); if (g_Config.rbot_hitbox_foot[chidawaredotcc]) { ImGui::SameLine();  ImGui::SliderFloat("        ", &g_Config.rbot_hitbox_foot_scale[chidawaredotcc], 0.f, 1.f, "Scale: %.2f"); }
	ImGui::PopItemWidth();

}
void Menu::RenderRagebot()
{
	static const char* LagCompMode[] = { "Best", "Newest", "All" };
	const char* BaimModes[] = { "Never", "Auto" };
	const char* ShootingModes[] = { "Normal mode", "In fakelag mode", "Fakelag while shooting" };

	ImGui::Columns(2, NULL, false); ImGui::BeginChild("#basicrage", ImVec2(0, 0), true);

	ImGui::Checkbox("Enabled switch", &g_Config.rbot_enable);
	ImGui::Checkbox("Baim in move", &g_Config.rbot_baim_while_moving);
	ImGui::Checkbox("Auto scope", &g_Config.rbot_autoscope);
	ImGui::Checkbox("Auto stop", &g_Config.rbot_autostop);
	//ImGui::Checkbox("Aim step", &g_Config.rbot_aimstep);
	ImGui::Checkbox("Auto crouch", &g_Config.rbot_autocrouch);

	ImGui::Checkbox("Fake duck", &g_Config.rbot_fakeduck); if (g_Config.rbot_fakeduck)
	{
		ImGui::SameLine();
		ImGui::Hotkey("", &g_Config.rbot_fakeduck_key);
	}

	ImGui::Combo("Shooting", &g_Config.rbot_shooting_mode, ShootingModes, IM_ARRAYSIZE(ShootingModes));
	ImGui::Checkbox("pUnlag", &g_Config.rbot_force_unlage);


	ImGui::Checkbox("Lag compensation", &g_Config.rbot_lagcompensation);
	if (g_Config.rbot_lagcompensation) ImGui::Combo("Records", &g_Config.rbot_lagcompensation_type, LagCompMode, IM_ARRAYSIZE(LagCompMode));
	ImGui::Checkbox("Animation fix", &g_Config.rbot_guccigang);
	ImGui::Checkbox("Extrapolation", &g_Config.rbot_extrapolation);
	ImGui::Checkbox("Resolver", &g_Config.rbot_resolver);

	ImGui::Combo("Baim mode", &g_Config.rbot_baimmode, BaimModes, IM_ARRAYSIZE(BaimModes));
	ImGui::Checkbox("Air baim", &g_Config.rbot_resolver_air_baim);

	ImGui::EndChild(); 

	ImGui::NextColumn();

	ImGui::BeginChild("#fakelags", ImVec2(0.f, 87.f), true); ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

	ImGui::Checkbox("Enable fakelags", &g_Config.misc_fakelag_enable);
	ImGui::SameLine();
	ImGui::Checkbox("On key", &g_Config.misc_fakelag_on_key);
	if (g_Config.misc_fakelag_on_key)
	{
		ImGui::SameLine();
		ImGui::PushItemWidth(-1);
		ImGui::Hotkey("", &g_Config.misc_fakelag_key);
		ImGui::PopItemWidth();
	}

	static const char* FakelagModesMove[] = { "Normal Mode", "Adaptive Mode", "On peek Mode" };
	static const char* FakelagModesAir[] = { "Normal Mode", "Adaptive Mode" };

	if (ImGui::BeginTabBar("FakelagSelected", ImGuiTabBarFlags_NoTooltip))
	{
		if (ImGui::BeginTabItem("Standing"))
		{
			ImGui::PushItemWidth(-1);
			ImGui::SliderInt("##misc.fakelag_ticks", &g_Config.misc_fakelag_ticks_standing, 0, 14, "Ticks: %.0f");
			ImGui::PopItemWidth();

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Moving"))
		{
			ImGui::PushItemWidth(145);
			ImGui::Combo("##", &g_Config.misc_fakelag_mode_moving, FakelagModesMove, IM_ARRAYSIZE(FakelagModesMove));
			ImGui::PopItemWidth();

			ImGui::SameLine();

			ImGui::PushItemWidth(145);
			ImGui::SliderInt("##misc.fakelag_ticks", &g_Config.misc_fakelag_ticks_moving, 0, 14, "Ticks: %.0f");
			ImGui::PopItemWidth();

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Air"))
		{
			ImGui::PushItemWidth(145);
			ImGui::Combo("###", &g_Config.misc_fakelag_mode_air, FakelagModesAir, IM_ARRAYSIZE(FakelagModesAir));
			ImGui::PopItemWidth();

			ImGui::SameLine();

			ImGui::PushItemWidth(145);
			ImGui::SliderInt("##misc.fakelag_ticks", &g_Config.misc_fakelag_ticks_air, 0, 14, "Ticks: %.0f");
			ImGui::PopItemWidth();

			ImGui::EndTabItem();
		}
	}
	ImGui::EndTabBar();

	ImGui::EndChild();

	ImGui::BeginChild("#hitboxes", ImVec2(0.f, 316.f), true);
	{
		if (ImGui::BeginTabBar("##ebat4to)))", ImGuiTabBarFlags_NoTooltip))
		{
			if (ImGui::BeginTabItem("Pistol"))
			{
				WeaponMenu((int)RbotWeapons::PISTOL);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Heavy"))
			{
				WeaponMenu((int)RbotWeapons::HEAVY);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Rifle"))
			{
				WeaponMenu((int)RbotWeapons::RIFLE);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Smg"))
			{
				WeaponMenu((int)RbotWeapons::SMG);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Scout"))
			{
				WeaponMenu((int)RbotWeapons::SCOUT);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Auto"))
			{
				WeaponMenu((int)RbotWeapons::AUTO);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Awp"))
			{
				WeaponMenu((int)RbotWeapons::AWP);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();

		}
	}	
	ImGui::EndChild(); //////////////////////////////////////////////////////////////
	ImGui::Columns(1, NULL, false);
}
void Menu::RenderVisuals()
{
	ImGui::Columns(2, NULL, false);
	ImGui::BeginChild("#basicvisuals", ImVec2(0, 0), true);

	static const char* BacktrackChamsModes[] = { "Last tick", "All ticks" };
	static const char* ChamsTypes[] = { "Normal", "Flat", "Wireframe", "Glass", "Metallic", "Glow",  "Xqz", "Metallic xqz", "Flat xqz", "Glow xqz" };
	static const char* deaqChamsTypes[] = { "Normal", "Flat", "Wireframe", "Glass", "Metallic", "Glow" };
	static const char* GlowTypes[] = { "Outline outer", "Cover", "Outline inner" };
	static const char* BoxTypes[] = { "Normal", "Edge", "3D" };
	if (ImGui::BeginTabBar("Visuals", ImGuiTabBarFlags_NoTooltip))
	{
		if (ImGui::BeginTabItem("Local ESP"))
		{

			ImGui::Text("Chams:"); ImGui::Spacing();
			ImGui::ColorEdit3("##Chams visible", g_Config.color_chams_local_visible); ImGui::SameLine(); ImGui::ColorEdit3("##Chams invisible", g_Config.color_chams_local_invisible); ImGui::SameLine(); ImGui::Checkbox("Enable chams", &g_Config.chams_local);
			if (g_Config.chams_local)
			{
				ImGui::ColorEdit3("##color_chams_scoped_visible", g_Config.color_chams_scoped_visible); ImGui::SameLine(); ImGui::Checkbox("Chams on scope", &g_Config.chams_local_scope);
				ImGui::Combo("Chams type", &g_Config.chams_mode_local, ChamsTypes, IM_ARRAYSIZE(ChamsTypes));
			}
			ImGui::ColorEdit3("##Chams fakelags visible", g_Config.color_chams_local_fakelag_visible); ImGui::SameLine(); ImGui::Checkbox("Enable fakelag chams", &g_Config.chams_local_fakelag);
			if (g_Config.chams_local_fakelag)
			{
				ImGui::Combo("Chams fakelags type", &g_Config.chams_mode_local_fakelag, deaqChamsTypes, IM_ARRAYSIZE(deaqChamsTypes));
			}

			ImGui::ColorEdit3("##Chams ghost visible", g_Config.color_chams_local_ghost_visible); ImGui::SameLine(); ImGui::Checkbox("Enable ghost chams", &g_Config.chams_local_ghost);
			if (g_Config.chams_local_ghost)
			{
				ImGui::Combo("Chams ghost type", &g_Config.chams_mode_local_ghost, deaqChamsTypes, IM_ARRAYSIZE(deaqChamsTypes));
			}

			ImGui::ColorEdit3("##Chams visible arms", g_Config.color_chams_local_arms_visible); ImGui::SameLine(); ImGui::ColorEdit3("##Chams hidden arms", g_Config.color_chams_local_arms_invisible); ImGui::SameLine(); ImGui::Checkbox("Enable arm chams", &g_Config.chams_local_arms);
			if (g_Config.chams_local_arms)
			{
				ImGui::Combo("Chams arm type", &g_Config.chams_mode_local_arms, ChamsTypes, IM_ARRAYSIZE(ChamsTypes));
			}
			ImGui::ColorEdit3("##Chams visible weapon", g_Config.color_chams_local_weapon_visible); ImGui::SameLine(); ImGui::ColorEdit3("##Chams hidden weapon", g_Config.color_chams_local_weapon_invisible); ImGui::SameLine(); ImGui::Checkbox("Enable weapon chams", &g_Config.chams_local_weapon);
			if (g_Config.chams_local_weapon)
			{
				ImGui::Combo("Chams weapon type", &g_Config.chams_mode_local_weapon, ChamsTypes, IM_ARRAYSIZE(ChamsTypes));
			}
			ImGui::Separator(); ImGui::Text("Esp:"); ImGui::Spacing();

			ImGui::Checkbox("Enable esp", &g_Config.esp_local_enable);
			ImGui::ColorEdit3("##Box", g_Config.color_esp_local_boxes); ImGui::SameLine(); ImGui::Checkbox("Box ", &g_Config.esp_local_boxes);
			if (g_Config.esp_local_boxes)
			{
				ImGui::SameLine();
				ImGui::Checkbox("Box Outline", &g_Config.esp_local_boxes_outline);
			}
			ImGui::Combo("Box type", &g_Config.esp_local_boxes_type, BoxTypes, IM_ARRAYSIZE(BoxTypes));
			ImGui::ColorEdit3("##Box fill visible", g_Config.color_esp_local_boxes_fill); ImGui::SameLine(); ImGui::Checkbox("Box Fill", &g_Config.esp_local_boxes_fill);
			ImGui::ColorEdit3("##3dbox", g_Config.color_esp_local_headbox); ImGui::SameLine(); ImGui::Checkbox("Head 3d box", &g_Config.esp_local_headbox);
			ImGui::ColorEdit3("##Name", g_Config.color_esp_local_names); ImGui::SameLine(); ImGui::Checkbox("Name", &g_Config.esp_local_names);
			ImGui::Checkbox("Health ", &g_Config.esp_local_health);
			if (g_Config.esp_local_health)
			{
				ImGui::SameLine();
				ImGui::Checkbox("Dont render health if 100", &g_Config.esp_local_health_norender);
			}
			ImGui::ColorEdit3("##Armor", g_Config.color_esp_local_armour); ImGui::SameLine(); ImGui::Checkbox("Armor ", &g_Config.esp_local_armour);
			if (g_Config.esp_local_armour)
			{
				ImGui::SameLine();
				ImGui::Checkbox("Dont render armor if 100", &g_Config.esp_local_armour_norender);
			}
			ImGui::ColorEdit3("##flags", g_Config.color_esp_local_flags); ImGui::SameLine(); ImGui::Checkbox("Flags", &g_Config.esp_local_flags);

			ImGui::ColorEdit3("##Lines visible", g_Config.color_esp_local_anglines);
			ImGui::SameLine();
			ImGui::ColorEdit3("##Lines hidden", g_Config.color_esp_local_lbylines);
			ImGui::SameLine();
			ImGui::ColorEdit3("##Lines desync", g_Config.color_esp_local_desync);
			ImGui::SameLine();
			ImGui::Checkbox("Angle Lines (ang/lby/fake)", &g_Config.esp_local_lines);

			ImGui::ColorEdit3("##Weapon", g_Config.color_esp_local_weapons); ImGui::SameLine(); ImGui::Checkbox("Weapon", &g_Config.esp_local_weapons);
			if (g_Config.esp_local_weapons)
			{
				ImGui::SameLine(); ImGui::Checkbox("Ammo", &g_Config.esp_local_weapons_ammo);
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Enemy ESP"))
		{
			ImGui::Text("Chams:"); ImGui::Spacing();
			ImGui::ColorEdit3("##Chams visible", g_Config.color_chams_enemy_visible); ImGui::SameLine(); ImGui::ColorEdit3("##Chams hidden", g_Config.color_chams_enemy_invisible); ImGui::SameLine(); ImGui::Checkbox("Enable chams", &g_Config.chams_enemy);
			if (g_Config.chams_enemy)
			{
				ImGui::Combo("Chams type", &g_Config.chams_mode_enemy, ChamsTypes, IM_ARRAYSIZE(ChamsTypes));
			}
			ImGui::Separator(); ImGui::Text("Glow:"); ImGui::Spacing();
			ImGui::ColorEdit3("##Glow", g_Config.color_glow_enemy); ImGui::SameLine(); ImGui::Checkbox("Enable glow", &g_Config.glow_enemy_enable);
			if (g_Config.glow_enemy_enable)
			{
				ImGui::Combo("Type ", &g_Config.glow_enemy_type, GlowTypes, IM_ARRAYSIZE(GlowTypes));
			}
			ImGui::Separator(); ImGui::Text("Esp:"); ImGui::Spacing();

			ImGui::Checkbox("Enable esp", &g_Config.esp_enemy_enable);
			ImGui::Checkbox("Visible only", &g_Config.esp_enemy_visible_only);
			ImGui::ColorEdit3("##Box visible", g_Config.color_esp_enemy_visible_boxes); ImGui::SameLine(); ImGui::ColorEdit3("##Box hidden", g_Config.color_esp_enemy_invisible_boxes); ImGui::SameLine(); ImGui::Checkbox("Box ", &g_Config.esp_enemy_boxes);
			if (g_Config.esp_enemy_boxes) {
				ImGui::SameLine();
				ImGui::Checkbox("Box Outline", &g_Config.esp_enemy_boxes_outline);
			}
			ImGui::Combo("Box type", &g_Config.esp_enemy_boxes_type, BoxTypes, IM_ARRAYSIZE(BoxTypes));
			ImGui::ColorEdit3("##Box fill visible", g_Config.color_esp_enemy_visible_boxes_fill); ImGui::SameLine(); ImGui::ColorEdit3("##Box fill hidden", g_Config.color_esp_enemy_invisible_boxes_fill); ImGui::SameLine(); ImGui::Checkbox("Box Fill", &g_Config.esp_enemy_boxes_fill);
			ImGui::ColorEdit3("##3dbox vis", g_Config.color_esp_enemy_visible_headbox); ImGui::SameLine(); ImGui::ColorEdit3("##3dbox invis", g_Config.color_esp_enemy_invisible_headbox); ImGui::SameLine(); ImGui::Checkbox("Head 3d box", &g_Config.esp_enemy_headbox);
			ImGui::ColorEdit3("##Name visible", g_Config.color_esp_enemy_visible_names); ImGui::SameLine(); ImGui::ColorEdit3("##Name hidden", g_Config.color_esp_enemy_invisible_names); ImGui::SameLine(); ImGui::Checkbox("Name", &g_Config.esp_enemy_names);
			ImGui::Checkbox("Health ", &g_Config.esp_enemy_health);
			if (g_Config.esp_enemy_health)
			{
				ImGui::SameLine();
				ImGui::Checkbox("Dont render health if 100", &g_Config.esp_enemy_health_norender);
			}
			ImGui::ColorEdit3("##Armor visible", g_Config.color_esp_enemy_visible_armour); ImGui::SameLine(); ImGui::ColorEdit3("##Armor hidden", g_Config.color_esp_enemy_invisible_armour); ImGui::SameLine(); ImGui::Checkbox("Armor ", &g_Config.esp_enemy_armour);
			if (g_Config.esp_enemy_armour)
			{
				ImGui::SameLine();
				ImGui::Checkbox("Dont render armor if 100", &g_Config.esp_enemy_armour_norender);
			}
			ImGui::ColorEdit3("##Flags visible", g_Config.color_esp_enemy_visible_flags); ImGui::SameLine(); ImGui::ColorEdit3("##flags hidden", g_Config.color_esp_enemy_invisible_flags); ImGui::SameLine(); ImGui::Checkbox("Flags", &g_Config.esp_enemy_flags);
			if (g_Config.esp_enemy_flags)
			{
				ImGui::SameLine();
				ImGui::Checkbox("Debug Esp", &g_Config.esp_misc_debug_overlay);
			}
			ImGui::ColorEdit3("##Lines visible anglines", g_Config.color_esp_enemy_visible_anglines);
			ImGui::SameLine();
			ImGui::ColorEdit3("##Lines visible lbylines", g_Config.color_esp_enemy_visible_lbylines);
			ImGui::SameLine();
			ImGui::SameLine();
			ImGui::ColorEdit3("##Lines hidden anglines", g_Config.color_esp_enemy_invisible_anglines);
			ImGui::SameLine();
			ImGui::ColorEdit3("##Lines hidden lbylines", g_Config.color_esp_enemy_invisible_lbylines);
			ImGui::SameLine();
			ImGui::Checkbox("Angle Lines (ang/lby)", &g_Config.esp_enemy_lines);

			ImGui::ColorEdit3("##Weapon visible", g_Config.color_esp_enemy_visible_weapons); ImGui::SameLine(); ImGui::ColorEdit3("##Weapon hidden", g_Config.color_esp_enemy_invisible_weapons); ImGui::SameLine(); ImGui::Checkbox("Weapon", &g_Config.esp_enemy_weapons);
			if (g_Config.esp_enemy_weapons)
			{
				ImGui::SameLine(); ImGui::Checkbox("Ammo", &g_Config.esp_enemy_weapons_ammo);
			}
			ImGui::ColorEdit3("##Snapline visible", g_Config.color_esp_enemy_visible_snaplines); ImGui::SameLine(); ImGui::ColorEdit3("##Snapline hidden", g_Config.color_esp_enemy_invisible_snaplines); ImGui::SameLine(); ImGui::Checkbox("Snapline", &g_Config.esp_enemy_snaplines);
			ImGui::ColorEdit3("##lc visible", g_Config.color_esp_enemy_visible_lcline); ImGui::SameLine(); ImGui::ColorEdit3("##lc hidden", g_Config.color_esp_enemy_invisible_lcline); ImGui::SameLine(); ImGui::Checkbox("Backtrack line", &g_Config.esp_enemy_lclines);
			ImGui::ColorEdit3("##POV visible", g_Config.color_esp_enemy_visible_pov); ImGui::SameLine(); ImGui::ColorEdit3("##POV hidden", g_Config.color_esp_enemy_invisible_pov); ImGui::SameLine(); ImGui::Checkbox("Player out of view", &g_Config.esp_enemy_pov);
			ImGui::ColorEdit3("##Resolver info visible", g_Config.color_esp_enemy_visible_info); ImGui::SameLine(); ImGui::ColorEdit3("##Resolver info hidden", g_Config.color_esp_enemy_invisible_info); ImGui::SameLine(); ImGui::Checkbox("Resolver info", &g_Config.esp_enemy_info);
			ImGui::ColorEdit3("##Lby timer visible", g_Config.color_esp_enemy_visible_lby_timer); ImGui::SameLine(); ImGui::ColorEdit3("##Lby timer hidden", g_Config.color_esp_enemy_invisible_lby_timer); ImGui::SameLine(); ImGui::Checkbox("Lby timer", &g_Config.esp_enemy_lby_timer);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Team ESP"))
		{
			ImGui::Text("Chams:"); ImGui::Spacing();
			ImGui::ColorEdit3("##Chams visible", g_Config.color_chams_team_visible); ImGui::SameLine(); ImGui::ColorEdit3("##Chams hidden", g_Config.color_chams_team_invisible); ImGui::SameLine(); ImGui::Checkbox("Enable chams", &g_Config.chams_team);
			if (g_Config.chams_team)
			{
				ImGui::Combo("Chams type", &g_Config.chams_mode_team, ChamsTypes, IM_ARRAYSIZE(ChamsTypes));
			}
			ImGui::Separator(); ImGui::Text("Glow:"); ImGui::Spacing();
			ImGui::ColorEdit3("##Glow", g_Config.color_glow_team); ImGui::SameLine(); ImGui::Checkbox("Enable glow", &g_Config.glow_team_enable);
			if (g_Config.glow_team_enable)
			{
				ImGui::Combo("Type ", &g_Config.glow_team_type, GlowTypes, IM_ARRAYSIZE(GlowTypes));
			}
			ImGui::Separator(); ImGui::Text("Esp:"); ImGui::Spacing();
			ImGui::Checkbox("Enable esp", &g_Config.esp_team_enable);
			ImGui::Checkbox("Visible only", &g_Config.esp_team_visible_only);
			ImGui::ColorEdit3("##Box visible", g_Config.color_esp_team_visible_boxes); ImGui::SameLine(); ImGui::ColorEdit3("##Box hidden", g_Config.color_esp_team_invisible_boxes); ImGui::SameLine(); ImGui::Checkbox("Box ", &g_Config.esp_team_boxes);
			if (g_Config.esp_team_boxes) {
				ImGui::SameLine();
				ImGui::Checkbox("Box Outline", &g_Config.esp_team_boxes_outline);
			}
			ImGui::Combo("Box type", &g_Config.esp_team_boxes_type, BoxTypes, IM_ARRAYSIZE(BoxTypes));
			ImGui::ColorEdit3("##Box fill visible", g_Config.color_esp_team_visible_boxes_fill); ImGui::SameLine(); ImGui::ColorEdit3("##Box fill hidden", g_Config.color_esp_team_invisible_boxes_fill); ImGui::SameLine(); ImGui::Checkbox("Box Fill", &g_Config.esp_team_boxes_fill);
			ImGui::ColorEdit3("##3dbox vis", g_Config.color_esp_team_visible_headbox); ImGui::SameLine(); ImGui::ColorEdit3("##3dbox invis", g_Config.color_esp_team_invisible_headbox); ImGui::SameLine(); ImGui::Checkbox("Head 3d box", &g_Config.esp_team_headbox);
			ImGui::ColorEdit3("##Name visible", g_Config.color_esp_team_visible_names); ImGui::SameLine(); ImGui::ColorEdit3("##Name hidden", g_Config.color_esp_team_invisible_names); ImGui::SameLine(); ImGui::Checkbox("Name", &g_Config.esp_team_names);
			ImGui::Checkbox("Health ", &g_Config.esp_team_health);
			if (g_Config.esp_team_health)
			{
				ImGui::SameLine();
				ImGui::Checkbox("Dont render health if 100", &g_Config.esp_team_health_norender);
			}
			ImGui::ColorEdit3("##Armor visible", g_Config.color_esp_team_visible_armour); ImGui::SameLine(); ImGui::ColorEdit3("##Armor hidden", g_Config.color_esp_team_invisible_armour); ImGui::SameLine(); ImGui::Checkbox("Armor ", &g_Config.esp_team_armour);
			if (g_Config.esp_team_armour)
			{
				ImGui::SameLine();
				ImGui::Checkbox("Dont render armor if 100", &g_Config.esp_team_armour_norender);
			}
			ImGui::ColorEdit3("##flags visible", g_Config.color_esp_team_visible_flags); ImGui::SameLine(); ImGui::ColorEdit3("##flags hidden", g_Config.color_esp_team_invisible_flags); ImGui::SameLine(); ImGui::Checkbox("Flags", &g_Config.esp_team_flags);

			ImGui::ColorEdit3("##Lines visible anglines", g_Config.color_esp_team_visible_anglines);
			ImGui::SameLine();
			ImGui::ColorEdit3("##Lines visible lbylines", g_Config.color_esp_team_visible_lbylines);
			ImGui::SameLine();
			ImGui::SameLine();
			ImGui::ColorEdit3("##Lines hidden anglines", g_Config.color_esp_team_invisible_anglines);
			ImGui::SameLine();
			ImGui::ColorEdit3("##Lines hidden lbylines", g_Config.color_esp_team_invisible_lbylines);
			ImGui::SameLine();
			ImGui::Checkbox("Angle Lines (ang/lby)", &g_Config.esp_team_lines);

			ImGui::ColorEdit3("##Weapon visible", g_Config.color_esp_team_visible_weapons); ImGui::SameLine(); ImGui::ColorEdit3("##Weapon hidden", g_Config.color_esp_team_invisible_weapons); ImGui::SameLine(); ImGui::Checkbox("Weapon", &g_Config.esp_team_weapons);
			if (g_Config.esp_team_weapons)
			{
				ImGui::SameLine(); ImGui::Checkbox("Ammo", &g_Config.esp_team_weapons_ammo);
			}
			ImGui::ColorEdit3("##Snapline visible", g_Config.color_esp_team_visible_snaplines); ImGui::SameLine(); ImGui::ColorEdit3("##Snapline hidden", g_Config.color_esp_team_invisible_snaplines); ImGui::SameLine(); ImGui::Checkbox("Snapline", &g_Config.esp_team_snaplines);
			ImGui::ColorEdit3("##POV visible", g_Config.color_esp_team_visible_pov); ImGui::SameLine(); ImGui::ColorEdit3("##POV hidden", g_Config.color_esp_team_invisible_pov); ImGui::SameLine(); ImGui::Checkbox("Player out of view", &g_Config.esp_team_pov);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Other ESP"))
		{
			ImGui::Text("Chams:"); ImGui::Spacing();

			ImGui::ColorEdit3("##Backtrack visible", g_Config.color_chams_backtrack_visible); ImGui::SameLine(); ImGui::ColorEdit3("##Backtrack hidden", g_Config.color_chams_backtrack_invisible); ImGui::SameLine(); ImGui::Checkbox("Backtrack chams", &g_Config.chams_backtrack);
			if (g_Config.chams_backtrack)
			{
				ImGui::Combo("BT chams type", &g_Config.chams_type_backtrack, BacktrackChamsModes, IM_ARRAYSIZE(BacktrackChamsModes));
				ImGui::Combo("BT chams mode", &g_Config.chams_mode_backtrack, ChamsTypes, IM_ARRAYSIZE(ChamsTypes));
			}
			ImGui::Separator(); ImGui::Text("Glow:"); ImGui::Spacing();

			ImGui::ColorEdit3("##Glow chickens", g_Config.color_glow_chickens); ImGui::SameLine(); ImGui::Checkbox("Enable chickens glow", &g_Config.glow_chickens);
			if (g_Config.glow_chickens)
			{
				ImGui::Combo("Chickens type", &g_Config.glow_chickens_type, GlowTypes, IM_ARRAYSIZE(GlowTypes));
			}
			ImGui::ColorEdit3("##Glow bomb carrier", g_Config.color_glow_c4_carrier); ImGui::SameLine(); ImGui::Checkbox("Enable bomb carrier glow", &g_Config.glow_c4_carrier);
			if (g_Config.glow_c4_carrier)
			{
				ImGui::Combo("Carrier type", &g_Config.glow_c4_carrier_type, GlowTypes, IM_ARRAYSIZE(GlowTypes));
			}
			ImGui::ColorEdit3("##Glow planted bomb", g_Config.color_glow_planted_c4); ImGui::SameLine(); ImGui::Checkbox("Enable planted bomb glow", &g_Config.glow_planted_c4);
			if (g_Config.glow_planted_c4)
			{
				ImGui::Checkbox("Warning color", &g_Config.glow_planted_warning);
				ImGui::Combo("Bomb type", &g_Config.glow_planted_c4_type, GlowTypes, IM_ARRAYSIZE(GlowTypes));
			}
			ImGui::ColorEdit3("##Glow defuse kits", g_Config.color_glow_defuse); ImGui::SameLine(); ImGui::Checkbox("Enable defuse kits glow", &g_Config.glow_defuse_kits);
			if (g_Config.glow_defuse_kits)
			{
				ImGui::Combo("Defuse type", &g_Config.glow_defuse_type, GlowTypes, IM_ARRAYSIZE(GlowTypes));
			}
			ImGui::ColorEdit3("##Glow dropped weapons", g_Config.color_glow_weapons); ImGui::SameLine(); ImGui::Checkbox("Enable dropped weapons glow", &g_Config.glow_weapons);
			if (g_Config.glow_weapons)
			{
				ImGui::Combo("Weapons type", &g_Config.glow_weapons_type, GlowTypes, IM_ARRAYSIZE(GlowTypes));
			}
			ImGui::ColorEdit3("##Glow grenade", g_Config.color_glow_grenade); ImGui::SameLine(); ImGui::Checkbox("Enable grenades glow", &g_Config.glow_grenade);
			if (g_Config.glow_grenade)
			{
				ImGui::Combo("Grenade type", &g_Config.glow_grenade_type, GlowTypes, IM_ARRAYSIZE(GlowTypes));
			}
			ImGui::Separator(); ImGui::Text("Esp:"); ImGui::Spacing();
			ImGui::ColorEdit3("##aa inc", g_Config.vis_misc_aa_circle_clr); ImGui::SameLine(); ImGui::Checkbox("AA indicator", &g_Config.esp_misc_aa_indicator);
			ImGui::Checkbox("Grenade Prediction", &g_Config.esp_misc_grenade_prediction);
			ImGui::Checkbox("Bomb clock", &g_Config.esp_planted_c4);
			ImGui::Checkbox("Bomb box", &g_Config.esp_box_c4);
			ImGui::Checkbox("Bomb timer", &g_Config.esp_timer_c4);
			ImGui::Checkbox("Event logger", &g_Config.esp_misc_event_logger);
			static const char* GrenadeTypes[] = { "None", "Text", "Box", "Both" };
			ImGui::Combo("Grenade", &g_Config.esp_misc_grenade, GrenadeTypes, IM_ARRAYSIZE(GrenadeTypes));//nade wtf
			static const char* snaplinestypes[] = { "Default", "Gladiator" };
			ImGui::Combo("Snaplines", &g_Config.esp_misc_snaplines_type, snaplinestypes, IM_ARRAYSIZE(snaplinestypes));
			static const char* droppedweaponstypes[] = { "None", "Text", "2d box", "3d box" };
			ImGui::ColorEdit3("##Dropped weapons", g_Config.color_esp_dropped_weapons); ImGui::SameLine();  ImGui::Combo("DW Type", &g_Config.esp_dropped_weapons_type, droppedweaponstypes, IM_ARRAYSIZE(droppedweaponstypes));
			if (g_Config.esp_dropped_weapons_type > 0) ImGui::Checkbox("DW ammo info", &g_Config.esp_dropped_weapons_info);
			static const char* CircleTypes[] = { "None", "Outline", "Fill" };
			ImGui::ColorEdit3("##Spread circle", g_Config.vis_misc_draw_circle_clr); ImGui::SameLine();  ImGui::Combo("Spread", &g_Config.vis_misc_draw_circle, CircleTypes, IM_ARRAYSIZE(CircleTypes));
			ImGui::Checkbox("Spectator list", &g_Config.esp_misc_spectator_list);

			ImGui::Checkbox("Dangerzone item esp", &g_Config.esp_misc_dangerzone_item_esp);
			
			ImGui::PushItemWidth(-1);
			if (g_Config.esp_misc_dangerzone_item_esp) { ImGui::SameLine(); ImGui::SliderFloat("##DZ Esp range", &g_Config.esp_misc_dangerzone_item_esp_dist, 0.f, 1000.f, "Distance: %.f");}
			ImGui::PopItemWidth();

			ImGui::ColorEdit3("Gun immunity", g_Config.color_esp_gun_immunity);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::EndChild(); ImGui::NextColumn(); ImGui::BeginChild("#miscvisuals", ImVec2(0, 0), true);
	ImGui::Text("Thirdperson: "); ImGui::Spacing();
	ImGui::Checkbox("Thirdperson", &g_Config.vis_misc_thirdperson);
	ImGui::PushItemWidth(-1);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
	if (g_Config.vis_misc_thirdperson) { ImGui::SameLine(); ImGui::SliderFloat("##Thirdperson", &g_Config.vis_misc_thirdperson_dist, 5.f, 200.0f, "Distance: %.2f"); }
	ImGui::PopStyleVar();
	ImGui::PushItemWidth(-1);
	ImGui::Hotkey("Thirdperson hotkey", &g_Config.vis_misc_thirdperson_hotkey);

	ImGui::Separator(); ImGui::Text("Misc:"); ImGui::Spacing();
	
	ImGui::Checkbox("No scope overlay", &g_Config.vis_misc_noscope);
	ImGui::ColorEdit3("##vis_misc_clr_beams", g_Config.vis_misc_clr_beams); ImGui::SameLine(); ImGui::Checkbox("Bullet tracers", &g_Config.vis_misc_bullettracer);
	ImGui::Checkbox("No flash", &g_Config.vis_misc_noflash);
	ImGui::Checkbox("Disable sniper zoom", &g_Config.vis_misc_disable_scope_zoom);
	ImGui::PushItemWidth(142);
	ImGui::SliderFloat("##esp_misc_pov_size", &g_Config.esp_misc_pov_size, 1.f, 150.f, "POV Size: %.2f");
	ImGui::PopItemWidth();

	ImGui::SameLine();

	ImGui::PushItemWidth(143);
	ImGui::SliderFloat("##esp_misc_pov_dist", &g_Config.esp_misc_pov_dist, 1.f, 150.f, "POV Distance: %.2f");
	ImGui::PopItemWidth();


	ImGui::PushItemWidth(142);
	ImGui::SliderFloat("##esp.misc.viewfov", &g_Config.misc_viewmodel_fov, 1.f, 150.f, "Viewmodel FOV: %.2f");
	ImGui::PopItemWidth();

	ImGui::SameLine();

	ImGui::PushItemWidth(143);
	ImGui::SliderFloat("##esp.misc.fov", &g_Config.misc_fov, 1.f, 150.f, "FOV: %.2f");
	ImGui::PopItemWidth();

	ImGui::Checkbox("No smoke", &g_Config.vis_misc_nosmoke);
	ImGui::Checkbox("No hands / gloves", &g_Config.misc_no_hands);
	ImGui::Checkbox("No visual recoil", &g_Config.misc_no_recoil);
	ImGui::Checkbox("Force crosshair", &g_Config.vis_misc_force_crosshair);
	ImGui::Checkbox("No post processing", &g_Config.vis_misc_disable_postprocessing);
	ImGui::Checkbox("Screenshot cleaner", &g_Config.vis_misc_screenshotreturner);
	ImGui::Checkbox("Localplayer animfix", &g_Config.vis_misc_localanimfix);
	ImGui::Checkbox("Zeus Range", &g_Config.vis_misc_drawteaserrange);
	ImGui::Checkbox("KZ Window", &g_Config.vis_misc_kzwindow);
	ImGui::Checkbox("Fullbright", &g_Config.vis_misc_fullbright);
	ImGui::ColorEdit3("##fightmode", g_Config.vis_misc_clr_modificate); ImGui::SameLine(); ImGui::Checkbox("World color", &g_Config.vis_misc_nightmode);


	ImGui::Checkbox("Hitmarker", &g_Config.vis_misc_hitmarker);

	ImGui::PushItemWidth(-1);
	if (g_Config.vis_misc_hitmarker) { ImGui::SameLine(); ImGui::SliderInt("##misc_hitmarker_sound_vol", &g_Config.vis_misc_hitmarker_volume, 0, 100, "Volume: %.f"); }
	ImGui::PushItemWidth(-1);

	static const char* hitmarkersounds[] = {
		"Cod",
		"Skeet",
		"Punch",
		"Metal",
		"Boom",
		"Warn (leet)"
	};
	ImGui::Combo("##Sound", &g_Config.vis_misc_hitmarker_sound, hitmarkersounds, IM_ARRAYSIZE(hitmarkersounds));
	//ImGui::Checkbox("Autowall crosshair", "vis_misc_autowall_crosshair");
	ImGui::EndChild(); ImGui::Columns(1, NULL, false);
}
void Menu::RenderMisc()
{
	static std::vector<std::string> configs;
	static auto load_configs = []() {
		std::vector<std::string> items = {};

		std::string path("C:\\nukeparadise");
		if (!fs::is_directory(path))
			fs::create_directories(path);

		for (auto& p : fs::directory_iterator(path))
			items.push_back(p.path().string().substr(path.length() + 1));

		return items;
	};
	static auto is_configs_loaded = false;
	if (!is_configs_loaded) {
		is_configs_loaded = true;
		configs = load_configs();
	}
	static std::string current_config;
	static char config_name[128] = "";
	ImGui::Columns(2, NULL, false); ImGui::BeginChild("#misc", ImVec2(0, 0), true);
	ImGui::Checkbox("Request file spam", &g_Config.misc_request_file);
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::Text("Only community servers!");
		ImGui::EndTooltip();
	}
	ImGui::PushItemWidth(-1);
	if (g_Config.misc_request_file) { ImGui::SameLine(); ImGui::Hotkey("##misc_community_func_key", &g_Config.misc_request_file_key); }
	ImGui::PopItemWidth();

	ImGui::Checkbox("Legit resolver", &g_Config.misc_legit_resolver);
	ImGui::PushItemWidth(-1);
	if (g_Config.misc_legit_resolver) { ImGui::SameLine(); ImGui::Hotkey("##misc_legit_resolver_key", &g_Config.misc_legit_resolver_key); }
	ImGui::PopItemWidth();

	ImGui::PushItemWidth(-1);
	ImGui::Checkbox("Airstuck", &g_Config.misc_airstuck); if (g_Config.misc_airstuck) { ImGui::SameLine(); ImGui::Hotkey("##misc_airstuck_key", &g_Config.misc_airstuck_key); }
	ImGui::PopItemWidth();

	ImGui::Checkbox("Bunny hop", &g_Config.misc_bhop);
	ImGui::Checkbox("Edge jump", &g_Config.misc_franzjump);
	ImGui::Checkbox("Slide walk", &g_Config.rbot_slidewalk);
	ImGui::Checkbox("Knife bot", &g_Config.misc_knifebot);
	ImGui::PushItemWidth(-1);
	if (g_Config.misc_knifebot) { ImGui::SameLine(); ImGui::SliderInt("##knifebot", &g_Config.misc_knifebot_dist, 5, 100, "Distance: %.f"); }

	ImGui::PopItemWidth();
	ImGui::Checkbox("Cash hack", &g_Config.misc_cash_hack);
	ImGui::PushItemWidth(-1);
	if (g_Config.misc_cash_hack) { ImGui::SameLine(); ImGui::SliderInt("##cashhax", &g_Config.misc_cash_hack_val, 1, 9999, "Money: %.f"); }
	ImGui::PopItemWidth();

	ImGui::Checkbox("Auto accept", &g_Config.misc_autoaccept);
	ImGui::Checkbox("Anti kick", &g_Config.misc_anti_kick);
	ImGui::Checkbox("Choke limit", &g_Config.misc_choke_limit);
	ImGui::Checkbox("Ping spike", &g_Config.misc_ping_spike);
	ImGui::PushItemWidth(-1);
	if (g_Config.misc_ping_spike) { ImGui::SameLine(); ImGui::SliderInt("##Latency", &g_Config.misc_ping_spike_val, 0, 25, "Latency: %.f"); }
	ImGui::PopItemWidth();

	ImGui::Checkbox("Slow walk", &g_Config.rbot_slowwalk); if (g_Config.rbot_slowwalk)
	{
		ImGui::SameLine();
		ImGui::PushItemWidth(100);
		ImGui::SliderFloat("##SWSpeed", &g_Config.rbot_slowwalk_mod, 0.f, 10.f, "Speed: %.2f");
		ImGui::PopItemWidth();

		ImGui::SameLine();

		ImGui::PushItemWidth(-1);
		ImGui::Hotkey("##slowfast", &g_Config.rbot_slowwalk_key);
		ImGui::PopItemWidth();
	}

	static const char* AutoStrafers[]{ "None", "Aimware", "Gladiator", "Gideon", "Money", "Legit", "Rage", "Bruh" };
	ImGui::Combo("Autostrafer", &g_Config.misc_autostrafe, AutoStrafers, IM_ARRAYSIZE(AutoStrafers));

	if (g_Config.misc_autostrafe == 1) { ImGui::SliderFloat("##Retrack", &g_Config.misc_retrack_speed, 2.f, 8.0f, "Retrack: %.2f"); }

	static const char* Clantags[]{ "None", "Dynamic", "Custom Static", "Custom Slide" };
	static char str0[128] = "";
	ImGui::Combo("Clantag", &g_Config.misc_clantagchanger_type, Clantags, IM_ARRAYSIZE(Clantags));
	if (g_Config.misc_clantagchanger_type == 2)
	{
		ImGui::InputText("##CLAN1", str0, IM_ARRAYSIZE(str0));
		ImGui::SameLine();
		if (ImGui::Button("Apply"))
			Utils::SetClantag(str0);
	}
	else if (g_Config.misc_clantagchanger_type == 3)
	{
		ImGui::InputText("##CLAN2", str0, IM_ARRAYSIZE(str0));
		ImGui::SameLine();
		if (ImGui::Button("Apply"))
			ClantagChanger::Get().SetCustomSlide(str0);
	}

	static const char* Spammers[]{ "None", "/r/arabfunny/", "EPIC Trolling" };
	ImGui::Combo("Spammer", &g_Config.misc_chat_spammer, Spammers, IM_ARRAYSIZE(Spammers));

	ImGui::Checkbox("Auto-reload weapons", &g_Config.misc_automatic_weapons);
	ImGui::Checkbox("Rank reveal", &g_Config.misc_showranks);
	ImGui::Checkbox("Name stealer", &g_Config.misc_name_stealer);

	ImGui::Checkbox("Enable buybot", &g_Config.misc_buybot);
	if (g_Config.misc_buybot)
	{
		static const char* Pistols[] = { "None", "Glock|Usp|P2000", "Duals", "Tec9|Fiveseven", "Deagle|R8" };
		static const char* Weapons[] = { "None", "Sg|Aug", "Ssg", "Auto", "Mac10", "P90", "Bizon", "Ak", "Awp" };

		ImGui::Combo("Pistols", &g_Config.misc_buybot_pistol, Pistols, IM_ARRAYSIZE(Pistols));
		ImGui::Combo("Weapon", &g_Config.misc_buybot_weapon, Weapons, IM_ARRAYSIZE(Weapons));

		std::vector<std::string> buyitem =
		{
			"Molotov",
			"Grenade",
			"Smoke",
			"Flash",
			"Decoy",
			"Armor",
			"Zeus",
			"Defuser"
		};

		static std::string pizdec = "";

		if (ImGui::BeginCombo("Other", pizdec.c_str())) {
			std::string preview = "";
			//ebat pizdec 
			if (g_Config.misc_buybot_grenade[0])
			{
				preview += "inc, ";
			}
			if (g_Config.misc_buybot_grenade[1])
			{
				preview += "he, ";
			}
			if (g_Config.misc_buybot_grenade[2])
			{
				preview += "smoke, ";
			}
			if (g_Config.misc_buybot_grenade[3])
			{
				preview += "flash, ";
			}
			if (g_Config.misc_buybot_grenade[4])
			{
				preview += "decoy, ";
			}
			if (g_Config.misc_buybot_grenade[5])
			{
				preview += "hk, ";
			}
			if (g_Config.misc_buybot_grenade[6])
			{
				preview += "zeus; ";
			}
			if (g_Config.misc_buybot_grenade[7])
			{
				preview += "def, ";
			}
			pizdec = preview;
			for (int i = 0; i < 8; i++) {
				ImGui::Selectable(buyitem[i].c_str(), &g_Config.misc_buybot_grenade[i], ImGuiSelectableFlags_::ImGuiSelectableFlags_DontClosePopups);
			}
			ImGui::EndCombo();
		}
	}

	ImGui::InputText("##NICK", g_Saver.Name, 127);

	ImGui::SameLine();

	if (ImGui::Button("Set Nick"))
	{
		//Utils::SetName(g_Saver.Name);
		g_Saver.NameTrigger = true;
	}

	ImGui::InputText("Fakename", g_Saver.Fakename, 127);
	static const char* FakeNameMethods[]{ "Fake Name Methods", "XAD Name (switch names)", "Invisible Name", "Skin troll", "Fake ban", "Vote troll" };

	ImGui::Combo("##FakeNameCombo", &g_Saver.FakeNameMethod, FakeNameMethods, IM_ARRAYSIZE(FakeNameMethods));

	ImGui::EndChild();

	ImGui::NextColumn();

	ImGui::BeginChild("#antiaims", ImVec2(0.f, 215.f), true);

	static const char* AA[]{ "None", "Legit", "Rage" };
	ImGui::Combo("Antiaim", &g_Config.rbot_aa, AA, IM_ARRAYSIZE(AA));

	if (ImGui::BeginTabBar("pAntiaim", ImGuiTabBarFlags_NoTooltip))
	{
		static const char* PitchAAs[] = { "None", "Emotion", "Down", "Up", "Zero", "Jitter", "Down jitter", "Up jitter", "Zero jitter", "Spin", "Up spin", "Down spin", "Random", "Switch", "Down switch", "Up switch", "Fake up", "Fake down", "Custom" };
		static const char* YawAAs[] = { "None", "Backwards", "Spinbot", "Lower body yaw", "Random", "Freestanding", "Custom" };
		static const char* YawAddAAs[] = { "None", "Jitter", "Switch", "Spin", "Random" };
		static const char* DesyncAA[] = { "None", "Static", "Balance" };

		if (ImGui::BeginTabItem("Main"))
		{
			ImGui::PushItemWidth(-1);
			ImGui::SliderFloat("##Spinbot speed", &g_Config.rbot_aa_spinbot_speed, -20.f, 20.f, "Spinbot speed: %.2f");
			ImGui::PopItemWidth();
			ImGui::Combo("Desync", &g_Config.rbot_aa_desync, DesyncAA, IM_ARRAYSIZE(DesyncAA));
			ImGui::Hotkey("Desync Hotkey ", &g_Config.rbot_aa_desync_key);
			ImGui::Hotkey("Manual AA right ", &g_Config.rbot_manual_key_right);
			ImGui::Hotkey("Manual AA left ", &g_Config.rbot_manual_key_left);
			ImGui::Hotkey("Manual AA back ", &g_Config.rbot_manual_key_back);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Standing"))
		{
			ImGui::Combo("Pitch AA", &g_Config.rbot_aa_stand_pitch, PitchAAs, IM_ARRAYSIZE(PitchAAs));
			ImGui::Combo("Real Yaw AA", &g_Config.rbot_aa_stand_real_yaw, YawAAs, IM_ARRAYSIZE(YawAAs));
			ImGui::SliderFloat("##aa.range", &g_Config.rbot_aa_stand_real_add_yaw_add_range, 0.f, 360.f, "Range: %.2f");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Moving"))
		{
			ImGui::Combo("Pitch AA", &g_Config.rbot_aa_move_pitch, PitchAAs, IM_ARRAYSIZE(PitchAAs));
			ImGui::Combo("Real Yaw AA", &g_Config.rbot_aa_move_real_yaw, YawAAs, IM_ARRAYSIZE(YawAAs));
			ImGui::Combo("Real Yaw Add", &g_Config.rbot_aa_move_real_add_yaw_add, YawAddAAs, IM_ARRAYSIZE(YawAddAAs));
			ImGui::SliderFloat("##aa.range", &g_Config.rbot_aa_move_real_add_yaw_add_range, 0.f, 360.f, "Range: %.2f");

			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("In Air"))
		{
			ImGui::Combo("Pitch AA", &g_Config.rbot_aa_air_pitch, PitchAAs, IM_ARRAYSIZE(PitchAAs));
			ImGui::Combo("Real Yaw AA", &g_Config.rbot_aa_air_real_yaw, YawAAs, IM_ARRAYSIZE(YawAAs));
			ImGui::Combo("Real Yaw Add", &g_Config.rbot_aa_air_real_add_yaw_add, YawAddAAs, IM_ARRAYSIZE(YawAddAAs));
			ImGui::SliderFloat("##aa.range", &g_Config.rbot_aa_air_real_add_yaw_add_range, 0.f, 360.f, "Range: %.2f");
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Legit"))
		{
			ImGui::Combo("Legit Desync", &g_Config.lbot_aa_legit_desync, DesyncAA, IM_ARRAYSIZE(DesyncAA));
			ImGui::Hotkey("Desync Hotkey ", &g_Config.lbot_aa_legit_desync_key);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::EndChild();

	ImGui::BeginChild("#config", ImVec2(0, 188), true);

	ImGui::Text("Config Name:");
	ImGui::InputText("##config_name", config_name, sizeof(config_name));
	ImGui::SameLine();
	if (ImGui::Button("Create") && config_name != "") {

		current_config = std::string(config_name);
		Config->Save(current_config + ".ini");
		is_configs_loaded = false;
		memset(config_name, 0, 32);
	}
	ImGui::ListBoxHeader("##configs", ImVec2(0, 78));
	{
		for (auto& config : configs) {
			if (ImGui::Selectable(config.c_str(), config == current_config)) {
				current_config = config;
			}
		}
	}
	ImGui::ListBoxFooter();


	if (!current_config.empty()) {
		if (ImGui::Button("Load Config"))
			Config->Load(current_config);
		ImGui::SameLine();
		if (ImGui::Button("Save Config"))
			Config->Save(current_config); //10 hours
		ImGui::SameLine();
		if (ImGui::Button("Delete Config") && fs::remove("C:\\nukeparadise\\" + current_config)) {
			current_config.clear();
			is_configs_loaded = false;
		}
		//ImGui::SameLine();
	}
	if (ImGui::Button("Refresh List"))
		is_configs_loaded = false;
	ImGui::SameLine();
	if (ImGui::Button("Unload Cheat"))
		g_Unload = true;

	//if (ImGui::Button("Invite nearby players (old)"))
	//{
	//	Lobby::Get().meme_inviter(false); //idk
	//}

	ImGui::EndChild(); ImGui::Columns(1, NULL, false);
}
void Menu::RenderList()
{
	ImGui::BeginChild("##players", ImVec2(0, 0), true); ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
	static const char* teams[4] = { "None", "Spec", "T", "CT" };
	ImGui::Columns(7, NULL, false);

	ImGui::SetColumnWidth(0, 20.f); ImGui::Text(("ID")); ImGui::NextColumn();
	ImGui::SetColumnWidth(1, 245.f); ImGui::Text(("Name")); ImGui::NextColumn();
	ImGui::SetColumnWidth(2, 40.f); ImGui::Text(("Live")); ImGui::NextColumn();
	ImGui::SetColumnWidth(3, 40.f); ImGui::Text(("Team")); ImGui::NextColumn();
	ImGui::SetColumnWidth(4, 50.f); ImGui::Text(("Money")); ImGui::NextColumn();
	ImGui::SetColumnWidth(5, 140.f); ImGui::Text(("SteamID")); ImGui::NextColumn();
	ImGui::SetColumnWidth(6, 100.f); ImGui::Text(("Features")); ImGui::NextColumn();
	ImGui::Separator();
	if (g_EngineClient->IsInGame()) {
		for (int i = 0; i < g_EngineClient->GetMaxClients(); i++) {
			C_BasePlayer* pPlayer = C_BasePlayer::GetPlayerByIndex(i);
			player_info_t info;
			if (pPlayer && g_EngineClient->GetPlayerInfo(i, &info)) {
				int teamNum = pPlayer->m_iTeamNum();
				if (teamNum > 3) teamNum = 0;
				ImGui::Text(std::to_string(info.userId).data()); ImGui::NextColumn();
				std::string name = pPlayer->GetName(false);
				if (name.find("\n") != std::string::npos)
				{
					ImGui::TextColored(ImVec4(255, 255, 0, 255), "Bruh Moment");
				}
				else
				{
					ImGui::Text(name.c_str());
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("Name: %s", (name.c_str()));
					ImGui::Text("Velocity: %.3f", pPlayer->m_vecVelocity().Length2D());
					if (pPlayer->m_bHasDefuser()) {
						ImGui::Text("-=HasDefuser=-");
					}
					if (pPlayer->m_bGunGameImmunity()) {
						ImGui::Text("-=GunGameImmunity=-");
					}
					if (pPlayer->m_bIsScoped()) {
						ImGui::Text("-=Scoped=-");
					}
					ImGui::EndTooltip();
				}
				ImGui::NextColumn();

				if (pPlayer->m_iHealth())
				{
					if (pPlayer->m_iHealth() > 100) {
						ImGui::TextColored(ImVec4(0, 0, 100, 100), "Over");
					}
					else if (pPlayer->m_iHealth() >= 25) {
						ImGui::TextColored(ImVec4(0, 100, 0, 100), "Alive");
					}
					else if (pPlayer->m_iHealth() >= 1) {
						ImGui::TextColored(ImVec4(100, 0, 0, 100), "Low");
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text(Utils::Format("%d", pPlayer->m_iHealth()));
						ImGui::EndTooltip();
					}
					ImGui::NextColumn();
				}
				else
				{
					ImGui::TextColored(ImVec4(100, 100, 100, 100), "Dead"); ImGui::NextColumn();
				}

				if (teamNum == 1) {
					ImGui::TextColored(ImVec4(0, 255, 0, 255), teams[teamNum]); ImGui::NextColumn();
				}
				else if (teamNum == 2) {
					ImGui::TextColored(ImVec4(255, 0, 0, 255), teams[teamNum]); ImGui::NextColumn();
				}
				else if (teamNum == 3) {
					ImGui::TextColored(ImVec4(0, 128, 255, 255), teams[teamNum]); ImGui::NextColumn();
				}
				else {
					ImGui::Text(teams[teamNum]); ImGui::NextColumn();
				}
				ImGui::Text(Utils::Format("$%d", static_cast<int>(pPlayer->m_iAccount()))); ImGui::NextColumn();
				ImGui::Text(info.szSteamID); ImGui::NextColumn();
				if (pPlayer != g_LocalPlayer)
				{
					if (ImGui::ButtonEx(("Steal##" + std::to_string(i)).data())) {
						char name[1024]; sprintf(name, "%s ", info.szName);
						Utils::SetName(name);
					}
					ImGui::SameLine();
				}

				if (ImGui::ButtonEx(("Kick##" + std::to_string(i)).data())) {
					Misc::Get().Kick(info.userId);
				}
				ImGui::NextColumn();

			}

		}
		ImGui::Separator();
	}
	ImGui::Columns(1, NULL, false);

	ImGui::PopStyleVar();
	ImGui::EndChild();

}