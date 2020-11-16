// used: random_device, mt19937, uniform_int_distribution
#include <random>

#include "misc.h"
// used: global variables
#include "../global.h"
// used: cheat variables
#include "../core/variables.h"
// used: convar interface
#include "../core/interfaces.h"
// used: angle-vector calculations
#include "../utilities/math.h"
// used: input system
#include "../utilities/inputsystem.h"
#include "../utilities/logging.h"
#include "visuals.h"

void CMiscellaneous::Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket)
{
	if (!pLocal->IsAlive())
		return;

	if (C::Get<bool>(Vars.bMiscNoCrouchCooldown))
		pCmd->iButtons |= IN_BULLRUSH;

	if (C::Get<bool>(Vars.bMiscBunnyHop))
		BunnyHop(pCmd, pLocal);

	//if (C::Get<int>(Vars.iMiscBlockBotKey) && IPT::IsKeyDown(C::Get<int>(Vars.iMiscBlockBotKey)))
	//	CMiscellaneous::Get().BlockBot(pCmd, pLocal);

	if (C::Get<bool>(Vars.bMiscRevealRanks) && pCmd->iButtons & IN_SCORE)
		I::Client->DispatchUserMessage(CS_UM_ServerRankRevealAll, 0U, 0, nullptr);
}

void CMiscellaneous::PostPrediction(CUserCmd* pCmd, int iPreFlags, int iPostFlags)
{
	if (C::Get<int>(Vars.iMiscJumpBugKey) && IPT::IsKeyDown(C::Get<int>(Vars.iMiscJumpBugKey)))
		JumpBug(pCmd, iPreFlags, iPostFlags);

	if (C::Get<int>(Vars.iMiscEdgeBugKey) && IPT::IsKeyDown(C::Get<int>(Vars.iMiscEdgeBugKey)))
		EdgeBug(pCmd, iPreFlags, iPostFlags);

	if (C::Get<int>(Vars.iMiscEdgeJumpKey) && IPT::IsKeyDown(C::Get<int>(Vars.iMiscEdgeJumpKey)))
	{
		EdgeJump(pCmd, iPreFlags, iPostFlags);

		if (C::Get<bool>(Vars.bMiscEdgeJumpLong))
			EdgeJumpLongJump(pCmd, iPreFlags, iPostFlags);
	}
		
}

void CMiscellaneous::Event(IGameEvent* pEvent, const FNV1A_t uNameHash)
{
	if (!I::Engine->IsInGame())
		return;
}

void CMiscellaneous::MovementCorrection(CUserCmd* pCmd, CBaseEntity* pLocal, QAngle& angOldViewPoint)
{
	static CConVar* cl_forwardspeed = I::ConVar->FindVar(XorStr("cl_forwardspeed"));
	if (cl_forwardspeed == nullptr)
		return;

	static CConVar* cl_sidespeed = I::ConVar->FindVar(XorStr("cl_sidespeed"));
	if (cl_sidespeed == nullptr)
		return;

	static CConVar* cl_upspeed = I::ConVar->FindVar(XorStr("cl_upspeed"));
	if (cl_upspeed == nullptr)
		return;

	// get max speed limits by convars
	const float flMaxForwardSpeed = cl_forwardspeed->GetFloat();
	const float flMaxSideSpeed = cl_sidespeed->GetFloat();
	const float flMaxUpSpeed = cl_upspeed->GetFloat();

	Vector vecForward = { }, vecRight = { }, vecUp = { };
	M::AngleVectors(angOldViewPoint, &vecForward, &vecRight, &vecUp);

	// we don't attempt on forward/right roll, and on up pitch/yaw
	vecForward.z = vecRight.z = vecUp.x = vecUp.y = 0.f;

	vecForward.Normalize();
	vecRight.Normalize();
	vecUp.Normalize();

	Vector vecOldForward = { }, vecOldRight = { }, vecOldUp = { };
	M::AngleVectors(pCmd->angViewPoint, &vecOldForward, &vecOldRight, &vecOldUp);

	// we don't attempt on forward/right roll, and on up pitch/yaw
	vecOldForward.z = vecOldRight.z = vecOldUp.x = vecOldUp.y = 0.f;

	vecOldForward.Normalize();
	vecOldRight.Normalize();
	vecOldUp.Normalize();

	const float flPitchForward = vecForward.x * pCmd->flForwardMove;
	const float flYawForward = vecForward.y * pCmd->flForwardMove;
	const float flPitchSide = vecRight.x * pCmd->flSideMove;
	const float flYawSide = vecRight.y * pCmd->flSideMove;
	const float flRollUp = vecUp.z * pCmd->flUpMove;

	// solve corrected movement
	const float x = vecOldForward.x * flPitchSide + vecOldForward.y * flYawSide + vecOldForward.x * flPitchForward + vecOldForward.y * flYawForward + vecOldForward.z * flRollUp;
	const float y = vecOldRight.x * flPitchSide + vecOldRight.y * flYawSide + vecOldRight.x * flPitchForward + vecOldRight.y * flYawForward + vecOldRight.z * flRollUp;
	const float z = vecOldUp.x * flYawSide + vecOldUp.y * flPitchSide + vecOldUp.x * flYawForward + vecOldUp.y * flPitchForward + vecOldUp.z * flRollUp;

	// clamp and apply corrected movement
	pCmd->flForwardMove = std::clamp(x, -flMaxForwardSpeed, flMaxForwardSpeed);
	pCmd->flSideMove = std::clamp(y, -flMaxSideSpeed, flMaxSideSpeed);
	pCmd->flUpMove = std::clamp(z, -flMaxUpSpeed, flMaxUpSpeed);
}

void CMiscellaneous::AutoPistol(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive())
		return;

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();

	if (pWeapon == nullptr)
		return;

	short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();
	const CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);

	// check for pistol and attack
	if (pWeaponData == nullptr || pWeaponData->bFullAuto || pWeaponData->nWeaponType != WEAPONTYPE_PISTOL || !(pCmd->iButtons & IN_ATTACK))
		return;

	if (pLocal->CanShoot(static_cast<CWeaponCSBase*>(pWeapon)))
		pCmd->iButtons |= IN_ATTACK;
	else
		pCmd->iButtons &= ~IN_ATTACK;
}

void CMiscellaneous::AutoQueue()
{
	using handle_match_start_fn = bool(__thiscall*)(void*, const char*, char*, char*, char*);
	using create_session_fn = void* (__stdcall*)(void);

	static auto singleton = *(uint8_t**)(MEM::FindPattern(CLIENT_DLL, XorStr("8B C8 89 0D ? ? ? ? 8D 41 08")) + 0x4);
	static auto handle_match_start = (handle_match_start_fn)MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 51 53 56 57 8B F9 8B 0D ? ? ? ? 8B 01 FF 50 34"));
	static auto create_session = (create_session_fn)MEM::FindPattern(CLIENT_DLL, XorStr("E8 ? ? ? ? 83 EC 14 E8"));

	static auto search_started = []() {
		if (!singleton)
			return false;
		if (auto ls = *(uint8_t**)singleton; ls) {
			return *(uint32_t*)(ls + 0x10) != 0;
		}
		return false;
	};

	if (auto match_session = I::MatchFramework->GetMatchSession()) {
		if (!search_started()) {
			auto session_settings = match_session->GetSessionSettings();
			session_settings->KeySetString("game/type", "classic");
			session_settings->KeySetString("game/mode", "casual");
			session_settings->KeySetString("game/mapgroupname", "mg_de_dust2");
			match_session->UpdateSessionSettings(session_settings);
			handle_match_start(*(uint8_t**)singleton, "", "", "", "");
		}
	}
	else {
		create_session();
	}
}

void CMiscellaneous::BunnyHop(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	static CConVar* sv_autobunnyhopping = I::ConVar->FindVar(XorStr("sv_autobunnyhopping"));

	if (sv_autobunnyhopping->GetBool())
		return;

	if (IPT::IsKeyDown(C::Get<int>(Vars.iMiscJumpBugKey)))
		return;

	if (pLocal->GetMoveType() == MOVETYPE_LADDER || pLocal->GetMoveType() == MOVETYPE_NOCLIP || pLocal->GetMoveType() == MOVETYPE_OBSERVER)
		return;

	static bool bLastJumped = false, bShouldFake = false;

	if (!bLastJumped && bShouldFake)
	{
		bShouldFake = false;
		pCmd->iButtons |= IN_JUMP;
	}
	else if (pCmd->iButtons & IN_JUMP)
	{
		if (pLocal->GetFlags() & FL_ONGROUND || pLocal->GetFlags() & FL_PARTIALGROUND)
		{
			bLastJumped = true;
			bShouldFake = true;
		}
		else
		{
			pCmd->iButtons &= ~IN_JUMP;
			bLastJumped = false;
		}
	}
	else
	{
		bLastJumped = false;
		bShouldFake = false;
	}
}

void CMiscellaneous::BlockBot(CUserCmd* pCmd, CBaseEntity* pLocal)
{
	// entity we are going to blockbot
	CBaseEntity* pTarget = nullptr;

	// maximum speed we can move sideways (usually 450)
	static CConVar* cl_sidespeed = I::ConVar->FindVar("cl_sidespeed");
	if (cl_sidespeed == nullptr)
		return;

	// maximum speed we can move foward (usually 450)
	static CConVar* cl_forwardspeed = I::ConVar->FindVar("cl_forwardspeed");
	if (cl_forwardspeed == nullptr)
		return;

	// maximum distance the target can be in order to be blockbotted
	float flMaxDistance = 250.f;

	if (pTarget == nullptr) // don't choose an entity while we are already blockbotting
	{
		for (int i = 1; i < I::Globals->nMaxClients; i++)
		{
			if (i == I::Engine->GetLocalPlayer())
				continue;

			CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);

			if (pEntity == nullptr || !pEntity->IsAlive() || pEntity->IsDormant() || pEntity->HasImmunity())
				continue;

			float flDistance = pLocal->GetAbsOrigin().DistTo(pEntity->GetAbsOrigin());

			if (flDistance < flMaxDistance)
			{
				flMaxDistance = flDistance;
				pTarget = pEntity;
			}
		}
	}

	if (pTarget == nullptr)
		return;

	QAngle angAngle = M::CalcAngle(pLocal->GetAbsOrigin(), pTarget->GetAbsOrigin());

	angAngle.y -= pCmd->angViewPoint.y;
	angAngle.Normalize();

	pCmd->flSideMove = angAngle.y > 0.f ? -cl_sidespeed->GetFloat() : cl_sidespeed->GetFloat();
}

void CMiscellaneous::JumpBug(CUserCmd* pCmd, int iPreFlags, int iPostFlags)
{
	if (!(iPreFlags & FL_ONGROUND) && (iPostFlags & FL_ONGROUND)) // if we are going to be on ground the next tick and aren't currently on ground
	{
		pCmd->iButtons |= IN_DUCK;
		pCmd->iButtons &= ~IN_JUMP;
	}
}

void CMiscellaneous::EdgeJump(CUserCmd* pCmd, int iPreFlags, int iPostFlags)
{
	if ((iPreFlags & FL_ONGROUND) && !(iPostFlags & FL_ONGROUND)) // if we are going to be on air the next tick and aren't currently on air
		pCmd->iButtons |= IN_JUMP;
}

void CMiscellaneous::EdgeBug(CUserCmd* pCmd, int iPreFlags, int iPostFlags)
{
	if (!(iPreFlags & FL_ONGROUND) && iPostFlags & FL_ONGROUND) // if we are going to be on ground the next tick and aren't currently on ground
		pCmd->iButtons |= IN_DUCK;
}

void CMiscellaneous::EdgeJumpLongJump(CUserCmd* pCmd, int iPreFlags, int iPostFlags)
{
	static bool bTimerStart = false;
	static int iTimer = 0;

	if (iPreFlags & FL_ONGROUND && !(iPostFlags & FL_ONGROUND))
		pCmd->iButtons |= IN_JUMP;

	if (pCmd->iButtons & IN_JUMP)
	{
		pCmd->iButtons |= IN_DUCK;

		if (!bTimerStart)
		{
			iTimer = I::Globals->iTickCount;
			bTimerStart = true;
		}
	}

	if (iTimer + 2 > I::Globals->iTickCount && bTimerStart)
		pCmd->iButtons |= IN_DUCK;
	else if (iTimer + 2 < I::Globals->iTickCount && bTimerStart)
		bTimerStart = false;

	if (pCmd->iButtons & IN_DUCK && !(pCmd->iButtons & IN_JUMP) && iPreFlags & FL_ONGROUND)
		pCmd->iButtons &= ~IN_DUCK; // poo fix, could also be ljtimer
}
