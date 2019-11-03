
#include "../valve_sdk/csgostructs.hpp"

#pragma once
class Logger
{
public:
    void Debug(std::string pre, std::string msg, Color clr = Color(25, 250, 25));
    void Info(std::string pre, std::string msg);
	void GameEvent(std::string pre, std::string msg);
	void Backtrack(std::string pre, std::string msg);
    void Error(std::string pre, std::string msg);
    void Warning(std::string pre, std::string msg);
    void Damage(std::string pre, std::string msg);
    void Log(std::string pre, std::string msg);
    void Success(std::string pre, std::string msg);
    void Clear();
private:
    void AddLog(std::string pre, std::string msg, Color preclr = Color::Red, Color clr = Color(255, 255, 255));
};

extern Logger g_Logger;