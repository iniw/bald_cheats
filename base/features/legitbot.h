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
// used: EntityObject_t struct
#include "../utilities/entitylistener.h"

/* main target structure */
struct Target_t
{
	CBaseEntity* pEntity = nullptr; // current entitity
	int			 iIndex = 0.f; // index of the entity
	Vector		 vecHitboxPos = { }; // hitbox position we are going to aim at
	QAngle		 angAngle = { }; // angle of the hitbox and our eye position
	float		 flAngleDelta = 0.f; // delta of that angle
	std::array<matrix3x4_t, MAXSTUDIOBONES> arrBoneMatrixes = { }; // proper matrix for more precise aiming

	/* aim at backtrack stuff */
	bool		 bShouldAimAtBacktrack = false; // true if aiming at a record is better than aiming at normal
	Record_t	 Record = { }; // backtrack record
};

constexpr std::array<int, 3U> arrHitboxes = { HITBOX_HEAD, HITBOX_CHEST, HITBOX_STOMACH }; // @note these should always match the ones in the menu
constexpr std::array<int, 5U> arrClosestHitboxes = { HITBOX_HEAD, HITBOX_NECK, HITBOX_THORAX, HITBOX_UPPER_CHEST, HITBOX_STOMACH }; // this is different to have more accurate closest hitbox selection

class CLegitBot : public CSingleton<CLegitBot>
{
public:
	// Get
	void Run(CUserCmd* pCmd, CBaseEntity* pLocal, bool& bSendPacket);
	/* returns the custom weapon type of the weapon we're holding */
	int	 GetWeaponType(CBaseEntity* pLocal);
	/* determines if the entity is aimbottable */
	bool IsValid(CBaseEntity* pEntity);
	/* used to draw stuff for debugging pourposes */
	void Draw();
	/* determines if localplayer can shoot */
	bool		 CanShoot(CBaseEntity* pLocal, CUserCmd* pCmd);

	Target_t m_Target; // current target
private:
	/* determines if the aimbot should run */
	bool		 ShouldRun(CBaseEntity* pLocal);
	/* fills the Target variable */
	bool		 GetTargetData(CBaseEntity* pLocal);
	/* returns the entity closest to the crosshair */
	CBaseEntity* GetBestEntity(CBaseEntity* pLocal);
	/* returns the position of the hitbox closest to the crosshair */
	Vector	     GetBestHitbox(CBaseEntity* pLocal);
	Vector	     GetBestHitbox(CBaseEntity* pLocal, Record_t Record); // used for aim at backtrack
	/* applies rcs and smoothing to the angle */
	void		 FinalizeAngle(CBaseEntity* pLocal, QAngle& angAngle);
	/* returns true if pressing aim key or mouse2 with revolver */
	bool		 ShouldShoot(CBaseEntity* pLocal, CUserCmd* pCmd);
	/* applies the angle */
	void		 ApplyAngle(CUserCmd* pCmd, QAngle angAngle);
	
	/* member variables */
	QAngle m_angLocalViewAngles = { }; // view angles
	Vector m_vecLocalEyePos = { }; // eye position
	CConVar* m_weapon_recoil_scale = I::ConVar->FindVar(XorStr("weapon_recoil_scale")); // used for accurate recoil compensation

	LegitbotVariables_t m_WeaponVars; // menu variables of our current weapon
	int m_iWeaponType = 0; // weapon type
};
