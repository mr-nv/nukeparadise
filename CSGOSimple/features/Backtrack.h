
#include "../singleton.hpp"
#include "../valve_sdk\csgostructs.hpp"
#include "Resolver.h"
#include "Rbot.h"

#pragma once

struct OriginalRecord
{
    OriginalRecord() {}

    OriginalRecord(C_BasePlayer* player)
    {
        m_vecOrigin = player->m_vecOrigin();
        m_angEyeAngles = player->m_angEyeAngles();
        m_fFlags = player->m_fFlags();
        m_flPoseParameter = player->m_flPoseParameter();
    }

    Vector m_vecOrigin = Vector(0, 0, 0);
    QAngle m_angEyeAngles = QAngle(0, 0, 0);
    int m_fFlags = 0;
    std::array<float, 24> m_flPoseParameter;
};

struct TickRecord
{
    TickRecord() {}

    TickRecord(C_BasePlayer* player, GlobalResolverDataStruct rdata)
    {
        m_ArmorValue = player->m_ArmorValue();
        m_bHasHeavyArmor = player->m_bHasHeavyArmor();
        m_bHasHelmet = player->m_bHasHelmet();
        m_flLowerBodyYawTarget = player->m_flLowerBodyYawTarget();
        m_iHealth = player->m_iHealth();
        m_fFlags = player->m_fFlags();
        m_vecVelocity = player->m_vecVelocity();
        m_flSimulationTime = player->m_flSimulationTime();
        m_flCycle = player->m_flCycle();
        m_nSequence = player->m_nSequence();
        m_angAbsOrigin = player->m_angAbsOrigin();
        m_flPoseParameter = player->m_flPoseParameter();
        matrixBuilt = player->SetupBones(matrix, 128, 256, m_flSimulationTime);
        resolverState = rdata.ResolverState;
        resolved = rdata.Resolved;
        resolverData = rdata;
        m_angEyeAngles = player->m_angEyeAngles();
        StudioHdr = matrixData[player->EntIndex()].StudioHdr;
        StudioSet = matrixData[player->EntIndex()].StudioSet;
        CanUse = true;
    }

    explicit operator bool() const noexcept
    {
        return m_flSimulationTime > 0.f;
    }

    bool operator>(const TickRecord& others)
    {
        return (m_flSimulationTime > others.m_flSimulationTime);
    }

    bool operator>=(const TickRecord& others)
    {
        return (m_flSimulationTime >= others.m_flSimulationTime);
    }

    bool operator<(const TickRecord& others)
    {
        return (m_flSimulationTime < others.m_flSimulationTime);
    }

    bool operator<=(const TickRecord& others)
    {
        return (m_flSimulationTime <= others.m_flSimulationTime);
    }

    bool operator==(const TickRecord& others)
    {
        return (m_flSimulationTime == others.m_flSimulationTime);
    }

    int m_ArmorValue = 0;
    bool m_bHasHeavyArmor = false;
    bool m_bHasHelmet = false;
    float m_flLowerBodyYawTarget = 0.f;
    int32_t m_iHealth = 0;
    int32_t m_fFlags = 0;
    Vector m_vecVelocity = Vector(0, 0, 0);
    float m_flSimulationTime = 0.f;
    float m_flCycle = 0.f;
    int m_nSequence = 0;
    Vector m_angAbsOrigin = Vector(0, 0, 0);
    std::array<float, 24> m_flPoseParameter;
    Vector m_vecOrigin = Vector(0, 0, 0);
    QAngle m_angEyeAngles = QAngle(0, 0, 0);
    matrix3x4_t	matrix[MAXSTUDIOBONES];
    bool matrixBuilt = false;
    int resolverState = 0;
    bool resolved = false;
    GlobalResolverDataStruct resolverData;
    studiohdr_t* StudioHdr;
    mstudiohitboxset_t* StudioSet;
    bool CanUse = false;
};

struct LegitTickRecord
{
    LegitTickRecord() {}

    LegitTickRecord(C_BasePlayer* player, GlobalResolverDataStruct rdata, std::deque<int> hitboxes_enabled)
    {
        player->PrecaceOptimizedHitboxes();

        m_ArmorValue = player->m_ArmorValue();
        m_bHasHeavyArmor = player->m_bHasHeavyArmor();
        m_bHasHelmet = player->m_bHasHelmet();
        m_fFlags = player->m_fFlags();
        m_vecVelocity = player->m_vecVelocity();
        m_flSimulationTime = player->m_flSimulationTime();
        matrixBuilt = player->SetupBones(matrix, 128, 256, m_flSimulationTime);
        resolverData = rdata;
        m_angEyeAngles = player->m_angEyeAngles();
        StudioHdr = matrixData[player->EntIndex()].StudioHdr;
        StudioSet = matrixData[player->EntIndex()].StudioSet;
        CanUse = true;
        Moving = player->m_vecVelocity().Length2D() > 0.1f;

        for (auto hb = hitboxes_enabled.begin(); hb != hitboxes_enabled.end(); hb++)
        {
            player->GetOptimizedHitboxPos(*hb, hitboxes[*hb]);
        }

        //for (int hitbox = 0; hitbox < HITBOX_MAX; hitbox++)
        //{
        //	player->GetOptimizedHitboxPos(hitbox, hitboxes[hitbox]);
        //}
    }

    explicit operator bool() const noexcept
    {
        return m_flSimulationTime > 0.f;
    }

    bool operator>(const LegitTickRecord& others)
    {
        return (m_flSimulationTime > others.m_flSimulationTime);
    }

    bool operator>=(const LegitTickRecord& others)
    {
        return (m_flSimulationTime >= others.m_flSimulationTime);
    }

    bool operator<(const LegitTickRecord& others)
    {
        return (m_flSimulationTime < others.m_flSimulationTime);
    }

    bool operator<=(const LegitTickRecord& others)
    {
        return (m_flSimulationTime <= others.m_flSimulationTime);
    }

    bool operator==(const LegitTickRecord& others)
    {
        return (m_flSimulationTime == others.m_flSimulationTime);
    }

    int m_ArmorValue = 0;
    bool m_bHasHeavyArmor = false;
    bool m_bHasHelmet = false;
    int32_t m_fFlags = 0;
    Vector m_vecVelocity = Vector(0, 0, 0);
    float m_flSimulationTime = 0.f;
    Vector m_vecOrigin = Vector(0, 0, 0);
    QAngle m_angEyeAngles = QAngle(0, 0, 0);
    matrix3x4_t	matrix[MAXSTUDIOBONES];
    bool matrixBuilt = false;
    GlobalResolverDataStruct resolverData;
    studiohdr_t* StudioHdr;
    mstudiohitboxset_t* StudioSet;
    bool CanUse = false;
    Vector hitboxes[HITBOX_MAX];
    bool Moving = false;
};

class Backtrack : public Singleton<Backtrack>
{
public:
    void OnCreateMove();
    float GetLerpTime();
    bool IsTickValid(int tick);
    bool RageBacktrack(C_BasePlayer* player, int i, float& damage, Vector& hitpos, TickRecord& _record);
    void LegitOnCreateMove(std::deque<int> hb_enabled);
    void FinishLegitBacktrack(CUserCmd* cmd);
    std::deque<LegitTickRecord> GetValidLegitRecords(int i, float lbot_backtrack_ms);
private:
    bool GetHitboxPos(int hb, mstudiohitboxset_t* hitboxset, matrix3x4_t matrix[MAXSTUDIOBONES], Vector& pos);
    std::deque<TickRecord> GetValidTicks(int i);
    std::deque<TickRecord> GetBestTicks(int i);
    bool GetBestPointForRecord(C_BasePlayer* player, TickRecord record, float minDmg, Vector& hitpos, float& damage, bool& willKill);
    std::vector<TickRecord> TickRecords[128];
    std::vector<LegitTickRecord> LegitTickRecords[128];
    void AddTick(C_BasePlayer* player, int i);
};