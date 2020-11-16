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
	if (pLocal == nullptr || !pLocal->IsAlive())
		return;

	int iWeaponType = CLegitBot::Get().GetWeaponType(pLocal);
	if (iWeaponType == -1)
		return;

	LegitbotVariables_t WeaponVars = C::Get<std::vector<LegitbotVariables_t>>(Vars.vecLegitVars)[iWeaponType];
	if (C::Get<bool>(Vars.bLegit) && WeaponVars.bAimAtBacktrack && CLegitBot::Get().m_Target.bShouldAimAtBacktrack) // if we are currently aiming at a record
		return;

	CBaseEntity* pEntity = GetBestEntity(pLocal);
	if (pEntity == nullptr)
		return;

	Record_t bestRecord = GetBestRecord(pLocal, pEntity->GetIndex());
	if (!IsValid(bestRecord.flSimtime))
		return;

	if (pCmd->iButtons & IN_ATTACK)
		pCmd->iTickCount = TIME_TO_TICKS(bestRecord.flSimtime + GetLerp());
}

void CBacktracking::Update(CBaseEntity* pLocal)
{
	m_cl_updaterate = I::ConVar->FindVar(XorStr("cl_updaterate"));
	m_sv_minupdaterate = I::ConVar->FindVar(XorStr("sv_minupdaterate"));
	m_sv_maxupdaterate = I::ConVar->FindVar(XorStr("sv_maxupdaterate"));
	m_cl_interp = I::ConVar->FindVar(XorStr("cl_interp"));
	m_cl_interp_ratio = I::ConVar->FindVar(XorStr("cl_interp_ratio"));
	m_sv_client_min_interp_ratio = I::ConVar->FindVar(XorStr("sv_client_min_interp_ratio"));
	m_sv_client_max_interp_ratio = I::ConVar->FindVar(XorStr("sv_client_max_interp_ratio"));
	m_sv_maxunlag = I::ConVar->FindVar(XorStr("sv_maxunlag"));

	if (!C::Get<bool>(Vars.bBacktracking) || pLocal == nullptr || !pLocal->IsAlive())
	{
		for (auto& record : m_arrRecords)
			record.clear();

		return;
	}

	for (int i = 1; i <= I::Globals->nMaxClients; i++)
	{
		if (i == I::Engine->GetLocalPlayer())
			continue;

		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);

		if (pEntity == nullptr || !pEntity->IsPlayer() || !pEntity->IsAlive() || pEntity->IsDormant() || pEntity->HasImmunity())
		{
			m_arrRecords[i].clear();
			continue;
		}

		if (!m_arrRecords[i].empty() && m_arrRecords[i].front().flSimtime == pEntity->GetSimulationTime())
			continue;

		Record_t Record = { };

		if (!pEntity->SetupBonesFixed(Record.arrBoneMatrixes, BONE_USED_BY_ANYTHING, I::Globals->flCurrentTime))
			continue;

		Record.flSimtime	 = pEntity->GetSimulationTime();
		Record.vecOrigin	 = pEntity->GetOrigin();
		Record.vecAbsOrigin	 = pEntity->GetAbsOrigin();
		Record.pStudioModel	 = I::ModelInfo->GetStudioModel(pEntity->GetModel());
		Record.vecHeadPos	 = pEntity->GetHitboxPosition(HITBOX_HEAD, Record.arrBoneMatrixes, Record.pStudioModel);	

		m_arrRecords[i].push_front(Record);
		
		if (auto itInvalid = std::find_if(std::cbegin(m_arrRecords[i]), std::cend(m_arrRecords[i]),
		[&](const Record_t& rec)
		{
			return !CBacktracking::Get().IsValid(rec.flSimtime);
		}); 
		itInvalid != std::cend(m_arrRecords[i]))
		{
			m_arrRecords[i].erase(itInvalid, std::cend(m_arrRecords[i]));
		}

		while (m_arrRecords[i].size() > TIME_TO_TICKS((C::Get<int>(Vars.iBacktrackingTime) + ((C::Get<bool>(Vars.bMiscFakeLatency)) ? C::Get<int>(Vars.iMiscFakeLatencyAmount) : 0.f )) / 1000.f))
		{
			m_arrRecords[i].pop_back();
		}
	}
}

float CBacktracking::GetLerp()
{	
	int iUpdateRate = 64;
	if (m_cl_updaterate)
		iUpdateRate = m_cl_updaterate->GetInt();

	if (m_sv_minupdaterate && m_sv_maxupdaterate)
		iUpdateRate = m_sv_maxupdaterate->GetInt();

	float flRatio = 1.f; 
	if (m_cl_interp_ratio)
		flRatio = m_cl_interp_ratio->GetFloat();
	
	float flLerp = I::Globals->flIntervalPerTick;
	if (m_cl_interp)
		float flLerp = m_cl_interp->GetFloat();

	if (m_sv_client_min_interp_ratio && m_sv_client_max_interp_ratio && m_sv_client_min_interp_ratio->GetFloat() != 1)
		flRatio = std::min(std::max(flRatio, m_sv_client_min_interp_ratio->GetFloat()), m_sv_client_max_interp_ratio->GetFloat());

	return std::max(flLerp, (flRatio / iUpdateRate));
}

bool CBacktracking::IsValid(float flSimtime)
{
	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (pLocal == nullptr)
		return false;

	float flCurTime = TICKS_TO_TIME(pLocal->GetTickBase());

	float flCorrect = 0.f;

	flCorrect += GetLatency(true, true); // add the total latency

	flCorrect += GetLerp(); // add interpolation

	if (C::Get<bool>(Vars.bMiscFakeLatency))
		flCorrect += C::Get<int>(Vars.iMiscFakeLatencyAmount); // add the fake latency

	flCorrect = std::clamp(flCorrect, 0.f, m_sv_maxunlag->GetFloat()); // clamp to unlag

	float flDeltaTime = flCorrect - (flCurTime - flSimtime);
	if (flSimtime < std::floorf(flCurTime - m_sv_maxunlag->GetFloat())) // account for deadtime
		return false;

	return std::fabs(flDeltaTime) <= 0.2f;
}

void CBacktracking::DrawHitbox(std::array<matrix3x4_t, MAXSTUDIOBONES> arrMatrix, studiohdr_t* pStudioModel)
{
	if (pStudioModel == nullptr)
		return;

	mstudiohitboxset_t* pSet = pStudioModel->GetHitboxSet(0);
	if (pSet == nullptr)
		return;

	static CConVar* sv_showlagcompensation_duration = I::ConVar->FindVar("sv_showlagcompensation_duration");

	for (int i = 0; i < pSet->nHitboxes; i++)
	{
		mstudiobbox_t* pHitbox = pSet->GetHitbox(i);

		if (pHitbox == nullptr)
			continue;

		Vector vecMin = M::VectorTransform(pHitbox->vecBBMin, arrMatrix.at(pHitbox->iBone));
		Vector vecMax = M::VectorTransform(pHitbox->vecBBMax, arrMatrix.at(pHitbox->iBone));

		I::DebugOverlay->DrawPill(vecMin, vecMax, pHitbox->flRadius, 255, 255, 255, 255, sv_showlagcompensation_duration->GetFloat(), 0, 0);
	}
}

CBaseEntity* CBacktracking::GetBestEntity(CBaseEntity* pLocal)
{
	CBaseEntity* pBestEntity = nullptr;
	float flBestDelta = std::numeric_limits<float>::max();
	Vector vecLocalEyePos = pLocal->GetEyePosition();
	QAngle angLocalViewAngles = I::Engine->GetViewAngles();

	for (int i = 1; i <= I::Globals->nMaxClients; i++)
	{
		if (i == I::Engine->GetLocalPlayer())
			continue;

		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);

		if (pEntity == nullptr || !pEntity->IsPlayer() || !pEntity->IsAlive() || pEntity->IsDormant() || pEntity->HasImmunity())
			continue;

		QAngle angAngle = M::CalcAngle(vecLocalEyePos, pEntity->GetEyePosition(false)).Clamped();
		float flDelta = (angAngle - angLocalViewAngles).Clamped().Length();

		if (flDelta < flBestDelta)
		{
			flBestDelta = flDelta;
			pBestEntity = pEntity;
		}
	}

	return pBestEntity;
}

Record_t CBacktracking::GetBestRecord(CBaseEntity* pLocal, int iIndex)
{
	Record_t bestRecord = { };
	float flBestDelta = std::numeric_limits<float>::max();
	static Vector vecLocalEyePos = pLocal->GetEyePosition();
	static QAngle angLocalViewAngles = I::Engine->GetViewAngles();

	for (auto& record : m_arrRecords[iIndex])
	{
		if (!IsValid(record.flSimtime))
			continue;

		QAngle angAngle = M::CalcAngle(vecLocalEyePos, record.vecHeadPos).Clamped();
		float flDelta = (angAngle - angLocalViewAngles).Clamped().Length();

		if (flDelta < flBestDelta)
		{
			flBestDelta = flDelta;
			bestRecord = record;
		}
	}

	return bestRecord;
}

float CBacktracking::GetLatency(bool bOutgoing, bool bIncoming)
{
	INetChannelInfo* pNetChannelInfo = I::Engine->GetNetChannelInfo();
	if (pNetChannelInfo == nullptr)
		return 0.f;

	float flTotal = 0.f;

	if (bOutgoing)
		flTotal += pNetChannelInfo->GetAvgLatency(FLOW_OUTGOING);

	if (bIncoming)
		flTotal += pNetChannelInfo->GetAvgLatency(FLOW_INCOMING);

	return flTotal;
}

void CBacktracking::ApplyData(Record_t record, CBaseEntity* pEntity)
{
	m_orgData.flSimtime		= pEntity->GetSimulationTime();
	m_orgData.vecOrigin		= pEntity->GetOrigin();
	m_orgData.vecAbsOrigin	= pEntity->GetAbsOrigin();
	
	pEntity->SetAbsOrigin(record.vecAbsOrigin);
	pEntity->GetOrigin() = record.vecOrigin;
	pEntity->GetSimulationTime() = record.flSimtime;
}

void CBacktracking::RestoreData(CBaseEntity* pEntity)
{
	pEntity->SetAbsOrigin(m_orgData.vecAbsOrigin);
	pEntity->GetOrigin() = m_orgData.vecOrigin;
	pEntity->GetSimulationTime() = m_orgData.flSimtime;
}

void CBacktracking::GetValidRecords(int iIndex, std::deque<Record_t>& deqRecords)
{
	for (auto& record : m_arrRecords[iIndex])
	{
		if (!IsValid(record.flSimtime))
			continue;

		deqRecords.push_front(record);
	}
}

void CBacktracking::Draw()
{
}
