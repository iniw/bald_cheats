#pragma once
// used: winapi includes, singleton
#include "../common.h"
// used: usercmd
#include "../sdk/datatypes/usercmd.h"
// used: baseentity, baseweapon
#include "../sdk/entity.h"
// used: timer
#include "../utilities.h"
// used: convar class, globals interface
#include "../core/interfaces.h"

class CTriggerBot : public CSingleton<CTriggerBot>
{
public:
	// Get
	void Run(CUserCmd* pCmd, CBaseEntity* pLocal);
private:
	/* runs the magnet trigger function */
	void		 MagnetTrigger(CUserCmd* pCmd, CBaseEntity* pLocal);
	/* returns the closest entity to the crosshair */
	CBaseEntity* GetClosestEntity(CBaseEntity* pLocal, QAngle angLocalViewAngles, Vector vecLocalEyePos);
	/* returns the closest hitbox to the crosshair */
	Vector		 GetBestHitbox(CBaseEntity* pLocal, CBaseEntity* pEntity, QAngle angLocalViewAngles, Vector vecLocalEyePos);
	/* finalizes the magnet triggerbot angle */
	void		 FinalizeAngle(CBaseEntity* pLocal, QAngle& angAngle, QAngle angLocalViewAngles, float flDelta);


	/* timer for delay before shoot */
	CTimer m_timer = { };
	CConVar* m_weapon_recoil_scale = I::ConVar->FindVar(XorStr("weapon_recoil_scale"));
	std::deque<int> m_arrHitBoxes = { };
};
