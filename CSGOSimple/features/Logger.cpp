
#include "Logger.h"
#include "../options.hpp"
#include "EventLogger.h"

void Logger::Debug(std::string pre, std::string msg, Color clr)
{
	g_CVar->ConsoleColorPrintf(clr, "[");
	g_CVar->ConsoleColorPrintf(clr, pre.data());
	g_CVar->ConsoleColorPrintf(clr, "] ");
	g_CVar->ConsoleColorPrintf(Color::White, msg.data());
	g_CVar->ConsoleDPrintf("\n");
}

void Logger::Info(std::string pre, std::string msg)
{
    AddLog(pre, msg, Color(0, 185, 255));
}

void Logger::Backtrack(std::string pre, std::string msg)
{
	AddLog(pre, msg, Color(0, 255, 0, 255));
}

void Logger::GameEvent(std::string pre, std::string msg)
{
	AddLog(pre, msg, Color(255, 200, 0, 255));
}

void Logger::Error(std::string pre, std::string msg)
{
    AddLog(pre, msg, Color(255, 55, 0));
}

void Logger::Warning(std::string pre, std::string msg)
{
    AddLog(pre, msg, Color(255, 180, 0));
}

void Logger::Damage(std::string pre, std::string msg)
{
    AddLog(pre, msg, Color(255, 100, 0));
}

void Logger::Log(std::string pre, std::string msg)
{
    AddLog(pre, msg, Color(200, 200, 200));
}

void Logger::Success(std::string pre, std::string msg)
{
    AddLog(pre, msg, Color(82, 255, 66));
}

void Logger::Clear()
{
    g_EngineClient->ExecuteClientCmd("clear");
}

void Logger::AddLog(std::string pre, std::string msg, Color preclr, Color clr)
{
    if (!g_EngineClient || !g_CVar || !g_Config.esp_misc_event_logger)
    {
        return;
    }

	g_CVar->ConsoleColorPrintf(preclr, "[");
    g_CVar->ConsoleColorPrintf(preclr, pre.data());
	g_CVar->ConsoleColorPrintf(preclr, "] ");
    g_CVar->ConsoleColorPrintf(clr, msg.data());
    g_CVar->ConsoleDPrintf("\n");

	std::string logpre ("[" + pre + "] ");
	
	Features::EventLogger->AddLog(logpre.c_str(), msg.c_str(), preclr, clr);
}

Logger g_Logger;