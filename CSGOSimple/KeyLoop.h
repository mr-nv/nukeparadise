
#include "singleton.hpp"
#include "valve_sdk/csgostructs.hpp"

#pragma once
class KeyLoop : public Singleton<KeyLoop>
{
public:
    void OnCreateMove();
};

