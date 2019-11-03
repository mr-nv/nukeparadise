#include "menu_helpers.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "droid.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/directx9/imgui_impl_dx9.h"

#include "Menu.h"
#include "features/visuals.hpp"
#include "RuntimeSaver.h"

static ConVar* cl_mouseenable = nullptr;

constexpr static float get_sidebar_item_width()
{
	return 150.0f;
}
constexpr static float get_sidebar_item_height()
{
	return  50.0f;
}

namespace ImGuiEx
{
	inline bool ColorEdit4(const char* label, Color* v, ImGuiColorEditFlags flags = 0)
	{
		auto clr = ImVec4
		{
			v->r() / 255.0f,
			v->g() / 255.0f,
			v->b() / 255.0f,
			v->a() / 255.0f
		};

		if (ImGui::ColorEdit4(label, &clr.x, flags))
		{
			v->SetColor(clr.x, clr.y, clr.z, clr.w);
			return true;
		}
		return false;
	}
	inline bool ColorEdit3(const char* label, Color* v)
	{
		return ColorEdit4(label, v, false);
	}
}

int MenuHelper::get_fps()
{
	using namespace std::chrono;
	static int count = 0;
	static auto last = high_resolution_clock::now();
	auto now = high_resolution_clock::now();
	static int fps = 0;

	count++;

	if (duration_cast<milliseconds>(now - last).count() > 1000)
	{
		fps = count;
		count = 0;
		last = now;
	}

	return fps;
}

void MenuHelper::Initialize()
{
	MenuHelper::Get()._visible = true;

	cl_mouseenable = g_CVar->FindVar("cl_mouseenable");

	Menu::Get().Initialize();
}

void MenuHelper::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
    cl_mouseenable->SetValue(true);
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void MenuHelper::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void MenuHelper::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
	VGSHelper::Get().Init();
}

static ImColor GetRainbowColor()
{
	static float rainbow;
	rainbow += 0.00005f;
	if (rainbow > 1.f)
		rainbow = 0.f;

	return ImColor::HSV(rainbow, 1.f, 1.f, 0.2f);
}

void MenuHelper::Render(IDirect3DDevice9* pDevice)
{
	if (!g_Saver.LoadTexture) {
		Menu::Get().TextureInit(pDevice);
		return;
	}

    ImGui::GetIO().MouseDrawCursor = MenuHelper::Get()._visible;

    Menu::Get().Render();
}

void MenuHelper::Show()
{
    MenuHelper::Get()._visible = true;
    cl_mouseenable->SetValue(false);
}

void MenuHelper::Hide()
{
    MenuHelper::Get()._visible = false;
    cl_mouseenable->SetValue(true);
}

void MenuHelper::Toggle()
{
    cl_mouseenable->SetValue(MenuHelper::Get()._visible);
    MenuHelper::Get()._visible = !MenuHelper::Get()._visible;
}

void MenuHelper::CreateStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowPadding = ImVec2(8, 8); 
	style.WindowMinSize = ImVec2(32, 32);
	style.WindowRounding = 0.0f;
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.FramePadding = ImVec2(4, 3);
	style.FrameRounding = 0.0f;
	style.FrameBorderSize = 1.0f;
	style.ItemSpacing = ImVec2(4, 4);    
	style.ItemInnerSpacing = ImVec2(4, 4);    
	style.TouchExtraPadding = ImVec2(0, 0);      
	style.IndentSpacing = 21.0f;            
	style.ColumnsMinSpacing = 6.0f;             
	style.ScrollbarSize = 8.0f;            
	style.ScrollbarRounding = 9.0f;           
	style.GrabMinSize = 10.0f;          
	style.GrabRounding = 0.4f;             
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.DisplayWindowPadding = ImVec2(22, 22);
	style.DisplaySafeAreaPadding = ImVec2(4, 4); 
	style.CurveTessellationTol = 1.25f;      
	style.AntiAliasedLines = true;         
	style.AntiAliasedFill = true;        
	style.DisplaySafeAreaPadding = ImVec2(4, 4);  
	style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.30f, 0.30f, 0.30f, 0.61f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.06f, 0.06f, 0.06f, 0.01f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.30f, 0.30f, 0.30f, 0.71f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.67f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.1f, 0.1f, 0.1f, 0.6f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.15f, 0.15f, 0.15f, 0.65f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.15f, 0.2f, 0.2f, 0.7f);
	style.Colors[ImGuiCol_TitleBg] = GetRainbowColor();
	style.Colors[ImGuiCol_TitleBgActive] = GetRainbowColor();
	style.Colors[ImGuiCol_TitleBgCollapsed] = GetRainbowColor();
	style.Colors[ImGuiCol_MenuBarBg] =				ImVec4(0.10f, 0.10f, 0.10f, 0.66f);
	style.Colors[ImGuiCol_ScrollbarBg] =			ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] =			ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] =	ImVec4(0.17f, 0.17f, 0.17f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] =	ImVec4(0.37f, 0.00f, 0.12f, 1.00f);	 
	style.Colors[ImGuiCol_CheckMark] =				ImVec4(0.60f, 0.05f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] =				ImVec4(0.29f, 0.29f, 0.3f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] =		ImVec4(1.00f, 0.00f, 0.27f, 1.00f);
	style.Colors[ImGuiCol_Button] =					ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] =			ImVec4(0.6f, 0.1f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] =			ImVec4(1.0f, 0.1f, 0.3f, 1.0f);
	style.Colors[ImGuiCol_Header] =					ImVec4(1.0f, 0.0f, 0.3f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] =			ImVec4(1.0f, 0.0f, 0.3f, 0.8f);
	style.Colors[ImGuiCol_HeaderActive] =			ImVec4(1.0f, 0.0f, 0.3f, 1.0f);
	style.Colors[ImGuiCol_Separator] =				ImVec4(0.1f, 0.1f, 0.1f, 0.90f);
	style.Colors[ImGuiCol_SeparatorHovered] =		ImVec4(1.0f, 0.0f, 0.30f, 0.70f);
	style.Colors[ImGuiCol_SeparatorActive] =		ImVec4(1.0f, 0.0f, 0.40f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] =				ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	style.Colors[ImGuiCol_ResizeGripHovered] =		ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] =		ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_PlotLines] =				ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] =		ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] =			ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] =	ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] =			ImVec4(1.00f, 0.00f, 0.30f, 0.35f);
	style.Colors[ImGuiCol_ModalWindowDarkening] =	ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	ImGui::SetColorEditOptions(ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoTooltip |ImGuiColorEditFlags__InputsMask | ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_AlphaBar);
}
