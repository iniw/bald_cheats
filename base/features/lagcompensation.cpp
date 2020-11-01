// used: GetBestTarget()
#include "legitbot.h"
#include "lagcompensation.h"

// used: globals interface
#include "../core/interfaces.h"
// used: backtrack time
#include "../core/config.h"
#include "../core/variables.h"
// used: CalcAngle
#include "../utilities/math.h"
// used: GetTickbase
#include "prediction.h"

void CLagCompensation::Run(CUserCmd* pCmd)
{
	/*
	 * we have much public info for that
	 * now it is your own way gl
	 */
}

void CLagCompensation::UpdateIncomingSequences(INetChannel* pNetChannel)
{
	if (pNetChannel == nullptr)
		return;

	// set to real sequence to update, otherwise needs time to get it work again
	if (nLastIncomingSequence == 0)
		nLastIncomingSequence = pNetChannel->iInSequenceNr;

	// check how much sequences we can spike
	if (pNetChannel->iInSequenceNr > nLastIncomingSequence)
	{
		nLastIncomingSequence = pNetChannel->iInSequenceNr;
		vecSequences.emplace_front(SequenceObject_t(pNetChannel->iInReliableState, pNetChannel->iOutReliableState, pNetChannel->iInSequenceNr, I::Globals->flRealTime));
	}

	// is cached too much sequences
	if (vecSequences.size() > 2048U)
		vecSequences.pop_back();
}

void CLagCompensation::ClearIncomingSequences()
{
	if (!vecSequences.empty())
	{
		nLastIncomingSequence = 0;
		vecSequences.clear();
	}
}

void CLagCompensation::AddLatencyToNetChannel(INetChannel* pNetChannel, float flLatency)
{
	for (const auto& sequence : vecSequences)
	{
		if (I::Globals->flRealTime - sequence.flCurrentTime >= flLatency)
		{
			pNetChannel->iInReliableState = sequence.iInReliableState;
			pNetChannel->iInSequenceNr = sequence.iSequenceNr;
			break;
		}
	}
}

void CBacktracking::Run(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	/*
	if (pLocal == nullptr)
		return;

	QAngle angViewPoint;
	I::Engine->GetViewAngles(angViewPoint);

	CBaseEntity* pEntity = GetBestEntity(pLocal);

	if (pEntity == nullptr)
		return;
	
	int iIndex = pEntity->GetIndex();

	Vector vecOldAbsOrigin = pEntity->GetAbsOrigin();

	if (pCmd->iButtons & IN_ATTACK)
	{
		pCmd->iTickCount = TIME_TO_TICKS(record.flSimtime + GetLerp());
	}
	*/
}

void CBacktracking::Update(CBaseEntity* pLocal)
{
	if (!C::Get<bool>(Vars.bBacktracking) || pLocal == nullptr || !pLocal->IsAlive())
	{
		for (auto& record : m_arrRecords)
			record.clear();

		return;
	}

	m_cl_updaterate = I::ConVar->FindVar("cl_updaterate");
	m_sv_minupdaterate = I::ConVar->FindVar("sv_minupdaterate");
	m_sv_maxupdaterate = I::ConVar->FindVar("sv_maxupdaterate");
	m_cl_interp = I::ConVar->FindVar("cl_interp");
	m_cl_interp_ratio = I::ConVar->FindVar("cl_interp_ratio");
	m_sv_client_min_interp_ratio = I::ConVar->FindVar("sv_client_min_interp_ratio");
	m_sv_client_max_interp_ratio = I::ConVar->FindVar("sv_client_max_interp_ratio");
	m_sv_maxunlag = I::ConVar->FindVar("sv_maxunlag");

	for (int i = 1; i <= I::Globals->nMaxClients; i++)
	{
		if (i == I::Engine->GetLocalPlayer())
			continue;

		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);

		if (!IsValid(pLocal, pEntity))
		{
			m_arrRecords[i].clear();
			continue;
		}

		if (!m_arrRecords[i].empty() && m_arrRecords[i].front().flSimtime == pEntity->GetSimulationTime())
			continue;

		Vector vecOldAbsOrigin = pEntity->GetAbsOrigin();

		Record_t recRecord { };

		*(Vector*)((uintptr_t)pEntity + 0xA0) = pEntity->GetOrigin();
		*(int*)((uintptr_t)pEntity + 0xA68) = 0;
		*(int*)((uintptr_t)pEntity + 0xA30) = 0;
		pEntity->InvalidateBoneCache();

		if (!pEntity->SetupBones(recRecord.arrMatrix.data(), MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, I::Globals->flCurrentTime))
			continue;

		recRecord.vecHitboxPos	= GetBestHitbox(pLocal, pEntity, recRecord.arrMatrix);
		recRecord.flSimtime		= pEntity->GetSimulationTime();
		recRecord.vecOrigin		= pEntity->GetOrigin();
		recRecord.vecAbsOrigin	= pEntity->GetAbsOrigin();
		recRecord.vecHeadPos	= pEntity->GetHitboxPosition(HITBOX_HEAD, recRecord.arrMatrix);
		recRecord.vecMins		= pEntity->GetVecMins();
		recRecord.vecMaxs		= pEntity->GetVecMaxs();
		recRecord.angAbsAngles	= pEntity->GetAbsAngles();
		recRecord.pModel		= I::ModelInfo->GetStudioModel(pEntity->GetModel());
		recRecord.vecViewOffset = pEntity->GetViewOffset();

		m_arrRecords[i].push_front(recRecord);

		if (auto invalid = std::find_if(std::cbegin(m_arrRecords[i]), std::cend(m_arrRecords[i]), 
			[&](const Record_t& rec) 
			{ 
				return !CBacktracking::Get().IsValid(rec.flSimtime); 
			}); 
			invalid != std::cend(m_arrRecords[i]))

		m_arrRecords[i].erase(invalid, std::cend(m_arrRecords[i]));

		while (m_arrRecords[i].size() > std::clamp(TIME_TO_TICKS(C::Get<int>(Vars.iBacktrackingTime) / 1000.f), 0, 12))
			m_arrRecords[i].pop_back();

	}
	std::deque<Record_t> deqRecords = m_arrRecords[CLegitBot::Get().m_Target.iIndex];

	float flBestDelta = std::numeric_limits<float>::max();

	if (!deqRecords.empty() && IsValid(pLocal, CLegitBot::Get().m_Target.pEntity))
	{
		for (auto& record : deqRecords)
		{
			QAngle angRecordAngle = M::CalcAngle(pLocal->GetEyePosition(), record.vecHitboxPos).Clamped();
			float flRecordAngleDelta = (angRecordAngle - I::Engine->GetViewAngles()).Clamped().Length();

			if (flRecordAngleDelta < flBestDelta)
			{
				m_BestRecord = record;
				m_flBestRecordDelta = flRecordAngleDelta;
				flBestDelta = flRecordAngleDelta;
			}
		}
	}
}

float CBacktracking::GetLerp()
{
	int iUpdateRate = m_cl_updaterate->GetInt();

	if (m_sv_minupdaterate && m_sv_maxupdaterate)
		iUpdateRate = m_sv_maxupdaterate->GetInt();

	float flRatio = m_cl_interp_ratio->GetFloat();

	if (flRatio == 0)
		flRatio = 1.0f;

	float flLerp = m_cl_interp->GetFloat();

	if (m_sv_client_min_interp_ratio && m_sv_client_max_interp_ratio && m_sv_client_min_interp_ratio->GetFloat() != 1)
		flRatio = std::clamp(flRatio, m_sv_client_min_interp_ratio->GetFloat(), m_sv_client_max_interp_ratio->GetFloat());

	return std::max(flLerp, (flRatio / iUpdateRate));
}

bool CBacktracking::IsValid(float flSimtime)
{
	const INetChannelInfo* pNetChannel = I::Engine->GetNetChannelInfo();
	if (!pNetChannel)
		return false;

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (pLocal == nullptr)
		return false;

	float flCurTime = I::Globals->flCurrentTime;

	float flCorrect = 0.f;

	flCorrect += pNetChannel->GetLatency(FLOW_OUTGOING);
	flCorrect += pNetChannel->GetLatency(FLOW_INCOMING);
	flCorrect += GetLerp();

	flCorrect = std::clamp(flCorrect, 0.f, m_sv_maxunlag->GetFloat());

	float flDeltaTime = flCorrect - (flCurTime - flSimtime);

	if (flSimtime < std::floorf(flCurTime - m_sv_maxunlag->GetFloat()))
		return false;

	return std::abs(flDeltaTime) <= 0.2f;	
}

void CBacktracking::DrawHitbox(std::array<matrix3x4_t, MAXSTUDIOBONES> arrMatrix, studiohdr_t* pModel)
{
	if (pModel == nullptr)
		return;

	mstudiohitboxset_t* pSet = pModel->GetHitboxSet(0);

	if (pSet == nullptr)
		return;

	for (int i = 0; i < pSet->nHitboxes; i++)
	{
		mstudiobbox_t* pHitbox = pSet->GetHitbox(i);

		if (pHitbox == nullptr)
			continue;

		Vector vecMin = M::VectorTransform(pHitbox->vecBBMin, arrMatrix.at(pHitbox->iBone));
		Vector vecMax = M::VectorTransform(pHitbox->vecBBMax, arrMatrix.at(pHitbox->iBone));

		I::DebugOverlay->DrawPill(vecMin, vecMax, pHitbox->flRadius, 255, 255, 255, 255, I::ConVar->FindVar("sv_showlagcompensation_duration")->GetFloat(), 0, 0);
	}
}

bool CBacktracking::IsValid(CBaseEntity* pLocal, CBaseEntity* pEntity)
{
	if (pEntity == nullptr) // obvious
		return false;

	if (!pEntity->IsPlayer()) // only aim at players
		return false;

	if (!pEntity->IsAlive()) // only aim at alive players
		return false;

	if (pEntity->IsDormant()) // only aim at non-dormant players
		return false;

	if (!pLocal->IsEnemy(pEntity)) // only aim at enemies
		return false;

	if (pEntity->HasImmunity()) // only aim at non-immune players
		return false;

	return true;
}

Vector CBacktracking::GetBestHitbox(CBaseEntity* pLocal, CBaseEntity* pEntity, std::array<matrix3x4_t, MAXSTUDIOBONES> arrCustomMatrix)
{
	float iBestDelta = std::numeric_limits<float>::max();
	int iWeaponType = CLegitBot::Get().GetWeaponType(pLocal);

	if ((C_GET_LEGITVAR_TYPE(iWeaponType, iAimHitbox) == (int)ELegitHitboxes::CLOSEST))
	{
		Vector vecBestHitboxPos{ };

		for (int i = 0; i < CLegitBot::Get().m_arrClosestHitboxes.size(); i++)
		{
			Vector vecHitboxPos = pEntity->GetHitboxPosition(CLegitBot::Get().m_arrClosestHitboxes.at(i), arrCustomMatrix);
			QAngle angAngle = M::CalcAngle(pLocal->GetEyePosition(), vecHitboxPos).Clamped();
			float flDelta = (angAngle - I::Engine->GetViewAngles()).Length();

			if (flDelta < iBestDelta)
			{
				vecBestHitboxPos = vecHitboxPos;
				iBestDelta = flDelta;
			}
		}

		return vecBestHitboxPos;
	}
	else
		return pEntity->GetHitboxPosition(CLegitBot::Get().m_arrHitboxes.at((C_GET_LEGITVAR_TYPE(iWeaponType, iAimHitbox) - 1)), arrCustomMatrix);
}

void CBacktracking::ApplyData(Record_t record, CBaseEntity* pEntity)
{
	m_orgData.flSimtime		= pEntity->GetSimulationTime();
	m_orgData.vecOrigin		= pEntity->GetOrigin();
	m_orgData.vecAbsOrigin	= pEntity->GetAbsOrigin();
	m_orgData.vecMins		= pEntity->GetVecMins();
	m_orgData.vecMaxs		= pEntity->GetVecMaxs();
	m_orgData.vecViewOffset = pEntity->GetViewOffset();
	m_orgData.angAbsAngles	= pEntity->GetAbsAngles();
	
	pEntity->SetAbsOrigin(record.vecAbsOrigin);
	pEntity->SetAbsAngles(record.angAbsAngles);
	pEntity->GetSimulationTime() = record.flSimtime;
	pEntity->GetOrigin()		 = record.vecOrigin;
	pEntity->GetVecMins()		 = record.vecMins;
	pEntity->GetVecMaxs()		 = record.vecMaxs;
	pEntity->GetViewOffset()	 = record.vecViewOffset;
}

void CBacktracking::RestoreData(CBaseEntity* pEntity)
{
	pEntity->SetAbsOrigin(m_orgData.vecAbsOrigin);
	pEntity->SetAbsAngles(m_orgData.angAbsAngles);
	pEntity->GetSimulationTime() = m_orgData.flSimtime;
	pEntity->GetOrigin() = m_orgData.vecOrigin;
	pEntity->GetVecMins() = m_orgData.vecMins;
	pEntity->GetVecMaxs() = m_orgData.vecMaxs;
	pEntity->GetViewOffset() = m_orgData.vecViewOffset;
}

std::deque<Record_t> CBacktracking::GetPlayerRecord(int iIndex)
{
	return m_arrRecords[iIndex];
}
