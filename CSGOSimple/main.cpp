//#define CONSOLE
#define NOMINMAX
#include <Windows.h>

#include "valve_sdk/sdk.hpp"
#include "helpers/utils.hpp"
#include "helpers/input.hpp"

#include "hooks.hpp"
#include "menu_helpers.hpp"
#include "config.hpp"
#include "render.hpp"
#include "features/EventHelper.h"
#include "ConsoleHelper.h"
#include "features/Logger.h"
#include "Sounds.h"
#include <filesystem>

namespace fs = std::filesystem;

bool bassinit()
{
	BASS::bass_lib_handle = BASS::bass_lib.LoadFromMemory(bass_dll_image, sizeof(bass_dll_image));
	static bool once;
	if (!once)
	{
		if (BASS_Init(-1, 44100, 0, nullptr, nullptr))
		{
			BASS_SetConfig(BASS_CONFIG_NET_PLAYLIST, 1);
			BASS_SetConfig(BASS_CONFIG_NET_PREBUF, 0);
			BASS::stream_sounds.cod = BASS_StreamCreateFile(TRUE, Sounds::cod, 0, sizeof(Sounds::cod), 0);
			BASS::stream_sounds.skeet = BASS_StreamCreateFile(TRUE, Sounds::skeet, 0, sizeof(Sounds::skeet), 0);
			BASS::stream_sounds.punch = BASS_StreamCreateFile(TRUE, Sounds::punch, 0, sizeof(Sounds::punch), 0);
			BASS::stream_sounds.metal = BASS_StreamCreateFile(TRUE, Sounds::metal, 0, sizeof(Sounds::metal), 0);
			BASS::stream_sounds.boom = BASS_StreamCreateFile(TRUE, Sounds::boom, 0, sizeof(Sounds::boom), 0);
			once = true;
		}
	}
	return true;
}

DWORD WINAPI OnDllAttach(LPVOID base)
{
	if (Utils::WaitForModules(10000, { L"client_panorama.dll", L"engine.dll", L"shaderapidx9.dll" }) == WAIT_TIMEOUT) { return FALSE; }
#ifdef CONSOLE
	Utils::AttachConsole();
#endif // CONSOLE
	try
	{
		Config->Setup();
		Interfaces::Initialize();
		Utils::ConsolePrint("interfaces loaded!\n");
		if (bassinit()) Utils::ConsolePrint("hitmarkers loaded!\n");
		NetvarSys::Get().Initialize();
		InputSys::Get().Initialize();
		Render::Get().Initialize();
		MenuHelper::Get().Initialize();
		Hooks::Initialize();
		//InputSys::Get().RegisterHotkey(VK_END, [base]() { g_Unload = true; });
		InputSys::Get().RegisterHotkey(VK_INSERT, [base]() { MenuHelper::Get().Toggle(); });
		EventHelper::Get().init();
		Utils::ConsolePrint("finished!\n");
		while (!g_Unload) Sleep(300);

		g_CVar->FindVar("crosshair")->SetValue(true);
		EventHelper::Get().unInit();
		Sleep(100);
		g_Logger.Debug("ENGINE", "Unloaded.", Color(255, 50, 25));
		FreeLibraryAndExitThread(static_cast<HMODULE> (base), 1);
	}
	catch (const std::exception& ex)
	{
#ifdef CONSOLE
		Utils::ConsolePrint(ex.what());
		Utils::ConsolePrint("An error occured during initialization:\n");
		Utils::ConsolePrint("%s\n", ex.what());
		Utils::ConsolePrint("Press any key to exit.\n");
		Utils::ConsoleReadKey();
		Utils::DetachConsole();
#endif // CONSOLE
		FreeLibraryAndExitThread(static_cast<HMODULE> (base), 1);
	}

	// unreachable
	//return TRUE;
	}

BOOL WINAPI OnDllDetach()
{
#ifdef CONSOLE
	Utils::DetachConsole();
#endif // CONSOLE
	Hooks::Shutdown();

	MenuHelper::Get().Shutdown();
	return TRUE;
}

BOOL WINAPI DllMain(
	_In_      HINSTANCE hinstDll,
	_In_      DWORD     fdwReason,
	_In_opt_  LPVOID    lpvReserved
)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hinstDll);
		CreateThread(nullptr, 0, OnDllAttach, hinstDll, 0, nullptr);
		return TRUE;

	case DLL_PROCESS_DETACH:
		if (lpvReserved == nullptr)
			return OnDllDetach();

		return TRUE;

	default:
		return TRUE;
	}
}
