#pragma once
// used: winapi includes
#include "../common.h"

// used: usercmd
#include "../sdk/datatypes/usercmd.h"
// used: listener event function
#include "../sdk/interfaces/igameeventmanager.h"
// used: baseentity
#include "../sdk/entity.h"

class CMiscellaneous : public CSingleton<CMiscellaneous>
{
public:
	// Get
	void Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket);
	/* post engine prediction movement features */
	void PostPrediction(CUserCmd* pCmd, int iPreFlags, int iPostFlags);
	/* event listener function */
	void Event(IGameEvent* pEvent, const FNV1A_t uNameHash);
	/* correct movement while anti-aiming */
	void MovementCorrection(CUserCmd* pCmd, CBaseEntity* pLocal, QAngle& angOldViewPoint);

	// Extra
	/* automatic shoot when pressed attack key */
	void AutoPistol(CUserCmd* pCmd, CBaseEntity* pLocal);
	/* fuck u dalves */
	void AutoQueue();
	/* blocks the entity closest to you */
	void BlockBot(CUserCmd* pCmd, CBaseEntity* pLocal);
private:
	// Movement
	/* automatic jump when steps on the ground */
	void BunnyHop(CUserCmd* pCmd, CBaseEntity* pLocal);
	/* prevent stamina for epic movement clips */
	void JumpBug(CUserCmd* pCmd, int iPreFlags, int iPostFlags);
	/* edgebug for epic movement clips */
	void EdgeBug(CUserCmd* pCmd, int iPreFlags, int iPostFlags);
	/* jumps at the edge of objects */
	void EdgeJump(CUserCmd* pCmd, int iPreFlags, int iPostFlags);
	/* longjump on edgejump for epic movement clips*/
	void EdgeJumpLongJump(CUserCmd* pCmd, int iPreFlags, int iPostFlags);
};
