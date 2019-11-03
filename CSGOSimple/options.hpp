#pragma once

#include <string>
#include <map>
#include "valve_sdk/Misc/Color.hpp"

#define FLOAT_TO_CLR(clr) clr[0] * 255.0f, clr[1] * 255.0f, clr[2] * 255.0f, clr[3] * 255.0f

extern std::map<int, const char*> k_weapon_names;

#include "helpers/KitParser.h"
#include "helpers/ItemDefinitions.h"
struct item_setting
{
	char name[32] = "Default";
	bool enabled = false;
	int definition_vector_index = 0;
	int definition_index = 1;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	int stat_trak = 0;
	float wear = 0.0f;
	char custom_name[32] = "";
};

struct lbot_settings {
	bool enabled = false;
	bool on_key = false;
	bool deathmatch = false;
	bool autopistol = false;
	bool autofire = false;
	bool autostop = false;
	bool check_smoke = false;
	bool check_flash = false;
	bool check_jump = false;
	bool autowall = false;
	bool silent = false;
	bool rcs = false;
	bool rcs_fov_enabled = false;
	bool rcs_smooth_enabled = false;
	bool humanize = false;
	struct {
		bool enabled = false;
		float time = 0.f;
	} backtrack;
	bool only_in_zoom = true;
	int aim_type = 1;
	int priority = 0;
	int fov_type = 0;
	int rcs_type = 0;
	int hitbox = 1;
	float fov = 0.f;
	float silent_fov = 0.f;
	float rcs_fov = 0.f;
	float smooth = 1;
	float rcs_smooth = 1;
	int shot_delay = 0;
	int kill_delay = 0;
	int rcs_x = 100;
	int rcs_y = 100;
	int rcs_start = 1;
	int min_damage = 1;
};

template <typename T = bool>
class Var {
public:
	std::string name;
	std::shared_ptr<T> value;
	int32_t size;
	Var(std::string name, T v) : name(name) {
		value = std::make_shared<T>(v);
		size = sizeof(T);
	}
	operator T() { return *value; }
	operator T* () { return &*value; }
	operator T() const { return *value; }
	//operator T*() const { return value; }
};

#define OPTION(type, var, val) type var = val

class Options
{
public:

	// Lbot
	OPTION(bool, lbot_set_automatically_weapon, false);
	OPTION(bool, lbot_ijustflippedaswitch, true);
	OPTION(int, lbot_on_fire_key, 0x0);
	OPTION(int, lbot_auto_fire_key, 0x0);
	OPTION(int, lbot_aa_legit_desync, 0);
	OPTION(int, lbot_aa_legit_desync_key, 0x0);

	std::map<int, lbot_settings> lbot_items = {};

	// Rbot
	OPTION(bool, rbot_enable, false);
	OPTION(bool, rbot_force_unlage, false);
	OPTION(bool, rbot_baim_while_moving, false);
	OPTION(bool, rbot_lby_prediction, false);
	OPTION(bool, rbot_flag_prediction, false);
	OPTION(bool, rbot_autoscope, false);
	OPTION(int, rbot_baimmode, 0);
	OPTION(bool, rbot_aimstep, false);
	OPTION(bool, rbot_fakeduck, false);
	OPTION(int, rbot_fakeduck_key, 0x0);
	OPTION(bool, rbot_guccigang, false);
	OPTION(bool, rbot_slowwalk, false);
	OPTION(float, rbot_slowwalk_mod, 0.f);
	OPTION(int, rbot_slowwalk_key, 0x0);
	OPTION(bool, rbot_resolver, false);
	OPTION(bool, rbot_resolver_air_baim, false);
	OPTION(int, rbot_shooting_mode, 0);
	OPTION(bool, rbot_lagcompensation, false);
	OPTION(int, rbot_lagcompensation_type, 0);
	OPTION(int, rbot_aa_desync, 0);
	OPTION(int, rbot_aa_desync_key, 0x0);
	OPTION(float, rbot_aa_desync_side, 1.0f);
	OPTION(int, rbot_aa, 0);
	OPTION(bool, rbot_slidewalk, false);
	OPTION(bool, rbot_extrapolation, false);
	OPTION(int, rbot_manual_aa_state, 0);
	OPTION(int, rbot_manual_key_right, 0x0);
	OPTION(int, rbot_manual_key_left, 0x0);
	OPTION(int, rbot_manual_key_back, 0x0);
	OPTION(bool, rbot_aa_lby_breaker, false);
	OPTION(bool, rbot_aa_fake_lby_breaker, false);
	OPTION(float, rbot_aa_lby_breaker_yaw, 0.f);
	OPTION(bool, rbot_aa_lby_breaker_freestanding, false);
	OPTION(bool, rbot_autostop, false);
	OPTION(bool, rbot_autocrouch, false);

	///////////////////////////////////////////////////////////////AA START
	OPTION(float, rbot_aa_spinbot_speed, 5.f);

	OPTION(int, rbot_aa_stand_pitch, 0);
	OPTION(int, rbot_aa_move_pitch, 0);
	OPTION(int, rbot_aa_air_pitch, 0);

	OPTION(float, rbot_aa_stand_pitch_custom, 0.f);
	OPTION(float, rbot_aa_move_pitch_custom, 0.f);
	OPTION(float, rbot_aa_air_pitch_custom, 0.f);

	OPTION(int, rbot_aa_stand_real_yaw, 0);
	OPTION(int, rbot_aa_move_real_yaw, 0);
	OPTION(int, rbot_aa_air_real_yaw, 0);

	OPTION(int, rbot_aa_stand_fake_yaw, 0);
	OPTION(int, rbot_aa_move_fake_yaw, 0);
	OPTION(int, rbot_aa_air_fake_yaw, 0);

	OPTION(int, rbot_aa_stand_real_add_yaw_add, 0);
	OPTION(int, rbot_aa_move_real_add_yaw_add, 0);
	OPTION(int, rbot_aa_air_real_add_yaw_add, 0);

	OPTION(int, rbot_aa_stand_fake_add_yaw_add, 0);
	OPTION(int, rbot_aa_move_fake_add_yaw_add, 0);
	OPTION(int, rbot_aa_air_fake_add_yaw_add, 0);

	OPTION(float, rbot_aa_stand_real_yaw_custom, 0.f);
	OPTION(float, rbot_aa_move_real_yaw_custom, 0.f);
	OPTION(float, rbot_aa_air_real_yaw_custom, 0.f);

	OPTION(float, rbot_aa_stand_fake_yaw_custom, 0.f);
	OPTION(float, rbot_aa_move_fake_yaw_custom, 0.f);
	OPTION(float, rbot_aa_air_fake_yaw_custom, 0.f);

	OPTION(int, rbot_aa_stand_real_add_yaw, 0);
	OPTION(int, rbot_aa_move_real_add_yaw, 0);
	OPTION(int, rbot_aa_air_real_add_yaw, 0);

	OPTION(int, rbot_aa_stand_fake_add_yaw, 0);
	OPTION(int, rbot_aa_move_fake_add_yaw, 0);
	OPTION(int, rbot_aa_air_fake_add_yaw, 0);

	OPTION(float, rbot_aa_stand_real_add_yaw_add_range, 0.f);
	OPTION(float, rbot_aa_move_real_add_yaw_add_range, 0.f);
	OPTION(float, rbot_aa_air_real_add_yaw_add_range, 0.f);

	OPTION(float, rbot_aa_stand_fake_add_yaw_add_range, 0.f);
	OPTION(float, rbot_aa_move_fake_add_yaw_add_range, 0.f);
	OPTION(float, rbot_aa_air_fake_add_yaw_add_range, 0.f);
	///////////////////////////////////////////////////////////////AA END

	OPTION(int, rbot_aa_fakewalk_key, 0x0); //dead

	float rbot_mindamage[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float rbot_min_hitchance[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	int rbot_baim_after_shots[7] = { 0, 0, 0, 0, 0, 0, 0 };

	float rbot_hitbox_head_scale[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float rbot_hitbox_neck_scale[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float rbot_hitbox_chest_scale[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float rbot_hitbox_pelvis_scale[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float rbot_hitbox_stomach_scale[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float rbot_hitbox_arm_scale[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float rbot_hitbox_leg_scale[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };
	float rbot_hitbox_foot_scale[7] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f };

	bool rbot_hitbox_head[7] = { false, false, false, false, false, false, false };
	bool rbot_hitbox_neck[7] = { false, false, false, false, false, false, false };
	bool rbot_hitbox_chest[7] = { false, false, false, false, false, false, false };
	bool rbot_hitbox_pelvis[7] = { false, false, false, false, false, false, false };
	bool rbot_hitbox_stomach[7] = { false, false, false, false, false, false, false };
	bool rbot_hitbox_arm[7] = { false, false, false, false, false, false, false };
	bool rbot_hitbox_leg[7] = { false, false, false, false, false, false, false };
	bool rbot_hitbox_foot[7] = { false, false, false, false, false, false, false };

	// ESP

	OPTION(bool, esp_local_enable, false);
	OPTION(bool, esp_local_boxes, false);
	OPTION(bool, esp_local_headbox, false);
	OPTION(bool, esp_local_boxes_fill, false);
	OPTION(bool, esp_local_boxes_outline, false);
	OPTION(int, esp_local_boxes_type, 0);
	OPTION(bool, esp_local_names, false);
	OPTION(bool, esp_local_lines, false);
	OPTION(bool, esp_local_health, false);
	OPTION(bool, esp_local_armour, false);
	OPTION(bool, esp_local_health_norender, false);
	OPTION(bool, esp_local_armour_norender, false);
	OPTION(bool, esp_local_weapons, false);
	OPTION(bool, esp_local_weapons_ammo, false);
	OPTION(bool, esp_local_flags, false);

	OPTION(bool, esp_team_enable, false);
	OPTION(bool, esp_team_visible_only, false);
	OPTION(bool, esp_team_boxes, false);
	OPTION(bool, esp_team_headbox, false);
	OPTION(bool, esp_team_boxes_fill, false);
	OPTION(bool, esp_team_boxes_outline, false);
	OPTION(int, esp_team_boxes_type, 0);
	OPTION(bool, esp_team_names, false);
	OPTION(bool, esp_team_lines, false);
	OPTION(bool, esp_team_health, false);
	OPTION(bool, esp_team_armour, false);
	OPTION(bool, esp_team_health_norender, false);
	OPTION(bool, esp_team_armour_norender, false);
	OPTION(bool, esp_team_weapons, false);
	OPTION(bool, esp_team_weapons_ammo, false);
	OPTION(bool, esp_team_snaplines, false);
	OPTION(bool, esp_team_pov, false);
	OPTION(bool, esp_team_flags, false);

	OPTION(bool, esp_enemy_enable, false);
	OPTION(bool, esp_enemy_visible_only, false);
	OPTION(bool, esp_enemy_boxes, false);
	OPTION(bool, esp_enemy_headbox, false);
	OPTION(bool, esp_enemy_boxes_fill, false);
	OPTION(bool, esp_enemy_boxes_outline, false);
	OPTION(int, esp_enemy_boxes_type, 0);
	OPTION(bool, esp_enemy_names, false);
	OPTION(bool, esp_enemy_lines, false);
	OPTION(bool, esp_enemy_health, false);
	OPTION(bool, esp_enemy_armour, false);
	OPTION(bool, esp_enemy_health_norender, false);
	OPTION(bool, esp_enemy_armour_norender, false);
	OPTION(bool, esp_enemy_weapons, false);
	OPTION(bool, esp_enemy_weapons_ammo, false);
	OPTION(bool, esp_enemy_snaplines, false);
	OPTION(bool, esp_enemy_lclines, false);
	OPTION(bool, esp_enemy_info, false);
	OPTION(bool, esp_enemy_lby_timer, false);
	OPTION(bool, esp_enemy_pov, false);
	OPTION(bool, esp_enemy_flags, false);
	OPTION(bool, esp_misc_spectator_list, false);
	OPTION(bool, esp_misc_grenade_prediction, false);
	OPTION(bool, esp_misc_bullettracer, false);
	OPTION(bool, esp_misc_event_logger, false);
	OPTION(bool, esp_misc_aa_indicator, false);
	OPTION(int, esp_dropped_weapons_type, 0);
	OPTION(bool, esp_dropped_weapons_info, false);
	OPTION(bool, esp_planted_c4, false);
	OPTION(bool, esp_box_c4, false);
	OPTION(bool, esp_timer_c4, false);
	OPTION(int, esp_misc_grenade, 0);
	OPTION(bool, esp_misc_debug_overlay, false);
	OPTION(int, esp_misc_snaplines_type, 0);
	OPTION(float, esp_misc_pov_dist, 25.f);
	OPTION(float, esp_misc_pov_size, 20.f);
	OPTION(bool, esp_misc_dangerzone_item_esp, false);
	OPTION(float, esp_misc_dangerzone_item_esp_dist, 0.f);

	OPTION(bool, vis_misc_drawteaserrange, false);
	OPTION(bool, vis_misc_noscope, false);
	OPTION(bool, vis_misc_bullettracer, false);
	OPTION(bool, vis_misc_autowall_crosshair, false);
	OPTION(bool, vis_misc_nosmoke, false);
	OPTION(bool, vis_misc_hitmarker, false);
	OPTION(int, vis_misc_hitmarker_sound, 0);
	OPTION(int, vis_misc_hitmarker_volume, 75);
	OPTION(bool, vis_misc_disable_postprocessing, false);
	OPTION(bool, vis_misc_kzwindow, false);
	OPTION(bool, vis_misc_fullbright, false);
	OPTION(bool, vis_misc_nightmode, false);
	OPTION(bool, vis_misc_force_crosshair, false);
	OPTION(bool, vis_misc_thirdperson, false);
	OPTION(bool, vis_misc_screenshotreturner, false);
	OPTION(bool, vis_misc_localanimfix, false);
	OPTION(int, vis_misc_thirdperson_hotkey, 0x43);
	OPTION(float, vis_misc_thirdperson_dist, 50.f);
	OPTION(bool, vis_misc_noflash, false);
	OPTION(int, vis_misc_draw_circle, 0);
	OPTION(bool, vis_misc_disable_scope_zoom, false);

	// GLOW
	OPTION(bool, glow_chickens, false);
	OPTION(int, glow_chickens_type, 0);
	OPTION(bool, glow_c4_carrier, false);
	OPTION(int, glow_c4_carrier_type, 0);
	OPTION(bool, glow_planted_c4, false); 
	OPTION(bool, glow_planted_warning, false);
	OPTION(int, glow_planted_c4_type, 0);
	OPTION(bool, glow_defuse_kits, false);
	OPTION(int, glow_defuse_type, 0);
	OPTION(bool, glow_grenade, false);
	OPTION(int, glow_grenade_type, 0);
	OPTION(bool, glow_weapons, false);
	OPTION(int, glow_weapons_type, 0);
	OPTION(bool, glow_team_enable, false);
	OPTION(int, glow_team_type, 0);
	OPTION(bool, glow_enemy_enable, false);
	OPTION(int, glow_enemy_type, 0);


	// CHAMS
	OPTION(bool, chams_local, false);
	OPTION(bool, chams_local_ghost, false);
	OPTION(bool, chams_local_fakelag, false);
	OPTION(bool, chams_local_scope, false);
	OPTION(bool, chams_local_weapon, false);
	OPTION(bool, chams_local_arms, false);
	OPTION(bool, chams_team, false);
	OPTION(bool, chams_enemy, false);
	OPTION(bool, chams_backtrack, false);
	OPTION(int, chams_mode_local, 1);
	OPTION(int, chams_mode_local_ghost, 1);
	OPTION(int, chams_mode_local_fakelag, 1);
	OPTION(int, chams_mode_local_weapon, 1);
	OPTION(int, chams_mode_local_arms, 1);
	OPTION(int, chams_mode_team, 1);
	OPTION(int, chams_mode_enemy, 1);
	OPTION(int, chams_type_backtrack, 0);
	OPTION(int, chams_mode_backtrack, 1);

	// SKINCHANGER

	struct {
		std::map<int, item_setting> m_items;
		std::unordered_map<std::string, std::string> m_icon_overrides;
		auto get_icon_override(const std::string original) const -> const char*
		{
			return m_icon_overrides.count(original) ? m_icon_overrides.at(original).data() : nullptr;
		}
	} skins;

	// MISC
	OPTION(bool, misc_bhop, false);
	OPTION(int, misc_autostrafe, 0);
	OPTION(bool, misc_ping_spike, false);
	OPTION(bool, misc_autoaccept, false);
	OPTION(bool, misc_autoblock, false);
	OPTION(float, misc_retrack_speed, 2.f);
	OPTION(int, misc_ping_spike_val, 0);
	OPTION(bool, misc_no_hands, false);
	OPTION(bool, misc_no_recoil, false);
	OPTION(bool, misc_showranks, false);
	OPTION(float, misc_viewmodel_fov, 90.0f);
	OPTION(float, misc_fov, 90.0f);
	OPTION(bool, misc_fakelag_enable, false);
	OPTION(int, misc_fakelag_ticks_standing, 0);
	OPTION(int, misc_fakelag_ticks_air, 0);
	OPTION(int, misc_fakelag_ticks_moving, 0);
	OPTION(int, misc_fakelag_mode_air, 0);
	OPTION(int, misc_fakelag_mode_moving, 0);
	OPTION(bool, misc_fakelag_on_key, false);
	OPTION(int, misc_fakelag_key, 0x0);
	OPTION(bool, misc_airstuck, false);
	OPTION(int, misc_airstuck_key, 0x0);
	OPTION(bool, misc_name_stealer, false);
	OPTION(bool, misc_automatic_weapons, false);
	OPTION(int, misc_chat_spammer, 0);
	OPTION(int, misc_request_file_key, 0x0);
	OPTION(bool, misc_request_file, false);
	OPTION(bool, misc_choke_limit, false);
	OPTION(int, misc_legit_resolver_key, 0x0);
	OPTION(bool, misc_legit_resolver, false);
	OPTION(int, misc_debugfunc_key, 0x0);
	OPTION(bool, misc_debugfunc, false);
	OPTION(float, misc_debugfunc_val, 10.f);
	OPTION(bool, misc_knifebot, false);
	OPTION(int, misc_knifebot_dist, 75);
	OPTION(bool, misc_franzjump, false);
	OPTION(bool, misc_cash_hack, false);
	OPTION(int, misc_cash_hack_val, 100);
	OPTION(bool, misc_anti_kick, false);
	OPTION(bool, misc_clantagchanger, false);
	OPTION(int, misc_clantagchanger_type, 0);
	OPTION(bool, misc_buybot, false);
	OPTION(int, misc_buybot_pistol, 0);
	OPTION(int, misc_buybot_weapon, 0);

	bool misc_buybot_grenade[8] = { false, false, false, false, false, false, false, false };

	// COLORS

	float color_esp_gun_immunity[4] = { 0.6f, 0.6f, 0.7f, 1.0f };
	float color_esp_local_boxes[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_esp_local_boxes_fill[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_esp_local_names[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_local_armour[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_esp_local_weapons[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_local_flags[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_local_anglines[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_local_lbylines[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_local_desync[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_local_headbox[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_team_visible_boxes[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float color_esp_team_visible_boxes_fill[4] = { 0.0f, 1.0f, 0.0f, 0.2f };
	float color_esp_team_visible_names[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_team_visible_armour[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float color_esp_team_visible_weapons[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_team_visible_snaplines[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float color_esp_team_visible_pov[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float color_esp_team_visible_flags[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float color_esp_team_visible_headbox[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float color_esp_team_visible_anglines[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float color_esp_team_visible_lbylines[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float color_esp_team_invisible_boxes[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_team_invisible_boxes_fill[4] = { 1.0f, 0.0f, 0.0f, 0.2f };
	float color_esp_team_invisible_names[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_team_invisible_armour[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float color_esp_team_invisible_weapons[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_team_invisible_snaplines[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_team_invisible_pov[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_team_invisible_flags[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_team_invisible_headbox[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_team_invisible_anglines[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_team_invisible_lbylines[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_enemy_visible_boxes_fill[4] = { 0.0f, 1.0f, 0.0f, 0.2f };
	float color_esp_enemy_visible_boxes[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float color_esp_enemy_visible_names[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible_armour[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible_weapons[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible_snaplines[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float color_esp_enemy_visible_info[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible_lby_timer[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible_pov[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible_flags[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible_lcline[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible_headbox[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible_anglines[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_visible_lbylines[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_boxes_fill[4] = { 1.0f, 0.0f, 0.0f, 0.2f };
	float color_esp_enemy_invisible_boxes[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_enemy_invisible_names[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_armour[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_weapons[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_snaplines[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_esp_enemy_invisible_info[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_lby_timer[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_pov[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_flags[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_lcline[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_headbox[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_anglines[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_enemy_invisible_lbylines[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	// COLORS

	float color_esp_crosshair[4] = { 0.5f, 0.0f, 0.5f, 1.0f };
	float color_esp_weapons[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_esp_defuse[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
	float color_esp_c4[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_esp_dropped_weapons[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_glow_team[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_glow_enemy[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float vis_misc_draw_circle_clr[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float vis_misc_aa_circle_clr[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float vis_misc_clr_modificate[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float vis_misc_clr_beams[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_glow_chickens[4] = { 0.0f, 0.5f, 0.0f, 1.0f };
	float color_glow_c4_carrier[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
	float color_glow_planted_c4[4] = { 0.5f, 0.0f, 0.5f, 1.0f };
	float color_glow_defuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_glow_weapons[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
	float color_glow_grenade[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
	float color_chams_backtrack_visible[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_chams_backtrack_invisible[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_chams_local_ghost_visible[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_chams_local_fakelag_visible[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_chams_scoped_visible[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_chams_local_visible[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_chams_local_invisible[4] = { 0.0f, 0.5f, 1.0f, 1.0f };	
	float color_chams_local_arms_visible[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_chams_local_weapon_visible[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
	float color_chams_team_visible[4] = { 0.0f, 1.0f, 0.5f, 1.0f };
	float color_chams_enemy_visible[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	float color_chams_local_arms_invisible[4] = { 1.0f, 0.5f, 0.0f, 1.0f };
	float color_chams_local_weapon_invisible[4] = { 1.0f, 0.5f, 0.0f, 1.0f };
	float color_chams_team_invisible[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_chams_enemy_invisible[4] = { 0.0f, 0.5f, 1.0f, 1.0f };
};

extern Options g_Config;
extern bool   g_Unload;