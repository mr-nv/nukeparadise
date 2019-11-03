#include <algorithm>
#include "visuals.hpp"
#include "../config.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"
#include "../features/Resolver.h"
#include "../features/AntiAim.h"
#include "../RuntimeSaver.h"
#include "../features/Logger.h"
#include "../features/LagCompensation.h"
#include "../ConsoleHelper.h"
#include "../features/Autowall.h"
#include "../helpers/math.hpp"
#include "../features/GrenadePrediction.h"
#include "../options.hpp"
#include "../features/Lbot.hpp"
#include "../features/EventLogger.h"
#include <ctime>
#define FLAG_MACRO std::pair<std::string, Color> // :joy:
#define FLAG(string, color) vecFlags.push_back(FLAG_MACRO(string, color)) //coz, why not
constexpr float SPEED_FREQ = 100 / 1.0f;
vgui::HFont spectatorlist_font;
vgui::HFont font;
vgui::HFont c4font;
vgui::HFont aafont;

int scan_hitboxes[] = {
	HITBOX_HEAD,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_CALF,
	HITBOX_CHEST,
	HITBOX_STOMACH
};
bool is100(int32_t& ind) { //rofl
	if (ind == (int32_t)100) {
		return true;
	}

	return false;
}
bool IsOnScreen(Vector origin, Vector& screen)
{
	if (!Utils::WorldToScreen(origin, screen))
		return false;

	int iScreenWidth, iScreenHeight;
	g_EngineClient->GetScreenSize(iScreenWidth, iScreenHeight);
	bool xOk = iScreenWidth > screen.x > 0, yOk = iScreenHeight > screen.y > 0;
	return xOk && yOk;
}
RECT GetStaticBBox(C_BaseEntity* ent)
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] =
	{
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];

	for (int i = 0; i < 8; i++)
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++)
	{
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++)
	{
		if (left > screen_points[i].x)
			left = screen_points[i].x;

		if (top < screen_points[i].y)
			top = screen_points[i].y;

		if (right < screen_points[i].x)
			right = screen_points[i].x;

		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}

	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}
RECT GetBBox(C_BaseEntity* ent, Vector pointstransf[])
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] =
	{
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector pos = ent->m_vecOrigin();
	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++)
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
		else
			pointstransf[i] = screen_points[i];

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++)
	{
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}
Visuals::Visuals()
{
	InitializeCriticalSection(&cs);
}
Visuals::~Visuals()
{
	DeleteCriticalSection(&cs);
}
void Visuals::Render()
{
}
bool Visuals::IsVisibleScan(C_BasePlayer* player)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];
	Vector eyePos = g_LocalPlayer->GetEyePos();

	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = g_LocalPlayer;

	if (!player->SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f))
	{
		return false;
	}

	auto studio_model = g_MdlInfo->GetStudiomodel(player->GetModel());
	if (!studio_model)
	{
		return false;
	}

	for (int i = 0; i < ARRAYSIZE(scan_hitboxes); i++)
	{
		auto hitbox = studio_model->GetHitboxSet(player->m_nHitboxSet())->GetHitbox(scan_hitboxes[i]);
		if (hitbox)
		{
			auto
				min = Vector{},
				max = Vector{};

			Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
			Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

			ray.Init(eyePos, (min + max) * 0.5);
			g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

			if (tr.hit_entity == player || tr.fraction > 0.97f)
				return true;
		}
	}
	return false;
}
bool Visuals::Player::Begin(C_BasePlayer* pl)
{
	if (pl->IsDormant() || !pl->IsAlive())
		return false;

	ctx.pl = pl;
	ctx.is_enemy = pl->IsEnemy();
	ctx.is_visible = Visuals::Get().IsVisibleScan(pl);

	auto head = pl->GetHitboxPos(HITBOX_HEAD);
	auto origin = pl->m_vecOrigin();

	head.z += 15;

	//if (!Math::WorldToScreen(head, ctx.head_pos) ||
	//	!Math::WorldToScreen(origin, ctx.feet_pos))
	//	return false;

	auto h = fabs(ctx.head_pos.y - ctx.feet_pos.y);
	auto w = h / 1.65f;

	Vector points_transformed[8];
	RECT bbox = GetBBox(pl, points_transformed);
	ctx.bbox = bbox;
	ctx.bbox.top = bbox.bottom;
	ctx.bbox.bottom = bbox.top;

	if (ctx.bbox.left > ctx.bbox.right)
	{
		ctx.bbox.left = ctx.bbox.right;
		ctx.ShouldDrawBox = false;
	}

	if (ctx.bbox.bottom < ctx.bbox.top)
	{
		ctx.bbox.bottom = ctx.bbox.top;
		ctx.ShouldDrawBox = false;
	}


	return true;
}
void Visuals::Player::RenderBox()
{
	if (!g_LocalPlayer || !ctx.ShouldDrawBox)
		return;

	float
		length_horizontal = (ctx.bbox.right - ctx.bbox.left) * 0.2f,
		length_vertical = (ctx.bbox.bottom - ctx.bbox.top) * 0.2f;

	switch (ctx.boxmode)
	{
	case 0:
		g_VGuiSurface->DrawSetColor(ctx.BoxClr);
		g_VGuiSurface->DrawOutlinedRect(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom);
		break;
	case 1:
		g_VGuiSurface->DrawSetColor(ctx.BoxClr);
		g_VGuiSurface->DrawLine(ctx.bbox.left, ctx.bbox.top, ctx.bbox.left + length_horizontal - 1, ctx.bbox.top);
		g_VGuiSurface->DrawLine(ctx.bbox.right - length_horizontal, ctx.bbox.top, ctx.bbox.right - 1, ctx.bbox.top);
		g_VGuiSurface->DrawLine(ctx.bbox.left, ctx.bbox.bottom - 1, ctx.bbox.left + length_horizontal - 1, ctx.bbox.bottom - 1);
		g_VGuiSurface->DrawLine(ctx.bbox.right - length_horizontal, ctx.bbox.bottom - 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1);

		g_VGuiSurface->DrawLine(ctx.bbox.left, ctx.bbox.top, ctx.bbox.left, ctx.bbox.top + length_vertical - 1);
		g_VGuiSurface->DrawLine(ctx.bbox.right - 1, ctx.bbox.top, ctx.bbox.right - 1, ctx.bbox.top + length_vertical - 1);
		g_VGuiSurface->DrawLine(ctx.bbox.left, ctx.bbox.bottom - length_vertical, ctx.bbox.left, ctx.bbox.bottom - 1);
		g_VGuiSurface->DrawLine(ctx.bbox.right - 1, ctx.bbox.bottom - length_vertical, ctx.bbox.right - 1, ctx.bbox.bottom - 1);
		break;
	case 2:
		Vector pointsTransformed[8];
		auto bbox = GetBBox(ctx.pl, pointsTransformed);
		if (bbox.right == 0 || bbox.bottom == 0 || bbox.left == 0 || bbox.top == 0)
			return;
		g_VGuiSurface->DrawSetColor(ctx.BoxClr);
		g_VGuiSurface->DrawLine(pointsTransformed[0].x, pointsTransformed[0].y, pointsTransformed[1].x, pointsTransformed[1].y);
		g_VGuiSurface->DrawLine(pointsTransformed[0].x, pointsTransformed[0].y, pointsTransformed[6].x, pointsTransformed[6].y);
		g_VGuiSurface->DrawLine(pointsTransformed[1].x, pointsTransformed[1].y, pointsTransformed[5].x, pointsTransformed[5].y);
		g_VGuiSurface->DrawLine(pointsTransformed[6].x, pointsTransformed[6].y, pointsTransformed[5].x, pointsTransformed[5].y);

		g_VGuiSurface->DrawLine(pointsTransformed[2].x, pointsTransformed[2].y, pointsTransformed[1].x, pointsTransformed[1].y);
		g_VGuiSurface->DrawLine(pointsTransformed[4].x, pointsTransformed[4].y, pointsTransformed[5].x, pointsTransformed[5].y);
		g_VGuiSurface->DrawLine(pointsTransformed[6].x, pointsTransformed[6].y, pointsTransformed[7].x, pointsTransformed[7].y);
		g_VGuiSurface->DrawLine(pointsTransformed[3].x, pointsTransformed[3].y, pointsTransformed[0].x, pointsTransformed[0].y);

		g_VGuiSurface->DrawLine(pointsTransformed[3].x, pointsTransformed[3].y, pointsTransformed[2].x, pointsTransformed[2].y);
		g_VGuiSurface->DrawLine(pointsTransformed[2].x, pointsTransformed[2].y, pointsTransformed[4].x, pointsTransformed[4].y);
		g_VGuiSurface->DrawLine(pointsTransformed[7].x, pointsTransformed[7].y, pointsTransformed[4].x, pointsTransformed[4].y);
		g_VGuiSurface->DrawLine(pointsTransformed[7].x, pointsTransformed[7].y, pointsTransformed[3].x, pointsTransformed[3].y);

		break;
	}
}
void Visuals::Player::RenderBoxOutline()
{
	if (!g_LocalPlayer || !ctx.ShouldDrawBox)
		return;

	//int mode = ctx.boxmode;

	if (!g_LocalPlayer->IsAlive())
		ctx.boxmode = 0;

	float
		length_horizontal = (ctx.bbox.right - ctx.bbox.left) * 0.2f,
		length_vertical = (ctx.bbox.bottom - ctx.bbox.top) * 0.2f;

	switch (ctx.boxmode)
	{
	case 0:

		g_VGuiSurface->DrawSetColor(Color::Black);
		g_VGuiSurface->DrawOutlinedRect(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1);
		g_VGuiSurface->DrawOutlinedRect(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1);

		break;
	case 1:

		g_VGuiSurface->DrawSetColor(Color::Black);
		g_VGuiSurface->DrawFilledRect(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.left + 1 + length_horizontal, ctx.bbox.top + 2);
		g_VGuiSurface->DrawFilledRect(ctx.bbox.right - 1 - length_horizontal, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.top + 2);
		g_VGuiSurface->DrawFilledRect(ctx.bbox.left - 1, ctx.bbox.bottom - 2, ctx.bbox.left + 1 + length_horizontal, ctx.bbox.bottom + 1);
		g_VGuiSurface->DrawFilledRect(ctx.bbox.right - 1 - length_horizontal, ctx.bbox.bottom - 2, ctx.bbox.right + 1, ctx.bbox.bottom + 1);

		g_VGuiSurface->DrawFilledRect(ctx.bbox.left - 1, ctx.bbox.top + 2, ctx.bbox.left + 2, ctx.bbox.top + 1 + length_vertical);
		g_VGuiSurface->DrawFilledRect(ctx.bbox.right - 2, ctx.bbox.top + 2, ctx.bbox.right + 1, ctx.bbox.top + 1 + length_vertical);
		g_VGuiSurface->DrawFilledRect(ctx.bbox.left - 1, ctx.bbox.bottom - 1 - length_vertical, ctx.bbox.left + 2, ctx.bbox.bottom - 2);
		g_VGuiSurface->DrawFilledRect(ctx.bbox.right - 2, ctx.bbox.bottom - 1 - length_vertical, ctx.bbox.right + 1, ctx.bbox.bottom - 2);

		break;
	case 2:
		break;
	}
}
void Visuals::Player::RenderName()
{
	//player_info_t info = ctx.pl->GetPlayerInfo();
	std::string name = ctx.pl->GetName(false);
	VGSHelper::Get().DrawNameText(name, ctx.bbox.left, ctx.bbox.right, ctx.bbox.top, ctx.NameClr);
}
void Visuals::Player::Render3DHead()
{
	if (ctx.pl == g_LocalPlayer && !g_Config.vis_misc_thirdperson)
		return;
	Vector headpos = ctx.pl->GetBonePos(8);
	VGSHelper::Get().Draw3DCube(7.f, ctx.pl->m_angEyeAngles(), headpos, ctx.EDHeadClr);
}
void Visuals::Player::DrawArrow() //fix plz
{
	if (!g_LocalPlayer || ctx.pl->IsDormant() || ctx.pl->IsWeapon() || !ctx.pl->IsPlayer() || ctx.pl->GetClientClass()->m_ClassID != CCSPlayer || ctx.pl == g_LocalPlayer || !ctx.pl->IsAlive() || ctx.pl->m_iHealth() <= 0)
		return;

	//alpha unused :(
	int alpha = std::clamp(floor(sin(g_GlobalVars->realtime * 2) * 127 + 128), 0.f, 0.5f * 255.f);
	Vector screenPos, p_client_viewangles;
	QAngle client_viewangles;
	int screen_width = 0, screen_height = 0;
	float radius = 50.f + g_Config.esp_misc_pov_dist;

	if (IsOnScreen(ctx.pl->GetHitboxPos(HITBOX_HEAD), screenPos))
		return;

	g_EngineClient->GetViewAngles(client_viewangles);
	g_EngineClient->GetScreenSize(screen_width, screen_height);

	const auto scr = Vector2D(screen_width / 2.f, screen_height / 2.f);
	const auto rot = DEG2RAD(client_viewangles.yaw - Math::gCalcAngle(g_LocalPlayer->GetEyePos(), ctx.pl->GetHitboxPos(HITBOX_HEAD)).y - 90);
	auto pos = scr + Vector2D(radius * cos(rot), radius * sin(rot));
	auto line = pos - scr;
	Vector2D arrowBase = pos - (line * (g_Config.esp_misc_pov_size / (2 * (tanf(45) / 2) * line.Length())));
	Vector2D normal = Vector2D(-line.y, line.x);
	Vector2D left = arrowBase + normal * (g_Config.esp_misc_pov_size / (2 * line.Length()));
	Vector2D right = arrowBase + normal * (-g_Config.esp_misc_pov_size / (2 * line.Length()));
	Vertex_t vertices[3];
	vertices[0].Init((Vector2D(left.x, left.y)));
	vertices[1].Init((Vector2D(right.x, right.y)));
	vertices[2].Init((pos));
	VGSHelper::Get().DrawTriangle(3, vertices, ctx.PovClr);
}
void Visuals::Player::DrawhealthIcon(int x, int y)
{
	Color cross_color = Color(255, 25, 50, 255);

	if (((float)g_GlobalVars->curtime - (int)(g_GlobalVars->curtime)) > 0.5f)
		cross_color = Color(255, 255, 0, 200);

	g_VGuiSurface->DrawSetColor(cross_color); // Cross
	g_VGuiSurface->DrawFilledRect(x + 4, y + 2, x + 6, y + 10);// top-bottom
	g_VGuiSurface->DrawFilledRect(x + 1, y + 5, x + 4, y + 7);// left
	g_VGuiSurface->DrawFilledRect(x + 6, y + 5, x + 9, y + 7);// right

	g_VGuiSurface->DrawSetColor(Color(0, 0, 0, 255)); // Outline
	g_VGuiSurface->DrawLine(x + 3, y + 1, x + 6, y + 1); // top
	g_VGuiSurface->DrawLine(x + 3, y + 1, x + 3, y + 4);
	g_VGuiSurface->DrawLine(x + 6, y + 1, x + 6, y + 4);
	g_VGuiSurface->DrawLine(x + 3, y + 7, x + 3, y + 10);// bottom
	g_VGuiSurface->DrawLine(x + 6, y + 7, x + 6, y + 10);
	g_VGuiSurface->DrawLine(x + 3, y + 10, x + 6, y + 10);
	g_VGuiSurface->DrawLine(x, y + 4, x, y + 7);// left
	g_VGuiSurface->DrawLine(x, y + 4, x + 3, y + 4);
	g_VGuiSurface->DrawLine(x, y + 7, x + 3, y + 7);
	g_VGuiSurface->DrawLine(x + 9, y + 4, x + 9, y + 7);// right
	g_VGuiSurface->DrawLine(x + 6, y + 4, x + 9, y + 4);
	g_VGuiSurface->DrawLine(x + 6, y + 7, x + 9, y + 7);

}
void Visuals::Player::RenderHealth()
{
	if (!ctx.ShouldDrawBox)
		return;

	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 6;

	static float prev_player_hp[65];

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 3;
	int h = box_h;

	int health = ctx.pl->m_iHealth();
	int player_index = ctx.pl->EntIndex();

	if (health > 100)
	{
		DrawhealthIcon(x - 4, y);
		return;
	}

	if (prev_player_hp[player_index] > health)
	{
		prev_player_hp[player_index] -= SPEED_FREQ * g_GlobalVars->frametime;
	}
	else
	{
		prev_player_hp[player_index] = health;
	}

	auto height = box_h - (((box_h * prev_player_hp[player_index]) / 100));

	g_VGuiSurface->DrawSetColor(Color::Black);
	g_VGuiSurface->DrawFilledRect(x, y, x + w, y + h);

	g_VGuiSurface->DrawSetColor(Color((255 - (int)prev_player_hp[player_index] * int(2.55f)), ((int)prev_player_hp[player_index] * int(2.55f)), 0, (int)(180.f)));
	g_VGuiSurface->DrawFilledRect(x + 1, y + height + 1, x + w - 1, y + h - 1);

}
void Visuals::Player::DrawAngleLines()
{
	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = ctx.pl;

	Math::AngleVectors(QAngle(0, ctx.pl->m_flLowerBodyYawTarget(), 0), forward);
	src3D = ctx.pl->m_vecOrigin();
	dst3D = src3D + (forward * 50.f);

	ray.Init(src3D, dst3D);
	g_EngineTrace->TraceRay(ray, 0, &filter, &tr);

	if (!Math::WorldToScreen(src3D, src) || !Math::WorldToScreen(tr.endpos, dst))
		return;

	g_VGuiSurface->DrawSetColor(ctx.LineLbyClr);
	g_VGuiSurface->DrawLine(src.x, src.y, dst.x, dst.y);

	if (ctx.pl == g_LocalPlayer && g_LocalPlayer->IsAlive())
	{
		Math::AngleVectors(QAngle(0, g_Saver.view_angle, 0), forward);
		dst3D = src3D + (forward * 50.f);

		ray.Init(src3D, dst3D);
		g_EngineTrace->TraceRay(ray, 0, &filter, &tr);

		if (!Math::WorldToScreen(src3D, src) || !Math::WorldToScreen(tr.endpos, dst))
			return;

		g_VGuiSurface->DrawSetColor(ctx.LineViewClr);
		g_VGuiSurface->DrawLine(src.x, src.y, dst.x, dst.y);
	}
	else
	{
		Math::AngleVectors(QAngle(0, ctx.pl->m_angEyeAngles().yaw, 0), forward);
		dst3D = src3D + (forward * 50.f);

		ray.Init(src3D, dst3D);
		g_EngineTrace->TraceRay(ray, 0, &filter, &tr);

		if (!Math::WorldToScreen(src3D, src) || !Math::WorldToScreen(tr.endpos, dst))
			return;

		g_VGuiSurface->DrawSetColor(ctx.LineViewClr);
		g_VGuiSurface->DrawLine(src.x, src.y, dst.x, dst.y);
	}

	if (ctx.pl == g_LocalPlayer && g_LocalPlayer->IsAlive() && g_Config.rbot_aa > 0)
	{
		Math::AngleVectors(QAngle(0, g_Saver.real_angle, 0), forward);
		dst3D = src3D + (forward * 50.f);

		ray.Init(src3D, dst3D);
		g_EngineTrace->TraceRay(ray, 0, &filter, &tr);

		if (!Math::WorldToScreen(src3D, src) || !Math::WorldToScreen(tr.endpos, dst))
			return;

		g_VGuiSurface->DrawSetColor(Color(g_Config.color_esp_local_desync));
		g_VGuiSurface->DrawLine(src.x, src.y, dst.x, dst.y);
	}
}
void Visuals::Player::RenderArmour()
{
	if (!ctx.ShouldDrawBox)
		return;

	static float prev_player_armor[65];

	int armor = ctx.pl->m_ArmorValue();
	int player_index = ctx.pl->EntIndex();

	if (armor <= 0) return;

	if (armor > 100) armor = 100;

	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float off = 3;

	if (prev_player_armor[player_index] > armor)
	{
		prev_player_armor[player_index] -= SPEED_FREQ * g_GlobalVars->frametime;
	}
	else
	{
		prev_player_armor[player_index] = armor;
	}

	auto height = box_h - (((box_h * prev_player_armor[player_index]) / 100));

	int x = ctx.bbox.right + off;
	int y = ctx.bbox.top;
	int w = 3;
	int h = box_h;

	g_VGuiSurface->DrawSetColor(Color::Black);
	g_VGuiSurface->DrawFilledRect(x, y, x + w, y + h);

	g_VGuiSurface->DrawSetColor(ctx.ArmourClr);
	g_VGuiSurface->DrawFilledRect(x + 1, y + height + 1, x + w - 1, y + h - 1);

}
void Visuals::Player::RenderLbyUpdateBar()
{
	if (!ctx.ShouldDrawBox)
		return;

	int i = ctx.pl->EntIndex();

	if (ctx.pl->m_vecVelocity().Length2D() > 0.1f || !(ctx.pl->m_fFlags() & FL_ONGROUND))
		return;

	if (!g_Resolver.GResolverData[i].CanuseLbyPrediction)
		return;

	float percent = 1.f - ((g_Resolver.GResolverData[i].NextPredictedLbyBreak - ctx.pl->m_flSimulationTime()) / 1.1f);

	if (percent < 0.f || percent > 1.f)
		return;

	RenderLine(ctx.lbyupdatepos, ctx.LbyTimerClr, percent);
}
void Visuals::Player::RenderWeaponName()
{
	wchar_t buf[80];
	auto clean_item_name = [](const char* name) -> const char*
	{
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	auto weapon = ctx.pl->m_hActiveWeapon().Get();

	if (!weapon) return;

	if (ctx.bbox.right == 0 || ctx.bbox.bottom == 0)
		return;

	if (weapon->m_hOwnerEntity().IsValid())
	{
		std::string name = clean_item_name(weapon->GetClientClass()->m_pNetworkName); //TODO: use localised hudname instead of this... // worst idea

		VGSHelper::Get().DrawWeaponText(name, ctx.bbox.left, ctx.bbox.right, ctx.bbox.bottom, ctx.WeaponClr);
	}

}
void Visuals::Player::RenderWeaponAmmo()
{
	auto weapon = ctx.pl->m_hActiveWeapon().Get();

	if (!weapon) return;

	int clip = (weapon->m_iClip1());
	int maxammo = (weapon->GetCSWeaponData()->iMaxClip1);

	bool ammogate = (clip >= 0 && maxammo >= 0 && clip != maxammo);

	if (!ammogate) return;

	if (ctx.bbox.right == 0 || ctx.bbox.bottom == 0)
		return;

	if (weapon->m_hOwnerEntity().IsValid())
	{
		VGSHelper::Get().DrawWeaponText(Utils::Format("%d / %d", static_cast<int>(clip), static_cast<int>(maxammo)), ctx.bbox.left, ctx.bbox.right, ctx.bbox.bottom + 9, ctx.WeaponClr);
	}

}
void Visuals::Player::RenderSnapline()
{
	int width, height;
	g_EngineClient->GetScreenSize(width, height);

	int screen_x = width * 0.5f,
		screen_y = height,
		target_x = ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) * 0.5,
		target_y = ctx.bbox.bottom,
		max_length = height * 0.3f;

	if (target_x == 0 ||
		target_y == 0)
		return;

	switch (g_Config.esp_misc_snaplines_type)
	{
	case 0:

		VGSHelper::Get().DrawLine(width / 2.f, (float)height, target_x, target_y, ctx.SnaplineClr);

		break;
	case 1:

		float length = sqrt(pow(target_x - screen_x, 2) + pow(target_y - screen_y, 2));
		if (length > max_length)
		{
			float
				x_normalized = (target_x - screen_x) / length,
				y_normalized = (target_y - screen_y) / length;
			target_x = screen_x + x_normalized * max_length;
			target_y = screen_y + y_normalized * max_length;
			VGSHelper::Get().DrawCircle(target_x + x_normalized * 3.5f, target_y + y_normalized * 3.5f, 4.0f, 12, ctx.SnaplineClr);
		}

		g_VGuiSurface->DrawSetColor(ctx.SnaplineClr);
		g_VGuiSurface->DrawLine(screen_x, screen_y, target_x, target_y);

		break;
	}

}
void Visuals::Player::DrawPlayerDebugInfo()
{
	if (!g_LocalPlayer || ctx.pl == g_LocalPlayer)
		return;

	if (ctx.bbox.right == 0 || ctx.bbox.bottom == 0)
		return;

	std::string t1 = "missed shots: " + std::to_string(g_Resolver.GResolverData[ctx.pl->EntIndex()].Shots);
	std::string t2 = "mode: " + std::to_string(g_Resolver.GResolverData[ctx.pl->EntIndex()].Shots);
	std::string t3 = "detected: ";
	std::string t4 = g_Resolver.GResolverData[ctx.pl->EntIndex()].Fake ? "fake" : "real";
	std::string t5 = "velocity: " + std::to_string(ctx.pl->m_vecVelocity().Length2D());
	int i = ctx.pl->EntIndex();

	switch (g_Resolver.GResolverData[i].mode)
	{
	case ResolverModes::NONE:
		t2 += "none";
		break;

	case ResolverModes::FREESTANDING:
		t2 += "FREESTANDING";
		break;

	case ResolverModes::EDGE:
		t2 += "EDGE";
		break;

	case ResolverModes::MOVE_STAND_DELTA:
		t2 += "MOVE_STAND_DELTA";
		break;

	case ResolverModes::FORCE_LAST_MOVING_LBY:
		t2 += "FORCE_LAST_MOVING_LBY";
		break;

	case ResolverModes::FORCE_FREESTANDING:
		t2 += "FORCE_FREESTANDING";
		break;

	case ResolverModes::BRUTFORCE_ALL_DISABLED:
		t2 += "BRUTFORCE_ALL_DISABLED";
		break;

	case ResolverModes::BRUTFORCE:
		t2 += "BRUTFORCE";
		break;

	case ResolverModes::FORCE_MOVE_STAND_DELTA:
		t2 += "FORCE_MOVE_STAND_DELTA";
		break;

	case ResolverModes::FORCE_LBY:
		t2 += "FORCE_LBY";
		break;

	case ResolverModes::MOVING:
		t2 += "MOVING";
		break;

	case ResolverModes::LBY_BREAK:
		t2 += "LBY_BREAK";
		break;

	case ResolverModes::SPINBOT:
		t2 += "SPINBOT";
		break;

	case ResolverModes::AIR_FREESTANDING:
		t2 += "AIR_FREESTANDING";
		break;

	case ResolverModes::AIR_BRUTFORCE:
		t2 += "AIR_BRUTFORCE";
		break;

	case ResolverModes::FAKEWALK_FREESTANDING:
		t2 += "FAKEWALK_FREESTANDING";
		break;

	case ResolverModes::FAKEWALK_BRUTFORCE:
		t2 += "FAKEWALK_BRUTFORCE";
		break;

	case ResolverModes::BACKWARDS:
		t2 += "BACKWARDS";
		break;

	case ResolverModes::FORCE_BACKWARDS:
		t2 += "FORCE_BACKWARDS";
		break;
	}

	switch (g_Resolver.GResolverData[i].detection)
	{
	case ResolverDetections::FAKEWALKING:
		t3 += "Fakewalking";
		break;

	case ResolverDetections::AIR:
		t3 += "Air";
		break;

	case ResolverDetections::MOVING:
		t3 += "Moving";
		break;

	case ResolverDetections::STANDING:
		t3 += "Standing";
		break;
	}

	VGSHelper::Get().DrawText(t1, ctx.bbox.right + 12.f, ctx.head_pos.y, Color::White);
	VGSHelper::Get().DrawText(t2, ctx.bbox.right + 12.f, ctx.head_pos.y + 14.f, Color::White);
	VGSHelper::Get().DrawText(t3, ctx.bbox.right + 12.f, ctx.head_pos.y + 28.f, Color::White);
	VGSHelper::Get().DrawText(t4, ctx.bbox.right + 12.f, ctx.head_pos.y + 42.f, Color::White);
	VGSHelper::Get().DrawText(t5, ctx.bbox.right + 12.f, ctx.head_pos.y + 56.f, Color::White);
}
void Visuals::Player::RenderLine(DrawSideModes mode, Color color, float percent)
{
	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
	float off = 4;
	float x = 0;
	float y = 0;
	float x2 = 0;
	float y2 = 0;

	//float x3 = 0;
	//float y3 = 0;
	switch (mode)
	{
	case DrawSideModes::TOP:
		off = ctx.PosHelper.top;
		x = ctx.bbox.left;
		y = ctx.bbox.top + off;
		x2 = x + (box_w * percent);
		y2 = y;
		ctx.PosHelper.top += 8;
		break;

	case DrawSideModes::RIGHT:
		off = ctx.PosHelper.right;
		x = ctx.bbox.right + off;
		y = ctx.bbox.top;
		x2 = x + 4;
		y2 = y + (box_h * percent);
		ctx.PosHelper.right += 8;
		break;

	case DrawSideModes::BOTTOM:
		off = ctx.PosHelper.bottom;
		x = ctx.bbox.left;
		y = ctx.bbox.bottom + off;
		x2 = x + (box_w * percent);
		y2 = y;
		ctx.PosHelper.bottom += 8;
		break;

	case DrawSideModes::LEFT:
		off = ctx.PosHelper.left;
		x = ctx.bbox.left - (off * 2);
		y = ctx.bbox.top;
		x2 = x + 4;
		y2 = y + (box_h * percent);
		ctx.PosHelper.left += 8;
		break;
	}

	//Render::Get().RenderBox(x, y, x + w, y + h, Color::Black, 1.f, true);
	//Render::Get().RenderBox(x + 1, y + 1, x + w - 1, y + height - 2, Color(0, 50, 255, 255), 1.f, true);
	if (mode == DrawSideModes::LEFT || mode == DrawSideModes::RIGHT)
		VGSHelper::Get().DrawFilledBox(x, y, x2, y + box_h, Color(0, 0, 0, 100));
	else
		VGSHelper::Get().DrawFilledBox(x, y, x + box_w, y2, Color(0, 0, 0, 100));

	VGSHelper::Get().DrawFilledBox(x + 1, y + 1, x2 - 1, y2 - 2, color);
}
void Visuals::Player::RenderResolverInfo()
{
	if (g_Resolver.GResolverData[ctx.pl->EntIndex()].Fake)
	{
		char* t1 = "Fake";
		auto sz = g_pDefaultFont->CalcTextSizeA(12, FLT_MAX, 0.0f, t1);
		//VGSHelper::Get().DrawText ( t1, ctx.bbox.right + 8.f, ctx.head_pos.y - sz.y + TextHeight, ctx.InfoClr, 12 );
		VGSHelper::Get().DrawText(t1, ctx.bbox.right + 2.f + ctx.PosHelper.right, ctx.head_pos.y - sz.y + TextHeight, ctx.WeaponClr, 12);
		TextHeight += 12.f;
	}

	if (g_Resolver.GResolverData[ctx.pl->EntIndex()].BreakingLC)
	{
		char* t1 = "LC";
		auto sz = g_pDefaultFont->CalcTextSizeA(12, FLT_MAX, 0.0f, t1);
		//VGSHelper::Get().DrawText ( t1, ctx.bbox.right + 8.f, ctx.head_pos.y - sz.y + TextHeight, ctx.InfoClr, 12 );
		VGSHelper::Get().DrawText(t1, ctx.bbox.right + 2.f + ctx.PosHelper.right, ctx.head_pos.y - sz.y + TextHeight, ctx.WeaponClr, 12);
		TextHeight += 12.f;
	}
}
void Visuals::Player::RenderFill()
{
	if (!g_EngineClient->IsInGame()) return; //pizda FIXED gde
	if (ctx.bbox.right == 0 || ctx.bbox.bottom == 0)
		return;
	if (!g_LocalPlayer || !ctx.ShouldDrawBox)
		return;
	g_VGuiSurface->DrawSetColor(Color(ctx.BoxFill));

	g_VGuiSurface->DrawFilledRect(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1);
}
void Visuals::Player::DrawFlags()
{
	if (ctx.bbox.right == 0 || ctx.bbox.bottom == 0)
		return;

	std::vector<FLAG_MACRO> vecFlags;

	player_info_t info = ctx.pl->GetPlayerInfo();

	if (g_Config.esp_misc_debug_overlay)
	{
		std::string t1 = "MISSED: " + std::to_string(g_Resolver.GResolverData[ctx.pl->EntIndex()].Shots);
		std::string t2 = "MODE: ";
		std::string t3 = "DETECTED: ";
		std::string t4 = g_Resolver.GResolverData[ctx.pl->EntIndex()].Resolved ? "RESOLVED: YES" : "RESOLVED: NO";
		std::string t5 = g_Resolver.GResolverData[ctx.pl->EntIndex()].Fake ? "FAKELAG: YES" : "FAKELAG: NO";
		int i = ctx.pl->EntIndex();

		switch (g_Resolver.GResolverData[i].mode)
		{
		case ResolverModes::NONE:
			t2 += "NONE";
			break;

		case ResolverModes::FREESTANDING:
			t2 += "FREESTANDING";
			break;

		case ResolverModes::EDGE:
			t2 += "EDGE";
			break;

		case ResolverModes::MOVE_STAND_DELTA:
			t2 += "MOVE_STAND_DELTA";
			break;

		case ResolverModes::FORCE_LAST_MOVING_LBY:
			t2 += "FORCE_LAST_MOVING_LBY";
			break;

		case ResolverModes::FORCE_FREESTANDING:
			t2 += "FORCE_FREESTANDING";
			break;

		case ResolverModes::BRUTFORCE_ALL_DISABLED:
			t2 += "BRUTFORCE_ALL_DISABLED";
			break;

		case ResolverModes::BRUTFORCE:
			t2 += "BRUTFORCE";
			break;

		case ResolverModes::FORCE_MOVE_STAND_DELTA:
			t2 += "FORCE_MOVE_STAND_DELTA";
			break;

		case ResolverModes::FORCE_LBY:
			t2 += "FORCE_LBY";
			break;

		case ResolverModes::MOVING:
			t2 += "MOVING";
			break;

		case ResolverModes::LBY_BREAK:
			t2 += "LBY_BREAK";
			break;

		case ResolverModes::SPINBOT:
			t2 += "SPINBOT";
			break;

		case ResolverModes::AIR_FREESTANDING:
			t2 += "AIR_FREESTANDING";
			break;

		case ResolverModes::AIR_BRUTFORCE:
			t2 += "AIR_BRUTFORCE";
			break;

		case ResolverModes::FAKEWALK_FREESTANDING:
			t2 += "FAKEWALK_FREESTANDING";
			break;

		case ResolverModes::FAKEWALK_BRUTFORCE:
			t2 += "FAKEWALK_BRUTFORCE";
			break;

		case ResolverModes::BACKWARDS:
			t2 += "BACKWARDS";
			break;

		case ResolverModes::FORCE_BACKWARDS:
			t2 += "FORCE_BACKWARDS";
			break;
		}

		switch (g_Resolver.GResolverData[i].detection)
		{
		case ResolverDetections::FAKEWALKING:
			t3 += "FAKEWALK";
			break;

		case ResolverDetections::AIR:
			t3 += "AIR";
			break;

		case ResolverDetections::MOVING:
			t3 += "MOVING";
			break;

		case ResolverDetections::STANDING:
			t3 += "STANDING";
			break;
		}

		FLAG(t1, ctx.FlagClr);
		FLAG(t2, ctx.FlagClr);
		FLAG(t3, ctx.FlagClr);
		FLAG(t4, ctx.FlagClr);
		FLAG(t5, ctx.FlagClr);

		FLAG(" ", ctx.FlagClr);
	}

	if (g_Resolver.GResolverData[ctx.pl->EntIndex()].Fake)
		FLAG("FAKE", ctx.FlagClr);

	if (g_Resolver.GResolverData[ctx.pl->EntIndex()].BreakingLC)
		FLAG("LC", ctx.FlagClr);

	if (ctx.pl->m_ArmorValue() > 0)
		FLAG(ctx.pl->GetArmorName(), ctx.FlagClr);

	if (ctx.pl->m_bIsScoped())
		FLAG("SCOPED", ctx.FlagClr);

	if (ctx.pl->m_bGunGameImmunity())
		FLAG("FROZED", ctx.FlagClr);

	int offset = 0; //smh, have to think about a better way just because of this lmao
	for (auto Text : vecFlags)
	{
		if (!ctx.is_armored)
			VGSHelper::Get().DrawText(Text.first.c_str(), ctx.bbox.right + 4, (ctx.bbox.top) + offset, Text.second, 12);
		else
			VGSHelper::Get().DrawText(Text.first.c_str(), ctx.bbox.right + 8, (ctx.bbox.top) + offset, Text.second, 12);

		offset += 9;
	}
}
void DrawZeusRange()
{
	if (!g_EngineClient->IsConnected() || !g_EngineClient->IsInGame() || !g_Config.vis_misc_thirdperson)
		return;

	auto local_player = g_LocalPlayer;
	if (!local_player) return;
	if (local_player->m_iHealth() <= 0) return;

	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon();
	if (!weapon) return;

	if (weapon->m_iItemDefinitionIndex() != WEAPON_TASER) return;

	float step = M_PI * 2.0 / 1023;
	float rad = 130.f;
	Vector origin = local_player->GetEyePos();

	static double rainbow;

	Vector screenPos;
	static Vector prevScreenPos;

	for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
	{
		Vector pos(rad * cos(rotation) + origin.x, rad * sin(rotation) + origin.y, origin.z);

		CTraceFilter filter;
		CGameTrace trace;
		filter.pSkip = local_player;
		Ray_t t;
		t.Init(origin, pos);

		g_EngineTrace->TraceRay(t, MASK_SHOT_BRUSHONLY, &filter, &trace);

		if (g_DebugOverlay->ScreenPosition(trace.endpos, screenPos))
			continue;

		if (!prevScreenPos.IsZero() && !screenPos.IsZero() && screenPos.DistTo(Vector(-107374176, -107374176, -107374176)) > 3.f && prevScreenPos.DistTo(Vector(-107374176, -107374176, -107374176)) > 3.f) {
			rainbow += 0.00001;
			if (rainbow > 1.f)
				rainbow = 0;
			Color color = Color::FromHSB(rainbow, 1.f, 1.f);
			VGSHelper::Get().DrawLine(prevScreenPos.x, prevScreenPos.y, screenPos.x, screenPos.y, color);
		}
		prevScreenPos = screenPos;
	}
}
/*
void Visuals::RenderBacktrackedSkelet()
{
	if (!g_Options.rage_lagcompensation)
		return;

	auto records = &CMBacktracking::Get().m_LagRecord[ESP_ctx.player->EntIndex()];
	if (records->size() < 2)
		return;

	Vector previous_screenpos;
	for (auto record = records->begin(); record != records->end(); record++)
	{
		if (!CMBacktracking::Get().IsTickValid(TIME_TO_TICKS(record->m_flSimulationTime)))
			continue;

		Vector screen_pos;
		if (!Math::WorldToScreen(record->m_vecHeadSpot, screen_pos))
			continue;

		if (previous_screenpos.IsValid())
		{
			if (*record == CMBacktracking::Get().m_RestoreLagRecord[ESP_ctx.player->EntIndex()].first)
				g_VGuiSurface->DrawSetColor(Color(255, 255, 0, 255));
			else
				g_VGuiSurface->DrawSetColor(Color(255, 255, 255, 255));
			g_VGuiSurface->DrawLine(screen_pos.x, screen_pos.y, previous_screenpos.x, previous_screenpos.y);
		}

		previous_screenpos = screen_pos;
	}
}*/
void Visuals::RenderWeapon(C_BaseCombatWeapon* ent, std::string weapon)
{

	if (ent->m_hOwnerEntity().IsValid() ||
		ent->m_vecOrigin() == Vector(0, 0, 0))
		return;

	int clip = (ent->m_iClip1());
	int maxammo = (ent->GetCSWeaponData()->iMaxClip1);

	bool ammogate = (clip >= 0 && maxammo >= 0 && clip != maxammo);

	char test[256]; sprintf(test, "%d / %d", static_cast<int>(clip), static_cast<int>(maxammo));

	Vector pointsTransformed[8];
	auto bbox = GetBBox(ent, pointsTransformed);
	if (bbox.right == 0 || bbox.bottom == 0 || bbox.left == 0 || bbox.top == 0)
		return;

	Color clr = Color(g_Config.color_esp_dropped_weapons);

	g_VGuiSurface->DrawSetColor(clr);
	switch (g_Config.esp_dropped_weapons_type)
	{
	case 0:
		break;
	case 1:
		VGSHelper::Get().DrawWeaponEntityText(weapon, bbox.left, bbox.right, bbox.top, clr);
		if (g_Config.esp_dropped_weapons_info && ammogate)
		{
			VGSHelper::Get().DrawWeaponEntityText(test, bbox.left, bbox.right, bbox.top + 9, clr);
		}
		break;
	case 2:
		g_VGuiSurface->DrawLine(bbox.left, bbox.top, bbox.right, bbox.top);
		g_VGuiSurface->DrawLine(bbox.left, bbox.bottom, bbox.right, bbox.bottom);
		g_VGuiSurface->DrawLine(bbox.left, bbox.top, bbox.left, bbox.bottom);
		g_VGuiSurface->DrawLine(bbox.right, bbox.top, bbox.right, bbox.bottom);

		VGSHelper::Get().DrawWeaponEntityText(weapon, bbox.left, bbox.right, bbox.top, clr);
		if (g_Config.esp_dropped_weapons_info && ammogate)
		{
			VGSHelper::Get().DrawWeaponEntityText(test, bbox.left, bbox.right, bbox.top + 9, clr);
		}
		break;
	case 3:
		g_VGuiSurface->DrawLine(pointsTransformed[0].x, pointsTransformed[0].y, pointsTransformed[1].x, pointsTransformed[1].y);
		g_VGuiSurface->DrawLine(pointsTransformed[0].x, pointsTransformed[0].y, pointsTransformed[6].x, pointsTransformed[6].y);
		g_VGuiSurface->DrawLine(pointsTransformed[1].x, pointsTransformed[1].y, pointsTransformed[5].x, pointsTransformed[5].y);
		g_VGuiSurface->DrawLine(pointsTransformed[6].x, pointsTransformed[6].y, pointsTransformed[5].x, pointsTransformed[5].y);

		g_VGuiSurface->DrawLine(pointsTransformed[2].x, pointsTransformed[2].y, pointsTransformed[1].x, pointsTransformed[1].y);
		g_VGuiSurface->DrawLine(pointsTransformed[4].x, pointsTransformed[4].y, pointsTransformed[5].x, pointsTransformed[5].y);
		g_VGuiSurface->DrawLine(pointsTransformed[6].x, pointsTransformed[6].y, pointsTransformed[7].x, pointsTransformed[7].y);
		g_VGuiSurface->DrawLine(pointsTransformed[3].x, pointsTransformed[3].y, pointsTransformed[0].x, pointsTransformed[0].y);

		g_VGuiSurface->DrawLine(pointsTransformed[3].x, pointsTransformed[3].y, pointsTransformed[2].x, pointsTransformed[2].y);
		g_VGuiSurface->DrawLine(pointsTransformed[2].x, pointsTransformed[2].y, pointsTransformed[4].x, pointsTransformed[4].y);
		g_VGuiSurface->DrawLine(pointsTransformed[7].x, pointsTransformed[7].y, pointsTransformed[4].x, pointsTransformed[4].y);
		g_VGuiSurface->DrawLine(pointsTransformed[7].x, pointsTransformed[7].y, pointsTransformed[3].x, pointsTransformed[3].y);

		VGSHelper::Get().DrawWeaponEntityText(weapon, bbox.left, bbox.right, bbox.top, clr);
		if (g_Config.esp_dropped_weapons_info && ammogate)
		{
			VGSHelper::Get().DrawWeaponEntityText(test, bbox.left, bbox.right, bbox.top + 9, clr);
		}
		break;
	}

}
void Visuals::RenderC4(C_BaseEntity* ent)
{

	Vector points_transformed[8];

	auto bbox = GetBBox(ent, points_transformed);
	auto pbbox = GetStaticBBox(ent);

	int x, y; g_EngineClient->GetScreenSize(x, y);

	float flblow = ent->m_flC4Blow();
	float ExplodeTimeRemaining = flblow - (g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick);

	float fldefuse = ent->m_flDefuseCountDown();
	float DefuseTimeRemaining = fldefuse - (g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick);

	int width, height;

	if (ExplodeTimeRemaining > 0 && !ent->m_bBombDefused())
	{

		float fraction = ExplodeTimeRemaining / ent->m_flTimerLength();
		int onscreenwidth = fraction * x;

		float red = 255 - (fraction * 255);
		float green = fraction * 255;

		if (g_Config.esp_timer_c4)
		{
			char temp[256]; sprintf_s(temp, "Explode in: %.1f", ExplodeTimeRemaining);

			g_VGuiSurface->DrawSetColor(red, green, 0, 220);
			g_VGuiSurface->DrawFilledRect(0, 0, onscreenwidth, 10);
			VGSHelper::Get().DrawC4Text(temp, onscreenwidth, 0, Color::White);
		}

		if (g_Config.esp_planted_c4 || g_Config.esp_box_c4)
		{
			if (pbbox.right == 0 || pbbox.bottom == 0)
				return;

			if (g_Config.esp_box_c4)
			{
				g_VGuiSurface->DrawSetColor(Color(20, 20, 20, 240));
				g_VGuiSurface->DrawLine(pbbox.left - 1, pbbox.bottom - 1, pbbox.left - 1, pbbox.top + 1);
				g_VGuiSurface->DrawLine(pbbox.right + 1, pbbox.top + 1, pbbox.right + 1, pbbox.bottom - 1);
				g_VGuiSurface->DrawLine(pbbox.left - 1, pbbox.top + 1, pbbox.right + 1, pbbox.top + 1);
				g_VGuiSurface->DrawLine(pbbox.right + 1, pbbox.bottom - 1, pbbox.left + -1, pbbox.bottom - 1);

				if (ExplodeTimeRemaining >= 10.f) g_VGuiSurface->DrawSetColor(Color(0, 255, 0));
				else if (ExplodeTimeRemaining < 10.f && ExplodeTimeRemaining >= 5.f) g_VGuiSurface->DrawSetColor(Color(255, 255, 0));
				else g_VGuiSurface->DrawSetColor(Color(255, 0, 0));

				g_VGuiSurface->DrawLine(pbbox.left, pbbox.bottom, pbbox.left, pbbox.top);
				g_VGuiSurface->DrawLine(pbbox.left, pbbox.top, pbbox.right, pbbox.top);
				g_VGuiSurface->DrawLine(pbbox.right, pbbox.top, pbbox.right, pbbox.bottom);
				g_VGuiSurface->DrawLine(pbbox.right, pbbox.bottom, pbbox.left, pbbox.bottom);
			}

			if (g_Config.esp_planted_c4)
			{
				int w = pbbox.right - pbbox.left;
				VGSHelper::Get().DrawPlantedText(ExplodeTimeRemaining, pbbox.left, pbbox.top, w);
			}
		}

	}

	C_BasePlayer* Defuser = (C_BasePlayer*)C_BasePlayer::get_entity_from_handle(ent->m_hBombDefuser());

	if (Defuser)
	{
		float fraction = DefuseTimeRemaining / ent->m_flTimerLength();
		int onscreenwidth = fraction * x;

		if (g_Config.esp_timer_c4)
		{
			char temp[256]; sprintf_s(temp, "Defuse in: %.1f", DefuseTimeRemaining);

			g_VGuiSurface->DrawSetColor(0, 0, 255, 220);
			g_VGuiSurface->DrawFilledRect(0, 10, onscreenwidth, 20);
			VGSHelper::Get().DrawC4Text(temp, onscreenwidth, 10, Color::White);
		}

		if (g_Config.esp_planted_c4)
		{
			if (pbbox.right == 0 || pbbox.bottom == 0)
				return;

			int w = pbbox.right - pbbox.left;
			VGSHelper::Get().DrawDefusedText(DefuseTimeRemaining, pbbox.left, pbbox.top, w);
		}
	}
}
void Visuals::DrawGrenade(C_BaseEntity* ent)
{
	const model_t* model = ent->GetModel();
	if (!model)
		return;

	studiohdr_t* hdr = g_MdlInfo->GetStudiomodel(model);
	if (!hdr)
		return;

	Color Nadecolor;
	std::string entityName = hdr->szName, icon_character;

	switch (ent->GetClientClass()->m_ClassID)
	{
	case CBaseCSGrenadeProjectile:
		if (entityName[16] == 's')
		{
			Nadecolor = Color(255, 255, 255, 200);
			entityName = "Flash";
			icon_character = "G";
		}
		else
		{
			Nadecolor = Color(255, 0, 0, 200);
			entityName = "Frag";
			icon_character = "H";
		}
		break;
	case CSmokeGrenadeProjectile:
		Nadecolor = Color(170, 170, 170, 200);
		entityName = "Smoke";
		icon_character = "P";
		break;
	case CMolotovProjectile:
		Nadecolor = Color(255, 165, 0, 200);
		entityName = "Fire";
		icon_character = "P";
		break;
	case CDecoyProjectile:
		Nadecolor = Color(255, 255, 0, 200);
		entityName = "Decoy";
		icon_character = "G";
		break;
	default:
		return;
	}

	Vector points_transformed[8];
	RECT size = GetBBox(ent, points_transformed);

	if (size.right == 0 || size.bottom == 0 || size.left == 0 || size.top == 0)
		return;

	if (g_Config.esp_misc_grenade > 1)
	{
		g_VGuiSurface->DrawSetColor(Color(20, 20, 20, 240));
		g_VGuiSurface->DrawLine(size.left - 1, size.bottom - 1, size.left - 1, size.top + 1);
		g_VGuiSurface->DrawLine(size.right + 1, size.top + 1, size.right + 1, size.bottom - 1);
		g_VGuiSurface->DrawLine(size.left - 1, size.top + 1, size.right + 1, size.top + 1);
		g_VGuiSurface->DrawLine(size.right + 1, size.bottom - 1, size.left + -1, size.bottom - 1);

		g_VGuiSurface->DrawSetColor(Nadecolor);
		g_VGuiSurface->DrawLine(size.left, size.bottom, size.left, size.top);
		g_VGuiSurface->DrawLine(size.left, size.top, size.right, size.top);
		g_VGuiSurface->DrawLine(size.right, size.top, size.right, size.bottom);
		g_VGuiSurface->DrawLine(size.right, size.bottom, size.left, size.bottom);
	}

	if (g_Config.esp_misc_grenade == 1 || g_Config.esp_misc_grenade == 3)
		VGSHelper::Get().DrawGrenadeText(entityName.c_str(), size.left, size.right, size.bottom, size.top, Nadecolor);

}
void Visuals::DrawDangerzoneItem(C_BaseEntity* ent, float maxRange)
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	ClientClass* cl = ent->GetClientClass();

	if (!cl)
		return;

	ClassId id = cl->m_ClassID;

	std::string name = "unknown";

	const model_t* itemModel = ent->GetModel();

	if (!itemModel)
		return;

	studiohdr_t* hdr = g_MdlInfo->GetStudiomodel(itemModel);

	if (!hdr)
		return;

	name = hdr->szName;

	if (id != ClassId::CPhysPropAmmoBox && id != ClassId::CPhysPropLootCrate && id != ClassId::CPhysPropRadarJammer && id != ClassId::CPhysPropWeaponUpgrade)
		return;

	Vector vPos2D;
	Vector vPos3D = ent->m_vecOrigin();

	//vPos3D
	if (g_LocalPlayer->m_vecOrigin().DistTo(vPos3D) > maxRange)
		return;

	if (!Math::WorldToScreen(vPos3D, vPos2D))
		return;

	if (name.find("case_pistol") != std::string::npos)
		name = "Pistol case";
	else if (name.find("case_light_weapon") != std::string::npos) // Reinforced!
		name = "Light case";
	else if (name.find("case_heavy_weapon") != std::string::npos)
		name = "Heavy case";
	else if (name.find("case_explosive") != std::string::npos)
		name = "Explosive case";
	else if (name.find("case_tools") != std::string::npos)
		name = "Tools case";
	else if (name.find("random") != std::string::npos)
		name = "Airdrop";
	else if (name.find("dz_armor_helmet") != std::string::npos)
		name = "Full armor";
	else if (name.find("dz_helmet") != std::string::npos)
		name = "Helmet";
	else if (name.find("dz_armor") != std::string::npos)
		name = "Armor";
	else if (name.find("upgrade_tablet") != std::string::npos)
		name = "Tablet upgrade";
	else if (name.find("briefcase") != std::string::npos)
		name = "Briefcase";
	else if (name.find("parachutepack") != std::string::npos)
		name = "Parachute";
	else if (name.find("dufflebag") != std::string::npos)
		name = "Cash dufflebag";
	else if (name.find("ammobox") != std::string::npos)
		name = "Ammobox";

	VGSHelper::Get().DrawText(name, vPos2D.x, vPos2D.y, Color::White, 12);
}
void Visuals::RenderSpectatorList()
{
	RECT scrn = GetViewport();
	int cnt = 0;
	for (int i = 1; i <= g_EntityList->GetHighestEntityIndex(); i++)
	{
		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);

		if (!player || player == nullptr)
			continue;

		player_info_t player_info;
		if (player != g_LocalPlayer)
		{
			if (g_EngineClient->GetPlayerInfo(i, &player_info) && !player->IsAlive() && !player->IsDormant())
			{
				auto observer_target = player->m_hObserverTarget();
				if (!observer_target)
					continue;

				auto target = observer_target.Get();
				if (!target)
					continue;

				player_info_t player_info2;
				if (g_EngineClient->GetPlayerInfo(target->EntIndex(), &player_info2))
				{
					char player_name[255] = { 0 };
					sprintf_s(player_name, "%s => %s", player_info.szName, player_info2.szName);
					int w, h;
					VGSHelper::Get().GetTextSize(spectatorlist_font, player_name, w, h);
					g_VGuiSurface->DrawSetColor(Color(0, 0, 0, 140));
					VGSHelper::Get().DrawFilledRect(scrn.right - 260, (scrn.bottom / 2) + (16 * cnt), 260, 16);
					VGSHelper::Get().DrawSpecText(player_name, scrn.right - w - 256, (scrn.bottom / 2) + (16 * cnt), target->EntIndex() == g_LocalPlayer->EntIndex() ? Color(250, 10, 20, 255) : Color(255, 255, 255, 255), 12);
					++cnt;
				}
			}
		}
	}
	VGSHelper::Get().DrawOutlinedRect(scrn.right - 261, (scrn.bottom / 2) - 1, 262, (16 * cnt) + 2, Color(0, 0, 0, 200));
	VGSHelper::Get().DrawOutlinedRect(scrn.right - 260, (scrn.bottom / 2), 260, (16 * cnt), Color(90, 90, 90, 160));
}
void Visuals::LbyIndicator()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	int x, y;
	g_EngineClient->GetScreenSize(x, y);

	bool Moving = g_LocalPlayer->m_vecVelocity().Length2D() > 0.1;
	bool InAir = !(g_LocalPlayer->m_fFlags() & FL_ONGROUND);

	Color clr = Color::Green;

	if (Moving && !InAir)
		clr = Color::Red;

	if (fabs(g_Saver.AARealAngle.yaw - g_LocalPlayer->m_flLowerBodyYawTarget()) < 35.f)
		clr = Color::Red;

	if (g_Saver.InFakewalk)
		clr = Color(255, 150, 0);

	float percent;

	if (Moving || InAir || g_Saver.InFakewalk)
		percent = 1.f;
	else
		percent = (g_Saver.NextLbyUpdate - g_GlobalVars->curtime) / 1.1f;

	percent = 1.f - percent;

	ImVec2 t = g_pDefaultFont->CalcTextSizeA(34.f, FLT_MAX, 0.0f, "LBY");
	float width = t.x * percent;

	Render::Get().RenderLine(9.f, y - 100.f - (CurrentIndicatorHeight - 34.f), 11.f + t.x, y - 100.f - (CurrentIndicatorHeight - 34.f), Color(0, 0, 0, 25), 4.f);

	if (width < t.x && width > 0.f)
		Render::Get().RenderLine(10.f, y - 100.f - (CurrentIndicatorHeight - 34.f), 10.f + width, y - 100.f - (CurrentIndicatorHeight - 34.f), clr, 2.f);

	Render::Get().RenderTextNoOutline("LBY", ImVec2(10, y - 100.f - CurrentIndicatorHeight), 34.f, clr);
	CurrentIndicatorHeight += 34.f;
}
void Visuals::Player::RenderBacktrackedSkelet()
{
	auto records = &LagCompensation::Get().m_LagRecord[ctx.pl->EntIndex()];
	if (records->size() < 2)
		return;

	Vector previous_screenpos;
	for (auto record = records->begin(); record != records->end(); record++)
	{
		if (!LagCompensation::Get().IsTickValid(TIME_TO_TICKS(record->m_flSimulationTime)))
			continue;

		Vector screen_pos;
		if (!Math::WorldToScreen(record->m_vecHeadSpot, screen_pos))
			continue;

		if (previous_screenpos.IsValid())
		{
			if (*record == LagCompensation::Get().m_RestoreLagRecord[ctx.pl->EntIndex()].first)
				g_VGuiSurface->DrawSetColor(Color(255, 255, 0, 255));
			else
				g_VGuiSurface->DrawSetColor(ctx.LCLineClr);
			g_VGuiSurface->DrawLine(screen_pos.x, screen_pos.y, previous_screenpos.x, previous_screenpos.y);
		}

		previous_screenpos = screen_pos;
	}
}
void Visuals::PingIndicator()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	INetChannelInfo* nci = g_EngineClient->GetNetChannelInfo();

	if (!nci)
		return;

	float ping = nci ? (nci->GetAvgLatency(FLOW_INCOMING)) * 1000.f : 0.0f;
	int x, y;
	g_EngineClient->GetScreenSize(x, y);

	float percent = ping / 100.f;
	ImVec2 t = g_pDefaultFont->CalcTextSizeA(34.f, FLT_MAX, 0.0f, "PING");
	float width = t.x * percent;

	int green = int(percent * 2.55f);
	int red = 255 - green;

	Render::Get().RenderLine(9.f, y - 100.f - (CurrentIndicatorHeight - 34.f), 11.f + t.x, y - 100.f - (CurrentIndicatorHeight - 34.f), Color(0, 0, 0, 25), 4.f);
	Render::Get().RenderLine(10.f, y - 100.f - (CurrentIndicatorHeight - 34.f), 10.f + width, y - 100.f - (CurrentIndicatorHeight - 34.f), Color(red, green, 0), 2.f);
	Render::Get().RenderTextNoOutline("PING", ImVec2(10, y - 100.f - CurrentIndicatorHeight), 34.f, Color(red, green, 0));
	CurrentIndicatorHeight += 34.f;
}
void Visuals::LCIndicator()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive() || g_LocalPlayer->m_fFlags() & FL_ONGROUND)
		return;

	int x, y;
	g_EngineClient->GetScreenSize(x, y);

	if ((g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		return;

	//ImVec2 t = g_pDefaultFont->CalcTextSizeA(34.f, FLT_MAX, 0.0f, "LBY");
	Render::Get().RenderTextNoOutline("LC", ImVec2(10, y - 100.f - CurrentIndicatorHeight), 34.f, g_Saver.LCbroken ? Color::Green : Color::Red);
	CurrentIndicatorHeight += 34.f;
}
void Visuals::AutowallCrosshair()
{
	/*
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;
	float Damage = 0.f;
	Autowall::Get().trace_awall(Damage);
	if (Damage != 0.f)
	{
		int x, y;
		g_EngineClient->GetScreenSize(x, y);

		float cx = x / 2.f, cy = y / 2.f;

		VGSHelper::Get().DrawText("Damage: "+std::to_string(Damage), cx, cy, Color::Green, 12);
	}
	*/
}
void Visuals::ManualAAIndicator()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	int x, y;
	g_EngineClient->GetScreenSize(x, y);
	float cx = x / 2.f;
	float cy = y / 2.f;

	Color blank = Color(1.f, 1.f, 1.f, g_Config.vis_misc_aa_circle_clr[3]);

	if (g_Config.rbot_aa == 2)
		switch (g_Config.rbot_manual_aa_state)
		{
		case 1:
			VGSHelper::Get().DrawAAText(L"▶", cx + 34, cy - 34, Color(g_Config.vis_misc_aa_circle_clr));
			VGSHelper::Get().DrawAAText(L"◀", cx - 64, cy - 34, blank);
			break;
		case 2:
			VGSHelper::Get().DrawAAText(L"▶", cx + 34, cy - 34, blank);
			VGSHelper::Get().DrawAAText(L"◀", cx - 64, cy - 34, Color(g_Config.vis_misc_aa_circle_clr));
			break;
		case 3:
			VGSHelper::Get().DrawAAText(L"▶", cx + 34, cy - 34, blank);
			VGSHelper::Get().DrawAAText(L"◀", cx - 64, cy - 34, blank);
			break;
		default:
			break;
		}
	else if (g_Config.rbot_aa == 1)
	{
		if (AntiAim::Get().side > 0)
		{
			VGSHelper::Get().DrawAAText(L"▶", cx + 34, cy - 34, Color(g_Config.vis_misc_aa_circle_clr));
			VGSHelper::Get().DrawAAText(L"◀", cx - 64, cy - 34, blank);
		}
		else if (AntiAim::Get().side < 0)
		{
			VGSHelper::Get().DrawAAText(L"▶", cx + 34, cy - 34, blank);
			VGSHelper::Get().DrawAAText(L"◀", cx - 64, cy - 34, Color(g_Config.vis_misc_aa_circle_clr));
		}
	}
}

void Visuals::SpreadCircle()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	C_BaseCombatWeapon* weapon = g_LocalPlayer->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	float spread = weapon->GetInaccuracy() * 1000;

	if (spread == 0.f)
		return;

	int x, y;
	g_EngineClient->GetScreenSize(x, y);
	float cx = x / 2.f;
	float cy = y / 2.f;


	switch (g_Config.vis_misc_draw_circle)
	{
	case 0:
		break;
	case 1:
		VGSHelper::Get().DrawCircle(cx, cy, spread, 35, Color(g_Config.vis_misc_draw_circle_clr));
		break;
	case 2:
		VGSHelper::Get().DrawFilledCircle(cx, cy, spread, 35, Color(g_Config.vis_misc_draw_circle_clr));
		break;
	}
}
void Visuals::RenderNoScoopeOverlay()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	static int cx;
	static int cy;
	static int w, h;

	g_EngineClient->GetScreenSize(w, h);
	cx = w / 2;
	cy = h / 2;

	if (g_LocalPlayer->m_bIsScoped())
	{
		VGSHelper::Get().DrawLine(0, cy, w, cy, Color(0, 0, 0, 255));
		VGSHelper::Get().DrawLine(cx, 0, cx, h, Color(0, 0, 0, 255));
	}
}
void Visuals::RenderHitmarker()
{
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive()) return;

	int w, h = 0;
	g_EngineClient->GetScreenSize(w, h);
	int cx, cy = 0;
	cx = w / 2;
	cy = h / 2;
	VGSHelper::Get().DrawLine(cx - 8, cy - 8, cx - 3, cy - 3, Color(255, 255, 255, hitmarkerAlpha));
	VGSHelper::Get().DrawLine(cx - 8, cy + 8, cx - 3, cy + 3, Color(255, 255, 255, hitmarkerAlpha));
	VGSHelper::Get().DrawLine(cx + 8, cy - 8, cx + 3, cy - 3, Color(255, 255, 255, hitmarkerAlpha));
	VGSHelper::Get().DrawLine(cx + 8, cy + 8, cx + 3, cy + 3, Color(255, 255, 255, hitmarkerAlpha));
	auto fade = ((1.f / 0.5f) * g_GlobalVars->frametime) * 255;

	if (hitmarkerAlpha > 0)
		hitmarkerAlpha -= fade;

	if (hitmarkerAlpha < 0)
		hitmarkerAlpha = 0;
}
void Visuals::RunHitmarker()
{
	hitmarkerAlpha = 255;
}
void Visuals::AddToDrawList()
{
	if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_EngineClient->IsInGame())
		return;

	for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i)
	{
		auto entity = C_BaseEntity::GetEntityByIndex(i);
		if (!entity) continue;
		if (i < 65 && (g_Config.esp_local_enable || g_Config.esp_team_enable || g_Config.esp_enemy_enable))
		{
			auto player = Player();

			if (player.Begin((C_BasePlayer*)entity))
			{
				bool Enemy = player.ctx.pl->IsEnemy();
				bool Local = player.ctx.pl == g_LocalPlayer;
				bool Team = Team = !Enemy && !Local;

				int width, height; g_EngineClient->GetScreenSize(width, height);

				bool gungate = player.ctx.pl->m_bGunGameImmunity();

				if (!gungate)
				{
					if (Local)
					{
						player.ctx.BoxClr = Color(g_Config.color_esp_local_boxes);
						player.ctx.BoxFill = Color(g_Config.color_esp_local_boxes_fill);
						player.ctx.NameClr = Color(g_Config.color_esp_local_names);
						player.ctx.ArmourClr = Color(g_Config.color_esp_local_armour);
						player.ctx.WeaponClr = Color(g_Config.color_esp_local_weapons);
						player.ctx.FlagClr = Color(g_Config.color_esp_local_flags);
						player.ctx.LineViewClr = Color(g_Config.color_esp_local_anglines);
						player.ctx.LineLbyClr = Color(g_Config.color_esp_local_lbylines);
						player.ctx.EDHeadClr = Color(g_Config.color_esp_local_headbox);
					}
					else
					{
						if (player.ctx.is_visible)
						{
							if (Team)
							{
								player.ctx.BoxClr = Color(g_Config.color_esp_team_visible_boxes);
								player.ctx.BoxFill = Color(g_Config.color_esp_team_visible_boxes_fill);
								player.ctx.NameClr = Color(g_Config.color_esp_team_visible_names);
								player.ctx.ArmourClr = Color(g_Config.color_esp_team_visible_armour);
								player.ctx.WeaponClr = Color(g_Config.color_esp_team_visible_weapons);
								player.ctx.SnaplineClr = Color(g_Config.color_esp_team_visible_snaplines);
								player.ctx.PovClr = Color(g_Config.color_esp_team_visible_pov);
								player.ctx.FlagClr = Color(g_Config.color_esp_team_visible_flags);
								player.ctx.LineViewClr = Color(g_Config.color_esp_team_visible_anglines);
								player.ctx.LineLbyClr = Color(g_Config.color_esp_team_visible_lbylines);
								player.ctx.EDHeadClr = Color(g_Config.color_esp_team_visible_headbox);
							}
							else
							{
								player.ctx.BoxClr = Color(g_Config.color_esp_enemy_visible_boxes);
								player.ctx.BoxFill = Color(g_Config.color_esp_enemy_visible_boxes_fill);
								player.ctx.NameClr = Color(g_Config.color_esp_enemy_visible_names);
								player.ctx.ArmourClr = Color(g_Config.color_esp_enemy_visible_armour);
								player.ctx.WeaponClr = Color(g_Config.color_esp_enemy_visible_weapons);
								player.ctx.SnaplineClr = Color(g_Config.color_esp_enemy_visible_snaplines);
								player.ctx.InfoClr = Color(g_Config.color_esp_enemy_visible_info);
								player.ctx.LbyTimerClr = Color(g_Config.color_esp_enemy_visible_lby_timer);
								player.ctx.PovClr = Color(g_Config.color_esp_enemy_visible_pov);
								player.ctx.FlagClr = Color(g_Config.color_esp_enemy_visible_flags);
								player.ctx.LineViewClr = Color(g_Config.color_esp_enemy_visible_anglines);
								player.ctx.LineLbyClr = Color(g_Config.color_esp_enemy_visible_lbylines);
								player.ctx.LCLineClr = Color(g_Config.color_esp_enemy_visible_lcline);
								player.ctx.EDHeadClr = Color(g_Config.color_esp_enemy_visible_headbox);
							}
						}
						else
						{
							if (Team)
							{
								player.ctx.BoxClr = Color(g_Config.color_esp_team_invisible_boxes);
								player.ctx.BoxFill = Color(g_Config.color_esp_team_invisible_boxes_fill);
								player.ctx.NameClr = Color(g_Config.color_esp_team_invisible_names);
								player.ctx.ArmourClr = Color(g_Config.color_esp_team_invisible_armour);
								player.ctx.WeaponClr = Color(g_Config.color_esp_team_invisible_weapons);
								player.ctx.SnaplineClr = Color(g_Config.color_esp_team_invisible_snaplines);
								player.ctx.PovClr = Color(g_Config.color_esp_team_invisible_pov);
								player.ctx.FlagClr = Color(g_Config.color_esp_team_invisible_flags);
								player.ctx.LineViewClr = Color(g_Config.color_esp_team_invisible_anglines);
								player.ctx.LineLbyClr = Color(g_Config.color_esp_team_invisible_lbylines);
								player.ctx.EDHeadClr = Color(g_Config.color_esp_team_invisible_headbox);
							}
							else
							{
								player.ctx.BoxClr = Color(g_Config.color_esp_enemy_invisible_boxes);
								player.ctx.BoxFill = Color(g_Config.color_esp_enemy_invisible_boxes_fill);
								player.ctx.NameClr = Color(g_Config.color_esp_enemy_invisible_names);
								player.ctx.ArmourClr = Color(g_Config.color_esp_enemy_invisible_armour);
								player.ctx.WeaponClr = Color(g_Config.color_esp_enemy_invisible_weapons);
								player.ctx.SnaplineClr = Color(g_Config.color_esp_enemy_invisible_snaplines);
								player.ctx.InfoClr = Color(g_Config.color_esp_enemy_invisible_info);
								player.ctx.LbyTimerClr = Color(g_Config.color_esp_enemy_invisible_lby_timer);
								player.ctx.PovClr = Color(g_Config.color_esp_enemy_invisible_pov);
								player.ctx.FlagClr = Color(g_Config.color_esp_enemy_invisible_flags);
								player.ctx.LineViewClr = Color(g_Config.color_esp_enemy_invisible_anglines);
								player.ctx.LineLbyClr = Color(g_Config.color_esp_enemy_invisible_lbylines);
								player.ctx.EDHeadClr = Color(g_Config.color_esp_enemy_invisible_headbox);
								player.ctx.LCLineClr = Color(g_Config.color_esp_enemy_invisible_lcline);
							}
						}
					}
				}
				else
				{
					player.ctx.BoxFill = Color(0, 0, 0, 0);
					player.ctx.BoxClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.NameClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.ArmourClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.WeaponClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.SnaplineClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.InfoClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.LbyTimerClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.PovClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.FlagClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.LineViewClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.LineLbyClr = Color(g_Config.color_esp_gun_immunity);
					player.ctx.EDHeadClr = Color(g_Config.color_esp_gun_immunity);
				}

				bool teamgate = !player.ctx.is_visible && g_Config.esp_team_visible_only;
				bool enemygate = !player.ctx.is_visible && g_Config.esp_enemy_visible_only;

				int healthgate = (player.ctx.pl->m_iHealth()); // mne tak proshe cho priebalsya))))
				int armorgate = (player.ctx.pl->m_ArmorValue());

				if (Enemy)
					player.ctx.boxmode = g_Config.esp_enemy_boxes_type;
				else if (Local)
					player.ctx.boxmode = g_Config.esp_local_boxes_type;
				else
					player.ctx.boxmode = g_Config.esp_team_boxes_type;

				if ((Local && g_Config.esp_local_enable && g_Config.esp_local_boxes &&
					g_Config.esp_local_boxes_outline) ||
					(!teamgate && Team && g_Config.esp_team_enable && g_Config.esp_team_boxes
						&& g_Config.esp_team_boxes_outline) ||
						(!enemygate && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_boxes
							&& g_Config.esp_enemy_boxes_outline))							player.RenderBoxOutline();

				if ((Local && g_Config.esp_local_enable && g_Config.esp_local_boxes) ||
					(!teamgate && Team && g_Config.esp_team_enable && g_Config.esp_team_boxes) ||
					(!enemygate && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_boxes))		player.RenderBox();

				if ((Local && g_Config.esp_local_enable && g_Config.esp_local_headbox) ||
					(!teamgate && Team && g_Config.esp_team_enable && g_Config.esp_team_headbox) ||
					(!enemygate && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_headbox))		player.Render3DHead();

				if ((Local && g_Config.esp_local_enable && g_Config.esp_local_boxes_fill) ||
					(!teamgate && Team && g_Config.esp_team_enable && g_Config.esp_team_boxes_fill) ||
					(!enemygate && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_boxes_fill))		player.RenderFill();

				if ((!teamgate && Team && g_Config.esp_team_enable && g_Config.esp_team_snaplines) ||
					(!enemygate && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_snaplines))	player.RenderSnapline();

				if ((Local && g_Config.esp_local_enable && g_Config.esp_local_lines) ||
					(!teamgate && Team && g_Config.esp_team_enable && g_Config.esp_team_lines) ||
					(!enemygate && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_lines))		player.DrawAngleLines();

				if ((Local && g_Config.esp_local_enable && g_Config.esp_local_names) ||
					(!teamgate && Team && g_Config.esp_team_enable && g_Config.esp_team_names) ||
					(!enemygate && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_names))		player.RenderName();

				if (!gungate && (Local && !(is100(healthgate) && g_Config.esp_local_health_norender) && g_Config.esp_local_enable && g_Config.esp_local_health) ||
					(!gungate && !teamgate && !(is100(healthgate) && g_Config.esp_team_health_norender) && Team && g_Config.esp_team_enable && g_Config.esp_team_health) ||
					(!gungate && !enemygate && !(is100(healthgate) && g_Config.esp_enemy_health_norender) && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_health))		player.RenderHealth();

				if (!gungate && (Local && !(is100(armorgate) && g_Config.esp_local_armour_norender) && g_Config.esp_local_enable && g_Config.esp_local_armour) ||
					(!gungate && !teamgate && !(is100(armorgate) && g_Config.esp_team_armour_norender) && Team && g_Config.esp_team_enable && g_Config.esp_team_armour) ||
					(!gungate && !enemygate && !(is100(armorgate) && g_Config.esp_enemy_armour_norender) && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_armour))
				{
					player.RenderArmour(); player.ctx.is_armored = true;
				}
				else {
					player.ctx.is_armored = false;
				}

				if (g_Config.rbot_lagcompensation && !enemygate && Enemy && g_Config.esp_enemy_lclines)				player.RenderBacktrackedSkelet();

				if ((Local && g_Config.esp_local_enable && g_Config.esp_local_weapons) ||
					(!teamgate && Team && g_Config.esp_team_enable && g_Config.esp_team_weapons) ||
					(!enemygate && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_weapons))	player.RenderWeaponName();

				if ((Local && g_Config.esp_local_enable && g_Config.esp_local_weapons_ammo & g_Config.esp_local_weapons) ||
					(!teamgate && Team && g_Config.esp_team_enable && g_Config.esp_team_weapons_ammo & g_Config.esp_team_weapons) ||
					(!enemygate && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_weapons_ammo & g_Config.esp_enemy_weapons))	player.RenderWeaponAmmo();

				if ((Local && g_Config.esp_local_enable && g_Config.esp_local_flags) ||
					(!teamgate && Team && g_Config.esp_team_enable && g_Config.esp_team_flags) ||
					(!enemygate && Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_flags))	player.DrawFlags();

				//if (!enemygate && Enemy && g_Config.esp_enemy_lby_timer)									player.RenderLbyUpdateBar();

				if ((Team && g_Config.esp_team_enable && g_Config.esp_team_pov) ||
					(Enemy && g_Config.esp_enemy_enable && g_Config.esp_enemy_pov))	player.DrawArrow();

			}
		}
		else if (entity->IsPlantedC4() && (g_Config.esp_planted_c4 || g_Config.esp_timer_c4)) RenderC4(entity);
		else if (entity->IsWeapon() && g_Config.esp_dropped_weapons_type > 0) RenderWeapon(static_cast<C_BaseCombatWeapon*> (entity), Utils::get_weapon_name(entity));
		if (g_LocalPlayer && g_LocalPlayer->InDangerzone() && g_Config.esp_misc_dangerzone_item_esp) DrawDangerzoneItem(entity, g_Config.esp_misc_dangerzone_item_esp_dist);
		if (g_Config.esp_misc_grenade > 0) DrawGrenade(entity);
	}

	if (g_Config.vis_misc_autowall_crosshair)	AutowallCrosshair();
	if (g_Config.vis_misc_draw_circle > 0)		SpreadCircle();
	if (g_Config.vis_misc_noscope)				RenderNoScoopeOverlay();
	if (g_Config.vis_misc_hitmarker)			RenderHitmarker();

	CCSGrenadeHint::Get().Paint();

	if (g_Config.vis_misc_drawteaserrange)			DrawZeusRange();

	if (g_Config.esp_misc_spectator_list)		RenderSpectatorList();

	CurrentIndicatorHeight = 0.f;

	if (g_Config.rbot_aa > 0 && g_Config.esp_misc_aa_indicator)
		ManualAAIndicator();

	Features::EventLogger->DrawPrefix();
	Features::EventLogger->DrawMain();
}
void VGSHelper::Init()
{
	namefont = g_VGuiSurface->CreateFont_(); g_VGuiSurface->SetFontGlyphSet(namefont, "Bell Gothic", 11, 450, 0, 0, FONTFLAG_OUTLINE);
	font = g_VGuiSurface->CreateFont_(); g_VGuiSurface->SetFontGlyphSet(font, "Bell Gothic", 10, 450, 0, 0, FONTFLAG_OUTLINE);
	eventfont = g_VGuiSurface->CreateFont_(); g_VGuiSurface->SetFontGlyphSet(eventfont, "Tahoma", 12, 500, 0, 0, FONTFLAG_DROPSHADOW);
	c4font = g_VGuiSurface->CreateFont_(); g_VGuiSurface->SetFontGlyphSet(c4font, "Bell Gothic", 11, 450, 0, 0, FONTFLAG_OUTLINE);
	aafont = g_VGuiSurface->CreateFont_(); g_VGuiSurface->SetFontGlyphSet(aafont, "Arial", 60, 500, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);

	spectatorlist_font = g_VGuiSurface->CreateFont_(); g_VGuiSurface->SetFontGlyphSet(spectatorlist_font, "Tahoma", 14, 350, 0, 0, FONTFLAG_OUTLINE);

	Inited = true;
}
void VGSHelper::DrawText(std::string text, float x, float y, Color color, int size)
{
	if (!Inited)
		Init();
	g_VGuiSurface->DrawClearApparentDepth();
	wchar_t buf[256];
	g_VGuiSurface->DrawSetTextFont(font);
	g_VGuiSurface->DrawSetTextColor(color);

	if (MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buf, 256))
	{
		g_VGuiSurface->DrawSetTextPos(x, y);
		g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
	}
}
//pizdec start
void VGSHelper::DrawSpecText(std::string text, float x, float y, Color color, int size)
{
	if (!Inited)
		Init();

	g_VGuiSurface->DrawClearApparentDepth();
	wchar_t buf[256];
	g_VGuiSurface->DrawSetTextFont(spectatorlist_font);
	g_VGuiSurface->DrawSetTextColor(color);

	if (MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buf, 256))
	{
		g_VGuiSurface->DrawSetTextPos(x, y);
		g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
	}
}
void VGSHelper::DrawNameText(std::string name, float x, float y, float z, Color color)
{
	if (!Inited)
		Init();

	wchar_t buf[128];
	name = (name.length() > 0 ? name : "##ERROR_empty_name");
	color = (name.length() > 0 ? color : Color::Red);

	if (MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, buf, 256) > 0)
	{
		int tw, th;
		g_VGuiSurface->GetTextSize(namefont, buf, tw, th);

		g_VGuiSurface->DrawSetTextFont(namefont);
		g_VGuiSurface->DrawSetTextColor(color);
		g_VGuiSurface->DrawSetTextPos(x + (y - x) * 0.5 - tw * 0.5, z - th + 1);
		g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
	}
}
void VGSHelper::DrawWeaponText(std::string weapon, float x, float y, float z, Color color)
{
	if (!Inited)
		Init();

	wchar_t buf[128];
	weapon = (weapon.length() > 0 ? weapon : "##ERROR_empty_name");

	if (MultiByteToWideChar(CP_UTF8, 0, weapon.c_str(), -1, buf, 256) > 0)
	{
		int tw, th;
		g_VGuiSurface->GetTextSize(font, buf, tw, th);

		g_VGuiSurface->DrawSetTextFont(font);
		g_VGuiSurface->DrawSetTextColor(color);
		g_VGuiSurface->DrawSetTextPos(x + (y - x) * 0.5 - tw * 0.5, z + 1);
		g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
	}
}
void VGSHelper::DrawWeaponEntityText(std::string weapon, float x, float y, float z, Color color)
{
	if (!Inited)
		Init();

	wchar_t buf[128];

	if (MultiByteToWideChar(CP_UTF8, 0, weapon.c_str(), -1, buf, 256) > 0)
	{
		int tw, th;
		g_VGuiSurface->GetTextSize(font, buf, tw, th);

		g_VGuiSurface->DrawSetTextFont(font);
		g_VGuiSurface->DrawSetTextColor(color);
		g_VGuiSurface->DrawSetTextPos(x + ((y - x) / 2) - (tw / 2), z + 1);
		g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
	}

}
void VGSHelper::DrawGrenadeText(std::string nade, float x, float y, float z, float i, Color color)
{
	if (!Inited)
		Init();

	wchar_t buf[128];

	if (MultiByteToWideChar(CP_UTF8, 0, nade.c_str(), -1, buf, 256))
	{
		int tw, th, rekt;
		g_VGuiSurface->GetTextSize(font, buf, tw, th);

		rekt = x + (y - x) * 0.5;
		rekt -= tw / 2;

		g_VGuiSurface->DrawSetTextFont(font);
		g_VGuiSurface->DrawSetTextColor(color);
		g_VGuiSurface->DrawSetTextPos(rekt, z + (i - z) + 2);
		g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
	}
}
void VGSHelper::DrawPlantedText(float plant_time, float x, float y, float w)
{
	if (!Inited)
		Init();

	wchar_t buf[256];

	char TimeToExplode[64]; sprintf_s(TimeToExplode, /*"Explode in: */"%.1f", plant_time);

	if (MultiByteToWideChar(CP_UTF8, 0, TimeToExplode, -1, buf, 256))
	{
		int tw, th;
		g_VGuiSurface->GetTextSize(c4font, buf, tw, th);

		g_VGuiSurface->DrawSetTextFont(c4font);
		g_VGuiSurface->DrawSetTextColor(Color::White);
		g_VGuiSurface->DrawSetTextPos((x + w * 0.5f) - tw * 0.5f, y + 2);
		g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
	}
}
void VGSHelper::DrawDefusedText(float defuse_time, float x, float y, float w)
{
	if (!Inited)
		Init();

	wchar_t buf[256];

	char TimeToDefuse[64]; sprintf_s(TimeToDefuse, /*"Defuse in: */"%.1f", defuse_time);

	if (MultiByteToWideChar(CP_UTF8, 0, TimeToDefuse, -1, buf, 256))
	{
		int tw, th;
		g_VGuiSurface->GetTextSize(c4font, buf, tw, th);

		g_VGuiSurface->DrawSetTextFont(c4font);
		g_VGuiSurface->DrawSetTextColor(Color::Blue);
		g_VGuiSurface->DrawSetTextPos((x + w * 0.5f) - tw * 0.5f, y + 12);
		g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
	}
}
void VGSHelper::DrawC4Text(std::string text, float x, float y, Color color)
{
	if (!Inited)
		Init();

	g_VGuiSurface->DrawClearApparentDepth();
	wchar_t buf[256];
	g_VGuiSurface->DrawSetTextFont(c4font);
	g_VGuiSurface->DrawSetTextColor(color);

	if (MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buf, 256))
	{
		g_VGuiSurface->DrawSetTextPos(x, y);
		g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
	}
}
void VGSHelper::DrawEventText(std::string text, float x, float y, Color color, int size)
{
	if (!Inited)
		Init();

	g_VGuiSurface->DrawClearApparentDepth();
	wchar_t buf[256];
	g_VGuiSurface->DrawSetTextFont(eventfont);
	g_VGuiSurface->DrawSetTextColor(color);

	if (MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buf, 256))
	{
		g_VGuiSurface->DrawSetTextPos(x, y);
		g_VGuiSurface->DrawPrintText(buf, wcslen(buf));
	}
}
void VGSHelper::DrawAAText(const wchar_t* text, float x, float y, Color color)
{
	if (!Inited)
		Init();
	//shonax
	g_VGuiSurface->DrawSetTextFont(aafont);
	g_VGuiSurface->DrawSetTextColor(color);
	g_VGuiSurface->DrawSetTextPos(x, y);

	g_VGuiSurface->DrawPrintText(text, wcslen(text));
}
//pizdec end
void VGSHelper::DrawLine(float x1, float y1, float x2, float y2, Color color, float size)
{
	/*
	if (outline) {
		g_VGuiSurface->DrawSetColor(Color::Black);
		//g_VGuiSurface->DrawSetApparentDepth(size + 1.f);
		//g_VGuiSurface->DrawLine(x1, y1, x2, y2);
		g_VGuiSurface->DrawFilledRect(x1 - size, y1 - size, x2 + size, y2 + size);
	}
	*/
	g_VGuiSurface->DrawSetColor(color);

	if (size == 1.f)
		g_VGuiSurface->DrawLine(x1, y1, x2, y2);
	else
		g_VGuiSurface->DrawFilledRect(x1 - (size / 2.f), y1 - (size / 2.f), x2 + (size / 2.f), y2 + (size / 2.f));
}
void VGSHelper::DrawBox(float x1, float y1, float x2, float y2, Color clr, float size)
{
	/*
	if (outline) {
		g_VGuiSurface->DrawSetColor(Color::Black);
		g_VGuiSurface->DrawFilledRect(x1 - 1.f, y1, x1 + 1.f, y2); // left
		g_VGuiSurface->DrawFilledRect(x2 - 1.f, y1, x2 + 1.f, y2); // right
		g_VGuiSurface->DrawFilledRect(x1, y1 - 1.f, x2, y1 + 1.f); // top
		g_VGuiSurface->DrawFilledRect(x1, y2 - 1.f, x2, y2 + 1.f); // bottom
	}
	*/
	//g_VGuiSurface->DrawSetColor(clr);
	//g_VGuiSurface->DrawSetApparentDepth(size);
	//g_VGuiSurface->DrawOutlinedRect(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2));
	DrawLine(x1, y1, x2, y1, clr, size);
	DrawLine(x1, y2, x2, y2, clr, size);
	DrawLine(x1, y1, x1, y2, clr, size);
	DrawLine(x2, y1, x2, y2, clr, size);

}
void VGSHelper::DrawFilledBox(float x1, float y1, float x2, float y2, Color clr)
{
	g_VGuiSurface->DrawSetColor(clr);
	//g_VGuiSurface->DrawSetApparentDepth(size);
	g_VGuiSurface->DrawFilledRect(static_cast<int> (x1), static_cast<int> (y1), static_cast<int> (x2), static_cast<int> (y2));

}
void VGSHelper::DrawTriangle(int count, Vertex_t* vertexes, Color c)
{
	static int Texture = g_VGuiSurface->CreateNewTextureID(true); // need to make a texture with procedural true
	unsigned char buffer[4] = { (unsigned char)c.r(), (unsigned char)c.g(), (unsigned char)c.b(), (unsigned char)c.a() }; // r,g,b,a

	g_VGuiSurface->DrawSetTextureRGBA(Texture, buffer, 1, 1); //Texture, char array of texture, width, height
	g_VGuiSurface->DrawSetColor(c); // keep this full color and opacity use the RGBA @top to set values.
	g_VGuiSurface->DrawSetTexture(Texture); // bind texture

	g_VGuiSurface->DrawTexturedPolygon(count, vertexes);
}
void VGSHelper::DrawBoxEdges(float x1, float y1, float x2, float y2, Color clr, float edge_size, float size)
{
	if (fabs(x1 - x2) < (edge_size * 2))
	{
		//x2 = x1 + fabs(x1 - x2);
		edge_size = fabs(x1 - x2) / 4.f;
	}

	DrawLine(x1, y1, x1, y1 + edge_size + (0.5f * edge_size), clr, size);
	DrawLine(x2, y1, x2, y1 + edge_size + (0.5f * edge_size), clr, size);
	DrawLine(x1, y2, x1, y2 - edge_size - (0.5f * edge_size), clr, size);
	DrawLine(x2, y2, x2, y2 - edge_size - (0.5f * edge_size), clr, size);
	DrawLine(x1, y1, x1 + edge_size, y1, clr, size);
	DrawLine(x2, y1, x2 - edge_size, y1, clr, size);
	DrawLine(x1, y2, x1 + edge_size, y2, clr, size);
	DrawLine(x2, y2, x2 - edge_size, y2, clr, size);
}
void VGSHelper::DrawCircle(float x, float y, float r, int seg, Color clr)
{
	g_VGuiSurface->DrawSetColor(clr);
	g_VGuiSurface->DrawOutlinedCircle(x, y, r, seg);
}
void VGSHelper::Polygon(int count, Vertex_t* Vertexs, Color color)
{
	static int Texture = g_VGuiSurface->CreateNewTextureID(true);
	unsigned char buffer[4] = { color.r(), color.g(), color.b(), color.a() };

	g_VGuiSurface->DrawSetTextureRGBA(Texture, buffer, 1, 1);
	g_VGuiSurface->DrawSetColor(Color(255, 255, 255, 255));
	g_VGuiSurface->DrawSetTexture(Texture);

	g_VGuiSurface->DrawTexturedPolygon(count, Vertexs);
}
void VGSHelper::DrawFilledTriangle(std::array<Vector2D, 3> points, Color color)
{
	std::array<Vertex_t, 3> vertices{ Vertex_t(points.at(0)), Vertex_t(points.at(1)), Vertex_t(points.at(2)) };
	Polygon(3, vertices.data(), color);
}
void VGSHelper::PolygonOutline(int count, Vertex_t* Vertexs, Color color, Color colorLine)
{
	static int x[128];
	static int y[128];

	Polygon(count, Vertexs, color);

	for (int i = 0; i < count; i++)
	{
		x[i] = Vertexs[i].m_Position.x;
		y[i] = Vertexs[i].m_Position.y;
	}

	PolyLine(x, y, count, colorLine);
}
void VGSHelper::PolyLine(int count, Vertex_t* Vertexs, Color colorLine)
{
	static int x[128];
	static int y[128];

	for (int i = 0; i < count; i++)
	{
		x[i] = Vertexs[i].m_Position.x;
		y[i] = Vertexs[i].m_Position.y;
	}

	PolyLine(x, y, count, colorLine);
}
void VGSHelper::PolyLine(int* x, int* y, int count, Color color)
{
	g_VGuiSurface->DrawSetColor(color);
	g_VGuiSurface->DrawPolyLine(x, y, count);
}
void VGSHelper::Draw3DCube(float scalar, QAngle angles, Vector middle_origin, Color outline)
{
	Vector forward, right, up;
	Math::AngleVectors(angles, forward, right, up);

	Vector points[8];
	points[0] = middle_origin - (right * scalar) + (up * scalar) - (forward * scalar); // BLT
	points[1] = middle_origin + (right * scalar) + (up * scalar) - (forward * scalar); // BRT
	points[2] = middle_origin - (right * scalar) - (up * scalar) - (forward * scalar); // BLB
	points[3] = middle_origin + (right * scalar) - (up * scalar) - (forward * scalar); // BRB

	points[4] = middle_origin - (right * scalar) + (up * scalar) + (forward * scalar); // FLT
	points[5] = middle_origin + (right * scalar) + (up * scalar) + (forward * scalar); // FRT
	points[6] = middle_origin - (right * scalar) - (up * scalar) + (forward * scalar); // FLB
	points[7] = middle_origin + (right * scalar) - (up * scalar) + (forward * scalar); // FRB

	Vector points_screen[8];
	for (int i = 0; i < 8; i++)
		if (!Math::WorldToScreen(points[i], points_screen[i]))
			return;

	g_VGuiSurface->DrawSetColor(outline);

	// Back frame
	g_VGuiSurface->DrawLine(points_screen[0].x, points_screen[0].y, points_screen[1].x, points_screen[1].y);
	g_VGuiSurface->DrawLine(points_screen[0].x, points_screen[0].y, points_screen[2].x, points_screen[2].y);
	g_VGuiSurface->DrawLine(points_screen[3].x, points_screen[3].y, points_screen[1].x, points_screen[1].y);
	g_VGuiSurface->DrawLine(points_screen[3].x, points_screen[3].y, points_screen[2].x, points_screen[2].y);

	// Frame connector
	g_VGuiSurface->DrawLine(points_screen[0].x, points_screen[0].y, points_screen[4].x, points_screen[4].y);
	g_VGuiSurface->DrawLine(points_screen[1].x, points_screen[1].y, points_screen[5].x, points_screen[5].y);
	g_VGuiSurface->DrawLine(points_screen[2].x, points_screen[2].y, points_screen[6].x, points_screen[6].y);
	g_VGuiSurface->DrawLine(points_screen[3].x, points_screen[3].y, points_screen[7].x, points_screen[7].y);

	// Front frame
	g_VGuiSurface->DrawLine(points_screen[4].x, points_screen[4].y, points_screen[5].x, points_screen[5].y);
	g_VGuiSurface->DrawLine(points_screen[4].x, points_screen[4].y, points_screen[6].x, points_screen[6].y);
	g_VGuiSurface->DrawLine(points_screen[7].x, points_screen[7].y, points_screen[5].x, points_screen[5].y);
	g_VGuiSurface->DrawLine(points_screen[7].x, points_screen[7].y, points_screen[6].x, points_screen[6].y);
}
void VGSHelper::FillRGBA(int x, int y, int w, int h, Color c)
{
	g_VGuiSurface->DrawSetColor(c);
	g_VGuiSurface->DrawFilledRect(x, y, x + w, y + h);
}
void VGSHelper::BorderBox(int x, int y, int w, int h, Color color, int thickness)
{
	FillRGBA(x, y, w, thickness, color);
	FillRGBA(x, y, thickness, h, color);
	FillRGBA(x + w, y, thickness, h, color);
	FillRGBA(x, y + h, w + thickness, thickness, color);
}
__inline void VGSHelper::DrawFilledRect(int x, int y, int w, int h)
{
	g_VGuiSurface->DrawFilledRect(x, y, x + w, y + h);
}
void VGSHelper::DrawRectOutlined(int x, int y, int w, int h, Color color, Color outlinedColor, int thickness)
{
	FillRGBA(x, y, w, h, color);
	BorderBox(x - 1, y - 1, w + 1, h + 1, outlinedColor, thickness);
}
void VGSHelper::DrawString(unsigned long font, int x, int y, Color color, unsigned long alignment, const char* msg, ...)
{
	if (!Inited)
		Init();

	va_list va_alist;
	char buf[1024];
	va_start(va_alist, msg);
	_vsnprintf(buf, sizeof(buf), msg, va_alist);
	va_end(va_alist);
	wchar_t wbuf[1024];
	MultiByteToWideChar(CP_UTF8, 0, buf, 256, wbuf, 256);

	int r = 255, g = 255, b = 255, a = 255;
	color.GetColor(r, g, b, a);

	int width, height;
	g_VGuiSurface->GetTextSize(font, wbuf, width, height);

	if (alignment & FONT_RIGHT)
		x -= width;
	if (alignment & FONT_CENTER)
		x -= width / 2;

	g_VGuiSurface->DrawSetTextFont(font);
	g_VGuiSurface->DrawSetTextColor(r, g, b, a);
	g_VGuiSurface->DrawSetTextPos(x, y - height / 2);
	g_VGuiSurface->DrawPrintText(wbuf, wcslen(wbuf));
}
void VGSHelper::DrawString(unsigned long font, bool center, int x, int y, Color c, const char* fmt, ...)
{
	if (!Inited)
		Init();

	wchar_t* pszStringWide = reinterpret_cast<wchar_t*>(malloc((strlen(fmt) + 1) * sizeof(wchar_t)));

	mbstowcs(pszStringWide, fmt, (strlen(fmt) + 1) * sizeof(wchar_t));

	TextW(center, font, x, y, c, pszStringWide);

	free(pszStringWide);
}
void VGSHelper::TextW(bool center, unsigned long font, int x, int y, Color c, wchar_t* pszString)
{
	if (center)
	{
		int wide, tall;
		g_VGuiSurface->GetTextSize(font, pszString, wide, tall);
		x -= wide / 2;
		y -= tall / 2;
	}
	g_VGuiSurface->DrawSetTextColor(c);
	g_VGuiSurface->DrawSetTextFont(font);
	g_VGuiSurface->DrawSetTextPos(x, y);
	g_VGuiSurface->DrawPrintText(pszString, (int)wcslen(pszString), FONT_DRAW_DEFAULT);
}
void VGSHelper::DrawCircle(int x, int y, float r, int step, Color color)
{
	float Step = M_PI * 2.0 / step;
	for (float a = 0; a < (M_PI * 2.0); a += Step)
	{
		float x1 = r * cos(a) + x;
		float y1 = r * sin(a) + y;
		float x2 = r * cos(a + Step) + x;
		float y2 = r * sin(a + Step) + y;
		g_VGuiSurface->DrawSetColor(color);
		g_VGuiSurface->DrawLine(x1, y1, x2, y2);
	}
}
void VGSHelper::DrawFilledCircle(float x, float y, float r, int seg, Color clr)
{
	static bool once = true;

	static std::vector<float> temppointsx;
	static std::vector<float> temppointsy;

	if (once)
	{
		float step = (float)M_PI * 2.0f / seg;
		for (float a = 0; a < (M_PI * 2.0f); a += step)
		{
			temppointsx.push_back(cosf(a));
			temppointsy.push_back(sinf(a));
		}
		once = false;
	}

	std::vector<int> pointsx;
	std::vector<int> pointsy;
	std::vector<Vertex_t> vertices;

	for (int i = 0; i < temppointsx.size(); i++)
	{
		float fx = r * temppointsx[i] + x;
		float fy = r * temppointsy[i] + y;
		pointsx.push_back(fx);
		pointsy.push_back(fy);

		vertices.push_back(Vertex_t(Vector2D(fx, fy)));
	}

	g_VGuiSurface->DrawSetColor(clr);
	g_VGuiSurface->DrawTexturedPolygon(seg, vertices.data());

	//DrawTexturedPoly(points, vertices.data(), color);
	//g_pSurface->DrawSetColor(outline);
	//g_pSurface->DrawPolyLine(pointsx.data(), pointsy.data(), points); // only if you want en extra outline
}
void VGSHelper::DrawOutlinedRect(int x, int y, int w, int h, Color& c)
{
	g_VGuiSurface->DrawSetColor(c);
	g_VGuiSurface->DrawOutlinedRect(x, y, x + w, y + h);
}
void VGSHelper::DrawPOutlinedRect(int x, int y, int w, int h, Color& c)
{
	g_VGuiSurface->DrawSetColor(c);
	g_VGuiSurface->DrawOutlinedRect(x, y, w, h);
}
void VGSHelper::GetTextSize(unsigned long font, const char* txt, int& width, int& height)
{
	size_t origsize = strlen(txt) + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t wcstring[newsize];
	int x, y;

	mbstowcs_s(&convertedChars, wcstring, origsize, txt, _TRUNCATE);

	g_VGuiSurface->GetTextSize(font, wcstring, x, y);

	width = x;
	height = y;
}
void VGSHelper::Draw3DCircle(Vector position, float radius, int seg, Color clr)
{
	Vector prev_scr_pos{ -1, -1, -1 };
	Vector scr_pos;

	float step = M_PI * 2.0 / seg;

	Vector origin = position;

	for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
	{
		Vector pos(radius * cos(rotation) + origin.x, radius * sin(rotation) + origin.y, origin.z + 2);
		Vector tracepos(origin.x, origin.y, origin.z + 2);

		Ray_t ray;
		trace_t trace;
		CTraceFilter filter;

		ray.Init(tracepos, pos);

		g_EngineTrace->TraceRay(ray, MASK_SPLITAREAPORTAL, &filter, &trace);

		if (Math::WorldToScreen(trace.endpos, scr_pos))
		{
			if (prev_scr_pos != Vector{ -1, -1, -1 })
			{
				g_VGuiSurface->DrawSetColor(clr);
				g_VGuiSurface->DrawLine(prev_scr_pos.x, prev_scr_pos.y, scr_pos.x, scr_pos.y);
			}
			prev_scr_pos = scr_pos;
		}
	}
}
ImVec2 VGSHelper::GetSize(std::string text, int size)
{
	if (!Inited)
		Init();

	wchar_t buf[256];
	int x, y;

	if (MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buf, 256))
	{
		g_VGuiSurface->GetTextSize(font, buf, x, y);
		return ImVec2(x, y);
	}

	return ImVec2(0, 0);
}
RECT Visuals::GetViewport()
{
	RECT viewport = { 0, 0, 0, 0 };
	int w, h;
	g_EngineClient->GetScreenSize(w, h);
	viewport.right = w; viewport.bottom = h;

	return viewport;
}
RECT VGSHelper::TextSize(const char* text)
{
	int apple = 0;
	char Buffer[2048] = { '\0' };

	size_t Size = strlen(text) + 1;
	wchar_t* WideBuffer = new wchar_t[Size];
	//mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);

	MultiByteToWideChar(CP_UTF8, MB_USEGLYPHCHARS, text, -1, WideBuffer, Size);
	int Width = 0, Height = 0;
	g_VGuiSurface->GetTextSize(eventfont, WideBuffer, Width, Height);
	RECT outcome = { 0, 0, Width, Height };
	delete[] WideBuffer;
	return outcome;
}