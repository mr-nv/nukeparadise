
#include "valve_sdk/csgostructs.hpp"

#pragma once

struct FakeLagData
{
	Vector pos = Vector(0, 0, 0);
	QAngle ang = QAngle(0, 0, 0);
};

struct MovotovInfoStruct
{
	Vector Position;
	float TimeToExpire;
};

struct SmokeInfoStruct
{
	Vector Position;
	float TimeToExpire;
};

struct ImpactInfoStruct
{
	Vector ShotPos = Vector(0, 0, 0);
	Vector Pos = Vector(0, 0, 0);
	float Time = 0.f;
	int Tick = 0;
};

struct BulletTracerInfoStruct
{
	bool DidHit = false;
	float Time = 0.f;
	Vector ShotPos = Vector(0, 0, 0);
	Vector HitPos = Vector(0, 0, 0);
};

struct HitmarkerInfoStruct
{
	float HitTime = 0.f;
	float Damage = 0.f;
};

struct RbotShotInfoStruct
{
	bool InLc = false;
	bool InLbyUpdate = false;
	bool Moving = false;
};

/*
 * player_old_flags
 */
struct player_old_flags
{
	int flags;
	float curtime;
};

/*
 * player_prediction_data
 */
struct player_prediction_data
{
	void reset()
	{
		prediction_stage = 0, tickbase = 0;
		random_seed = 0;
		in_prediction = false;
		curtime = 0.f, frametime = 0.f;
		prediction_random_seed = nullptr;
	}

	int prediction_stage = 0, tickbase = 0;
	unsigned random_seed = 0;
	bool in_prediction = false;
	float curtime = 0.f, frametime = 0.f;
	unsigned* prediction_random_seed = nullptr;
	player_old_flags non_predicted;
};

class RuntimeSaver
{
public:
	QAngle AARealAngle = QAngle(0, 0, 0);
	QAngle AAFakeAngle = QAngle(0, 0, 0);
	bool FakelagCurrentlyEnabled = false;
	FakeLagData FakelagData;
	float curtime = 0.f;

	//Lby update
	bool InLbyUpdate = false;
	float NextLbyUpdate = 0.f;

	//AnimationFixData
	bool HasChokedLast = false;
	QAngle LastRealAnglesBChoke = QAngle(0, 0, 0);

	//HitposHelper
	std::vector<ImpactInfoStruct> hitpositions;

	//BulletTracers
	std::vector<BulletTracerInfoStruct> BulletTracers;

	//OLD PLAYER DATA
	QAngle OLDviewangles;
	float OLDforwardmove;
	float OLDsidemove;
	int OLDflags;
	Vector OLDvelocity;
	bool OLDsendpacket;
	ClientFrameStage_t OLDstage;

	//Rbot
	bool RbotAADidShot = false;
	bool CurrentInLbyBreak = false;
	bool CurrentShouldSkipAnimations = false;
	float DesyncYaw = 0.f;
	float RealYaw = 0.f;
	float next_lby = 0.0f;
	bool broke_lby = false;

	float real_angle = 0.0f;
	float view_angle = 0.0f;

	//Fakewalk
	bool InFakewalk = false;
	int LastBacktrackTick = -1;
	bool LCbroken = false;
	HitmarkerInfoStruct HitmarkerInfo;
	player_prediction_data PredictionData;
	RbotShotInfoStruct RbotShotInfo;
	Vector LastShotEyePos = Vector(0, 0, 0);
	bool RbotDidLastShot = false;

	//Other

	int FakeLagValue;

	bool RequestForceUpdate = false;

	bool useless = false;

	bool LoadTexture = false;

	bool Colorama = false;

	int AntiKickMethod = 0;

	DWORD AATimer = 0;

	char Fakename[127] = "";

	char Name[127] = "";

	bool NameTrigger = false;

	int FakeNameMethod = 0;
};

extern RuntimeSaver g_Saver;