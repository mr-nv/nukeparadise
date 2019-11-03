
#include "csgostructs.hpp"
#include "../Helpers/Math.hpp"
#include "../Helpers/Utils.hpp"

CGameRules* g_GameRules = nullptr;

bool C_BaseEntity::IsPlayer()
{
    return this->GetClientClass()->m_ClassID == ClassId::CCSPlayer;
}

bool C_BaseEntity::IsWeapon()
{
    return CallVFunction<bool(__thiscall*)(C_BaseEntity*)>(this, 165)(this);
}

bool C_BaseEntity::IsPlantedC4()
{
    return GetClientClass()->m_ClassID == ClassId::CPlantedC4;
}

bool C_BaseEntity::IsDefuseKit()
{
    return GetClientClass()->m_ClassID == ClassId::CBaseAnimating;
}

CCSWeaponInfo* C_BaseCombatWeapon::GetCSWeaponData()
{
    return CallVFunction<CCSWeaponInfo*(__thiscall*)(void*)>(this, 456)(this); //444
}

bool C_BaseCombatWeapon::HasBullets()
{
    return !IsReloading() && m_iClip1() > 0;
}

bool C_BaseCombatWeapon::IsWeaponSwitching()
{
	//if (this->IsSniper() && !this->IsAuto())
	//	return false;

	static int g_iLastWeap = 0;
	static bool g_bWeapSwitched = false;

	if (this->m_iItemDefinitionIndex() != g_iLastWeap)
	{
		g_bWeapSwitched = true;
		g_iLastWeap = this->m_iItemDefinitionIndex();
	}

	float flServerTime = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;
	float flNextAttackTime = this->m_flNextPrimaryAttack();

	if (flServerTime - flNextAttackTime >= 1.f && g_bWeapSwitched)
		g_bWeapSwitched = false;

	return g_bWeapSwitched;
}

bool C_BaseCombatWeapon::CanFire()
{
    static decltype(this) stored_weapon = nullptr;
    static auto stored_tick = 0;
    if (stored_weapon != this || stored_tick >= g_LocalPlayer->m_nTickBase())
    {
        stored_weapon = this;
        stored_tick = g_LocalPlayer->m_nTickBase();
        return false; //cannot shoot first tick after switch
    }

    if (IsReloading() || m_iClip1() <= 0 || !g_LocalPlayer)
    {
        return false;
    }

    auto flServerTime = g_LocalPlayer->m_nTickBase() * g_GlobalVars->interval_per_tick;

    return m_flNextPrimaryAttack() <= flServerTime;
}

bool C_BaseCombatWeapon::IsGrenade()
{
    return GetCSWeaponData()->WeaponType == WEAPONTYPE_GRENADE;
}

bool C_BaseCombatWeapon::IsC4()
{
	return GetCSWeaponData()->WeaponType == WEAPONTYPE_C4;
}

bool C_BaseCombatWeapon::IsGun()
{
    switch (GetCSWeaponData()->WeaponType)
    {
        case WEAPONTYPE_C4:
            return false;
        case WEAPONTYPE_GRENADE:
            return false;
        case WEAPONTYPE_KNIFE:
            return false;
        case WEAPONTYPE_UNKNOWN:
            return false;
        default:
            return true;
    }
}

bool C_BaseCombatWeapon::IsKnife()
{
    if (this->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER)
    {
        return false;
    }
    return GetCSWeaponData()->WeaponType == WEAPONTYPE_KNIFE;
}

bool C_BaseCombatWeapon::IsRifle()
{
    switch (GetCSWeaponData()->WeaponType)
    {
        case WEAPONTYPE_RIFLE:
            return true;
        case WEAPONTYPE_SUBMACHINEGUN:
            return true;
        case WEAPONTYPE_SHOTGUN:
            return true;
        case WEAPONTYPE_MACHINEGUN:
            return true;
        default:
            return false;
    }
}

bool C_BaseCombatWeapon::IsPistol()
{
    switch (GetCSWeaponData()->WeaponType)
    {
        case WEAPONTYPE_PISTOL:
            return true;
        default:
            return false;
    }
}

bool C_BaseCombatWeapon::IsSubmachinegun()
{
    switch (GetCSWeaponData()->WeaponType)
    {
        case WEAPONTYPE_SUBMACHINEGUN:
            return true;
        default:
            return false;
    }
}

bool C_BaseCombatWeapon::IsSniper()
{
	switch (GetCSWeaponData()->WeaponType)
	{
	case WEAPONTYPE_SNIPER_RIFLE:
		return true;
	default:
		return false;
	}
}

bool C_BaseCombatWeapon::IsShotgun()
{
    switch (GetCSWeaponData()->WeaponType)
    {
        case WEAPONTYPE_SHOTGUN:
            return true;
        default:
            return false;
    }
}

bool C_BaseCombatWeapon::IsMachinegun()
{
    switch (GetCSWeaponData()->WeaponType)
    {
        case WEAPONTYPE_MACHINEGUN:
            return true;
        default:
            return false;
    }
}

bool C_BaseCombatWeapon::Is_no_aim() {
	int idx = m_iItemDefinitionIndex();

	return (idx == WEAPON_C4 || idx == WEAPON_KNIFE || idx == WEAPON_KNIFE_BAYONET || idx == WEAPON_KNIFE_BUTTERFLY || idx == WEAPON_KNIFE_FALCHION
		|| idx == WEAPON_KNIFE_FLIP || idx == WEAPON_KNIFE_GUT || idx == WEAPON_KNIFE_KARAMBIT || idx == WEAPON_KNIFE_M9_BAYONET || idx == WEAPON_KNIFE_PUSH
		|| idx == WEAPON_KNIFE_SURVIVAL_BOWIE || idx == WEAPON_KNIFE_T || idx == WEAPON_KNIFE_TACTICAL || idx == WEAPON_FLASHBANG || idx == WEAPON_HEGRENADE
		|| idx == WEAPON_SMOKEGRENADE || idx == WEAPON_MOLOTOV || idx == WEAPON_DECOY || idx == WEAPON_INCGRENADE);
}


bool C_BaseCombatWeapon::IsZeus()
{
    if (this->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER)
    {
        return true;
    }
    return false;
}

bool C_BaseCombatWeapon::IsReloading()
{
    static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "C6 87 ? ? ? ? ? 8B 06 8B CE FF 90") + 2);
    return *(bool*)((uintptr_t)this + inReload);
}

float C_BaseCombatWeapon::GetInaccuracy()
{
    return CallVFunction<float(__thiscall*)(void*)>(this, 478)(this); //467
}

float C_BaseCombatWeapon::GetSpread()
{
    return CallVFunction<float(__thiscall*)(void*)>(this, 448)(this); //436 440
}

float C_BaseCombatWeapon::GetCone()
{
	return CallVFunction<float(__thiscall*)(void*)>(this, 484)(this); //436 440
}

void C_BaseCombatWeapon::UpdateAccuracyPenalty()
{
    CallVFunction<void(__thiscall*)(void*)>(this, 479)(this); //467
}

CUtlVector<IRefCounted*>& C_BaseCombatWeapon::m_CustomMaterials()
{
    static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "83 BE ? ? ? ? ? 7F 67") + 2) - 12;
    return *(CUtlVector<IRefCounted*>*)((uintptr_t)this + inReload);
}

bool* C_BaseCombatWeapon::m_bCustomMaterialInitialized()
{
    static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "C6 86 ? ? ? ? ? FF 50 04") + 2);
    return (bool*)((uintptr_t)this + currentCommand);
}

CUserCmd*& C_BasePlayer::m_pCurrentCommand()
{
    static auto currentCommand = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "89 BE ? ? ? ? E8 ? ? ? ? 85 FF") + 2);
    return *(CUserCmd**)((uintptr_t)this + currentCommand);
}

int C_BasePlayer::GetNumAnimOverlays()
{
    return *(int*)((DWORD)this + 0x297C);
}

AnimationLayer* C_BasePlayer::GetAnimOverlays()
{
    // to find offset: use 9/12/17 dll
    // sig: 55 8B EC 51 53 8B 5D 08 33 C0
    return *(AnimationLayer**)((DWORD)this + 0x2980); //10608
}
AnimationLayer* C_BasePlayer::GetAnimOverlay(int i)
{
    if (i < 15)
    {
        return &GetAnimOverlays()[i];
    }
    return nullptr;
}

int C_BasePlayer::GetSequenceActivity(int sequence)
{
    auto hdr = g_MdlInfo->GetStudiomodel(this->GetModel());

    if (!hdr)
    {
        return -1;
    }

    // sig for stuidohdr_t version: 53 56 8B F1 8B DA 85 F6 74 55
    // sig for C_BaseAnimating version: 55 8B EC 83 7D 08 FF 56 8B F1 74 3D
    // c_csplayer vfunc 242, follow calls to find the function.

    static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "55 8B EC 83 7D 08 FF 56 8B F1 74 3D"));

    return get_sequence_activity(this, hdr, sequence);
}

CCSGOPlayerAnimState* C_BasePlayer::GetPlayerAnimState()
{
	return *(CCSGOPlayerAnimState * *)((DWORD)this + 0x3900);
}

void C_BasePlayer::UpdateAnimationState(CCSGOPlayerAnimState* state, QAngle angle)
{
    static auto UpdateAnimState = Utils::PatternScan(
                                      GetModuleHandle(L"client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");
    /*static auto UpdateAnimState = Utils::PatternScan(
    GetModuleHandle(L"client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24");
    */
    if (!UpdateAnimState)
    {
        return;
    }

    __asm
    {
        push 0
    }

    __asm
    {
        mov ecx, state

        movss xmm1, dword ptr[angle + 4]
        movss xmm2, dword ptr[angle]

        call UpdateAnimState
    }
}

void C_BasePlayer::ResetAnimationState(CCSGOPlayerAnimState* state)
{
    using ResetAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*);
    static auto ResetAnimState = (ResetAnimState_t)Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "56 6A 01 68 ? ? ? ? 8B F1");
    if (!ResetAnimState)
    {
        return;
    }

    ResetAnimState(state);
}

void C_BasePlayer::CreateAnimationState(CCSGOPlayerAnimState* state)
{
    using CreateAnimState_t = void(__thiscall*)(CCSGOPlayerAnimState*, C_BasePlayer*);
    static auto CreateAnimState = (CreateAnimState_t)Utils::PatternScan(GetModuleHandle(L"client_panorama.dll"), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46");
    if (!CreateAnimState)
    {
        return;
    }

    CreateAnimState(state, this);
}

Vector C_BasePlayer::GetEyePos()
{
    return m_vecOrigin() + m_vecViewOffset();
}

player_info_t C_BasePlayer::GetPlayerInfo()
{
    player_info_t info;
    g_EngineClient->GetPlayerInfo(EntIndex(), &info);
    return info;
}

bool C_BasePlayer::IsAlive()
{
    return m_lifeState() == LIFE_ALIVE || this->m_iHealth() > 0;
}

bool C_BasePlayer::TeamMate()
{
	if (Utils::IsDangerZone)
		return (g_LocalPlayer->m_nSurvivalTeam() == -1) ? false : (g_LocalPlayer->m_nSurvivalTeam() == this->m_nSurvivalTeam());
	else
		return this->m_iTeamNum() == g_LocalPlayer->m_iTeamNum();
}

bool C_BasePlayer::IsFlashed()
{
    if (m_flFlashDuration() > 0.f)
    {
        return true;
    }
    return false;
}

bool C_BasePlayer::HasC4()
{
    static auto fnHasC4
        = reinterpret_cast<bool(__thiscall*)(void*)>(
              Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "56 8B F1 85 F6 74 31")
          );

    return fnHasC4(this);
}

Vector C_BasePlayer::GetHitboxPos(int hitbox_id)
{
    matrix3x4_t boneMatrix[MAXSTUDIOBONES];

    if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f))
    {
        auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
        if (studio_model)
        {
            auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
            if (hitbox)
            {
                auto
                min = Vector{},
                max = Vector{};

                Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
                Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

                return (min + max) / 2.0f;
            }
        }
    }
    return Vector{};
}

mstudiobbox_t* C_BasePlayer::GetHitbox(int hitbox_id)
{
    matrix3x4_t boneMatrix[MAXSTUDIOBONES];

    if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f))
    {
        auto studio_model = g_MdlInfo->GetStudiomodel(GetModel());
        if (studio_model)
        {
            auto hitbox = studio_model->GetHitboxSet(0)->GetHitbox(hitbox_id);
            if (hitbox)
            {
                return hitbox;
                //auto
                //	min = Vector{},
                //	max = Vector{};

                //Math::VectorTransform(hitbox->bbmin, boneMatrix[hitbox->bone], min);
                //Math::VectorTransform(hitbox->bbmax, boneMatrix[hitbox->bone], max);

                //return (min + max) / 2.0f;
            }
        }
    }
    return nullptr;
}

bool C_BasePlayer::GetHitboxPos(int hitbox, Vector& output)
{
    if (hitbox >= HITBOX_MAX)
    {
        return false;
    }

    const model_t* model = this->GetModel();
    if (!model)
    {
        return false;
    }

    studiohdr_t* studioHdr = g_MdlInfo->GetStudiomodel(model);
    if (!studioHdr)
    {
        return false;
    }

    matrix3x4_t matrix[MAXSTUDIOBONES];
    if (!this->SetupBones(matrix, MAXSTUDIOBONES, 0x100, 0))
    {
        return false;
    }

    mstudiobbox_t* studioBox = studioHdr->GetHitboxSet(0)->GetHitbox(hitbox);
    if (!studioBox)
    {
        return false;
    }

    Vector min, max;

    Math::VectorTransform(studioBox->bbmin, matrix[studioBox->bone], min);
    Math::VectorTransform(studioBox->bbmax, matrix[studioBox->bone], max);

    output = (min + max) * 0.5f;

    return true;
}

bool CanUsePrecached = false;
matrix3x4_t PrecachedMatrix[MAXSTUDIOBONES];
mstudiohitboxset_t* PrecachedStudioBoxSet;
void C_BasePlayer::PrecaceOptimizedHitboxes()
{
    CanUsePrecached = false;
    const model_t* model = this->GetModel();
    if (!model)
    {
        return;
    }

    studiohdr_t* studioHdr = g_MdlInfo->GetStudiomodel(model);
    if (!studioHdr)
    {
        return;
    }

    if (!this->SetupBones(PrecachedMatrix, MAXSTUDIOBONES, 0x100, 0))
    {
        return;
    }

    PrecachedStudioBoxSet = studioHdr->GetHitboxSet(0);
    if (!PrecachedStudioBoxSet)
    {
        return;
    }
    CanUsePrecached = true;
}

bool C_BasePlayer::GetOptimizedHitboxPos(int hitbox, Vector& output)
{
    if (!CanUsePrecached)
    {
        return false;
    }
    if (hitbox >= HITBOX_MAX)
    {
        return false;
    }

    Vector min, max;

    mstudiobbox_t* studioBox = PrecachedStudioBoxSet->GetHitbox(hitbox);
    if (!studioBox)
    {
        return false;
    }

    Math::VectorTransform(studioBox->bbmin, PrecachedMatrix[studioBox->bone], min);
    Math::VectorTransform(studioBox->bbmax, PrecachedMatrix[studioBox->bone], max);

    output = (min + max) * 0.5f;

    return true;
}

Vector C_BasePlayer::GetBonePos(int bone)
{
    matrix3x4_t boneMatrix[MAXSTUDIOBONES];
    if (SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f))
    {
        return boneMatrix[bone].at(3);
    }
    return Vector{};
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, int hitbox)
{
    CGameTrace tr;
    Ray_t ray;
    CTraceFilter filter;
    filter.pSkip = this;

    auto endpos = player->GetHitboxPos(hitbox);

    ray.Init(GetEyePos(), endpos);
    g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

    return tr.hit_entity == player || tr.fraction > 0.97f;
}

bool C_BasePlayer::CanSeePlayer(C_BasePlayer* player, const Vector& pos)
{
    CGameTrace tr;
    Ray_t ray;
    CTraceFilter filter;
    filter.pSkip = this;

    //auto start = GetEyePos();
    //auto dir = (pos - start).Normalized();

    ray.Init(GetEyePos(), pos);
    g_EngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

    return tr.hit_entity == player || tr.fraction > 0.97f;
}

void C_BasePlayer::UpdateClientSideAnimation()
{
    return CallVFunction<void(__thiscall*)(void*)>(this, 223)(this);
}

void C_BasePlayer::SetAngle2(QAngle wantedang)
{
    typedef void(__thiscall* SetAngleFn)(void*, const QAngle&);
    static SetAngleFn SetAngle2 = reinterpret_cast<SetAngleFn>(Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1"));
    SetAngle2(this, wantedang);
}

void C_BasePlayer::InvalidateBoneCache()
{
    static auto InvalidateBoneCacheFn = Utils::PatternScan(GetModuleHandleA("client_panorama.dll"), "80 3D ?? ?? ?? ?? ?? 74 16 A1 ?? ?? ?? ?? 48 C7 81");
    reinterpret_cast<void(__fastcall*)(void*)>(InvalidateBoneCacheFn)(this);
}

float C_BasePlayer::GetMaxDesyncAngle()
{
	/*auto animstate = this->GetBasePlayerAnimState();
	float speedfactor = std::clamp(animstate->m_flFeetSpeedForwardsOrSideWays, 0.f, 1.f);
	float unk1 = ((animstate->m_flStopToFullRunningFraction * -.3f) - .2f) * speedfactor;
	float unk2 = unk1 + 1.f;

	if (animstate->m_fDuckAmount > 0.0f)
	{
		float max_velocity = std::clamp(animstate->m_flFeetSpeedUnknownForwardOrSideways, 0.f, 1.f);
		float duck_speed = animstate->m_fDuckAmount * max_velocity;
		unk2 += (duck_speed * (0.5f - unk2));
	}

	return *(float*)((uintptr_t)animstate + 0x334) * unk2;*/

	if (!this)
		return 0.f;

	auto anim_state = this->GetPlayerAnimState();
	if (!anim_state)
		return 0.f;

	float duck_amount = anim_state->m_fDuckAmount;
	float speed_fraction = Math::Maximum<float>(0, Math::Minimum<float>(anim_state->m_flFeetSpeedForwardsOrSideWays, 1));
	float speed_factor = Math::Maximum<float>(0, Math::Minimum<float>(1, anim_state->m_flFeetSpeedUnknownForwardOrSideways));

	float yaw_modifier = (((anim_state->m_flStopToFullRunningFraction * -0.3f) - 0.2f) * speed_fraction) + 1.0f;

	if (duck_amount > 0.f)
		yaw_modifier += ((duck_amount * speed_factor) * (0.5f - yaw_modifier));

	return *(float*)((uintptr_t)anim_state + 0x334) * yaw_modifier;
}

float C_BaseEntity::DesyncValue() {
//а когда
	if (!this)
		return 0.f;

	auto anim_state = this->AnimStatev2();
	if (!anim_state)
		return 0.f;

	float duck_amount = anim_state->m_fDuckAmount;
	float speed_fraction = Math::Minimum< float >(0, Math::Minimum< float >(anim_state->m_flFeetSpeedForwardsOrSideWays, 1));
	float speed_factor = Math::Maximum< float >(0, Math::Maximum< float >(1, anim_state->m_flFeetSpeedUnknownForwardOrSideways));

	float yaw_modifier = (((anim_state->m_flStopToFullRunningFraction * -0.3f) - 0.2f) * speed_fraction) + 1.0f;

	if (duck_amount > 0.f) {
		yaw_modifier += ((duck_amount * speed_factor) * (0.5f - yaw_modifier));
	}

	return *(float*)((uintptr_t)anim_state + 0x334) * yaw_modifier;
}

int C_BasePlayer::m_nMoveType()
{
    return *(int*)((uintptr_t)this + 0x25C);
}

void C_BasePlayer::SetVAngles(QAngle angles)
{
    static auto deadflag = NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag");
    *(QAngle*)((DWORD)this + deadflag + 0x4) = angles;
}

QAngle* C_BasePlayer::GetVAngles()
{
    static auto deadflag = NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag");
    return (QAngle*)((uintptr_t)this + deadflag + 0x4);
}

void C_BaseAttributableItem::SetGloveModelIndex(int modelIndex)
{
    return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, modelIndex);
}

float C_BasePlayer::GetFlashBangTime()
{

    static uint32_t m_flFlashBangTime = *(uint32_t*)((uint32_t)Utils::PatternScan(GetModuleHandleA("client_panorama.dll"),
                                        "F3 0F 10 86 ?? ?? ?? ?? 0F 2F 40 10 76 30") + 4);
    return *(float*)(this + m_flFlashBangTime);
    //return *(float*)((uintptr_t)this + 0xa308);
}

void C_BaseViewModel::SendViewModelMatchingSequence(int sequence)
{
    return CallVFunction<void(__thiscall*)(void*, int)>(this, 246)(this, sequence);//241
}

CUtlVector<IRefCounted*>& C_EconItemView::m_CustomMaterials()
{
    return *(CUtlVector<IRefCounted*>*)((uintptr_t)this + 0x14);
}

CUtlVector<IRefCounted*>& C_EconItemView::m_VisualsDataProcessors()
{
    static auto inReload = *(uint32_t*)(Utils::PatternScan(GetModuleHandleW(L"client_panorama.dll"), "81 C7 ? ? ? ? 8B 4F 0C 8B 57 04 89 4C 24 0C") + 2);
    return *(CUtlVector<IRefCounted*>*)((uintptr_t)this + inReload);
}

float_t C_BasePlayer::m_flSpawnTime()
{
    return *(float_t*)((uintptr_t)this + 0xA290);
}

VarMapping_t* C_BasePlayer::VarMapping()
{
    return reinterpret_cast<VarMapping_t*>((DWORD)this + 0x24);
}
