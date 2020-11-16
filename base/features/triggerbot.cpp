#include "triggerbot.h"

// used: cheat variables
#include "../core/variables.h"
// used: max distance, classindex, itemdefinitionindex
#include "../sdk/definitions.h"
// used: angle/vector calculations
#include "../utilities/math.h"
// used: is key pressed for bind
#include "../utilities/inputsystem.h"
// used: firebullet data
#include "autowall.h"
/* used: canshoot */
#include "legitbot.h"

void CTriggerBot::Run(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive())
		return;

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();
	if (pWeapon == nullptr)
		return;

	short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();
	CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);
	if (pWeaponData == nullptr || !pWeaponData->IsGun())
		return;

	// check is using key activation and key held
	if (C::Get<int>(Vars.iTriggerKey) == 0 || !IPT::IsKeyDown(C::Get<int>(Vars.iTriggerKey)))
	{
		m_timer.Reset();
		return;
	}

	// get view and add punch
	QAngle angView = pCmd->angViewPoint;
	angView += pLocal->GetPunch() * m_weapon_recoil_scale->GetFloat();

	Vector vecStart, vecEnd, vecForward;
	M::AngleVectors(angView, &vecForward);

	vecStart = pLocal->GetEyePosition();
	vecForward *= pWeaponData->flRange;
	vecEnd = vecStart + vecForward;

	Trace_t trace = { };
	if (C::Get<bool>(Vars.bTriggerAutoWall))
	{
		FireBulletData_t data = { };

		// get autowall damage and data from it
		float flDamage = CAutoWall::Get().GetDamage(pLocal, vecEnd, data);

		// check for minimal damage
		if (flDamage < C::Get<int>(Vars.iTriggerMinimumDamage))
			return;

		// copy trace from autowall
		trace = data.enterTrace;
	}
	else
	{
		// otherwise ray new trace
		Ray_t ray(vecStart, vecEnd);
		CTraceFilter filter(pLocal);
		I::EngineTrace->TraceRay(ray, MASK_SHOT, &filter, &trace);
	}

	m_arrHitBoxes.clear(); 

	if (C::Get<bool>(Vars.bTriggerHead))
		m_arrHitBoxes.push_back(HITBOX_HEAD);

	if (C::Get<bool>(Vars.bTriggerChest))
		m_arrHitBoxes.push_back(HITBOX_CHEST);

	if (C::Get<bool>(Vars.bTriggerStomach))
		m_arrHitBoxes.push_back(HITBOX_STOMACH);

	if (!CLegitBot::Get().CanShoot(pLocal, pCmd))
		return;

	if (C::Get<bool>(Vars.bTriggerMagnet))
		MagnetTrigger(pCmd, pLocal);

	CBaseEntity* pTraceEntity = trace.pHitEntity;

	// check is trace player valid and enemy
	if (pTraceEntity == nullptr || !pTraceEntity->IsAlive() || pTraceEntity->IsDormant() || !pTraceEntity->IsPlayer() || pTraceEntity->HasImmunity() || !pLocal->IsEnemy(pTraceEntity))
	{
		m_timer.Reset();
		return;
	}

	// hitgroup filters check
		// head
	if ((C::Get<bool>(Vars.bTriggerHead) && trace.iHitGroup == HITGROUP_HEAD) ||
		// chest
		(C::Get<bool>(Vars.bTriggerChest) && trace.iHitGroup == HITGROUP_CHEST) ||
		// stomach
		(C::Get<bool>(Vars.bTriggerStomach) && trace.iHitGroup == HITGROUP_STOMACH))
	{
		// check is delay elapsed
		if (C::Get<int>(Vars.iTriggerDelay) > 0)
		{
			if (m_timer.Elapsed() < C::Get<int>(Vars.iTriggerDelay))
				return;
		}

		pCmd->iButtons |= IN_ATTACK;
	}
}

void CTriggerBot::MagnetTrigger(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	Vector vecLocalEyePos = pLocal->GetEyePosition();
	QAngle angLocalViewAngles = I::Engine->GetViewAngles();

	CBaseEntity* pEntity = GetClosestEntity(pLocal, angLocalViewAngles, vecLocalEyePos);
	if (pEntity == nullptr)
		return;

	Vector vecHitBoxPos = GetBestHitbox(pLocal, pEntity, angLocalViewAngles, vecLocalEyePos);
	QAngle angAngle = M::CalcAngle(vecLocalEyePos, vecHitBoxPos).Clamped();
	float flDelta = (angAngle - angLocalViewAngles).Clamped().Length();

	FinalizeAngle(pLocal, angAngle, angLocalViewAngles, flDelta);

	pCmd->angViewPoint = angAngle;
	I::Engine->SetViewAngles(angAngle);
}

CBaseEntity* CTriggerBot::GetClosestEntity(CBaseEntity* pLocal, QAngle angLocalViewAngles, Vector vecLocalEyePos)
{
	CBaseEntity* pBestEntity = nullptr;
	float flBestDelta = C::Get<float>(Vars.flTriggerMagnetFov);

	for (int i = 1; i <= I::Globals->nMaxClients; i++)
	{
		if (i == I::Engine->GetLocalPlayer())
			continue;

		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);

		if (pEntity == nullptr || !pEntity->IsAlive() ||pEntity->IsDormant() || !pLocal->IsEnemy(pEntity) || pEntity->HasImmunity() || 
			!(pLocal->IsVisible(pEntity, pEntity->GetEyePosition(false)) && !C::Get<bool>(Vars.bTriggerAutoWall)))
			continue;

		QAngle angAngle = (M::CalcAngle(vecLocalEyePos, pEntity->GetEyePosition(false)) - pLocal->GetPunch() * m_weapon_recoil_scale->GetFloat()).Clamped();
		float flDelta = (angAngle - angLocalViewAngles).Clamped().Length();

		if (flDelta < flBestDelta)
		{
			flBestDelta = flDelta;
			pBestEntity = pEntity;
		}
	}

	return pBestEntity;
}

Vector CTriggerBot::GetBestHitbox(CBaseEntity* pLocal, CBaseEntity* pEntity, QAngle angLocalViewAngles, Vector vecLocalEyePos)
{
	float flBestDelta = std::numeric_limits<float>::max();
	Vector vecBestHitboxPos = { };

	std::array<matrix3x4_t, MAXSTUDIOBONES> arrCustomMatrix = { };
	if (!pEntity->SetupBonesFixed(arrCustomMatrix, BONE_USED_BY_HITBOX, I::Globals->flCurrentTime))
		return { };

	for (int i = 0; i < m_arrHitBoxes.size(); i++)
	{
		Vector vecHitboxPos = pEntity->GetHitboxPosition(m_arrHitBoxes[i], arrCustomMatrix);
		QAngle angAngle = (M::CalcAngle(vecLocalEyePos, vecHitboxPos) - pLocal->GetPunch() * m_weapon_recoil_scale->GetFloat()).Clamped();
		float flDelta = (angAngle - angLocalViewAngles).Clamped().Length();

		if (!pLocal->IsVisible(pEntity, vecHitboxPos) && CAutoWall::Get().GetDamage(pLocal, vecHitboxPos) < C::Get<int>(Vars.iTriggerMinimumDamage))
			continue;

		if (flDelta < flBestDelta)
		{
			vecBestHitboxPos = vecHitboxPos;
			flBestDelta = flDelta;
		}
	}

	return vecBestHitboxPos;
}

void CTriggerBot::FinalizeAngle(CBaseEntity* pLocal, QAngle& angAngle, QAngle angLocalViewAngles, float flDelta)
{
	angAngle.Normalize();
	angAngle.Clamp();

	if (C::Get<bool>(Vars.bTriggerMagnetRCS))
	{
		angAngle -= pLocal->GetPunch() * m_weapon_recoil_scale->GetFloat();	
		angAngle.Clamp();
	}

	if (C::Get<float>(Vars.flTriggerMagnetSmooth) > 1.f && flDelta > 1.f)
	{
		angAngle = angLocalViewAngles + (angAngle - angLocalViewAngles).Clamped() / C::Get<float>(Vars.flTriggerMagnetSmooth);
		angAngle.Clamp();
	}
}
