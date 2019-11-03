#pragma once
#include <string>
#include <vector>

#include "../singleton.hpp"
#include "../valve_sdk\csgostructs.hpp"
#include "../features/visuals.hpp"
#include "../options.hpp"
#include "Resolver.h"
#include "HitPossitionHelper.h"
#include "../config.hpp"
#include "../RuntimeSaver.h"
#include "Rbot.h"
#include <Windows.h>
#include "../Sounds.h"
#include "Logger.h"
namespace Features
{
	struct EventLog_t
	{
		float time;
		int yOffset, alpha;
		std::string text;
		RECT rekt;
		Color col;
	};

	class CEventLogger
	{
		public:
			void AddLog(const char* prefix, const char* text, Color prefixcolor, Color textcolor);

			void AddPrefixLog(Color prefixcolor, const char* str, ... );
			void AddMainLog(RECT rekt, Color textcolor, const char* str, ...);

			void DrawPrefix();
			void DrawMain();
		private:
			std::vector<EventLog_t> prefix_logs;
			std::vector<EventLog_t> main_logs;
	};

	extern CEventLogger* EventLogger;
}