
#include "ConsoleHelper.h"
#include "helpers/utils.hpp"

void CH::Write(std::string text)
{
    Utils::ConsolePrint(text.data());
}

void CH::WriteLine(std::string text)
{
    Write(text + "\n");
}

void CH::WriteLine(int text)
{
    WriteLine(std::string(std::to_string(text) + "\n"));
}

void CH::WriteLine(float text)
{
    WriteLine(std::string(std::to_string(text) + "\n"));
}

void CH::WriteLine(Vector text)
{
    WriteLine(std::string("x: " + std::to_string(text.x) + "y: " + std::to_string(text.y) + "z: " + std::to_string(text.z) + "\n"));
}

void CH::WriteLine(QAngle text)
{
    WriteLine(std::string("pitch: " + std::to_string(text.pitch) + "yaw: " + std::to_string(text.yaw) + "roll: " + std::to_string(text.roll) + "\n"));
}


CH Con;