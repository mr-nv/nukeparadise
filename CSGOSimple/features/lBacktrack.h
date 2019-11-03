#pragma once
#include "../options.hpp"
#include "../valve_sdk/csgostructs.hpp"
#include <map>
#include <deque>
#include "../valve_sdk/math/Vector.hpp"
#include "../valve_sdk/math/VMatrix.hpp"
#include "../valve_sdk/misc/CUserCmd.hpp"

struct backtrack_data {
	float simTime;
	Vector hitboxPos;
	matrix3x4_t boneMatrix[128];
};

class lBacktrack {
public:
	void OnMove(CUserCmd* pCmd);

	float correct_time = 0.0f;
	float latency = 0.0f;
	float lerp_time = 0.0f;
	std::map<int, std::deque<backtrack_data>> data;
};
extern lBacktrack g_lBacktrack;
