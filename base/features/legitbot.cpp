#include "legitbot.h"

// used: main window open state
#include "../core/menu.h"
// used: key state
#include "../utilities/inputsystem.h"
// used: globals, cliententitylist interfaces
#include "../core/interfaces.h"
// used: CalcAngle
#include "../utilities/math.h"
// used: autowall
#include "autowall.h"
// used: drawing features for debugging pourposes
#include "../utilities/draw.h"
// used: GetTickbase()
#include "../features/prediction.h"
#include "../utilities/logging.h"

void CLegitBot::Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket)
{
	if (!W::bMainOpened && GetWeaponType(pLocal) != -1)
		 C::Get<int>(Vars.iLegitWeapon) = m_iWeaponType = GetWeaponType(pLocal); // set the menu weapon as the one we're holding

	m_WeaponVars		 = C::Get<std::vector<LegitbotVariables_t>>(Vars.vecLegitVars)[m_iWeaponType];
	m_angLocalViewAngles = I::Engine->GetViewAngles();
	m_vecLocalEyePos	 = pLocal->GetEyePosition();

	if (!ShouldRun(pLocal))
		return;

	if (!GetTargetData(pLocal))
		return;

	if (!CanShoot(pLocal, pCmd))
		return;

	FinalizeAngle(pLocal, m_Target.angAngle);

	if (ShouldShoot(pLocal, pCmd))
		ApplyAngle(pCmd, m_Target.angAngle);
}

bool CLegitBot::GetTargetData(CBaseEntity* pLocal)
{
	m_Target.pEntity = GetBestEntity(pLocal);
	if(m_Target.pEntity == nullptr)
		return false;

	if (!m_Target.pEntity->SetupBonesFixed(m_Target.arrBoneMatrixes, BONE_USED_BY_HITBOX, I::Globals->flCurrentTime))
		return false;

	m_Target.vecHitboxPos = GetBestHitbox(pLocal);
	m_Target.angAngle = M::CalcAngle(m_vecLocalEyePos, m_Target.vecHitboxPos).Clamped();
	m_Target.flAngleDelta = (m_Target.angAngle - m_angLocalViewAngles).Clamped().Length();
	m_Target.iIndex = m_Target.pEntity->GetIndex();
	m_Target.bShouldAimAtBacktrack = false;

	std::deque<Record_t> deqRecords = { }; CBacktracking::Get().GetValidRecords(m_Target.iIndex, deqRecords);
	if (!deqRecords.empty() && m_WeaponVars.bAimAtBacktrack)
	{
		for (auto& Record : deqRecords)
		{
			Vector vecHitboxPosition = GetBestHitbox(pLocal, Record);
			QAngle angRecordAngle = M::CalcAngle(m_vecLocalEyePos, vecHitboxPosition).Clamped();
			float flRecordAngleDelta = (angRecordAngle - m_angLocalViewAngles).Clamped().Length();

			if (flRecordAngleDelta < m_Target.flAngleDelta)
			{
				m_Target.bShouldAimAtBacktrack = true;
				m_Target.Record = Record;
				m_Target.vecHitboxPos = vecHitboxPosition;
				m_Target.angAngle = angRecordAngle;
				m_Target.flAngleDelta = flRecordAngleDelta;
			}
		}
	}

	if (m_Target.flAngleDelta > m_WeaponVars.flAimFov)
		return false;

	return true;
}

CBaseEntity* CLegitBot::GetBestEntity(CBaseEntity* pLocal)
{
	CBaseEntity* pBestEntity = nullptr;
	float flBestDelta = std::numeric_limits<float>::max();

	for (int i = 1; i <= I::Globals->nMaxClients; i++)
	{
		if (i == I::Engine->GetLocalPlayer())
			continue;

		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);

		if (!IsValid(pEntity))
			continue;

		QAngle angAngle = (M::CalcAngle(m_vecLocalEyePos, pEntity->GetEyePosition(false)) - pLocal->GetPunch() * m_weapon_recoil_scale->GetFloat()).Clamped();
		float flDelta = (angAngle - m_angLocalViewAngles).Clamped().Length();

		if (flDelta < flBestDelta)
		{
			flBestDelta = flDelta;
			pBestEntity = pEntity;
		}
	}

	return pBestEntity;
}

Vector CLegitBot::GetBestHitbox(CBaseEntity* pLocal)
{
	Vector vecBestHitboxPos = { };

	switch (m_WeaponVars.iAimHitbox)
	{
	case (int)ELegitHitboxes::CLOSEST:
	{
		float flBestDelta = std::numeric_limits<float>::max();

		for (int i = 0; i < arrClosestHitboxes.size(); i++)
		{
			Vector vecHitboxPos = m_Target.pEntity->GetHitboxPosition(arrClosestHitboxes[i], m_Target.arrBoneMatrixes);
			QAngle angAngle = (M::CalcAngle(m_vecLocalEyePos, vecHitboxPos) - pLocal->GetPunch() * m_weapon_recoil_scale->GetFloat()).Clamped();
			float flDelta = (angAngle - m_angLocalViewAngles).Clamped().Length();

			if (flDelta < flBestDelta)
			{
				vecBestHitboxPos = vecHitboxPos;
				flBestDelta = flDelta;
			}
		}

		break;
	}
	default:
		vecBestHitboxPos = m_Target.pEntity->GetHitboxPosition(arrHitboxes[m_WeaponVars.iAimHitbox - 1], m_Target.arrBoneMatrixes);
	}

	if (m_WeaponVars.iAimPriorityHitbox) // if we have a hitbox to prioritize
	{
		Vector vecPriorityHitboxPos = m_Target.pEntity->GetHitboxPosition(arrHitboxes[m_WeaponVars.iAimPriorityHitbox - 1], m_Target.arrBoneMatrixes);
		QAngle angAngle = (M::CalcAngle(m_vecLocalEyePos, vecPriorityHitboxPos) - pLocal->GetPunch() * m_weapon_recoil_scale->GetFloat()).Clamped();
		float flDelta = (angAngle - m_angLocalViewAngles).Clamped().Length();

		bool bIsVisible = pLocal->IsVisible(m_Target.pEntity, vecPriorityHitboxPos);
		float flDamage = CAutoWall::Get().GetDamage(pLocal, vecPriorityHitboxPos);

		if (flDelta <= m_WeaponVars.flAimFov && (bIsVisible || flDamage >= m_WeaponVars.iAimAutoWallMinDamage)) // if it's valid, use that
			return vecPriorityHitboxPos;
	}

	return vecBestHitboxPos;
}

Vector CLegitBot::GetBestHitbox(CBaseEntity* pLocal, Record_t Record)
{
	Vector vecBestHitboxPos = { };

	switch (m_WeaponVars.iAimHitbox)
	{
	case (int)ELegitHitboxes::CLOSEST:
	{
		float flBestDelta = std::numeric_limits<float>::max();

		for (int i = 0; i < arrClosestHitboxes.size(); i++)
		{
			Vector vecHitboxPos = m_Target.pEntity->GetHitboxPosition(arrClosestHitboxes[i], Record.arrBoneMatrixes, Record.pStudioModel);
			QAngle angAngle = M::CalcAngle(m_vecLocalEyePos, vecHitboxPos).Clamped();
			float flDelta = (angAngle - m_angLocalViewAngles).Clamped().Length();

			if (flDelta < flBestDelta)
			{
				vecBestHitboxPos = vecHitboxPos;
				flBestDelta = flDelta;
			}
		}

		break;
	}
	default:
		vecBestHitboxPos = m_Target.pEntity->GetHitboxPosition(arrHitboxes[m_WeaponVars.iAimHitbox - 1], Record.arrBoneMatrixes, Record.pStudioModel);
	}

	if (m_WeaponVars.iAimPriorityHitbox) // if we have a hitbox to prioritize
	{
		Vector vecPriorityHitboxPos = m_Target.pEntity->GetHitboxPosition(arrHitboxes[m_WeaponVars.iAimPriorityHitbox - 1], Record.arrBoneMatrixes, Record.pStudioModel);
		QAngle angAngle = (M::CalcAngle(m_vecLocalEyePos, vecPriorityHitboxPos) - pLocal->GetPunch() * m_weapon_recoil_scale->GetFloat()).Clamped();
		float flDelta = (angAngle - m_angLocalViewAngles).Clamped().Length();

		if (flDelta <= m_WeaponVars.flAimFov) // if it's valid, use that, no traces for now since i don't restore enough data for backtrack
			return vecPriorityHitboxPos;
	}

	return vecBestHitboxPos;
}

void CLegitBot::FinalizeAngle(CBaseEntity* pLocal, QAngle& angAngle)
{
	angAngle.Normalize(); 
	angAngle.Clamp();

	if (m_WeaponVars.bAimRCS)
	{
		angAngle -= pLocal->GetPunch() * m_weapon_recoil_scale->GetFloat();
		angAngle.Clamp();
	}

	if (!m_WeaponVars.bAimSilent && m_WeaponVars.flAimSmooth > 1.f)
	{
		angAngle = m_angLocalViewAngles + (angAngle - m_angLocalViewAngles).Clamped() / m_WeaponVars.flAimSmooth;
		angAngle.Clamp();
	}
}

void CLegitBot::ApplyAngle(CUserCmd* pCmd, QAngle angAngle)
{
	if (m_Target.bShouldAimAtBacktrack)
	{
		CBacktracking::Get().ApplyData(m_Target.Record, m_Target.pEntity);
		pCmd->iTickCount = TIME_TO_TICKS(m_Target.Record.flSimtime + CBacktracking::Get().GetLerp());
		//if (pCmd->iButtons & IN_ATTACK)	CBacktracking::Get().DrawHitbox(m_Target.Record.arrBoneMatrixes, m_Target.Record.pStudioModel);
	}

	pCmd->angViewPoint = angAngle;
	if (!m_WeaponVars.bAimSilent)
		I::Engine->SetViewAngles(angAngle);

	if (m_Target.bShouldAimAtBacktrack)
		CBacktracking::Get().RestoreData(m_Target.pEntity);
}

bool CLegitBot::ShouldShoot(CBaseEntity* pLocal, CUserCmd* pCmd)
{
	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();
	if (pWeapon == nullptr)
		return false;

	short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	bool bIsVisible = pLocal->IsVisible(m_Target.pEntity, m_Target.vecHitboxPos);

	float flDamage = CAutoWall::Get().GetDamage(pLocal, m_Target.vecHitboxPos);

	if ((bIsVisible || flDamage >= m_WeaponVars.iAimAutoWallMinDamage) && (IPT::IsKeyDown(m_WeaponVars.iAimKey) || (nDefinitionIndex == WEAPON_REVOLVER && pCmd->iButtons & IN_SECOND_ATTACK)))
		return true;
	
	return false;
}

bool CLegitBot::CanShoot(CBaseEntity* pLocal, CUserCmd* pCmd)
{
	CWeaponCSBase* pWeapon = static_cast<CWeaponCSBase*>(pLocal->GetWeapon());
	if (pWeapon == nullptr)
		return false;

	short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);
	if (pWeaponData == nullptr)
		return false;

	float flServerTime = TICKS_TO_TIME(pLocal->GetTickBase());

	if (pWeapon->GetAmmo() <= 0) // only aim with weapons that have ammo
		return false;

	if (pWeapon->IsReloading()) // don't aim while reloading @test: pretty sure GetNextAttack() handles this
		return false;

	if (nDefinitionIndex == WEAPON_REVOLVER && pWeapon->GetFireReadyTime() > flServerTime && !(pCmd->iButtons & IN_SECOND_ATTACK)) // don't aim while cocking the r8
		return false;

	if (pLocal->GetNextAttack() > flServerTime) // don't aim while doing animations like pulling out the weapon etc
		return false;

	if (!pWeaponData->bFullAuto && pWeapon->GetNextPrimaryAttack() > flServerTime && nDefinitionIndex != WEAPON_REVOLVER) // only aim between shots if the weapon is full auto (except r8)
		return false;

	return true;
}


bool CLegitBot::IsValid(CBaseEntity* pEntity)
{
	if (pEntity == nullptr)
		return false;

	if (!pEntity->IsAlive()) // only aim at alive players
		return false;

	if (pEntity->IsDormant()) // only aim at non-dormant players
		return false;

	if (pEntity->HasImmunity()) // only aim at non-immune players
		return false;

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (pLocal == nullptr)
		return false;

	if (!pLocal->IsEnemy(pEntity)) // only aim at enemies
		return false;

	if (!pLocal->IsVisible(pEntity, pEntity->GetEyePosition(false)) && !m_WeaponVars.bAimAutoWall) // only aim at non-visible players if autowall is enabled
		return false;

	return true;
}

bool CLegitBot::ShouldRun(CBaseEntity* pLocal)
{
	if (W::bMainOpened)
		return false;

	if (m_WeaponVars.iAimKey == 0)
		return false;

	return true;
}

int CLegitBot::GetWeaponType(CBaseEntity* pLocal)
{
	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();
	if (pWeapon == nullptr)
		return -1;

	short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();

	CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);
	if (pWeaponData == nullptr)
		return -1;

	switch (pWeaponData->nWeaponType)
	{
	case WEAPONTYPE_RIFLE:
		return (int)ELegitWeaponTypes::RIFLES;
	case WEAPONTYPE_SNIPER:
		return (int)ELegitWeaponTypes::SNIPERS;
	case WEAPONTYPE_PISTOL:
		return nDefinitionIndex == WEAPON_REVOLVER || nDefinitionIndex == WEAPON_DEAGLE ? (int)ELegitWeaponTypes::HEAVY_PISTOLS : (int)ELegitWeaponTypes::PISTOLS;
	case WEAPONTYPE_SUBMACHINEGUN:
		return (int)ELegitWeaponTypes::SMGS;
	default:
		return (int)ELegitWeaponTypes::OTHER;
	}
}

void CLegitBot::Draw()
{

	const ImVec2 vecScreenSize = ImGui::GetIO().DisplaySize;
	/*
	if (C::Get<bool>(Vars.bLegitDrawFOV))
	{
		float flRadius = std::tanf(M_DEG2RAD(m_WeaponVars.flAimFov) / 2) / std::tanf(M_DEG2RAD(G::flFov) / 2) * vecScreenSize.x;
		D::AddCircle(ImVec2(vecScreenSize.x / 2.f, vecScreenSize.y / 2.f), flRadius, Color(75, 75, 75, 120), 20, IMGUI_CIRCLE_FILLED);
	}
	//draw current aimpoint
	Vector2D vecScreen = { };
	if (!D::WorldToScreen(m_Target.vecHitboxPos, vecScreen))
		return;

	D::AddRect(ImVec2(vecScreen.x - 1.f, vecScreen.y - 1.f), ImVec2(vecScreen.x + 1.f, vecScreen.y + 1.f), Color(255, 255, 255, 255), IMGUI_RECT_FILLED);
	*/
}