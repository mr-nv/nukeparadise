#pragma once

#include "../singleton.hpp"

#include "../render.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"


#define FLOW_OUTGOING 0
#define FLOW_INCOMING 1
#define MAX_FLOWS 2

enum class DrawSideModes : int
{
	TOP,
	RIGHT,
	BOTTOM,
	LEFT
};

struct PosAdjustmentStruct
{
	int top = 4;
	int right = 4;
	int bottom = 4;
	int left = 4;
};

class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_armored; //beta
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
			int			  boxmode = 0;
			DrawSideModes healthpos = DrawSideModes::LEFT;
			DrawSideModes armourpos = DrawSideModes::RIGHT;
			DrawSideModes lbyupdatepos = DrawSideModes::BOTTOM;
			PosAdjustmentStruct PosHelper;
			Color BoxClr = Color(0, 0, 0);
			Color BoxFill = Color(0, 0, 0);
			Color NameClr = Color(0, 0, 0);
			Color ArmourClr = Color(0, 0, 0);
			Color WeaponClr = Color(0, 0, 0);
			Color SnaplineClr = Color(0, 0, 0);
			Color PovClr = Color(0, 0, 0);
			Color FlagClr = Color(0, 0, 0);

			Color EDHeadClr = Color(0, 0, 0);

			Color LCLineClr = Color(0, 0, 0);

			Color LineViewClr = Color(0, 0, 0);
			Color LineLbyClr = Color(0, 0, 0);

			Color InfoClr = Color(0, 0, 0);
			Color LbyTimerClr = Color::Blue;
			bool ShouldDrawBox = true;
		} ctx;

		bool Begin(C_BasePlayer* pl);
		void RenderBox();
		void RenderBoxOutline();
		void RenderName();
		void Render3DHead();
		void DrawArrow();
		void DrawhealthIcon(int x, int y);
		void RenderWeaponName();
		void RenderWeaponAmmo();
		void DrawFlags();
		void RenderFill();
		void RenderHealth();
		void DrawAngleLines();
		void RenderArmour();
		void RenderSkelet();
		void RenderLbyUpdateBar();
		void RenderBacktrackedSkelet();
		void RenderSnapline();

		void DrawPlayerDebugInfo();

		void RenderLine(DrawSideModes mode, Color color, float percent);

		void RenderResolverInfo();
		float TextHeight = 12.f;
	};
	void RenderWeapon(C_BaseCombatWeapon* ent, std::string weapon);
	void RenderC4(C_BaseEntity* ent);
	//void RenderBullettracers();
	void DrawGrenade(C_BaseEntity* ent);
	void DrawDangerzoneItem(C_BaseEntity* ent, float maxRange);


	void RenderSpectatorList();

	/* Local indicators */
	void LbyIndicator();
	void PingIndicator();
	void LCIndicator();

	void AutowallCrosshair();

	/* aa indicator */
	void ManualAAIndicator();

	/* local misc */

	void SpreadCircle();

	void RenderNoScoopeOverlay();

	float CurrentIndicatorHeight = 0.f;

	void RenderHitmarker();

	void RunHitmarker();

	int hitmarkerAlpha = 0;

	RECT GetViewport();

	bool IsVisibleScan(C_BasePlayer* player);
public:
	void AddToDrawList();
	void Render();
};

class VGSHelper : public Singleton<VGSHelper>
{
public:
	void Init();
	void DrawText(std::string text, float x, float y, Color color, int size = 15);

	void DrawSpecText(std::string text, float x, float y, Color color, int size = 15);
	void DrawNameText(std::string name, float x, float y, float z, Color color);
	void DrawWeaponText(std::string weapon, float x, float y, float z, Color color);
	void DrawWeaponEntityText(std::string entity, float x, float y, float z, Color color);
	void DrawGrenadeText(std::string nade, float x, float y, float z, float i, Color color);
	void DrawPlantedText(float plant_time, float x, float y, float w);
	void DrawDefusedText(float defuse_time, float x, float y, float w);
	void DrawC4Text(std::string text, float x, float y, Color color);
	void DrawEventText(std::string text, float x, float y, Color color, int size);
	void DrawAAText(const wchar_t* text, float x, float y, Color color);
	void DrawLine(float x1, float y1, float x2, float y2, Color color, float size = 1.f);
	void DrawBox(float x1, float y1, float x2, float y2, Color clr, float size = 1.f);
	void DrawFilledBox(float x1, float y1, float x2, float y2, Color clr);
	void DrawTriangle(int count, Vertex_t* vertexes, Color c);
	void DrawBoxEdges(float x1, float y1, float x2, float y2, Color clr, float edge_size, float size = 1.f);
	void DrawCircle(float x, float y, float r, int seg, Color clr);

	void Polygon(int count, Vertex_t* Vertexs, Color color);
	void DrawFilledTriangle(std::array<Vector2D, 3> points, Color color);
	void PolygonOutline(int count, Vertex_t* Vertexs, Color color, Color colorLine);
	void PolyLine(int count, Vertex_t* Vertexs, Color colorLine);
	void PolyLine(int* x, int* y, int count, Color color);
	void Draw3DCube(float scalar, QAngle angles, Vector middle_origin, Color outline);
	void FillRGBA(int x, int y, int w, int h, Color color);
	void BorderBox(int x, int y, int w, int h, Color color, int thickness);
	__inline void VGSHelper::DrawFilledRect(int x, int y, int w, int h);
	void DrawRectOutlined(int x, int y, int w, int h, Color color, Color outlinedColor, int thickness);
	void DrawString(unsigned long font, int x, int y, Color color, unsigned long alignment, const char* msg, ...);
	void DrawString(unsigned long font, bool center, int x, int y, Color c, const char* fmt, ...);

	void TextW(bool center, unsigned long font, int x, int y, Color c, wchar_t* pszString);

	void DrawCircle(int x, int y, float r, int step, Color color);

	void DrawFilledCircle(float x, float y, float r, int seg, Color clr);

	void DrawOutlinedRect(int x, int y, int w, int h, Color& c);
	void DrawPOutlinedRect(int x, int y, int w, int h, Color& c);
	void Draw3DCircle(Vector position, float radius, int seg, Color clr);
	void GetTextSize(unsigned long font, const char* txt, int& width, int& height);

	ImVec2 GetSize(std::string text, int size = 15);
	RECT TextSize(const char* text);
private:
	bool Inited = false;
	vgui::HFont font;
	vgui::HFont namefont;
	vgui::HFont eventfont;
	vgui::HFont c4font;
	vgui::HFont aafont;
	vgui::HFont spectatorlist_font;
};