#pragma once
#include <random>

// used: winapi includes, singleton
#include "../common.h"
// used: itemdefinitionindex; baseentity, baseweapon classes
#include "../sdk/entity.h"
// used: listener event function
#include "../sdk/interfaces/igameeventmanager.h"
// used: general game data and arrays
#include "../sdk/gamedata.h"
// used: SkinchangerVariables_t
#include "../core/variables.h"

// @todo: add valve vdf/vpk parser and get skins, rarity things, etc with it
class CSkinChanger : public CSingleton<CSkinChanger>
{
public:
	// Get
	void Run(CBaseEntity* pLocal);
	/* knife kill icons replace */
	void Event(IGameEvent* pEvent, const FNV1A_t uNameHash);
	/* dump stikers and paintkits */
	void Dump();
	/* fixes knife sequences */
	bool FixSequences(CRecvProxyData* pData, CBaseViewModel* pViewmodel);

	/* member variables */
	std::map<int, SkinchangerVariables_t> mapSkinchangerVars;
	std::vector<GD::PaintKit_t> vecSkinKits;
	std::vector<GD::PaintKit_t> vecGloveKits;
private:
	/* random int generator */
	inline int RandomInt(int iMin, int iMax)
	{
		return rand() % (iMax - iMin + 1) + iMin;
	}
	/* fixes knife sequences */
	int GetCorrectSequence(const short nDefinitionIndex, const int iSequence, CBaseEntity* pOwner);
	/* creates glove entity */
	CBaseCombatWeapon* MakeGlove(int iEntry, int iSerial);
	/* apply skins and models to weapons */
	void Weapons(CBaseEntity* pLocal);
	void Gloves(CBaseEntity* pLocal);
	void Apply(CBaseCombatWeapon* pWeapon, SkinchangerVariables_t WeaponVars, unsigned int iXuidLow);
};
