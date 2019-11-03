//ne rabotaet =))))))))))))))))))))))))))))))))))))
#include "LagCompensation.h"
#include "Rbot.h"
void LagCompensation::RageBacktrack(C_BasePlayer* target, CUserCmd* usercmd, Vector& aim_point, float& damage, bool& hitchanced)
{
	auto firedShots = g_LocalPlayer->m_iShotsFired();
	float CDamage = 0.f;

	if (StartLagCompensation(target))
	{
		LagRecord cur_record;
		auto& m_LagRecords = this->m_LagRecord[target->EntIndex()];
		while (FindViableRecord(target, &cur_record))
		{
			auto iter = std::find(m_LagRecords.begin(), m_LagRecords.end(), cur_record);
			if (iter == m_LagRecords.end())
				continue;

			if (iter->m_bNoGoodSpots)
			{
				// Already awalled from same spot, don't try again like a dumbass.
				if (iter->m_vecLocalAimspot == g_LocalPlayer->GetEyePos())
					continue;
				else
					iter->m_bNoGoodSpots = false;
			}

			if (!iter->m_bMatrixBuilt)
			{
				if (!target->SetupBones(iter->matrix, 128, 256, iter->m_flSimulationTime))
					continue;

				iter->m_bMatrixBuilt = true;
			}

			//float mindmg = 20;
			bool willkill = true;

			Rbot::Get().GetBestHitboxPoint(target, damage, aim_point, BaimMode::NONE, willkill, iter->matrix);
			//bool bruh;
			auto baim = Rbot::Get().GetBAimStatus();
			if (*baim == BaimMode::FORCE_BAIM || *baim == BaimMode::BAIM) {
				Rbot::Get().GetBestHitboxPoint(target, damage, aim_point, BaimMode::FORCE_BAIM, willkill, iter->matrix);
			}
			else {
				Rbot::Get().GetBestHitboxPoint(target, damage, aim_point, BaimMode::NONE, willkill, iter->matrix);
			}
			if (!aim_point.IsValid())
			{
				FinishLagCompensation(target);
				iter->m_bNoGoodSpots = true;
				iter->m_vecLocalAimspot = g_LocalPlayer->GetEyePos();
				continue;
			}

			QAngle aimAngle = Math::CalcAngle(g_LocalPlayer->GetEyePos(), aim_point);// -(g_Options.rage_norecoil ? g_LocalPlayer->m_aimPunchAngle() * 2.f : QAngle(0, 0, 0));

			//hitchanced = AimRage::Get().HitChance(aimAngle, target, g_Options.rage_hitchance_amount);
			hitchanced = Rbot::Get().HitChance(aimAngle, target, Rbot::Get().get_HitChina());

			this->current_record[target->EntIndex()] = *iter;
			break;
		}
		FinishLagCompensation(target);
		ProcessCMD(target->EntIndex(), usercmd);
	}
}

void LagCompensation::FrameUpdatePostEntityThink()
{
	static auto sv_unlag = g_CVar->FindVar("sv_unlag");
	if (g_GlobalVars->maxClients <= 1 || !sv_unlag->GetBool())
	{
		ClearHistory();
		return;
	}

	for (int i = 1; i <= g_GlobalVars->maxClients; i++)
	{
		C_BasePlayer* player = C_BasePlayer::GetPlayerByIndex(i);

		auto& lag_records = this->m_LagRecord[i];

		if (!IsPlayerValid(player))
		{
			if (lag_records.size() > 0)
				lag_records.clear();

			continue;
		}

		int32_t ent_index = player->EntIndex();
		float_t sim_time = player->m_flSimulationTime();

		LagRecord cur_lagrecord;

		RemoveBadRecords(ent_index, lag_records);

		if (lag_records.size() > 0)
		{
			auto& tail = lag_records.back();

			if (tail.m_flSimulationTime == sim_time)
				continue;
		}

		//UpdateAnimations(player); // update animations
		cur_lagrecord.SaveRecord(player); // first let's create the record

		if (!lag_records.empty()) // apply specific stuff that is needed
		{
			auto& temp_lagrecord = lag_records.back();
			int32_t priority_level = GetPriorityLevel(player, &temp_lagrecord);

			cur_lagrecord.m_iPriority = priority_level;
			cur_lagrecord.m_flPrevLowerBodyYaw = temp_lagrecord.m_flPrevLowerBodyYaw;
			cur_lagrecord.m_arrflPrevPoseParameters = temp_lagrecord.m_arrflPrevPoseParameters;

			if (priority_level == 3)
				cur_lagrecord.m_angAngles.yaw = temp_lagrecord.m_angAngles.yaw;
		}

		lag_records.emplace_back(cur_lagrecord);
	}
}

void LagCompensation::ProcessCMD(int iTargetIdx, CUserCmd* usercmd)
{
	LagRecord recentLR = m_RestoreLagRecord[iTargetIdx].first;
	if (!IsTickValid(TIME_TO_TICKS(recentLR.m_flSimulationTime)))
		usercmd->tick_count = TIME_TO_TICKS(C_BasePlayer::GetPlayerByIndex(iTargetIdx)->m_flSimulationTime() + GetLerpTime());
	else
		usercmd->tick_count = TIME_TO_TICKS(recentLR.m_flSimulationTime + GetLerpTime());

}

void LagCompensation::RemoveBadRecords(int Idx, std::deque<LagRecord>& records)
{
	auto& m_LagRecords = records; // Should use rbegin but can't erase
	for (auto lag_record = m_LagRecords.begin(); lag_record != m_LagRecords.end(); lag_record++)
	{
		if (!IsTickValid(TIME_TO_TICKS(lag_record->m_flSimulationTime)))
		{
			m_LagRecords.erase(lag_record);
			if (!m_LagRecords.empty())
				lag_record = m_LagRecords.begin();
			else break;
		}
	}
}

bool LagCompensation::StartLagCompensation(C_BasePlayer* player)
{
	backtrack_records.clear();

	enum
	{
		// Only try to awall the "best" records, otherwise fail.
		TYPE_BEST_RECORDS,
		// Only try to awall the newest and the absolute best record.
		TYPE_BEST_AND_NEWEST,
		// Awall everything (fps killer)
		TYPE_ALL_RECORDS,
	};

	auto& m_LagRecords = this->m_LagRecord[player->EntIndex()];
	m_RestoreLagRecord[player->EntIndex()].second.SaveRecord(player);

	switch (g_Config.rbot_lagcompensation_type)
	{
	case TYPE_BEST_RECORDS:
	{
		for (auto it : m_LagRecords)
		{
			if (it.m_iPriority >= 1 || (it.m_vecVelocity.Length2D() > 10.f)) // let's account for those moving fags aswell -> it's experimental and not supposed what this lagcomp mode should do
				backtrack_records.emplace_back(it);
		}
		break;
	}
	case TYPE_BEST_AND_NEWEST:
	{
		LagRecord newest_record = LagRecord();
		for (auto it : m_LagRecords)
		{
			if (it.m_flSimulationTime > newest_record.m_flSimulationTime)
				newest_record = it;

			if (it.m_iPriority >= 1 /*&& !(it.m_nFlags & FL_ONGROUND) && it.m_vecVelocity.Length2D() > 150*/)
				backtrack_records.emplace_back(it);
		}
		backtrack_records.emplace_back(newest_record);
		break;
	}
	case TYPE_ALL_RECORDS:
		// Ouch, the fps drop will be H U G E.
		backtrack_records = m_LagRecords;
		break;
	}

	std::sort(backtrack_records.begin(), backtrack_records.end(), [](LagRecord const& a, LagRecord const& b) { return a.m_iPriority > b.m_iPriority; });
	return backtrack_records.size() > 0;
}

bool LagCompensation::FindViableRecord(C_BasePlayer* player, LagRecord* record)
{
	auto& m_LagRecords = this->m_LagRecord[player->EntIndex()];

	// Ran out of records to check. Go back.
	if (backtrack_records.empty())
	{
		return false;
	}

	LagRecord
		recentLR = *backtrack_records.begin(),
		prevLR;

	// Should still use m_LagRecords because we're checking for LC break.
	auto iter = std::find(m_LagRecords.begin(), m_LagRecords.end(), recentLR);
	auto idx = std::distance(m_LagRecords.begin(), iter);
	if (0 != idx) prevLR = *std::prev(iter);

	// Saving first record for processcmd.
	m_RestoreLagRecord[player->EntIndex()].first = recentLR;

	if (!IsTickValid(TIME_TO_TICKS(recentLR.m_flSimulationTime)))
	{
		backtrack_records.pop_front();
		return backtrack_records.size() > 0; // RET_NO_RECORDS true false
	}

	// Remove a record...
	backtrack_records.pop_front();

	if ((0 != idx) && (recentLR.m_vecOrigin - prevLR.m_vecOrigin).LengthSqr() > 4096.f)
	{
		/*float simulationTimeDelta = recentLR.m_flSimulationTime - prevLR.m_flSimulationTime;

		int simulationTickDelta = clamp(TIME_TO_TICKS(simulationTimeDelta), 1, 15);

		for (; simulationTickDelta > 0; simulationTickDelta--)
			RebuildGameMovement::Get().FullWalkMove(player);*/
		FakelagFix(player);

		// Bandage fix so we "restore" to the lagfixed player.
		m_RestoreLagRecord[player->EntIndex()].second.SaveRecord(player);
		*record = m_RestoreLagRecord[player->EntIndex()].second;

		// Clear so we don't try to bt shit we can't
		backtrack_records.clear();

		return true; // Return true so we still try to aimbot.
	}
	else
	{
		player->InvalidateBoneCache();

		player->GetCollideable()->OBBMins() = recentLR.m_vecMins;
		player->GetCollideable()->OBBMaxs() = recentLR.m_vecMax;

		player->SetAbsAngles(QAngle(0, recentLR.m_angAngles.yaw, 0));
		player->SetAbsOriginal(recentLR.m_vecOrigin);

		player->m_fFlags() = recentLR.m_nFlags;

		int layerCount = player->GetNumAnimOverlays();
		for (int i = 0; i < layerCount; ++i)
		{
			AnimationLayer* currentLayer = player->GetAnimOverlay(i);
			currentLayer->m_nOrder = recentLR.m_LayerRecords[i].m_nOrder;
			currentLayer->m_nSequence = recentLR.m_LayerRecords[i].m_nSequence;
			currentLayer->m_flWeight = recentLR.m_LayerRecords[i].m_flWeight;
			currentLayer->m_flCycle = recentLR.m_LayerRecords[i].m_flCycle;
		}

		player->m_flPoseParameter() = recentLR.m_arrflPoseParameters;

		*record = recentLR;
		return true;
	}
}

void LagCompensation::FinishLagCompensation(C_BasePlayer* player)
{
	int idx = player->EntIndex();

	player->InvalidateBoneCache();

	player->GetCollideable()->OBBMins() = m_RestoreLagRecord[idx].second.m_vecMins;
	player->GetCollideable()->OBBMaxs() = m_RestoreLagRecord[idx].second.m_vecMax;

	player->SetAbsAngles(QAngle(0, m_RestoreLagRecord[idx].second.m_angAngles.yaw, 0));
	player->SetAbsOriginal(m_RestoreLagRecord[idx].second.m_vecOrigin);

	player->m_fFlags() = m_RestoreLagRecord[idx].second.m_nFlags;

	int layerCount = player->GetNumAnimOverlays();
	for (int i = 0; i < layerCount; ++i)
	{
		AnimationLayer* currentLayer = player->GetAnimOverlay(i);
		currentLayer->m_nOrder = m_RestoreLagRecord[idx].second.m_LayerRecords[i].m_nOrder;
		currentLayer->m_nSequence = m_RestoreLagRecord[idx].second.m_LayerRecords[i].m_nSequence;
		currentLayer->m_flWeight = m_RestoreLagRecord[idx].second.m_LayerRecords[i].m_flWeight;
		currentLayer->m_flCycle = m_RestoreLagRecord[idx].second.m_LayerRecords[i].m_flCycle;
	}

	player->m_flPoseParameter() = m_RestoreLagRecord[idx].second.m_arrflPoseParameters;
}

int LagCompensation::GetPriorityLevel(C_BasePlayer* player, LagRecord* lag_record)
{
	int priority = 0;

	if (lag_record->m_flPrevLowerBodyYaw != player->m_flLowerBodyYawTarget())
	{
		lag_record->m_angAngles.yaw = player->m_flLowerBodyYawTarget();
		priority = 3;
	}

	if ((player->m_flPoseParameter()[1] > (0.85f) && lag_record->m_arrflPrevPoseParameters[1] <= (0.85f)) || (player->m_flPoseParameter()[1] <= (0.85f) && lag_record->m_arrflPrevPoseParameters[1] > (0.85f)))
		priority = 1;

	lag_record->m_flPrevLowerBodyYaw = player->m_flLowerBodyYawTarget();
	lag_record->m_arrflPrevPoseParameters = player->m_flPoseParameter();

	return priority;
}

void LagCompensation::SimulateMovement(Vector& velocity, Vector& origin, C_BasePlayer* player, int& flags, bool was_in_air)
{
	if (!(flags & FL_ONGROUND))
		velocity.z -= (g_GlobalVars->frametime * g_CVar->FindVar("sv_gravity")->GetFloat());
	else if (was_in_air)
		velocity.z = g_CVar->FindVar("sv_jump_impulse")->GetFloat();

	const Vector mins = player->GetCollideable()->OBBMins();
	const Vector max = player->GetCollideable()->OBBMaxs();

	const Vector src = origin;
	Vector end = src + (velocity * g_GlobalVars->frametime);

	Ray_t ray;
	ray.Init(src, end, mins, max);

	trace_t trace;
	CTraceFilter filter;
	filter.pSkip = (void*)(player);

	g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

	if (trace.fraction != 1.f)
	{
		for (int i = 0; i < 2; i++)
		{
			velocity -= trace.plane.normal * velocity.Dot(trace.plane.normal);

			const float dot = velocity.Dot(trace.plane.normal);
			if (dot < 0.f)
			{
				velocity.x -= dot * trace.plane.normal.x;
				velocity.y -= dot * trace.plane.normal.y;
				velocity.z -= dot * trace.plane.normal.z;
			}

			end = trace.endpos + (velocity * (g_GlobalVars->interval_per_tick * (1.f - trace.fraction)));

			ray.Init(trace.endpos, end, mins, max);
			g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

			if (trace.fraction == 1.f)
				break;
		}
	}

	origin = trace.endpos;
	end = trace.endpos;
	end.z -= 2.f;

	ray.Init(origin, end, mins, max);
	g_EngineTrace->TraceRay(ray, MASK_PLAYERSOLID, &filter, &trace);

	flags &= ~(1 << 0);

	if (trace.DidHit() && trace.plane.normal.z > 0.7f)
		flags |= (1 << 0);
}

void LagCompensation::FakelagFix(C_BasePlayer* player)
{
	auto& lag_records = this->m_LagRecord[player->EntIndex()];

	if (lag_records.size() > 2)
	{
		bool bChocked = TIME_TO_TICKS(player->m_flSimulationTime() - lag_records.back().m_flSimulationTime) > 1;
		bool bInAir = false;

		if (!(player->m_fFlags() & FL_ONGROUND) || !(lag_records.back().m_nFlags & FL_ONGROUND))
			bInAir = true;

		if (bChocked)
		{
			player->m_vecOrigin() = lag_records.back().m_vecOrigin;
			player->SetAbsOriginal(lag_records.back().m_vecAbsOrigin);
			player->m_vecVelocity() = lag_records.back().m_vecVelocity;
			player->m_fFlags() = lag_records.back().m_nFlags;
		}

		Vector data_origin = player->m_vecOrigin();
		Vector data_velocity = player->m_vecVelocity();
		int data_flags = player->m_fFlags();

		if (bChocked)
		{
			SimulateMovement(data_velocity, data_origin, player, data_flags, bInAir);

			player->m_vecOrigin() = data_origin;
			player->SetAbsOriginal(data_origin);
			player->m_vecVelocity() = data_velocity;

			player->m_fFlags() &= 0xFFFFFFFE;
			auto penultimate_record = *std::prev(lag_records.end(), 2);
			if ((lag_records.back().m_nFlags & FL_ONGROUND) && (penultimate_record.m_nFlags & FL_ONGROUND))
				player->m_fFlags() |= 1;
			if (*(float*)((uintptr_t)player->GetAnimOverlay(0) + 0x138) > 0.f)
				player->m_fFlags() |= 1;
		}
	}
}


void LagCompensation::UpdateAnimations(C_BasePlayer* player)
{
	CCSGOPlayerAnimState* state = player->GetPlayerAnimState();
	if (state)
	{
		// backup
		const float curtime = g_GlobalVars->curtime;
		const float frametime = g_GlobalVars->frametime;

		static auto host_timescale = g_CVar->FindVar(("host_timescale"));

		g_GlobalVars->frametime = g_GlobalVars->interval_per_tick * host_timescale->GetFloat();
		g_GlobalVars->curtime = player->m_flSimulationTime();

		int backup_eflags = player->m_iEFlags();

		// SetLocalVelocity
		player->m_iEFlags() &= ~0x1000; // InvalidatePhysicsRecursive(VELOCITY_CHANGED); EFL_DIRTY_ABSVELOCITY = 0x1000
		player->m_vecAbsVelocity() = player->m_vecVelocity();

		// invalidates prior animations
		if (state->m_iLastClientSideAnimationUpdateFramecount == g_GlobalVars->framecount)
			state->m_iLastClientSideAnimationUpdateFramecount = g_GlobalVars->framecount - 1;

		player->m_bClientSideAnimation() = true;

		// updates local animations + poses + calculates new abs angle based on eyeangles and other stuff
		player->UpdateClientSideAnimation();

		player->m_bClientSideAnimation() = false;

		// restore
		player->m_iEFlags() = backup_eflags;

		g_GlobalVars->curtime = curtime;
		g_GlobalVars->frametime = frametime;

		player->InvalidateBoneCache();
		player->SetupBones(nullptr, -1, 0x7FF00, g_GlobalVars->curtime);
	}
}

void LagCompensation::SetOverwriteTick(C_BasePlayer* player, QAngle angles, float_t correct_time, uint32_t priority)
{
	int idx = player->EntIndex();
	LagRecord overwrite_record;
	auto& m_LagRecords = this->m_LagRecord[player->EntIndex()];

	if (!IsTickValid(TIME_TO_TICKS(correct_time)))
	{
		char found3[256];
		sprintf(found3, "Dev Error: failed to overwrite tick, delta too big. Priority: %d", priority);
		g_Logger.Backtrack("BACKTRACK", found3);
	}
		//g_CVar->ConsoleColorPrintf(Color(255, 0, 0, 255), "Dev Error: failed to overwrite tick, delta too big. Priority: %d\n", priority);

	overwrite_record.SaveRecord(player);
	overwrite_record.m_angAngles = angles;
	overwrite_record.m_iPriority = priority;
	overwrite_record.m_flSimulationTime = correct_time;
	m_LagRecords.emplace_back(overwrite_record);
}

bool LagCompensation::IsTickValid(int tick)
{
	// better use polak's version than our old one, getting more accurate results

	INetChannelInfo* nci = g_EngineClient->GetNetChannelInfo();

	static auto sv_maxunlag = g_CVar->FindVar("sv_maxunlag");

	if (!nci || !sv_maxunlag)
		return false;

	float correct = clamp(nci->GetLatency(FLOW_OUTGOING) + GetLerpTime(), 0.f, sv_maxunlag->GetFloat());

	float deltaTime = correct - (g_GlobalVars->curtime - TICKS_TO_TIME(tick));

	return fabsf(deltaTime) < 0.2f;
}

bool LagCompensation::IsPlayerValid(C_BasePlayer* player)
{
	if (!player)
		return false;

	if (!player->IsPlayer())
		return false;

	if (player == g_LocalPlayer)
		return false;

	if (player->IsDormant())
		return false;

	if (!player->IsAlive())
		return false;

	if (player->m_iTeamNum() == g_LocalPlayer->m_iTeamNum())
		return false;

	if (player->m_bGunGameImmunity())
		return false;

	return true;
}

float LagCompensation::GetLerpTime()
{
	static auto cl_interp = g_CVar->FindVar("cl_interp");
	static auto cl_interp_ratio = g_CVar->FindVar("cl_interp_ratio");
	static auto sv_client_min_interp_ratio = g_CVar->FindVar("sv_client_min_interp_ratio");
	static auto sv_client_max_interp_ratio = g_CVar->FindVar("sv_client_max_interp_ratio");
	static auto cl_updaterate = g_CVar->FindVar("cl_updaterate");
	static auto sv_minupdaterate = g_CVar->FindVar("sv_minupdaterate");
	static auto sv_maxupdaterate = g_CVar->FindVar("sv_maxupdaterate");

	auto updaterate = std::clamp(cl_updaterate->GetFloat(), sv_minupdaterate->GetFloat(), sv_maxupdaterate->GetFloat());
	auto lerp_ratio = std::clamp(cl_interp_ratio->GetFloat(), sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());
	return std::clamp(lerp_ratio / updaterate, cl_interp->GetFloat(), 1.0f);
}

void LagRecord::SaveRecord(C_BasePlayer* player)
{
	m_vecOrigin = player->m_vecOrigin();
	m_vecAbsOrigin = player->m_angAbsOrigin(); // FIX
	m_angAngles = player->m_angEyeAngles();
	m_flSimulationTime = player->m_flSimulationTime();
	m_vecMins = player->GetCollideable()->OBBMins();
	m_vecMax = player->GetCollideable()->OBBMaxs();
	m_nFlags = player->m_fFlags();
	m_vecVelocity = player->m_vecVelocity();

	int layerCount = player->GetNumAnimOverlays();
	for (int i = 0; i < layerCount; i++)
	{
		AnimationLayer* currentLayer = player->GetAnimOverlay(i);
		m_LayerRecords[i].m_nOrder = currentLayer->m_nOrder;
		m_LayerRecords[i].m_nSequence = currentLayer->m_nSequence;
		m_LayerRecords[i].m_flWeight = currentLayer->m_flWeight;
		m_LayerRecords[i].m_flCycle = currentLayer->m_flCycle;
	}
	m_arrflPoseParameters = player->m_flPoseParameter();

	m_iTickCount = g_GlobalVars->tickcount;
	m_vecHeadSpot = player->GetBonePos(8);
}
