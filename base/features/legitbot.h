#pragma once
// used: winapi includes, singleton
#include "../common.h"
// used: usercmd
#include "../sdk/datatypes/usercmd.h"
// used: baseentity
#include "../sdk/entity.h"
// used: convar class
#include "../sdk/convar.h"
// used: convar interface
#include "../core/interfaces.h"
// used: local pointer
#include "../global.h"
// used: Record_t
#include "lagcompensation.h"
// used: cheat variables
#include "../core/variables.h"

/* main target structure */
struct Target_t
{
	CBaseEntity* pEntity; // current entitity
	int iHitbox; // current hitbox 
	Vector vecHitboxPos; // hitbox position we are going to aim at
	QAngle angAngle; // angle of the hitbox and our eyepos
	float flAngleDelta; // delta of that angle
	int iIndex;	// index of the entity
	bool bShouldAimAtBacktrack = false; // true if aiming at a record is better than aiming at normal
	Record_t Record;
};


class CLegitBot : public CSingleton<CLegitBot>
{
public:
	// Get
	void Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket);

	/* returns the custom weapon type of the weapon we're holding */
	int			 GetWeaponType(CBaseEntity* pLocal);

	const std::array<int, 3U> m_arrHitboxes = { HITBOX_HEAD, HITBOX_CHEST, HITBOX_STOMACH }; // @note these should always match the ones in the menu
	const std::array<int, 5U> m_arrClosestHitboxes = { HITBOX_HEAD, HITBOX_NECK, HITBOX_THORAX, HITBOX_UPPER_CHEST, HITBOX_STOMACH };

	/* member variables */
	Target_t m_Target;

private:
	/* determines if the aimbot should run */
	bool		 ShouldRun(CBaseEntity* pLocal);

	/* determines if localplayer can shoot */
	bool		 CanShoot(CBaseEntity* pLocal, CUserCmd* pCmd);

	/* fills the Target variable */
	bool		 GetTargetData(CBaseEntity* pLocal, Target_t& Target);

	/* determines if the entity is aimbottable */
	bool		 IsValid(CBaseEntity* pLocal, CBaseEntity* pEntity);

	/* returns the entity closest to the crosshair */
	CBaseEntity* GetBestEntity(CBaseEntity* pLocal);

	/* returns the position of the hitbox closest to the crosshair */
	Vector	     GetBestHitbox(CBaseEntity* pLocal, CBaseEntity* pEntity);

	/* applies rcs and smoothing to the angle */
	void		 FinalizeAngle(CBaseEntity* pLocal, QAngle& angAngle);

	/* returns true if pressing aim key or mouse2 with revolver */
	bool		 ShouldShoot(CBaseEntity* pLocal, CUserCmd* pCmd);

	/* applies the angle */
	void		 ApplyAngle(CUserCmd* pCmd, QAngle angAngle);

	QAngle m_angLocalViewAngles;
	Vector m_vecLocalEyePos;
	LegitbotVariables_t m_WeaponVars;
	int m_iWeaponType = 0;
	CConVar* m_weapon_recoil_scale = I::ConVar->FindVar(XorStr("weapon_recoil_scale"));
};
