#pragma once
// used: define to add values to variables vector
#include "config.h"
// used: EItemDefinitionIndex enum
#include "../sdk/entity.h"

#pragma region variables_array_entries
enum class ELegitWeaponTypes : int
{
	RIFLES = 0,
	SNIPERS,
	PISTOLS,
	HEAVY_PISTOLS,
	SMGS,
	OTHER
};

enum class ELegitHitboxes : int
{
	CLOSEST = 0,
	HEAD,
	CHEST,
	STOMACH
};

enum EVisualsInfoFlags : int
{
	INFO_FLAG_ARMOR = 0,
	INFO_FLAG_KIT,
	INFO_FLAG_ZOOM,
	INFO_FLAG_MAX
};

enum EVisualsRemovals : int
{
	REMOVAL_POSTPROCESSING = 0,
	REMOVAL_FLASHBANG,
	REMOVAL_PUNCH,
	REMOVAL_SMOKE,
	REMOVAL_SCOPE,
	REMOVAL_3DSKY,
	REMOVAL_SHADOWS,
	REMOVAL_DECALS,
	REMOVAL_MAX
};
#pragma endregion

#pragma region variables_combo_entries
enum class EAntiAimPitchType : int
{
	NONE = 0,
	UP,
	DOWN,
	ZERO
};

enum class EAntiAimYawType : int
{
	NONE = 0,
	DESYNC
};

enum class EVisualsBoxType : int
{
	NONE = 0,
	FULL,
	CORNERS
};

enum class EVisualsGlowStyle : int
{
	OUTER = 0,
	COVER,
	INNER
};

enum class EVisualsEnemiesChams : int
{
	COVERED = 0,
	FLAT,
	WIREFRAME,
	REFLECTIVE
};

enum class EVisualsViewModelChams : int
{
	NO_DRAW = 0,
	COVERED,
	FLAT,
	WIREFRAME,
	GLOW,
	SCROLL,
	CHROME
};
#pragma endregion

struct LegitbotVariables_t
{
	int		iAimKey	= 0;
	float	flAimFov = 0.0f;
	int		iAimHitbox = 0;
	bool	bAimAtBacktrack = false;
	bool	bAimSilent = false;
	float	flAimSmooth = 1.0f;
	bool	bAimRCS = false;
	bool	bAimAutoWall = false;
	int		iAimAutoWallMinDamage = 0;

	LegitbotVariables_t()
	{
		iAimKey = 0;
		flAimFov = 0.0f;
		iAimHitbox = 0;
		bAimAtBacktrack = false;
		bAimSilent = false;
		flAimSmooth = 1.0f;
		bAimRCS = false;
		bAimAutoWall = false;
		iAimAutoWallMinDamage = 0;
	}
};

struct SkinchangerVariables_t
{
	bool		bEnabled = false;
	int			iPaintKit = 0;
	int			iPaintKitIndex = 0;
	int			iSeed = 0;
	bool		bStatTrak = false;
	int			iStatTrak = 0;
	int			iQuality = 0;
	bool		bNameTag = false;
	std::string szNameTag = "";
	float		flWear = 0.1f;
	int			iDefinitionIndex = 0;
	int			iDefinitionIndexOverride = 0;

	SkinchangerVariables_t()
	{
		bEnabled = false;
		iPaintKit = 0;
		iPaintKitIndex = 0;
		iSeed = 0;
		bStatTrak = false;
		iStatTrak = 0;
		iQuality = 0;
		bNameTag = false;
		szNameTag = "";
		flWear = 0.0f;
		iDefinitionIndex = 0;
		iDefinitionIndexOverride = 0;
	}
};

struct Variables_t
{
	#pragma region variables_legit
	// aimbot
	C_ADD_VARIABLE(bool, bLegit, false);
	C_ADD_VARIABLE(int, iLegitWeapon, false);
	C_ADD_VARIABLE_VECTOR(LegitbotVariables_t, 6U, vecLegitVars, LegitbotVariables_t());

	//backtracking
	C_ADD_VARIABLE(bool, bBacktracking, false);
	C_ADD_VARIABLE(int, iBacktrackingTime, 200);

	// trigger
	C_ADD_VARIABLE(bool, bTrigger, false);
	C_ADD_VARIABLE(int, iTriggerKey, 0);
	C_ADD_VARIABLE(int, iTriggerDelay, 0);
	C_ADD_VARIABLE(bool, bTriggerAutoWall, false);
	C_ADD_VARIABLE(int, iTriggerMinimumDamage, 70);

	C_ADD_VARIABLE(bool, bTriggerHead, true);
	C_ADD_VARIABLE(bool, bTriggerChest, true);
	C_ADD_VARIABLE(bool, bTriggerStomach, true);
	C_ADD_VARIABLE(bool, bTriggerArms, false);
	C_ADD_VARIABLE(bool, bTriggerLegs, false);
	#pragma endregion

	#pragma region variables_visuals
	C_ADD_VARIABLE(bool, bEsp, false);

	// main
	C_ADD_VARIABLE(bool, bEspMain, false);
	C_ADD_VARIABLE(bool, bEspMainEnemies, false);
	C_ADD_VARIABLE(bool, bEspMainWeapons, false);
	C_ADD_VARIABLE(bool, bEspMainGrenades, false);
	C_ADD_VARIABLE(bool, bEspMainBomb, false);

	// players
	C_ADD_VARIABLE(bool, bEspMainPlayerBox, false);
	C_ADD_VARIABLE(Color, colEspMainPlayerBox, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainPlayerSkeleton, false);
	C_ADD_VARIABLE(Color, colEspMainPlayerSkeleton, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainPlayerBacktrack, false);
	C_ADD_VARIABLE(Color, colEspMainPlayerBacktrack, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainPlayerRadar, false);
	C_ADD_VARIABLE(bool, bEspMainPlayerInfo, false);

	/* left */
	C_ADD_VARIABLE(bool, bEspMainPlayerHealth, false);
	C_ADD_VARIABLE(Color, colEspMainPlayerHealth, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainPlayerOverrideHealthColor, false);

	/* top */
	C_ADD_VARIABLE(bool, bEspMainPlayerName, false);
	C_ADD_VARIABLE(Color, colEspMainPlayerName, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainPlayerFlash, false);
	C_ADD_VARIABLE(Color, colEspMainPlayerFlash, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainPlayerNadeKill, false);
	C_ADD_VARIABLE(Color, colEspMainPlayerNadeKill, Color(255, 255, 255, 255));

	/* right */
	C_ADD_VARIABLE_VECTOR(bool, INFO_FLAG_MAX, vecEspMainPlayerFlags, false);

	/* bottom */
	C_ADD_VARIABLE(bool, bEspMainPlayerWeaponText, false);
	C_ADD_VARIABLE(Color, colEspMainPlayerWeaponText, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainPlayerWeaponIcon, false);
	C_ADD_VARIABLE(Color, colEspMainPlayerWeaponIcon, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainPlayerAmmo, false);
	C_ADD_VARIABLE(Color, colEspMainPlayerAmmo, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainPlayerDistance, false);

	// weapons
	C_ADD_VARIABLE(bool, bEspMainWeaponBox, false);
	C_ADD_VARIABLE(Color, colEspMainWeaponBox, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainWeaponText, false);
	C_ADD_VARIABLE(Color, colEspMainWeaponText, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainWeaponIcon, false);
	C_ADD_VARIABLE(Color, colEspMainWeaponIcon, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainWeaponAmmo, false);
	C_ADD_VARIABLE(Color, colEspMainWeaponAmmo, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainWeaponDistance, false);
	C_ADD_VARIABLE(Color, colEspMainWeaponDistance, Color(255, 255, 255, 255));

	// grenades
	C_ADD_VARIABLE(bool, bEspMainGrenadesFrame, false);
	C_ADD_VARIABLE(bool, bEspMainGrenadesName, false);
	C_ADD_VARIABLE(Color, colEspMainGrenadesName, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(bool, bEspMainGrenadesDuration, false);
	C_ADD_VARIABLE(bool, bEspMainGrenadesDistance, false);
	C_ADD_VARIABLE(Color, colEspMainGrenadesDistance, Color(255, 255, 255, 255));

	// glow
	C_ADD_VARIABLE(bool, bEspGlow, false);
	C_ADD_VARIABLE(bool, bEspGlowEnemies, false);
	C_ADD_VARIABLE(bool, bEspGlowEnemiesVisible, false);
	C_ADD_VARIABLE(Color, colEspGlowEnemiesVisible, Color(230, 20, 60, 128));
	C_ADD_VARIABLE(bool, bEspGlowEnemiesWall, false);
	C_ADD_VARIABLE(Color, colEspGlowEnemiesWall, Color(255, 235, 240, 128));
	C_ADD_VARIABLE(bool, bEspGlowWeapons, false);
	C_ADD_VARIABLE(bool, bEspGlowWeaponsVisible, false);
	C_ADD_VARIABLE(Color, colEspGlowWeaponsVisible, Color(80, 0, 225, 128));
	C_ADD_VARIABLE(bool, bEspGlowWeaponsWall, false);
	C_ADD_VARIABLE(Color, colEspGlowWeaponsWall, Color(80, 0, 225, 128));
	C_ADD_VARIABLE(bool, bEspGlowGrenades, false);
	C_ADD_VARIABLE(bool, bEspGlowGrenadesVisible, false);
	C_ADD_VARIABLE(Color, colEspGlowGrenadesVisible, Color(180, 130, 30, 128));
	C_ADD_VARIABLE(bool, bEspGlowGrenadesWall, false);
	C_ADD_VARIABLE(Color, colEspGlowGrenadesWall, Color(180, 130, 30, 128));

	C_ADD_VARIABLE(bool, bEspGlowBomb, false);
	C_ADD_VARIABLE(bool, bEspGlowBombVisible, false);
	C_ADD_VARIABLE(Color, colEspGlowBombVisible, Color(140, 220, 80, 128));
	C_ADD_VARIABLE(bool, bEspGlowBombWall, false);
	C_ADD_VARIABLE(Color, colEspGlowBombWall, Color(140, 220, 80, 128));
	C_ADD_VARIABLE(bool, bEspGlowPlantedBomb, false);
	C_ADD_VARIABLE(bool, bEspGlowPlantedBombVisible, false);
	C_ADD_VARIABLE(Color, colEspGlowPlantedBombVisible, Color(140, 220, 80, 128));
	C_ADD_VARIABLE(bool, bEspGlowPlantedBombWall, false);
	C_ADD_VARIABLE(Color, colEspGlowPlantedBombWall, Color(140, 220, 80, 128));	

	// chams
	C_ADD_VARIABLE(bool, bEspChams, false);
	C_ADD_VARIABLE(bool, bEspChamsEnemies, false);
	C_ADD_VARIABLE(bool, bEspChamsViewModel, false);
	C_ADD_VARIABLE(bool, bEspChamsBacktrack, false)

	C_ADD_VARIABLE(bool, bEspChamsEnemiesVisible, false);
	C_ADD_VARIABLE(Color, colEspChamsEnemiesVisible, Color(180, 65, 65, 255));
	C_ADD_VARIABLE(bool, bEspChamsEnemiesWall, false);
	C_ADD_VARIABLE(Color, colEspChamsEnemiesWall, Color(180, 180, 40, 255));
	C_ADD_VARIABLE(int, iEspChamsEnemiesVisible, static_cast<int>(EVisualsEnemiesChams::COVERED));
	C_ADD_VARIABLE(int, iEspChamsEnemiesWall, static_cast<int>(EVisualsEnemiesChams::COVERED));

	C_ADD_VARIABLE(int, iEspChamsViewModel, static_cast<int>(EVisualsViewModelChams::WIREFRAME));
	C_ADD_VARIABLE(Color, colEspChamsViewModel, Color(80, 255, 45, 255));
	C_ADD_VARIABLE(Color, colEspChamsViewModelAdditional, Color(0, 0, 255, 255));

	C_ADD_VARIABLE(int, iEspChamsBacktrackType, 0);
	C_ADD_VARIABLE(bool, bEspChamsEnemiesBacktrack, false);
	C_ADD_VARIABLE(Color, colEspChamsEnemiesBacktrack, Color(25, 25, 25, 75));
	C_ADD_VARIABLE(int, iEspChamsEnemiesBacktrack, static_cast<int>(EVisualsEnemiesChams::COVERED));

	// world
	C_ADD_VARIABLE(bool, bWorldNightMode, false);
	C_ADD_VARIABLE(bool, bWorldFog, false);
	C_ADD_VARIABLE(bool, bWorldFish, false);
	C_ADD_VARIABLE(Color, colWorldFogPrimary, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(int, iWorldFogStart, 200);
	C_ADD_VARIABLE(int, iWorldFogEnd, 1500);
	C_ADD_VARIABLE(bool, bSpectatorList, false);
	C_ADD_VARIABLE(int, iWorldThirdPersonKey, 0);
	C_ADD_VARIABLE(int, iWorldThirdPersonOffset, 150);
	C_ADD_VARIABLE_VECTOR(bool, REMOVAL_MAX, vecWorldRemovals, false);

	// on-screen
	C_ADD_VARIABLE(float, flScreenCameraFOV, 0.f);
	C_ADD_VARIABLE(float, flScreenViewModelFOV, 0.f);
	C_ADD_VARIABLE(bool, bScreenSniperCrosshair, false);
	C_ADD_VARIABLE(bool, bScreenHitMarker, false);
	C_ADD_VARIABLE(bool, bScreenHitMarkerDamage, false);
	C_ADD_VARIABLE(bool, bScreenHitMarkerSound, false);
	C_ADD_VARIABLE(float, flScreenHitMarkerTime, 1.0f);
	C_ADD_VARIABLE(int, iScreenHitMarkerGap, 4);
	C_ADD_VARIABLE(int, iScreenHitMarkerLenght, 10);
	C_ADD_VARIABLE(Color, colScreenHitMarker, Color(255, 255, 255, 255));
	C_ADD_VARIABLE(Color, colScreenHitMarkerDamage, Color(255, 25, 25, 255));
	#pragma endregion

	#pragma region variables_misc
	// movement
	C_ADD_VARIABLE(bool, bMiscBunnyHop, false);
	C_ADD_VARIABLE(int, iMiscBunnyHopChance, 100);
	C_ADD_VARIABLE(bool, bMiscAutoStrafe, false);
	C_ADD_VARIABLE(int, iMiscJumpBugKey, 0);
	C_ADD_VARIABLE(int, iMiscEdgeBugKey, 0);
	C_ADD_VARIABLE(int, iMiscEdgeJumpKey, 0);
	C_ADD_VARIABLE(bool, bMiscEdgeJumpLong, false);
	C_ADD_VARIABLE(bool, bMiscNoCrouchCooldown, false);
	C_ADD_VARIABLE(bool, bMiscVeloIndicator, false);
	C_ADD_VARIABLE(Color, colMiscVeloIndicator, Color(255, 255, 255, 255));

	// general
	C_ADD_VARIABLE(bool, bMiscAutoAccept, false);
	C_ADD_VARIABLE(bool, bMiscAutoPistol, false);
	C_ADD_VARIABLE(int, iMiscBlockBotKey, 0);
	C_ADD_VARIABLE(bool, bMiscFakeLatency, false);
	C_ADD_VARIABLE(int, iMiscFakeLatencyAmount, 100);
	C_ADD_VARIABLE(bool, bMiscRevealRanks, false);
	C_ADD_VARIABLE(bool, bMiscUnlockInventory, false);
	C_ADD_VARIABLE(bool, bMiscAntiUntrusted, true);
	#pragma endregion

	#pragma region variables_skinchanger
	C_ADD_VARIABLE(int, iSkinchangerWeapon, 0);
	C_ADD_VARIABLE(int, iSkinchangerKnife, 0);
	C_ADD_VARIABLE(int, iSkinchangerGlove, 0);
	C_ADD_VARIABLE(int, iSkinchangerPaintKit, 0);
	C_ADD_VARIABLE_MAP(int, SkinchangerVariables_t, mapSkinchangerVars, 0, SkinchangerVariables_t());
#pragma endregion

	#pragma region variables_menu
	C_ADD_VARIABLE(int, iMenuKey, VK_INSERT);
	C_ADD_VARIABLE(int, iPanicKey, VK_END);
	#pragma endregion
};

inline Variables_t Vars;
