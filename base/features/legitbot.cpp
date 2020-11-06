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

	if (!GetTargetData(pLocal, m_Target))
		return;

	if (!CanShoot(pLocal, pCmd))
		return;

	FinalizeAngle(pLocal, m_Target.angAngle);

	if (ShouldShoot(pLocal, pCmd))
		ApplyAngle(pCmd, m_Target.angAngle);
}

bool CLegitBot::GetTargetData(CBaseEntity* pLocal, Target_t& Target)
{	
	Target.pEntity = GetBestEntity(pLocal);
	if (Target.pEntity == nullptr)
		return false;

	Target.vecHitboxPos = GetBestHitbox(pLocal, Target.pEntity);
	Target.angAngle = M::CalcAngle(m_vecLocalEyePos, Target.vecHitboxPos).Clamped();
	Target.flAngleDelta = (Target.angAngle - m_angLocalViewAngles).Clamped().Length();
	Target.iIndex = Target.pEntity->GetIndex();
	Target.bShouldAimAtBacktrack = false;

	std::deque<Record_t> deqRecords = CBacktracking::Get().GetPlayerRecord(Target.iIndex);
	if (deqRecords.empty() || !m_WeaponVars.bAimAtBacktrack) // if we shouldn't aim at backtrack, return
		return true;

	for (auto& record : deqRecords)
	{
		if (!CBacktracking::Get().IsValid(record.flSimtime))
			continue;

		QAngle angRecordAngle = M::CalcAngle(m_vecLocalEyePos, record.vecHitboxPos).Clamped();
		float flRecordAngleDelta = (angRecordAngle - m_angLocalViewAngles).Clamped().Length();	

		if (flRecordAngleDelta < Target.flAngleDelta)
		{
			Target.bShouldAimAtBacktrack = true;
			Target.Record = record;
			Target.vecHitboxPos = record.vecHitboxPos;
			Target.angAngle = angRecordAngle;
			Target.flAngleDelta = flRecordAngleDelta;
		}
	}
	return true;
}

CBaseEntity* CLegitBot::GetBestEntity(CBaseEntity* pLocal)
{
	CBaseEntity* pBestEntity = nullptr;
	float flBestDelta = m_WeaponVars.flAimFov;

	for (int i = 1; i <= I::Globals->nMaxClients; i++)
	{
		if (i == I::Engine->GetLocalPlayer())
			continue;

		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);

		if (!IsValid(pLocal, pEntity))
			continue;

		QAngle angAngle = (M::CalcAngle(m_vecLocalEyePos, pEntity->GetEyePosition(false)) - pLocal->GetPunch() * m_weapon_recoil_scale->GetFloat()).Clamped();
		float flDelta = (angAngle - m_angLocalViewAngles).Clamped().Length();

		if (flDelta <= flBestDelta)
		{
			flBestDelta = flDelta;
			pBestEntity = pEntity;
		}
	}

	return pBestEntity;
}

Vector CLegitBot::GetBestHitbox(CBaseEntity* pLocal, CBaseEntity* pEntity)
{
	float iBestDelta = std::numeric_limits<float>::max();

	if (m_WeaponVars.iAimHitbox == (int)ELegitHitboxes::CLOSEST)
	{
		Vector vecBestHitboxPos{ };

		for (int i = 0; i < m_arrClosestHitboxes.size(); i++)
		{
				Vector vecHitboxPos = pEntity->GetHitboxPosition(m_arrClosestHitboxes.at(i));
				QAngle angAngle = M::CalcAngle(m_vecLocalEyePos, vecHitboxPos).Clamped();
				float flDelta = (angAngle - m_angLocalViewAngles).Clamped().Length();

				if (flDelta < iBestDelta)
				{
					vecBestHitboxPos = vecHitboxPos;
					iBestDelta = flDelta;
				}
		}

		return vecBestHitboxPos;
	}
	else
		return pEntity->GetHitboxPosition(m_arrHitboxes.at(m_WeaponVars.iAimHitbox - 1));
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
		//if (pCmd->iButtons & IN_ATTACK)
		//	CBacktracking::Get().DrawHitbox(m_Target.Record.arrMatrix, m_Target.Record.pModel);
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

	if (IPT::IsKeyDown(m_WeaponVars.iAimKey) || (nDefinitionIndex == WEAPON_REVOLVER && pCmd->iButtons & IN_SECOND_ATTACK)) // also aimbot with the revolver's mouse2
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


bool CLegitBot::IsValid(CBaseEntity* pLocal, CBaseEntity* pEntity)
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

	bool bIsVisible = pLocal->IsVisible(pEntity, pEntity->GetEyePosition(false));

	if (!bIsVisible && !m_WeaponVars.bAimAutoWall) // only aim at non-visible players if autowall is enabled
		return false;

	float flDamage = CAutoWall::Get().GetDamage(pLocal, GetBestHitbox(pLocal, pEntity));

	if (!bIsVisible && flDamage < m_WeaponVars.iAimAutoWallMinDamage) // only aim at non-visible players if damaga > mindamage
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