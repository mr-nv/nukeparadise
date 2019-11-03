
#include "../singleton.hpp"
#include "../valve_sdk/csgostructs.hpp"

struct PointScanStruct
{
	Vector pos;
	bool center = false;
};

struct CanHitStruct
{
	float damage = 0.f;
	bool CanHit = false;
	Vector pos = Vector(0, 0, 0);
};

enum class BaimMode : int
{
	NONE,
	BAIM,
	FORCE_BAIM
};

struct RbotMatrixData
{
	matrix3x4_t matrix[MAXSTUDIOBONES];
	bool gotMatrix = false;
	studiohdr_t* StudioHdr;
	mstudiohitboxset_t* StudioSet;
};

#pragma once
class Rbot : public Singleton<Rbot>
{
public:
	bool Revolver(CUserCmd* cmd, C_BaseCombatWeapon* weapon);
	float get_MinDmg();
	float get_HitChina();
	int get_BaimMax();
	bool AimStep(QAngle& angle);
	void CreateMove(CUserCmd* cmd, bool& bSendPacket);
	void OnFireEvent(IGameEvent* event);
	bool GetBestHitboxPoint(C_BasePlayer* entity, float& damage, Vector& hitbox, BaimMode baim, bool& WillKill, matrix3x4_t matrix[MAXSTUDIOBONES] = nullptr, mstudiohitboxset_t* StudioSet = nullptr, bool NoPointscale = false);
	void PrecacheShit();
	bool HitChance(QAngle angles, C_BasePlayer* ent, float chance);
	void FakeDuck(CUserCmd* cmd, bool& bSendPackets);
	BaimMode* GetBAimStatus();
	void AccuracyBoost(CUserCmd* cmd);
	int GetTickbase(CUserCmd* ucmd);
private:
	bool InFakeLag(C_BasePlayer* player);
	float Simtimes[128];

	void AutoStop(CUserCmd* cmd, C_BasePlayer* target);
	void AutoCrouch(CUserCmd* cmd);


	BaimMode baim;
	CUserCmd* CurrentCmd = nullptr;

	void ZeusBot(CUserCmd* cmd, C_BaseCombatWeapon* weapon);

	int LastRbotEnemyIndex = -1;
	//bool LastMissedDueToSpread = false;
	//bool HitChance(QAngle angles, C_BasePlayer *ent, float chance, TickRecord record);
	int FindBestEntity(CUserCmd* cmd, C_BaseCombatWeapon* weapon, Vector& hitpos, bool& bBacktrack);
	bool IsAbleToShoot();
	std::vector<PointScanStruct> GetPointsForScan(C_BasePlayer* entity, int hitbox, mstudiohitboxset_t* hitset, matrix3x4_t matrix[MAXSTUDIOBONES], float pointscale);

	//bool PrecachePointers(C_BasePlayer * entity);
	//matrix3x4_t PrecachedMatrix[128];
	//studiohdr_t* PrecachedHdr;
	//mstudiohitboxset_t *PrecachedSet;

	void UpdateConfigData();
	int	activeweapon = 0;
	bool HitboxHead = false;
	bool HitboxNeck = false;
	bool HitboxChest = false;
	bool HitboxPelvis = false;
	bool HitboxStomach = false;
	bool HitboxArm = false;
	bool HitboxLeg = false;
	bool HitboxFoot = false;
	float MinDmg = FLT_MAX;
	float HitChina = FLT_MAX;
	float BaimMax = INT_MAX;
	float HitboxHeadScale = 0.f;
	float HitboxNeckScale = 0.f;
	float HitboxChestScale = 0.f;
	float HitboxPelvisScale = 0.f;
	float HitboxStomachScale = 0.f;
	float HitboxArmScale = 0.f;
	float HitboxLegScale = 0.f;
	float HitboxFootScale = 0.f;
};

extern RbotMatrixData matrixData[128];