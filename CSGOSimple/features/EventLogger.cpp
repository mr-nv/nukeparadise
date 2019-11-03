// tekhnologii

#include "EventLogger.h"

#include <algorithm>
#include <cstdarg>

using namespace Features;

CEventLogger* Features::EventLogger = new CEventLogger();

void CEventLogger::AddLog(const char* prefix, const char* text, Color prefixcolor, Color textcolor)
{
	RECT prefixsize = VGSHelper::Get().TextSize(prefix);

	EventLogger->AddPrefixLog(prefixcolor, prefix);
	EventLogger->AddMainLog(prefixsize, textcolor, text);
}

void CEventLogger::AddPrefixLog(Color prefixcolor, const char* str, ...)
{
	if (!str)
		return;

	RECT outcome = { 0, 0, 0, 0 };

	if (prefix_logs.size() > 7)
		prefix_logs.erase(prefix_logs.begin());

	va_list va_args;
	char buffer[2048] = { 0 };
	va_start(va_args, str);
	_vsnprintf(buffer, sizeof(buffer), str, va_args);
	va_end(va_args);
	prefix_logs.push_back({ g_GlobalVars->realtime, -15, 0, std::string(buffer), outcome, prefixcolor });
}

void CEventLogger::DrawPrefix()
{
	for (int i = 0; i < prefix_logs.size(); i++)
	{
		auto& l = prefix_logs.at(i);

		if (l.alpha == 0 && g_GlobalVars->realtime - l.time >= 5)
			prefix_logs.erase(prefix_logs.begin() + i);

		if (l.yOffset < 0 && l.alpha < 255 && g_GlobalVars->realtime - l.time < 5)
		{
			l.yOffset += 1;
			l.alpha += 17;
			continue;
		}

		if (l.yOffset <= 0 && l.alpha <= 255 && l.alpha > 0 && g_GlobalVars->realtime - l.time >= 5)
		{
			l.yOffset -= 1;
			l.alpha -= 17;
			continue;
		}
	}

	auto offset = 0;
	auto textSize = 13;

	for (auto& l : prefix_logs)
	{
		VGSHelper::Get().DrawEventText(l.text, 5, 5 + offset + l.yOffset, l.col, 15);
		offset += textSize + l.yOffset;
	}
}

void CEventLogger::AddMainLog(RECT rekt, Color textcolor, const char* str, ...)
{
	if (!str)
		return;

	if (main_logs.size() > 7)
		main_logs.erase(main_logs.begin());

	va_list va_args;
	char buffer[2048] = { 0 };
	va_start(va_args, str);
	_vsnprintf(buffer, sizeof(buffer), str, va_args);
	va_end(va_args);
	main_logs.push_back({ g_GlobalVars->realtime, -15, 0, std::string(buffer), rekt, textcolor });
}

void CEventLogger::DrawMain()
{
	for (int i = 0; i < main_logs.size(); i++)
	{
		auto& l = main_logs.at(i);

		if (l.alpha == 0 && g_GlobalVars->realtime - l.time >= 5)
			main_logs.erase(main_logs.begin() + i);

		if (l.yOffset < 0 && l.alpha < 255 && g_GlobalVars->realtime - l.time < 5)
		{
			l.yOffset += 1;
			l.alpha += 17;
			continue;
		}

		if (l.yOffset <= 0 && l.alpha <= 255 && l.alpha > 0 && g_GlobalVars->realtime - l.time >= 5)
		{
			l.yOffset -= 1;
			l.alpha -= 17;
			continue;
		}
	}

	auto offset = 0;
	auto textSize = 13;

	for (auto& l : main_logs)
	{
		VGSHelper::Get().DrawEventText(l.text.c_str(), 5 + l.rekt.right, 5 + offset + l.yOffset, l.col, 15);
		offset += textSize + l.yOffset;
	}
}

//VGSHelper::Get().DrawEventText("PRIVET PRIKOLER !!! ETO TEST, 0123456789 - [nuke] - [PARADISE]", 5, 5 + offset + l.yOffset, Color::Red, 15);