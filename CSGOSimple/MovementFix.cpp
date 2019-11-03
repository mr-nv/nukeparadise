
#include "MovementFix.h"
#include "helpers/math.hpp"

void MovementFix::aaCorrect(QAngle vOldAngles, CUserCmd* pCmd, float fOldForward, float fOldSidemove)
{
	float deltaView = pCmd->viewangles.yaw - vOldAngles.yaw;
	float f1;
	float f2;

	if (vOldAngles.yaw < 0.f)
	{
		f1 = 360.0f + vOldAngles.yaw;
	}
	else
	{
		f1 = vOldAngles.yaw;
	}

	if (pCmd->viewangles.yaw < 0.0f)
	{
		f2 = 360.0f + pCmd->viewangles.yaw;
	}
	else
	{
		f2 = pCmd->viewangles.yaw;
	}

	if (f2 < f1)
	{
		deltaView = abs(f2 - f1);
	}
	else
	{
		deltaView = 360.0f - abs(f1 - f2);
	}
	deltaView = 360.0f - deltaView;

	pCmd->forwardmove = cos(DEG2RAD(deltaView)) * fOldForward + cos(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
	pCmd->sidemove = sin(DEG2RAD(deltaView)) * fOldForward + sin(DEG2RAD(deltaView + 90.f)) * fOldSidemove;

	pCmd->buttons &= ~IN_MOVERIGHT;
	pCmd->buttons &= ~IN_MOVELEFT;
	pCmd->buttons &= ~IN_FORWARD;
	pCmd->buttons &= ~IN_BACK;
}
void MovementFix::Correct(QAngle& wish_angle, CUserCmd* m_pcmd) {
	auto angle_vectors = [](const QAngle& angles, Vector* forward, Vector* right, Vector* up) {
		static auto sincos = [](float radians, float* sine, float* cosine) {
			*sine = sin(radians);
			*cosine = cos(radians);
		};

		float sr, sp, sy, cr, cp, cy;

		sincos(DEG2RAD(angles[1]), &sy, &cy);
		sincos(DEG2RAD(angles[0]), &sp, &cp);
		sincos(DEG2RAD(angles[2]), &sr, &cr);

		if (forward) {
			forward->x = cp * cy;
			forward->y = cp * sy;
			forward->z = -sp;
		}

		if (right) {
			right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
			right->y = (-1 * sr * sp * sy + -1 * cr * cy);
			right->z = -1 * sr * cp;
		}

		if (up) {
			up->x = (cr * sp * cy + -sr * -sy);
			up->y = (cr * sp * sy + -sr * cy);
			up->z = cr * cp;
		}
	};

	Vector view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
	auto viewangles = m_pcmd->viewangles;
	viewangles.Normalize();

	angle_vectors(wish_angle, &view_fwd, &view_right, &view_up);
	angle_vectors(viewangles, &cmd_fwd, &cmd_right, &cmd_up);

	float v8 = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
	float v10 = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
	float v12 = sqrtf(view_up.z * view_up.z);

	Vector norm_view_fwd((1.f / v8) * view_fwd.x, (1.f / v8) * view_fwd.y, 0.f);
	Vector norm_view_right((1.f / v10) * view_right.x, (1.f / v10) * view_right.y, 0.f);
	Vector norm_view_up(0.f, 0.f, (1.f / v12) * view_up.z);

	float v14 = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
	float v16 = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
	float v18 = sqrtf(cmd_up.z * cmd_up.z);

	Vector norm_cmd_fwd((1.f / v14) * cmd_fwd.x, (1.f / v14) * cmd_fwd.y, 0.f);
	Vector norm_cmd_right((1.f / v16) * cmd_right.x, (1.f / v16) * cmd_right.y, 0.f);
	Vector norm_cmd_up(0.f, 0.f, (1.f / v18) * cmd_up.z);

	float v22 = norm_view_fwd.x * m_pcmd->forwardmove;
	float v26 = norm_view_fwd.y * m_pcmd->forwardmove;
	float v28 = norm_view_fwd.z * m_pcmd->forwardmove;
	float v24 = norm_view_right.x * m_pcmd->sidemove;
	float v23 = norm_view_right.y * m_pcmd->sidemove;
	float v25 = norm_view_right.z * m_pcmd->sidemove;
	float v30 = norm_view_up.x * m_pcmd->upmove;
	float v27 = norm_view_up.z * m_pcmd->upmove;
	float v29 = norm_view_up.y * m_pcmd->upmove;

	m_pcmd->forwardmove = ((((norm_cmd_fwd.x * v24) + (norm_cmd_fwd.y * v23)) + (norm_cmd_fwd.z * v25))
		+ (((norm_cmd_fwd.x * v22) + (norm_cmd_fwd.y * v26)) + (norm_cmd_fwd.z * v28)))
		+ (((norm_cmd_fwd.y * v30) + (norm_cmd_fwd.x * v29)) + (norm_cmd_fwd.z * v27));
	m_pcmd->sidemove = ((((norm_cmd_right.x * v24) + (norm_cmd_right.y * v23)) + (norm_cmd_right.z * v25))
		+ (((norm_cmd_right.x * v22) + (norm_cmd_right.y * v26)) + (norm_cmd_right.z * v28)))
		+ (((norm_cmd_right.x * v29) + (norm_cmd_right.y * v30)) + (norm_cmd_right.z * v27));
	m_pcmd->upmove = ((((norm_cmd_up.x * v23) + (norm_cmd_up.y * v24)) + (norm_cmd_up.z * v25))
		+ (((norm_cmd_up.x * v26) + (norm_cmd_up.y * v22)) + (norm_cmd_up.z * v28)))
		+ (((norm_cmd_up.x * v30) + (norm_cmd_up.y * v29)) + (norm_cmd_up.z * v27));

	m_pcmd->forwardmove = Math::clamp(m_pcmd->forwardmove, -450.f, 450.f);
	m_pcmd->sidemove = Math::clamp(m_pcmd->sidemove, -450.f, 450.f);
	m_pcmd->upmove = Math::clamp(m_pcmd->upmove, -320.f, 320.f);
}
