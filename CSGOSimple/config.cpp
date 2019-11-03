#include "config.hpp"
#include "options.hpp"
#include "helpers/math.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"
#include <iomanip>
#include "Shlobj_core.h"
#include "helpers/utils.hpp"
#pragma comment(lib,"Shell32.lib")

// For ints, chars, floats, and bools
void CConfig::SetupValue(int& value, int def, std::string category, std::string name) { value = def; ints.push_back(new ConfigValue< int >(category, name, &value, def)); }
void CConfig::SetupValue(char* value, char* def, std::string category, std::string name) { value = def; chars.push_back(new ConfigValue< char >(category, name, value, *def)); }
void CConfig::SetupValue(float& value, float def, std::string category, std::string name) { value = def; floats.push_back(new ConfigValue< float >(category, name, &value, def)); }
void CConfig::SetupValue(bool& value, bool def, std::string category, std::string name) { value = def; bools.push_back(new ConfigValue< bool >(category, name, &value, def)); }
void CConfig::SetupColor(float value[4], std::string name) {
	SetupValue(value[0], value[0], ("colors"), name + "_r");
	SetupValue(value[1], value[1], ("colors"), name + "_g");
	SetupValue(value[2], value[2], ("colors"), name + "_b");
	SetupValue(value[3], value[3], ("colors"), name + "_a");
}
void CConfig::SetupLegit() {
	SetupValue(g_Config.lbot_set_automatically_weapon, false, ("lbot"), ("set_automatically_weapon"));
	SetupValue(g_Config.lbot_ijustflippedaswitch, true, ("lbot"), ("ijustflippedaswitch"));

	SetupValue(g_Config.lbot_auto_fire_key, 0x0, ("lbot"), ("lbot_auto_fire_key"));
	SetupValue(g_Config.lbot_on_fire_key, 0x0, ("lbot"), ("lbot_on_fire_key"));

	SetupValue(g_Config.lbot_aa_legit_desync, 0, ("lbot"), ("lbot_aa_legit_desync"));
	SetupValue(g_Config.lbot_aa_legit_desync_key, 0x0, ("lbot"), ("lbot_aa_legit_desync_key"));

	for (auto& [key, val] : k_weapon_names) {
		SetupValue(g_Config.lbot_items[key].enabled, false, (val), ("enabled"));
		SetupValue(g_Config.lbot_items[key].deathmatch, false, (val), ("deathmatch"));
		SetupValue(g_Config.lbot_items[key].autopistol, false, (val), ("autopistol"));
		SetupValue(g_Config.lbot_items[key].autofire, false, (val), ("autofire"));
		SetupValue(g_Config.lbot_items[key].autostop, false, (val), ("autostop"));
		SetupValue(g_Config.lbot_items[key].check_smoke, false, (val), ("check_smoke"));
		SetupValue(g_Config.lbot_items[key].check_flash, false, (val), ("check_flash"));
		SetupValue(g_Config.lbot_items[key].check_jump, false, (val), ("check_jump"));
		SetupValue(g_Config.lbot_items[key].autowall, false, (val), ("autowall"));
		SetupValue(g_Config.lbot_items[key].silent, false, (val), ("silent"));
		SetupValue(g_Config.lbot_items[key].rcs, false, (val), ("rcs"));
		SetupValue(g_Config.lbot_items[key].rcs_fov_enabled, false, (val), ("rcs_fov_enabled"));
		SetupValue(g_Config.lbot_items[key].rcs_smooth_enabled, false, (val), ("rcs_smooth_enabled"));
		SetupValue(g_Config.lbot_items[key].backtrack.enabled, false, (val), ("backtrack_enabled"));
		SetupValue(g_Config.lbot_items[key].backtrack.time, 0.f, (val), ("backtrack_ticks"));
		SetupValue(g_Config.lbot_items[key].only_in_zoom, false, (val), ("only_in_zoom"));
		SetupValue(g_Config.lbot_items[key].aim_type, 1, (val), ("aim_type"));
		SetupValue(g_Config.lbot_items[key].priority, 0, (val), ("priority"));
		SetupValue(g_Config.lbot_items[key].fov_type, 0, (val), ("fov_type"));
		SetupValue(g_Config.lbot_items[key].rcs_type, 0, (val), ("rcs_type"));
		SetupValue(g_Config.lbot_items[key].hitbox, 1, (val), ("hitbox"));
		SetupValue(g_Config.lbot_items[key].fov, 0.f, (val), ("fov"));
		SetupValue(g_Config.lbot_items[key].silent_fov, 0.f, (val), ("silent_fov"));
		SetupValue(g_Config.lbot_items[key].rcs_fov, 0.f, (val), ("rcs_fov"));
		SetupValue(g_Config.lbot_items[key].smooth, 1, (val), ("smooth"));
		SetupValue(g_Config.lbot_items[key].rcs_smooth, 1, (val), ("rcs_smooth"));
		SetupValue(g_Config.lbot_items[key].shot_delay, 0, (val), ("shot_delay"));
		SetupValue(g_Config.lbot_items[key].kill_delay, 0, (val), ("kill_delay"));
		SetupValue(g_Config.lbot_items[key].rcs_x, 100, (val), ("rcs_x"));
		SetupValue(g_Config.lbot_items[key].rcs_y, 100, (val), ("rcs_y"));
		SetupValue(g_Config.lbot_items[key].rcs_start, 1, (val), ("rcs_start"));
		SetupValue(g_Config.lbot_items[key].min_damage, 1, (val), ("min_damage"));
	}
}

void CConfig::SetupRage() {
	SetupValue(g_Config.rbot_enable, false, ("rbot"), ("enable"));
	SetupValue(g_Config.rbot_baim_while_moving, false, ("rbot"), ("baim_while_moving"));
	SetupValue(g_Config.rbot_force_unlage, false, ("rbot"), ("force_unlage"));
	SetupValue(g_Config.rbot_lby_prediction, false, ("rbot"), ("lby_prediction"));
	SetupValue(g_Config.rbot_flag_prediction, false, ("rbot"), ("flag_prediction"));
	SetupValue(g_Config.rbot_autoscope, false, ("rbot"), ("autoscope"));
	SetupValue(g_Config.rbot_baimmode, 0, ("rbot"), ("baimmode"));
	SetupValue(g_Config.rbot_aimstep, false, ("rbot"), ("aimstep"));
	SetupValue(g_Config.rbot_guccigang, false, ("rbot"), ("guccigang"));
	SetupValue(g_Config.rbot_extrapolation, false, ("rbot"), ("extrapolation"));
	SetupValue(g_Config.rbot_resolver, false, ("rbot"), ("resolver"));
	SetupValue(g_Config.rbot_resolver_air_baim, false, ("rbot"), ("resolver_air_baim"));
	SetupValue(g_Config.rbot_shooting_mode, 0, ("rbot"), ("shooting_mode"));
	SetupValue(g_Config.rbot_lagcompensation, false, ("rbot"), ("lagcompensation")); //todo ?
	SetupValue(g_Config.rbot_lagcompensation_type, 0, ("rbot"), ("lagcompensation_type")); //todo !

	// desync shit start
	SetupValue(g_Config.rbot_aa_desync, 0, ("rbot"), ("aa_desync"));
	SetupValue(g_Config.rbot_aa_desync_key, 0x0, ("rbot"), ("aa_desync_key"));
	SetupValue(g_Config.rbot_aa_desync_side, 1.f, ("rbot"), ("aa_desync_side"));
	// desync shit end

	SetupValue(g_Config.rbot_aa, false, ("rbot"), ("aa"));
	SetupValue(g_Config.rbot_slidewalk, false, ("rbot"), ("slidewalk"));
	SetupValue(g_Config.rbot_manual_aa_state, 0, ("rbot"), ("manual_aa_state"));
	SetupValue(g_Config.rbot_manual_key_right, 0x0, ("rbot"), ("manual_key_right"));
	SetupValue(g_Config.rbot_manual_key_left, 0x0, ("rbot"), ("manual_key_left"));
	SetupValue(g_Config.rbot_manual_key_back, 0x0, ("rbot"), ("manual_key_back"));

	// key rage shit start
	SetupValue(g_Config.rbot_fakeduck, false, ("rbot"), ("fakeduck"));
	SetupValue(g_Config.rbot_slowwalk, false, ("rbot"), ("slowwalk"));
	SetupValue(g_Config.rbot_slowwalk_mod, 0.f, ("rbot"), ("slowwalk_mod"));
	SetupValue(g_Config.rbot_fakeduck_key, 0x0, ("rbot"), ("fakeduck_key"));
	SetupValue(g_Config.rbot_slowwalk_key, 0x0, ("rbot"), ("slowwalk_key"));
	// key rage shit end

	SetupValue(g_Config.rbot_aa_lby_breaker, false, ("rbot"), ("aa_lby_breaker"));
	SetupValue(g_Config.rbot_aa_fake_lby_breaker, false, ("rbot"), ("aa_fake_lby_breaker"));
	SetupValue(g_Config.rbot_aa_lby_breaker_yaw, 0.f, ("rbot"), ("aa_lby_breaker_yaw"));
	SetupValue(g_Config.rbot_aa_lby_breaker_freestanding, false, ("rbot"), ("aa_lby_breaker_freestanding"));
	SetupValue(g_Config.rbot_autostop, false, ("rbot"), ("autostop"));
	SetupValue(g_Config.rbot_autocrouch, false, ("rbot"), ("autocrouch"));

	SetupValue(g_Config.rbot_aa_spinbot_speed, 5.f, ("rbot"), ("aa_spinbot_speed"));
	SetupValue(g_Config.rbot_aa_stand_pitch, 0, ("rbot"), ("aa_stand_pitch"));
	SetupValue(g_Config.rbot_aa_move_pitch, 0, ("rbot"), ("aa_move_pitch"));
	SetupValue(g_Config.rbot_aa_air_pitch, 0, ("rbot"), ("aa_air_pitch"));
	SetupValue(g_Config.rbot_aa_stand_pitch_custom, 0.f, ("rbot"), ("aa_stand_pitch_custom"));
	SetupValue(g_Config.rbot_aa_move_pitch_custom, 0.f, ("rbot"), ("aa_move_pitch_custom"));
	SetupValue(g_Config.rbot_aa_air_pitch_custom, 0.f, ("rbot"), ("aa_air_pitch_custom"));

	SetupValue(g_Config.rbot_aa_stand_real_add_yaw_add, 0, ("rbot"), ("aa_stand_real_add_yaw_add"));
	SetupValue(g_Config.rbot_aa_move_real_add_yaw_add, 0, ("rbot"), ("aa_move_real_add_yaw_add"));
	SetupValue(g_Config.rbot_aa_air_real_add_yaw_add, 0, ("rbot"), ("aa_air_real_add_yaw_add"));
	SetupValue(g_Config.rbot_aa_stand_fake_add_yaw_add, 0, ("rbot"), ("aa_stand_fake_add_yaw_add"));
	SetupValue(g_Config.rbot_aa_move_fake_add_yaw_add, 0, ("rbot"), ("aa_move_fake_add_yaw_add"));
	SetupValue(g_Config.rbot_aa_air_fake_add_yaw_add, 0, ("rbot"), ("aa_air_fake_add_yaw_add"));

	SetupValue(g_Config.rbot_aa_stand_real_yaw, 0, ("rbot"), ("aa_stand_real_yaw"));
	SetupValue(g_Config.rbot_aa_move_real_yaw, 0, ("rbot"), ("aa_move_real_yaw"));
	SetupValue(g_Config.rbot_aa_air_real_yaw, 0, ("rbot"), ("aa_air_real_yaw"));
	SetupValue(g_Config.rbot_aa_stand_fake_yaw, 0, ("rbot"), ("aa_stand_fake_yaw"));
	SetupValue(g_Config.rbot_aa_move_fake_yaw, 0, ("rbot"), ("aa_move_fake_yaw"));
	SetupValue(g_Config.rbot_aa_air_fake_yaw, 0, ("rbot"), ("aa_air_fake_yaw"));

	SetupValue(g_Config.rbot_aa_stand_real_add_yaw_add_range, 0.f, ("rbot"), ("aa_stand_real_add_yaw_add_range"));
	SetupValue(g_Config.rbot_aa_move_real_add_yaw_add_range, 0.f, ("rbot"), ("aa_move_real_add_yaw_add_range"));
	SetupValue(g_Config.rbot_aa_air_real_add_yaw_add_range, 0.f, ("rbot"), ("aa_air_real_add_yaw_add_range"));
	SetupValue(g_Config.rbot_aa_stand_fake_add_yaw_add_range, 0.f, ("rbot"), ("aa_stand_fake_add_yaw_add_range"));
	SetupValue(g_Config.rbot_aa_move_fake_add_yaw_add_range, 0.f, ("rbot"), ("aa_move_fake_add_yaw_add_range"));
	SetupValue(g_Config.rbot_aa_air_fake_add_yaw_add_range, 0.f, ("rbot"), ("aa_air_fake_add_yaw_add_range"));

	//$
	for (int a = 0; a <= 7; a++)
	{
		SetupValue(g_Config.rbot_mindamage[a], 0.f, ("rbot"), (Utils::Format("mindamage_%d", a)));
		SetupValue(g_Config.rbot_min_hitchance[a], 0.f, ("rbot"), (Utils::Format("min_hitchance_%d", a)));
		SetupValue(g_Config.rbot_baim_after_shots[a], 0, ("rbot"), (Utils::Format("baim_after_shots_%d", a)));

		SetupValue(g_Config.rbot_hitbox_head_scale[a], 0.f, ("rbot"), (Utils::Format("hitbox_head_scale_%d", a)));
		SetupValue(g_Config.rbot_hitbox_neck_scale[a], 0.f, ("rbot"), (Utils::Format("hitbox_neck_scale_%d", a)));
		SetupValue(g_Config.rbot_hitbox_chest_scale[a], 0.f, ("rbot"), (Utils::Format("hitbox_chest_scale_%d", a)));
		SetupValue(g_Config.rbot_hitbox_pelvis_scale[a], 0.f, ("rbot"), (Utils::Format("hitbox_pelvis_scale_%d", a)));
		SetupValue(g_Config.rbot_hitbox_stomach_scale[a], 0.f, ("rbot"), (Utils::Format("hitbox_stomach_scale_%d", a)));
		SetupValue(g_Config.rbot_hitbox_arm_scale[a], 0.f, ("rbot"), (Utils::Format("hitbox_arm_scale_%d", a)));
		SetupValue(g_Config.rbot_hitbox_leg_scale[a], 0.f, ("rbot"), (Utils::Format("hitbox_leg_scale_%d", a)));
		SetupValue(g_Config.rbot_hitbox_foot_scale[a], 0.f, ("rbot"), (Utils::Format("hitbox_foot_scale_%d", a)));
		SetupValue(g_Config.rbot_hitbox_head[a], false, ("rbot"), (Utils::Format("hitbox_head_%d", a)));
		SetupValue(g_Config.rbot_hitbox_neck[a], false, ("rbot"), (Utils::Format("hitbox_neck_%d", a)));
		SetupValue(g_Config.rbot_hitbox_chest[a], false, ("rbot"), (Utils::Format("hitbox_chest_%d", a)));
		SetupValue(g_Config.rbot_hitbox_pelvis[a], false, ("rbot"), (Utils::Format("hitbox_pelvis_%d", a)));
		SetupValue(g_Config.rbot_hitbox_stomach[a], false, ("rbot"), (Utils::Format("hitbox_stomach_%d", a)));
		SetupValue(g_Config.rbot_hitbox_arm[a], false, ("rbot"), (Utils::Format("hitbox_arm_%d", a)));
		SetupValue(g_Config.rbot_hitbox_leg[a], false, ("rbot"), (Utils::Format("hitbox_leg_%d", a)));
		SetupValue(g_Config.rbot_hitbox_foot[a], false, ("rbot"), (Utils::Format("hitbox_foot_%d", a)));
	}

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

}

void CConfig::SetupVisuals() {
	SetupValue(g_Config.vis_misc_noscope, false, ("visual"), ("noscope"));
	SetupValue(g_Config.vis_misc_drawteaserrange, false, ("visual"), ("drawteaserrange"));
	SetupValue(g_Config.vis_misc_bullettracer, false, ("visual"), ("bullettracer"));
	SetupValue(g_Config.vis_misc_autowall_crosshair, false, ("visual"), ("autowall_crosshair"));
	SetupValue(g_Config.vis_misc_nosmoke, false, ("visual"), ("nosmoke"));
	SetupValue(g_Config.vis_misc_hitmarker, false, ("visual"), ("hitmarker"));
	SetupValue(g_Config.vis_misc_hitmarker_sound, false, ("visual"), ("hitmarker_sound"));
	SetupValue(g_Config.vis_misc_disable_postprocessing, false, ("visual"), ("disable_postprocessing"));
	SetupValue(g_Config.vis_misc_fullbright, false, ("visual"), ("fullbright"));
	SetupValue(g_Config.vis_misc_nightmode, false, ("visual"), ("nightmode"));
	SetupValue(g_Config.vis_misc_force_crosshair, false, ("visual"), ("force_crosshair"));
	SetupValue(g_Config.vis_misc_kzwindow, false, ("visual"), ("misc_kzwindow"));
	SetupValue(g_Config.vis_misc_hitmarker_volume, 75, ("visual"), ("hitmarker_volume"));
	SetupValue(g_Config.vis_misc_localanimfix, false, ("visual"), ("localanimfix"));
	SetupValue(g_Config.vis_misc_screenshotreturner, false, ("visual"), ("screenshotreturner"));
	SetupValue(g_Config.vis_misc_thirdperson, false, ("visual"), ("thirdperson"));
	SetupValue(g_Config.vis_misc_thirdperson_dist, 50.f, ("visual"), ("thirdperson_dist"));
	SetupValue(g_Config.vis_misc_thirdperson_hotkey, false, ("visual"), ("thirdperson_hotkey"));
	SetupValue(g_Config.vis_misc_noflash, false, ("visual"), ("noflash"));
	SetupValue(g_Config.vis_misc_draw_circle, false, ("visual"), ("draw_circle"));
	SetupValue(g_Config.vis_misc_disable_scope_zoom, false, ("visual"), ("disable_scope_zoom"));
}

void CConfig::SetupChams() {
	SetupValue(g_Config.chams_local, false, ("chams"), ("local"));
	SetupValue(g_Config.chams_local_ghost, false, ("chams"), ("local_ghost"));
	SetupValue(g_Config.chams_local_fakelag, false, ("chams"), ("local_fakelag"));
	SetupValue(g_Config.chams_local_scope, false, ("chams"), ("local_scope"));
	SetupValue(g_Config.chams_local_arms, false, ("chams"), ("local_arms"));
	SetupValue(g_Config.chams_local_weapon, false, ("chams"), ("local_weapon"));
	SetupValue(g_Config.chams_team, false, ("chams"), ("team"));
	SetupValue(g_Config.chams_enemy, false, ("chams"), ("enemy"));
	SetupValue(g_Config.chams_type_backtrack, false, ("chams"), ("type_backtrack"));
	SetupValue(g_Config.chams_backtrack, false, ("chams"), ("backtrack"));
	SetupValue(g_Config.chams_mode_backtrack, 1, ("chams"), ("mode_backtrack"));
	SetupValue(g_Config.chams_mode_local_ghost, 1, ("chams"), ("mode_local_ghost"));
	SetupValue(g_Config.chams_mode_local_fakelag, 1, ("chams"), ("mode_local_fakelag"));
	SetupValue(g_Config.chams_mode_local, 1, ("chams"), ("mode_local"));
	SetupValue(g_Config.chams_mode_local_arms, 1, ("chams"), ("mode_local_arms"));
	SetupValue(g_Config.chams_mode_local_weapon, 1, ("chams"), ("mode_local_weapon"));
	SetupValue(g_Config.chams_mode_team, 1, ("chams"), ("mode_team"));
	SetupValue(g_Config.chams_mode_enemy, 1, ("chams"), ("mode_enemy"));
}

void CConfig::SetupGlow() {

	SetupValue(g_Config.glow_team_enable, false, ("glow"), ("team_enabled"));
	SetupValue(g_Config.glow_team_type, 0, ("glow"), ("team_type"));
	SetupValue(g_Config.glow_enemy_enable, false, ("glow"), ("enemy_enabled"));
	SetupValue(g_Config.glow_enemy_type, 0, ("glow"), ("enemy_type"));
	SetupValue(g_Config.glow_chickens, false, ("glow"), ("chickens_enabled"));
	SetupValue(g_Config.glow_chickens_type, 0, ("glow"), ("glow_chickens_type"));
	SetupValue(g_Config.glow_planted_c4, false, ("glow"), ("planted_c4_enabled"));
	SetupValue(g_Config.glow_planted_c4_type, 0, ("glow"), ("planted_c4_type"));
	SetupValue(g_Config.glow_planted_warning, false, ("glow"), ("planted_warning"));
	SetupValue(g_Config.glow_defuse_kits, false, ("glow"), ("defuse_kits_enabled"));
	SetupValue(g_Config.glow_defuse_type, 0, ("glow"), ("defuse_kits_type"));
	SetupValue(g_Config.glow_grenade, false, ("glow"), ("grenade_enabled"));
	SetupValue(g_Config.glow_grenade_type, 0, ("glow"), ("grenade_type"));
	SetupValue(g_Config.glow_weapons, false, ("glow"), ("glow_weapons_enabled"));
	SetupValue(g_Config.glow_weapons_type, 0, ("glow"), ("glow_weapons_type"));
	SetupValue(g_Config.glow_c4_carrier, false, ("glow"), ("c4_carrier_enabled"));
	SetupValue(g_Config.glow_c4_carrier_type, 0, ("glow"), ("c4_carrier_type"));
}

void CConfig::SetupEsp() {
	SetupValue(g_Config.esp_local_enable, false, ("esp"), ("local_enabled"));
	SetupValue(g_Config.esp_local_boxes, false, ("esp"), ("local_boxes"));
	SetupValue(g_Config.esp_local_boxes_fill, false, ("esp"), ("local_boxes_fill"));
	SetupValue(g_Config.esp_local_boxes_outline, false, ("esp"), ("local_boxes_outline"));
	SetupValue(g_Config.esp_local_boxes_type, false, ("esp"), ("local_boxes_type"));
	SetupValue(g_Config.esp_local_names, false, ("esp"), ("local_names"));
	SetupValue(g_Config.esp_local_health, false, ("esp"), ("local_health"));
	SetupValue(g_Config.esp_local_armour, false, ("esp"), ("local_armour"));
	SetupValue(g_Config.esp_local_health_norender, false, ("esp"), ("local_health_norender"));
	SetupValue(g_Config.esp_local_armour_norender, false, ("esp"), ("local_armour_norender"));
	SetupValue(g_Config.esp_local_weapons, false, ("esp"), ("local_weapons"));
	SetupValue(g_Config.esp_local_weapons_ammo, false, ("esp"), ("local_weapons_ammo"));
	SetupValue(g_Config.esp_local_flags, false, ("esp"), ("local_flags"));
	SetupValue(g_Config.esp_local_lines, false, ("esp"), ("local_lines"));
	SetupValue(g_Config.esp_local_headbox, false, ("esp"), ("local_headbox"));

	SetupValue(g_Config.esp_team_enable, false, ("esp"), ("team_enabled"));
	SetupValue(g_Config.esp_team_visible_only, false, ("esp"), ("team_visible_only"));
	SetupValue(g_Config.esp_team_boxes, false, ("esp"), ("team_boxes"));
	SetupValue(g_Config.esp_team_boxes_fill, false, ("esp"), ("team_boxes_fill"));
	SetupValue(g_Config.esp_team_boxes_outline, false, ("esp"), ("team_boxes_outline"));
	SetupValue(g_Config.esp_team_boxes_type, false, ("esp"), ("team_boxes_type"));
	SetupValue(g_Config.esp_team_names, false, ("esp"), ("team_names"));
	SetupValue(g_Config.esp_team_health, false, ("esp"), ("team_health"));
	SetupValue(g_Config.esp_team_armour, false, ("esp"), ("team_armour"));
	SetupValue(g_Config.esp_team_health_norender, false, ("esp"), ("team_health_norender"));
	SetupValue(g_Config.esp_team_armour_norender, false, ("esp"), ("team_armour_norender"));
	SetupValue(g_Config.esp_team_weapons, false, ("esp"), ("team_weapons"));
	SetupValue(g_Config.esp_team_weapons_ammo, false, ("esp"), ("team_weapons_ammo"));
	SetupValue(g_Config.esp_team_snaplines, false, ("esp"), ("team_snaplines"));
	SetupValue(g_Config.esp_team_pov, false, ("esp"), ("team_pov"));
	SetupValue(g_Config.esp_team_flags, false, ("esp"), ("team_flags"));
	SetupValue(g_Config.esp_team_lines, false, ("esp"), ("team_lines"));
	SetupValue(g_Config.esp_team_headbox, false, ("esp"), ("team_headbox"));

	SetupValue(g_Config.esp_enemy_enable, false, ("esp"), ("enemy_enabled"));
	SetupValue(g_Config.esp_enemy_visible_only, false, ("esp"), ("enemy_visible_only"));
	SetupValue(g_Config.esp_enemy_boxes, false, ("esp"), ("enemy_boxes"));
	SetupValue(g_Config.esp_enemy_boxes_fill, false, ("esp"), ("enemy_boxes_fill"));
	SetupValue(g_Config.esp_enemy_boxes_outline, false, ("esp"), ("enemy_boxes_outline"));
	SetupValue(g_Config.esp_enemy_boxes_type, false, ("esp"), ("enemy_boxes_type"));
	SetupValue(g_Config.esp_enemy_names, false, ("esp"), ("enemy_names"));
	SetupValue(g_Config.esp_enemy_health, false, ("esp"), ("enemy_health"));
	SetupValue(g_Config.esp_enemy_armour, false, ("esp"), ("enemy_armour"));
	SetupValue(g_Config.esp_enemy_health_norender, false, ("esp"), ("enemy_health_norender"));
	SetupValue(g_Config.esp_enemy_armour_norender, false, ("esp"), ("enemy_armour_norender"));
	SetupValue(g_Config.esp_enemy_weapons, false, ("esp"), ("enemy_weapons"));
	SetupValue(g_Config.esp_enemy_weapons_ammo, false, ("esp"), ("enemy_weapons_ammo"));
	SetupValue(g_Config.esp_enemy_snaplines, false, ("esp"), ("enemy_snaplines"));
	SetupValue(g_Config.esp_enemy_info, false, ("esp"), ("enemy_info"));
	SetupValue(g_Config.esp_enemy_lby_timer, false, ("esp"), ("enemy_lby_timer"));
	SetupValue(g_Config.esp_enemy_pov, false, ("esp"), ("enemy_pov"));
	SetupValue(g_Config.esp_enemy_lclines, false, ("esp"), ("enemy_lclines"));
	SetupValue(g_Config.esp_enemy_flags, false, ("esp"), ("enemy_flags"));
	SetupValue(g_Config.esp_enemy_lines, false, ("esp"), ("enemy_lines"));
	SetupValue(g_Config.esp_enemy_headbox, false, ("esp"), ("enemy_headbox"));

	SetupValue(g_Config.esp_misc_grenade_prediction, false, ("esp"), ("misc_grenade_prediction"));
	SetupValue(g_Config.esp_misc_bullettracer, false, ("esp"), ("misc_bullettracer"));
	SetupValue(g_Config.esp_misc_debug_overlay, false, ("esp"), ("misc_debug_overlay"));
	SetupValue(g_Config.esp_misc_aa_indicator, false, ("esp"), ("misc_aa_indicator"));
	SetupValue(g_Config.esp_misc_event_logger, false, ("esp"), ("event_logger"));
	SetupValue(g_Config.esp_misc_snaplines_type, 0, ("esp"), ("misc_snaplines_type"));
	SetupValue(g_Config.esp_misc_pov_dist, 25.f, ("esp"), ("misc_pov_dist"));
	SetupValue(g_Config.esp_misc_pov_size, 20.f, ("esp"), ("misc_pov_size"));
	SetupValue(g_Config.esp_dropped_weapons_type, 0, ("esp"), ("dropped_weapons_type"));
	SetupValue(g_Config.esp_dropped_weapons_info, false, ("esp"), ("dropped_weapons_info"));
	SetupValue(g_Config.esp_planted_c4, false, ("esp"), ("planted_c4"));
	SetupValue(g_Config.esp_box_c4, false, ("esp"), ("box_c4"));
	SetupValue(g_Config.esp_timer_c4, 0, ("esp"), ("timer_c4"));
	SetupValue(g_Config.esp_misc_grenade, 0, ("esp"), ("misc_grenade"));
	SetupValue(g_Config.esp_misc_spectator_list, false, ("esp"), ("misc_spectator_list"));
	SetupValue(g_Config.esp_misc_dangerzone_item_esp, false, ("esp"), ("misc_dangerzone_item_esp"));
	SetupValue(g_Config.esp_misc_dangerzone_item_esp_dist, false, ("esp"), ("misc_dangerzone_item_esp_dist"));
}

void CConfig::SetupMisc() {
	SetupValue(g_Config.misc_bhop, false, ("misc"), ("bhop"));
	SetupValue(g_Config.misc_autostrafe, false, ("misc"), ("autostrafe"));
	SetupValue(g_Config.misc_autoaccept, 0, ("misc"), ("autoaccept"));
	SetupValue(g_Config.misc_autoblock, false, ("misc"), ("autoblock"));
	SetupValue(g_Config.misc_no_hands, false, ("misc"), ("no_hands"));
	SetupValue(g_Config.misc_no_recoil, false, ("misc"), ("no_recoil"));
	SetupValue(g_Config.misc_retrack_speed, 4.f, ("misc"), ("retrack_speed"));

	//che za hujnya gde eto blyat dolzhno byt nahuj
	SetupValue(g_Config.misc_fakelag_enable, false, ("rbot"), ("fakelag_enable"));
	SetupValue(g_Config.misc_fakelag_ticks_standing, 0, ("rbot"), ("fakelag_ticks_standing"));
	SetupValue(g_Config.misc_fakelag_ticks_air, 0, ("rbot"), ("fakelag_ticks_air"));
	SetupValue(g_Config.misc_fakelag_mode_air, 0, ("rbot"), ("fakelag_mode_air"));
	SetupValue(g_Config.misc_fakelag_ticks_moving, 0, ("rbot"), ("fakelag_ticks_moving"));
	SetupValue(g_Config.misc_fakelag_mode_moving, 0, ("rbot"), ("fakelag_mode_moving"));
	SetupValue(g_Config.misc_fakelag_on_key, false, ("rbot"), ("fakelag_on_key"));
	SetupValue(g_Config.misc_fakelag_key, 0x0, ("rbot"), ("fakelag_key"));

	SetupValue(g_Config.misc_debugfunc_key, 0x0, ("misc"), ("debugfunc_key"));
	SetupValue(g_Config.misc_debugfunc_val, 10.f, ("misc"), ("debugfunc_val"));
	SetupValue(g_Config.misc_debugfunc, false, ("misc"), ("debugfunc"));

	SetupValue(g_Config.misc_legit_resolver_key, 0x0, ("misc"), ("legit_resolver_key"));
	SetupValue(g_Config.misc_legit_resolver, false, ("misc"), ("legit_resolver"));
	SetupValue(g_Config.misc_choke_limit, false, ("misc"), ("choke_limit"));

	SetupValue(g_Config.misc_airstuck_key, 0x0, ("misc"), ("airstuck_key"));
	SetupValue(g_Config.misc_airstuck, false, ("misc"), ("airstuck"));
	SetupValue(g_Config.misc_automatic_weapons, false, ("misc"), ("automatic_weapons"));
	SetupValue(g_Config.misc_knifebot, false, ("misc"), ("knifebot"));
	SetupValue(g_Config.misc_knifebot_dist, 75, ("misc"), ("knifebot_dist"));
	SetupValue(g_Config.misc_cash_hack, false, ("misc"), ("cash_hack"));
	SetupValue(g_Config.misc_franzjump, false, ("misc"), ("franzjump"));
	SetupValue(g_Config.misc_cash_hack_val, 100, ("misc"), ("cash_hack_val"));
	SetupValue(g_Config.misc_ping_spike, false, ("misc"), ("ping_spike"));
	SetupValue(g_Config.misc_ping_spike_val, 0, ("misc"), ("ping_spike_val"));
	SetupValue(g_Config.misc_request_file_key, 0x0, ("misc"), ("request_file_key"));
	SetupValue(g_Config.misc_request_file, false, ("misc"), ("request_file"));
	SetupValue(g_Config.misc_debugfunc_val, 2, ("misc"), ("debugfunc_val"));
	SetupValue(g_Config.misc_fov, 90.0f, ("misc"), ("fov"));
	SetupValue(g_Config.misc_viewmodel_fov, 60.0f, ("misc"), ("viewmodel_fov"));
	SetupValue(g_Config.misc_showranks, false, ("misc"), ("showranks"));
	SetupValue(g_Config.misc_anti_kick, false, ("misc"), ("anti_kick"));
	SetupValue(g_Config.misc_clantagchanger, false, ("misc"), ("clantagchanger"));
	SetupValue(g_Config.misc_clantagchanger_type, 0, ("misc"), ("clantagchanger_type"));

	SetupValue(g_Config.misc_buybot, false, ("misc"), ("buybot"));
	SetupValue(g_Config.misc_buybot_pistol, 0, ("misc"), ("buybot_pistol"));
	SetupValue(g_Config.misc_buybot_weapon, 0, ("misc"), ("buybot_weapon"));

	SetupValue(g_Config.misc_buybot_grenade[0], false, ("lbot"), ("misc_buybot_grenade_0"));
	SetupValue(g_Config.misc_buybot_grenade[1], false, ("lbot"), ("misc_buybot_grenade_1"));
	SetupValue(g_Config.misc_buybot_grenade[2], false, ("lbot"), ("misc_buybot_grenade_2"));
	SetupValue(g_Config.misc_buybot_grenade[3], false, ("lbot"), ("misc_buybot_grenade_3"));
	SetupValue(g_Config.misc_buybot_grenade[4], false, ("lbot"), ("misc_buybot_grenade_4"));
	SetupValue(g_Config.misc_buybot_grenade[5], false, ("lbot"), ("misc_buybot_grenade_5"));
	SetupValue(g_Config.misc_buybot_grenade[6], false, ("lbot"), ("misc_buybot_grenade_6"));
	SetupValue(g_Config.misc_buybot_grenade[7], false, ("lbot"), ("misc_buybot_grenade_7"));
}


void CConfig::SetupSkins() {
	for (auto& key : k_weapons_names) {
		SetupValue(g_Config.skins.m_items[key.definition_index].name, "Default", (key.name), ("name"));
		SetupValue(g_Config.skins.m_items[key.definition_index].enabled, 0, (key.name), ("enabled"));
		SetupValue(g_Config.skins.m_items[key.definition_index].definition_vector_index, 0, (key.name), ("definition_vector_index"));
		SetupValue(g_Config.skins.m_items[key.definition_index].definition_index, 1, (key.name), ("definition_index"));
		SetupValue(g_Config.skins.m_items[key.definition_index].paint_kit_vector_index, 0, (key.name), ("paint_kit_vector_index"));
		SetupValue(g_Config.skins.m_items[key.definition_index].paint_kit_index, 0, (key.name), ("paint_kit_index"));
		SetupValue(g_Config.skins.m_items[key.definition_index].definition_override_vector_index, 0, (key.name), ("definition_override_vector_index"));
		SetupValue(g_Config.skins.m_items[key.definition_index].definition_override_index, 0, (key.name), ("definition_override_index"));
		SetupValue(g_Config.skins.m_items[key.definition_index].seed, 0, (key.name), ("seed"));
		SetupValue(g_Config.skins.m_items[key.definition_index].stat_trak, 0, (key.name), ("stat_trak"));
		SetupValue(g_Config.skins.m_items[key.definition_index].wear, 0.0f, (key.name), ("wear"));
		SetupValue(g_Config.skins.m_items[key.definition_index].custom_name, "", (key.name), ("custom_name"));
	}
}


void CConfig::SetupColors() {
	SetupColor(g_Config.color_esp_gun_immunity, "color_esp_gun_immunity");
	SetupColor(g_Config.color_esp_local_boxes, "color_esp_local_boxes");
	SetupColor(g_Config.color_esp_local_boxes_fill, "color_esp_local_boxes_fill");
	SetupColor(g_Config.color_esp_local_names, "color_esp_local_names");
	SetupColor(g_Config.color_esp_local_armour, "color_esp_local_armour");
	SetupColor(g_Config.color_esp_local_weapons, "color_esp_local_weapons");
	SetupColor(g_Config.color_esp_local_flags, "color_esp_local_flags");
	SetupColor(g_Config.color_esp_local_headbox, "color_esp_local_headbox");
	SetupColor(g_Config.color_esp_local_desync, "color_esp_local_desync");
	SetupColor(g_Config.color_esp_local_lbylines, "color_esp_local_lbylines");
	SetupColor(g_Config.color_esp_team_visible_boxes, "color_esp_team_visible_boxes");
	SetupColor(g_Config.color_esp_team_visible_boxes_fill, "color_esp_team_visible_boxes_fill");
	SetupColor(g_Config.color_esp_team_visible_names, "color_esp_team_visible_names");
	SetupColor(g_Config.color_esp_team_visible_armour, "color_esp_team_visible_armour");
	SetupColor(g_Config.color_esp_team_visible_weapons, "color_esp_team_visible_weapons");
	SetupColor(g_Config.color_esp_team_visible_snaplines, "color_esp_team_visible_snaplines");
	SetupColor(g_Config.color_esp_team_visible_pov, "color_esp_team_visible_pov");
	SetupColor(g_Config.color_esp_team_visible_flags, "color_esp_team_visible_flags");
	SetupColor(g_Config.color_esp_team_visible_headbox, "color_esp_team_visible_flags");
	SetupColor(g_Config.color_esp_team_visible_anglines, "color_esp_team_visible_anglines");
	SetupColor(g_Config.color_esp_team_visible_lbylines, "color_esp_team_visible_lbylines");
	SetupColor(g_Config.color_esp_team_invisible_boxes, "color_esp_team_invisible_boxes");
	SetupColor(g_Config.color_esp_team_invisible_boxes_fill, "color_esp_team_invisible_boxes_fill");
	SetupColor(g_Config.color_esp_team_invisible_names, "color_esp_team_invisible_names");
	SetupColor(g_Config.color_esp_team_invisible_armour, "color_esp_team_invisible_armour");
	SetupColor(g_Config.color_esp_team_invisible_weapons, "color_esp_team_invisible_weapons");
	SetupColor(g_Config.color_esp_team_invisible_snaplines, "color_esp_team_invisible_snaplines");
	SetupColor(g_Config.color_esp_team_invisible_pov, "color_esp_enemy_invisible_pov");
	SetupColor(g_Config.color_esp_team_invisible_flags, "color_esp_team_invisible_flags");
	SetupColor(g_Config.color_esp_team_invisible_headbox, "color_esp_team_invisible_headbox");
	SetupColor(g_Config.color_esp_team_invisible_anglines, "color_esp_team_invisible_anglines");
	SetupColor(g_Config.color_esp_team_invisible_lbylines, "color_esp_team_invisible_lbylines");
	SetupColor(g_Config.color_esp_enemy_visible_boxes, "color_esp_enemy_visible_boxes");
	SetupColor(g_Config.color_esp_enemy_visible_boxes_fill, "color_esp_enemy_visible_boxes_fill");
	SetupColor(g_Config.color_esp_enemy_visible_names, "color_esp_enemy_visible_names");
	SetupColor(g_Config.color_esp_enemy_visible_armour, "color_esp_enemy_visible_armour");
	SetupColor(g_Config.color_esp_enemy_visible_weapons, "color_esp_enemy_visible_weapons");
	SetupColor(g_Config.color_esp_enemy_visible_snaplines, "color_esp_enemy_visible_snaplines");
	SetupColor(g_Config.color_esp_enemy_visible_info, "color_esp_enemy_visible_info");
	SetupColor(g_Config.color_esp_enemy_visible_lby_timer, "color_esp_enemy_visible_lby_timer");
	SetupColor(g_Config.color_esp_enemy_visible_pov, "color_esp_enemy_visible_pov");
	SetupColor(g_Config.color_esp_enemy_visible_flags, "color_esp_enemy_visible_flags");
	SetupColor(g_Config.color_esp_enemy_visible_lcline, "color_esp_enemy_visible_lcline");
	SetupColor(g_Config.color_esp_enemy_visible_headbox, "color_esp_enemy_visible_headbox");
	SetupColor(g_Config.color_esp_enemy_visible_anglines, "color_esp_enemy_visible_anglines");
	SetupColor(g_Config.color_esp_enemy_visible_lbylines, "color_esp_enemy_visible_lbylines");
	SetupColor(g_Config.color_esp_enemy_invisible_boxes, "color_esp_enemy_invisible_boxes");
	SetupColor(g_Config.color_esp_enemy_invisible_boxes_fill, "color_esp_enemy_invisible_boxes_fill");
	SetupColor(g_Config.color_esp_enemy_invisible_names, "color_esp_enemy_invisible_names");
	SetupColor(g_Config.color_esp_enemy_invisible_armour, "color_esp_enemy_invisible_armour");
	SetupColor(g_Config.color_esp_enemy_invisible_weapons, "color_esp_enemy_invisible_weapons");
	SetupColor(g_Config.color_esp_enemy_invisible_snaplines, "color_esp_enemy_invisible_snaplines");
	SetupColor(g_Config.color_esp_enemy_invisible_info, "color_esp_enemy_invisible_info");
	SetupColor(g_Config.color_esp_enemy_invisible_lby_timer, "color_esp_enemy_invisible_lby_timer");
	SetupColor(g_Config.color_esp_enemy_invisible_pov, "color_esp_enemy_invisible_lby_timer");
	SetupColor(g_Config.color_esp_enemy_invisible_flags, "color_esp_enemy_invisible_flags");
	SetupColor(g_Config.color_esp_enemy_invisible_lcline, "color_esp_enemy_invisible_lcline");
	SetupColor(g_Config.color_esp_enemy_invisible_headbox, "color_esp_enemy_invisible_headbox");
	SetupColor(g_Config.color_esp_enemy_invisible_anglines, "color_esp_enemy_invisible_anglines");
	SetupColor(g_Config.color_esp_enemy_invisible_lbylines, "color_esp_enemy_invisible_lbylines");
	SetupColor(g_Config.vis_misc_draw_circle_clr, "vis_misc_draw_circle_clr");
	SetupColor(g_Config.vis_misc_aa_circle_clr, "vis_misc_aa_circle_clr");
	SetupColor(g_Config.vis_misc_clr_modificate, "vis_misc_clr_modificate");
	SetupColor(g_Config.vis_misc_clr_beams, "vis_misc_clr_beams");
	SetupColor(g_Config.color_esp_dropped_weapons, "color_esp_dropped_weapons");
	SetupColor(g_Config.color_chams_scoped_visible, "color_chams_scoped_visible");
	SetupColor(g_Config.color_chams_local_visible, "color_chams_local_visible");
	SetupColor(g_Config.color_chams_team_visible, "color_chams_team_visible");
	SetupColor(g_Config.color_chams_enemy_visible, "color_chams_enemy_visible");
	SetupColor(g_Config.color_chams_local_arms_visible, "color_chams_local_arms_visible");
	SetupColor(g_Config.color_chams_local_weapon_visible, "color_chams_local_weapon_visible");
	SetupColor(g_Config.color_chams_local_fakelag_visible, "color_chams_local_fakelag_visible");
	SetupColor(g_Config.color_chams_local_ghost_visible, "color_chams_local_ghost_visible");
	SetupColor(g_Config.color_chams_team_invisible, "color_chams_team_invisible");
	SetupColor(g_Config.color_chams_enemy_invisible, "color_chams_enemy_invisible");
	SetupColor(g_Config.color_chams_local_arms_invisible, "color_chams_local_arms_invisible");
	SetupColor(g_Config.color_chams_local_weapon_invisible, "color_chams_local_weapon_invisible");
	SetupColor(g_Config.color_chams_backtrack_visible, "color_chams_backtrack_visible");
	SetupColor(g_Config.color_chams_backtrack_invisible, "color_chams_backtrack_invisible");
	SetupColor(g_Config.color_glow_team, "color_glow_team");
	SetupColor(g_Config.color_glow_enemy, "color_glow_enemy");
	SetupColor(g_Config.color_glow_chickens, "color_glow_chickens");
	SetupColor(g_Config.color_glow_c4_carrier, "color_glow_c4_carrier");
	SetupColor(g_Config.color_glow_planted_c4, "color_glow_planted_c4");
	SetupColor(g_Config.color_glow_defuse, "color_glow_defuse");
	SetupColor(g_Config.color_glow_weapons, "color_glow_weapons");
	SetupColor(g_Config.color_glow_grenade, "color_glow_grenade");
}

void CConfig::Setup() {
	CConfig::SetupLegit();
	CConfig::SetupSkins();
	CConfig::SetupRage();
	CConfig::SetupEsp();
	CConfig::SetupVisuals();
	CConfig::SetupChams();
	CConfig::SetupGlow();
	CConfig::SetupMisc();
	CConfig::SetupColors();
}

void CConfig::Save(const std::string& name) {
	if (name.empty())
		return;

	CreateDirectoryA(u8"C:\\nukeparadise\\", NULL);
	std::string file = u8"C:\\nukeparadise\\" + name;

	for (auto value : ints) {
		WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
	}

	for (auto value : floats) WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
	for (auto value : bools) WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());
}

void CConfig::Load(const std::string& name) {
	if (name.empty())
		return;

	g_ClientState->ForceFullUpdate();

	CreateDirectoryA(u8"C:\\nukeparadise\\", NULL);
	std::string file = u8"C:\\nukeparadise\\" + name;

	char value_l[32] = { '\0' };
	for (auto value : ints) {
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "0", value_l, 32, file.c_str()); *value->value = atoi(value_l);
	}

	for (auto value : floats) {
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "0.0f", value_l, 32, file.c_str()); *value->value = atof(value_l);
	}

	for (auto value : bools) {
		GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "false", value_l, 32, file.c_str()); *value->value = !strcmp(value_l, "true");
	}
}

CConfig* Config = new CConfig();
