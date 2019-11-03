
#include "Utils.hpp"

#define NOMINMAX
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <vector>

#include "../valve_sdk/csgostructs.hpp"
#include "Math.hpp"


HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;



namespace Utils
{
	auto get_export(const char* module_name, const char* export_name) -> void*
	{
		HMODULE mod;
		while (!((mod = GetModuleHandleA(module_name))))
			Sleep(100);
		return reinterpret_cast<void*>(GetProcAddress(mod, export_name));
	}

	unsigned int FindInDataMap(datamap_t* pMap, const char* name)
	{
		while (pMap)
		{
			for (int i = 0; i < pMap->dataNumFields; i++)
			{
				if (pMap->dataDesc[i].fieldName == NULL)
				{
					continue;
				}

				if (strcmp(name, pMap->dataDesc[i].fieldName) == 0)
				{
					return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];
				}

				if (pMap->dataDesc[i].fieldType == FIELD_EMBEDDED)
				{
					if (pMap->dataDesc[i].td)
					{
						unsigned int offset;

						if ((offset = FindInDataMap(pMap->dataDesc[i].td, name)) != 0)
						{
							return offset;
						}
					}
				}
			}
			pMap = pMap->baseMap;
		}

		return 0;
	}

	void AttachConsole()
	{
		_old_out = GetStdHandle(STD_OUTPUT_HANDLE);
		_old_err = GetStdHandle(STD_ERROR_HANDLE);
		_old_in = GetStdHandle(STD_INPUT_HANDLE);

		::AllocConsole() && ::AttachConsole(GetCurrentProcessId());

		_out = GetStdHandle(STD_OUTPUT_HANDLE);
		_err = GetStdHandle(STD_ERROR_HANDLE);
		_in = GetStdHandle(STD_INPUT_HANDLE);

		SetConsoleMode(_out,
			ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

		SetConsoleMode(_in,
			ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
			ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
	}

	void DetachConsole()
	{
		if (_out && _err && _in)
		{
			FreeConsole();

			if (_old_out)
			{
				SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
			}
			if (_old_err)
			{
				SetStdHandle(STD_ERROR_HANDLE, _old_err);
			}
			if (_old_in)
			{
				SetStdHandle(STD_INPUT_HANDLE, _old_in);
			}
		}
	}

	bool ConsolePrint(const char* fmt, ...)
	{
		if (!_out)
		{
			return false;
		}

		char buf[1024];
		va_list va;

		va_start(va, fmt);
		_vsnprintf_s(buf, 1024, fmt, va);
		va_end(va);

		return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
	}

	char ConsoleReadKey()
	{
		if (!_in)
		{
			return false;
		}

		auto key = char{ 0 };
		auto keysread = DWORD{ 0 };

		ReadConsoleA(_in, &key, 1, &keysread, nullptr);

		return key;
	}

	inline RECT viewport() {
		RECT viewport = { 0, 0, 0, 0 };

		int width, height;
		g_EngineClient->GetScreenSize(width, height);
		viewport.right = width;
		viewport.bottom = height;

		return viewport;
	}

	bool WorldToScreen(const Vector& world, Vector& screen)
	{
		float matrix[4][4];
		auto find_point = [](Vector& point, int screen_w, int screen_h, int degrees) -> void {
			float x2 = screen_w * 0.5f;
			float y2 = screen_h * 0.5f;

			float d = sqrt(pow((point.x - x2), 2) + (pow((point.y - y2), 2))); //Distance
			float r = degrees / d; //Segment ratio

			point.x = r * point.x + (1 - r) * x2; //find point that divides the segment
			point.y = r * point.y + (1 - r) * y2; //into the ratio (1-r):r
		};
		float w = matrix[3][0] * world.x + matrix[3][1] * world.y + matrix[3][2] * world.z + matrix[3][3];
		auto
			screen_width = viewport().right,
			screen_height = viewport().bottom;

		float inverse_width = -1.0f / w;
		bool behind = true;

		if (w > 0.01) {
			inverse_width = 1.0f / w;
			behind = false;
		}

		screen.x = (float)((screen_width / 2) + (0.5 * ((matrix[0][0] * world.x
			+ matrix[0][1] * world.y
			+ matrix[0][2] * world.z
			+ matrix[0][3]) * inverse_width) * screen_width + 0.5));

		screen.y = (float)((screen_height / 2) - (0.5 * ((matrix[1][0] * world.x
			+ matrix[1][1] * world.y
			+ matrix[1][2] * world.z
			+ matrix[1][3]) * inverse_width) * screen_height + 0.5));

		if (screen.x > screen_width || screen.x < 0 || screen.y > screen_height || screen.y < 0 || behind) {
			find_point(screen, screen_width, screen_height, screen_height / 2);
			return false;
		}

		return !(behind);
		return true;

	}

	int WaitForModules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules)
	{
		bool signaled[32] = { 0 };
		bool success = false;

		std::uint32_t totalSlept = 0;

		if (timeout == 0)
		{
			for (auto& mod : modules)
			{
				if (GetModuleHandleW(std::data(mod)) == NULL)
				{
					return WAIT_TIMEOUT;
				}
			}
			return WAIT_OBJECT_0;
		}

		if (timeout < 0)
		{
			timeout = INT32_MAX;
		}

		while (true)
		{
			for (auto i = 0u; i < modules.size(); ++i)
			{
				auto& module = *(modules.begin() + i);
				if (!signaled[i] && GetModuleHandleW(std::data(module)) != NULL)
				{
					signaled[i] = true;

					//
					// Checks if all modules are signaled
					//
					bool done = true;
					for (auto j = 0u; j < modules.size(); ++j)
					{
						if (!signaled[j])
						{
							done = false;
							break;
						}
					}
					if (done)
					{
						success = true;
						goto exit;
					}
				}
			}
			if (totalSlept > std::uint32_t(timeout))
			{
				break;
			}
			Sleep(10);
			totalSlept += 10;
		}

	exit:
		return success ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
	}

	std::uint8_t* PatternScan(void* module, const char* signature)
	{
		static auto pattern_to_byte = [](const char* pattern)
		{
			auto bytes = std::vector<int>{};
			auto start = const_cast<char*>(pattern);
			auto end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current)
			{
				if (*current == '?')
				{
					++current;
					if (*current == '?')
					{
						++current;
					}
					bytes.push_back(-1);
				}
				else
				{
					bytes.push_back(strtoul(current, &current, 16));
				}
			}
			return bytes;
		};

		auto dosHeader = (PIMAGE_DOS_HEADER)module;
		auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

		auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
		auto patternBytes = pattern_to_byte(signature);
		auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

		auto s = patternBytes.size();
		auto d = patternBytes.data();

		for (auto i = 0ul; i < sizeOfImage - s; ++i)
		{
			bool found = true;
			for (auto j = 0ul; j < s; ++j)
			{
				if (scanBytes[i + j] != d[j] && d[j] != -1)
				{
					found = false;
					break;
				}
			}
			if (found)
			{
				return &scanBytes[i];
			}
		}
		return nullptr;
	}

	void SetClantag(const char* tag)
	{
		static auto fnClantagChanged = (int(__fastcall*)(const char*, const char*))PatternScan(GetModuleHandleW(L"engine.dll"), "53 56 57 8B DA 8B F9 FF 15");

		fnClantagChanged(tag, tag);
	}

	void SetName(const char* name)
	{
		static auto nameConvar = g_CVar->FindVar("name");
		nameConvar->m_fnChangeCallbacks.m_Size = 0;

		static auto do_once = (nameConvar->SetValue("\n\xAD\xAD\xAD­­­"), true); // old
		if (do_once) nameConvar->SetValue(name);
	}

	inline void clamp_angles(Vector& angles)
	{
		if (angles.x > 89.0f)
			angles.x = 89.0f;

		else
			if (angles.x < -89.0f)
				angles.x = -89.0f;

		if (angles.y > 180.0f)
			angles.y = 180.0f;

		else
			if (angles.y < -180.0f)
				angles.y = -180.0f;

		angles.z = 0;
	}

	float GetFov(Vector qAngles, Vector vecSource)
	{
		Vector delta = qAngles - vecSource;
		clamp_angles(delta);
		return sqrtf(powf(delta.x, 2.0f) + powf(delta.y, 2.0f));
	}

	void RankRevealAll()
	{
		g_CHLClient->DispatchUserMessage(50, 0, 0, nullptr);
	}
	//sensum
	std::string get_weapon_name(void* weapon)
	{
		static const auto V_UCS2ToUTF8 = static_cast<int(*)(const wchar_t* ucs2, char* utf8, int len)>(get_export("vstdlib.dll", "V_UCS2ToUTF8"));

		if (!weapon)
			return "";

		const auto wide_name = g_Localize->Find(((C_BaseCombatWeapon*)weapon)->GetCSWeaponData()->szHudName);

		char weapon_name[256];
		V_UCS2ToUTF8(wide_name, weapon_name, sizeof(weapon_name));

		return weapon_name;
	}
	float get_interpolation_compensation()
	{
		static const auto cl_interp = g_CVar->FindVar("cl_interp");
		static const auto max_ud_rate = g_CVar->FindVar("sv_maxupdaterate");
		static const auto cl_interp_ratio = g_CVar->FindVar("cl_interp_ratio");
		static const auto c_min_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
		static const auto c_max_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");

		float ratio = cl_interp_ratio->GetFloat();
		if (ratio == 0)
			ratio = 1.0f;

		if (c_min_ratio && c_max_ratio && c_min_ratio->GetFloat() != 1)
			ratio = std::clamp(ratio, c_min_ratio->GetFloat(), c_max_ratio->GetFloat());

		const auto ud_rate = max_ud_rate->GetInt();

		return std::max(cl_interp->GetFloat(), (ratio / ud_rate));
	}
	//heck
	const char* Format(const char* str, ...)
	{
		char format[4096];
		va_list list;
		va_start(list, str);
		vsnprintf(format, sizeof(format), str, list);
		va_end(list);
		return format;
	}

	float ServerTime(CUserCmd* cmd) {
		static int tick = 0;
		static CUserCmd* last_command;

		if (!cmd)
			return tick * g_GlobalVars->interval_per_tick;

		if (!last_command || last_command->hasbeenpredicted)
			tick = g_LocalPlayer->m_nTickBase();
		else
			tick++;

		last_command = cmd;
	}

	bool IsDangerZone()
	{
		static auto game_mode = g_CVar->FindVar("game_mode");
		static auto game_type = g_CVar->FindVar("game_type");

		return game_mode->GetInt() == 0 && game_type->GetInt() == 6;
	}

	int pWeaponType()
	{
		if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive()) { return 10; }
		auto weapon = g_LocalPlayer->m_hActiveWeapon();
		if (!weapon || !weapon->IsWeapon()) { return 10; }
		if (weapon->IsKnife() || weapon->IsGrenade() || weapon->IsPlantedC4() || weapon->IsDefuseKit()) return 10;
		if (weapon->Is_no_aim()) return 10;

		if (weapon->GetCSWeaponData()->WeaponType == WEAPONTYPE_PISTOL)
			return (int)RbotWeapons::PISTOL;

		else if (weapon->m_iItemDefinitionIndex() == WEAPON_SSG08)
			return (int)RbotWeapons::SCOUT;
		else if (weapon->m_iItemDefinitionIndex() == WEAPON_AWP)
			return (int)RbotWeapons::AWP;
		else if (weapon->m_iItemDefinitionIndex() == WEAPON_SCAR20 || weapon->m_iItemDefinitionIndex() == WEAPON_G3SG1)
			return (int)RbotWeapons::AUTO;

		else if (weapon->GetCSWeaponData()->WeaponType == WEAPONTYPE_RIFLE)
			return (int)RbotWeapons::RIFLE;

		else if (weapon->GetCSWeaponData()->WeaponType == WEAPONTYPE_SUBMACHINEGUN)
			return (int)RbotWeapons::SMG;

		else if (weapon->GetCSWeaponData()->WeaponType == WEAPONTYPE_SHOTGUN || weapon->GetCSWeaponData()->WeaponType == WEAPONTYPE_MACHINEGUN)
			return (int)RbotWeapons::HEAVY;

		return 10;
	}

}