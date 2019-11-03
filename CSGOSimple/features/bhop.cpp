#include "bhop.hpp"
#include "../helpers/math.hpp"
#include "../MovementFix.h"
#include "../options.hpp"
#include "../helpers/input.hpp"

void BunnyHop::OnCreateMove(CUserCmd* cmd)
{
	static bool jumped_last_tick = false;
	static bool should_fake_jump = false;

	if (!jumped_last_tick && should_fake_jump)
	{
		should_fake_jump = false;
		cmd->buttons |= IN_JUMP;
	}
	else if (cmd->buttons & IN_JUMP || should_fake_jump)
	{
		if (should_fake_jump)
		{
			cmd->buttons |= IN_JUMP;
			jumped_last_tick = true;
			should_fake_jump = false;
		}
		else if (g_LocalPlayer->m_fFlags() & FL_ONGROUND)
		{
			jumped_last_tick = true;
			should_fake_jump = true;
			cmd->buttons |= IN_JUMP;
		}
		else
		{
			cmd->buttons &= ~IN_JUMP;
			jumped_last_tick = false;
		}
	}
	else
	{
		jumped_last_tick = false;
		should_fake_jump = false;
	}
}

void BunnyHop::aw_AutoStrafe(CUserCmd* cmd, QAngle va)
{
	if (GetAsyncKeyState(0x41) || GetAsyncKeyState(0x57) || GetAsyncKeyState(0x53) || GetAsyncKeyState(0x44))
		return;

	bool on_ground = (g_LocalPlayer->m_fFlags() & FL_ONGROUND) && !(cmd->buttons & IN_JUMP);
	if (on_ground) {
		return;
	}

	static auto side = 1.0f;
	side = -side;

	auto velocity = g_LocalPlayer->m_vecVelocity();
	velocity.z = 0.0f;

	QAngle wish_angle = cmd->viewangles;

	auto speed = velocity.Length2D();
	auto ideal_strafe = Math::clamp(RAD2DEG(atan(15.f / speed)), 0.0f, 90.0f);

	if (cmd->forwardmove > 0.0f)
		cmd->forwardmove = 0.0f;

	static auto cl_sidespeed = g_CVar->FindVar("cl_sidespeed");

	static float old_yaw = 0.f;
	auto yaw_delta = std::remainderf(wish_angle.yaw - old_yaw, 360.0f);
	auto abs_angle_delta = abs(yaw_delta);
	old_yaw = wish_angle.yaw;

	if (abs_angle_delta <= ideal_strafe || abs_angle_delta >= 30.0f) {
		QAngle velocity_direction;
		Math::VectorAngles(velocity, velocity_direction);
		auto velocity_delta = std::remainderf(wish_angle.yaw - velocity_direction.yaw, 360.0f);
		auto retrack = Math::clamp(RAD2DEG(atan(30.0f / speed)), 0.0f, 90.0f) * g_Config.misc_retrack_speed;
		if (velocity_delta <= retrack || speed <= 15.0f) {
			if (-(retrack) <= velocity_delta || speed <= 15.0f) {
				wish_angle.yaw += side * ideal_strafe;
				cmd->sidemove = cl_sidespeed->GetFloat() * side;
			}
			else {
				wish_angle.yaw = velocity_direction.yaw - retrack;
				cmd->sidemove = cl_sidespeed->GetFloat();
			}
		}
		else {
			wish_angle.yaw = velocity_direction.yaw + retrack;
			cmd->sidemove = -cl_sidespeed->GetFloat();
		}

		Math::MovementFix(cmd, wish_angle, cmd->viewangles);
	}
	else if (yaw_delta > 0.0f) {
		cmd->sidemove = -cl_sidespeed->GetFloat();
	}
	else if (yaw_delta < 0.0f) {
		cmd->sidemove = cl_sidespeed->GetFloat();
	}
}

void BunnyHop::glad_AutoStrafe(CUserCmd* cmd)
{
	if (GetAsyncKeyState(0x41) || GetAsyncKeyState(0x57) || GetAsyncKeyState(0x53) || GetAsyncKeyState(0x44))
		return;

	if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND)) {
		if (cmd->mousedx > 1 || cmd->mousedx < -1) {
			cmd->sidemove = Math::clamp(cmd->mousedx < 0.f ? -400.f : 400.f, -400, 400);
		}
		else {
			if (g_LocalPlayer->m_vecVelocity().Length2D() == 0 || g_LocalPlayer->m_vecVelocity().Length2D() == NAN || g_LocalPlayer->m_vecVelocity().Length2D() == INFINITE)
			{
				cmd->forwardmove = 400;
				return;
			}
			cmd->forwardmove = Math::clamp(5850.f / g_LocalPlayer->m_vecVelocity().Length2D(), -400, 400);
			if (cmd->forwardmove < -400 || cmd->forwardmove > 400)
				cmd->forwardmove = 0;
			cmd->sidemove = Math::clamp((cmd->command_number % 2) == 0 ? -400.f : 400.f, -400, 400);
			if (cmd->sidemove < -400 || cmd->sidemove > 400)
				cmd->sidemove = 0;
		}
	}
}

void BunnyHop::gideon_AutoStrafe(CUserCmd* cmd)
{
	if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND)) {
		{
			if (GetAsyncKeyState(VK_CHAR_A) || GetAsyncKeyState(VK_CHAR_S) || GetAsyncKeyState(VK_CHAR_D) || GetAsyncKeyState(VK_CHAR_W)) {
				if (GetAsyncKeyState(VK_CHAR_A) || GetAsyncKeyState(VK_CHAR_D)) {
					if (GetAsyncKeyState(VK_CHAR_A))
						cmd->sidemove = -(10000.f) / (g_LocalPlayer->m_vecVelocity().Length2D() + 1);
					if (GetAsyncKeyState(VK_CHAR_D))
						cmd->sidemove = (10000.f) / (g_LocalPlayer->m_vecVelocity().Length2D() + 1);

					cmd->forwardmove = (cmd->command_number % 2) == 0 ? -450.f : 450.f;
				}

				if (GetAsyncKeyState(VK_CHAR_S) || GetAsyncKeyState(VK_CHAR_W)) {
					if (GetAsyncKeyState(VK_CHAR_W))
						cmd->forwardmove = (10000.f) / (g_LocalPlayer->m_vecVelocity().Length2D() + 1);
					if (GetAsyncKeyState(VK_CHAR_S))
						cmd->forwardmove = (10000.f) / (g_LocalPlayer->m_vecVelocity().Length2D() + 1) * -1;

					if (!GetAsyncKeyState(VK_CHAR_A) && !GetAsyncKeyState(VK_CHAR_D))
						cmd->sidemove = (cmd->command_number % 2) == 0 ? -450.f : 450.f;
				}
			}
			else {
				if (cmd->mousedx > 1 || cmd->mousedx < -1) {
					cmd->sidemove = cmd->mousedx < 0.f ? -450.f : 450.f;
				}
				else {
					cmd->forwardmove = (10000.f) / (g_LocalPlayer->m_vecVelocity().Length2D() + 1);
					cmd->sidemove = (cmd->command_number % 2) == 0 ? -450.f : 450.f;
				}
			}
		}

	}
}

void BunnyHop::money_AutoStrafe(CUserCmd* cmd) {

	if (g_LocalPlayer->m_vecVelocity().Length2D() > 20) {
		if (cmd->mousedx > 1 || cmd->mousedx < -1) {
			cmd->sidemove = Math::clamp(cmd->mousedx < 0.f ? -450.0f : 450.0f, -450.0f, 450.0f);
		}
		else {
			cmd->forwardmove = 10000.f / g_LocalPlayer->m_vecVelocity().Length();
			cmd->sidemove = (cmd->command_number % 2) == 0 ? -450.0f : 450.0f;
			if (cmd->forwardmove > 450.0f)
				cmd->forwardmove = 450.0f;
		}
	}
}

void BunnyHop::legit_AutoStrafe(CUserCmd* cmd) {
	if (cmd->mousedx > 1 || cmd->mousedx < -1) {
		cmd->sidemove = cmd->mousedx < 0.f ? -400.f : 400.f;
	}
}

void BunnyHop::rage_AutoStrafe(CUserCmd* cmd) {
	static bool leftRight;
	bool inMove = cmd->buttons & IN_FORWARD || cmd->buttons & IN_BACK || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT;

	if (cmd->buttons & IN_FORWARD && g_LocalPlayer->m_vecVelocity().Length() <= 50.0f)
		cmd->forwardmove = 450.0f;

	float yaw_change = 0.0f;
	if (g_LocalPlayer->m_vecVelocity().Length() > 50.f)
		yaw_change = 30.0f * fabsf(30.0f / g_LocalPlayer->m_vecVelocity().Length());


	C_BaseCombatWeapon* activeWeapon = g_LocalPlayer->m_hActiveWeapon();
	if (activeWeapon && !activeWeapon->m_iClip1() == 0 && cmd->buttons & IN_ATTACK)
		yaw_change = 0.0f;

	QAngle viewAngles;
	g_EngineClient->GetViewAngles(viewAngles);

	if (!inMove)
	{
		if (leftRight || cmd->mousedx > 1)
		{
			viewAngles.yaw += yaw_change;
			cmd->sidemove = 450.0f;
		}
		else if (!leftRight || cmd->mousedx < 1)
		{
			viewAngles.yaw -= yaw_change;
			cmd->sidemove = -450.0f;
		}

		leftRight = !leftRight;
	}

	Math::NormalizeAngles(viewAngles);
	Math::ClampAngles(viewAngles);

	Math::CorrectMovement(viewAngles, cmd, cmd->forwardmove, cmd->sidemove);
}

void BunnyHop::bruh_AutoStrafe(CUserCmd* cmd, QAngle va) {
	//BLYAT zabyl udalit
	static bool leftRight;
	bool inMove = cmd->buttons & IN_BACK || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT;

	if (cmd->buttons & IN_FORWARD && g_LocalPlayer->m_vecVelocity().Length() <= 50.0f)
	{
		cmd->forwardmove = 250.0f;
	}

	float yaw_change = 0.0f;
	if (g_LocalPlayer->m_vecVelocity().Length() > 50.f)
	{
		yaw_change = 30.0f * fabsf(30.0f / g_LocalPlayer->m_vecVelocity().Length());
	}

	C_BaseCombatWeapon* ActiveWeapon = g_LocalPlayer->m_hActiveWeapon();
	if (ActiveWeapon && ActiveWeapon->CanFire() && cmd->buttons & IN_ATTACK)
	{
		yaw_change = 0.0f;
	}

	QAngle viewAngles = va;

	bool OnGround = (g_LocalPlayer->m_fFlags() & FL_ONGROUND);
	if (!OnGround && !inMove)
	{
		if (leftRight || cmd->mousedx > 1)
		{
			viewAngles.yaw += yaw_change;
			cmd->sidemove = 350.0f;
		}
		else if (!leftRight || cmd->mousedx < 1)
		{
			viewAngles.yaw -= yaw_change;
			cmd->sidemove = -350.0f;
		}

		leftRight = !leftRight;
	}
	viewAngles.Normalize();
	Math::ClampAngles(viewAngles);
}