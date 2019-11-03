#pragma once
#include "..\valve_sdk\sdk.hpp"
#include "..\singleton.hpp"

class NightMode : public Singleton<NightMode>
{
public:
	void Apply(bool ForceUpdate, float r, float g, float b, float a);
	void Revert();
private:
	

	bool Active;
};

