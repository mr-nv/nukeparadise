#pragma once

#include <string>
#include "singleton.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
struct IDirect3DDevice9;

class MenuHelper : public Singleton<MenuHelper>
{
public:
	void Initialize();
	void Shutdown();

	void OnDeviceLost();
	void OnDeviceReset();

	void Render(IDirect3DDevice9* pDevice);

	void Show();
	void Hide();
	void Toggle();

	bool IsVisible() const
	{
		return _visible;
	}
	int get_fps();

	/* menu components */
	class components
	{
	public:
		bool StartWindow(std::string title, ImVec2 size, ImGuiWindowFlags flags);
		void EndWindow();
		void NavbarIcons(char* items[], char* hovered_items[], int size, int& selected, ImFont* font);
		void Navbar(char* items[], int size, int& selected);
		void HelpMarker(const char* text);
		void Checkbox(std::string text, std::string settings_name);
		void ColorCheckbox(std::string text, std::string settings_name, std::string settings_name_color);
		void ColorCheckbox2(std::string text, std::string settings_name, std::string settings_name_color, std::string settings_name_color2);
		void Label(std::string text);
		void ComboBox(std::string text, const char* items[], int size, std::string settings_name);
		void ComboCheckBox(std::string text, std::string items[], std::string setting_names[], int size);
		void BeginChild(std::string id, ImVec2 size, bool border = true, ImGuiWindowFlags flags = 0);
		void EndChild();
		void Columns(int count, bool border, char* id = NULL);
		void NextColumn();
		void SameLine();
		void SliderInt(std::string text, std::string settings_name, int min, int max);
		void SliderFloat(std::string text, std::string settings_name, float min, float max);
		void Hotkey(std::string text, std::string settings_name);
		bool Button(std::string text);
		void Spacing();

	private:
		bool InColumns = false;
		int ColumnSize = 0;
		int ColumnCurrent = 0;
		bool FrameStarted = false;
		ImGuiStyle* style;
		float x = 0.f;
		float y = 0.f;
		int CurrentItem = 0;
		bool NavbarHovered[128][128];
	};

	void CreateStyle();
	ImGuiStyle        _style;
	bool              _visible;
};