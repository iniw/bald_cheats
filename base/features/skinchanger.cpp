#include "skinchanger.h"

// used: create entity
#include "../utilities.h"
// used: client, engine interfaces
#include "../core/interfaces.h"
// used: cheat variables
#include "../core/config.h"
#include "../core/variables.h"
#include "../utilities/logging.h"
// used: _bstr_t
#include <comdef.h>

void CSkinChanger::Run(CBaseEntity* pLocal)
{
	CBaseViewModel* pViewmodel = I::ClientEntityList->GetFromHandle<CBaseViewModel>(pLocal->GetViewModelHandle());
	if (pViewmodel == nullptr)
		return;

	CBaseCombatWeapon* pViewmodelWeapon = I::ClientEntityList->GetFromHandle<CBaseCombatWeapon>(pViewmodel->GetWeaponHandle());
	if (pViewmodelWeapon == nullptr)
		return;

	CBaseWeaponWorldModel* pViewmodelWorld = I::ClientEntityList->GetFromHandle<CBaseWeaponWorldModel>(pViewmodelWeapon->GetWorldModelHandle());
	if (pViewmodelWorld == nullptr)
		return;

	Weapons(pLocal);
	Gloves(pLocal);

	short nActiveDefinitionIndex = pViewmodelWeapon->GetItemDefinitionIndex();

	if (mapItemList.find(nActiveDefinitionIndex) == mapItemList.end())
		return;	
	
	pViewmodel->GetModelIndex() = I::ModelInfo->GetModelIndex(mapItemList[nActiveDefinitionIndex].szModel);
	pViewmodelWorld->GetModelIndex() = I::ModelInfo->GetModelIndex(mapItemList[nActiveDefinitionIndex].szModel) + 1;
	//pWorldModel->GetModelIndex() = I::ModelInfo->GetModelIndex(mapItemList[nActiveDefinitionIndex].szModel);
}

void CSkinChanger::Weapons(CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive())
		return;

	PlayerInfo_t pInfo;
	if (!I::Engine->GetPlayerInfo(I::Engine->GetLocalPlayer(), &pInfo))
		return;

	if (const auto hWeapons = pLocal->GetWeaponsHandle(); hWeapons != nullptr)
	{
		for (std::size_t i = 0U; hWeapons[i] != INVALID_EHANDLE_INDEX; i++)
		{
			CBaseCombatWeapon* pWeapon = I::ClientEntityList->Get<CBaseCombatWeapon>(hWeapons[i]);
			if (pWeapon == nullptr)
				continue;

			short nDefinitionIndex = pWeapon->IsKnife() ? WEAPON_KNIFE : pWeapon->GetItemDefinitionIndex();

			if (mapItemList.find(nDefinitionIndex) == mapItemList.end())
				continue;

			SkinchangerVariables_t WeaponVars = C::Get<std::map<int, SkinchangerVariables_t>>(Vars.mapSkinchangerVars)[nDefinitionIndex];

			Apply(pWeapon, WeaponVars, pInfo.nXuidLow);
		}
	}
}

void CSkinChanger::Gloves(CBaseEntity* pLocal)
{
	CBaseHandle* hWereables = pLocal->GetWearablesHandle();
	
	static CBaseHandle hDefaultGlove = CBaseHandle(0);

	CBaseHandle hGlove = hWereables[0];

	CBaseCombatWeapon* pGlove = I::ClientEntityList->GetFromHandle<CBaseCombatWeapon>(hGlove);

	if (pGlove == nullptr)
	{
		CBaseCombatWeapon* pOurGlove = I::ClientEntityList->GetFromHandle<CBaseCombatWeapon>(hDefaultGlove);
		if (pOurGlove)
		{
			hWereables[0] = hDefaultGlove;
			pGlove = pOurGlove;
		}
	}

	if (!pLocal->IsAlive())
	{
		if (pGlove)
		{
			pGlove->GetClientNetworkable()->SetDestroyedOnRecreateEntities();
			pGlove->GetClientNetworkable()->Release();
		}
		return;
	}

	if (pGlove == nullptr)
	{
		int iEntry = I::ClientEntityList->GetHighestEntityIndex() + 1;
		int iSerial = rand() % 0x1000;

		pGlove = MakeGlove(iEntry, iSerial);

		if (pGlove == nullptr)		
			return;

		hWereables[0] = iEntry | iSerial << 16;

		// let's store it in case we somehow lose it.
		hDefaultGlove = hWereables[0];
	}

	PlayerInfo_t pInfo;
	if (!I::Engine->GetPlayerInfo(I::Engine->GetLocalPlayer(), &pInfo))
		return;

	pGlove->GetGloveIndex() = -1;
	pGlove->GetItemIDHigh() = -1;

	SkinchangerVariables_t WeaponVars = C::Get<std::map<int, SkinchangerVariables_t>>(Vars.mapSkinchangerVars)[GLOVE_T];

	Apply(pGlove, WeaponVars, pInfo.nXuidLow);

	pGlove->GetEntityQuality() = 1;
}

void CSkinChanger::Apply(CBaseCombatWeapon* pWeapon, SkinchangerVariables_t WeaponVars, unsigned int iXuidLow)
{
	if (!WeaponVars.bEnabled)
		return;

	pWeapon->GetItemIDHigh()		= -1;
	pWeapon->GetOwnerXuidLow()		= iXuidLow;
	pWeapon->GetFallbackPaintKit()	= WeaponVars.iPaintKit;
	pWeapon->GetFallbackSeed()		= WeaponVars.iSeed;
	pWeapon->GetFallbackWear()		= WeaponVars.flWear;
	
	if (WeaponVars.bStatTrak)
		pWeapon->GetFallbackStatTrak() = WeaponVars.iStatTrak;

	if (WeaponVars.bNameTag)
		std::strcpy(pWeapon->GetCustomName(), WeaponVars.szNameTag.data());

	if (WeaponVars.iDefinitionIndexOverride && pWeapon->GetItemDefinitionIndex() != WeaponVars.iDefinitionIndexOverride)
	{
		pWeapon->SetModelIndex(I::ModelInfo->GetModelIndex(mapItemList.at(WeaponVars.iDefinitionIndexOverride).szModel));
		pWeapon->GetItemDefinitionIndex() = WeaponVars.iDefinitionIndexOverride;
		pWeapon->GetEntityQuality() = 3;
		pWeapon->GetClientNetworkable()->PostDataUpdate(DATA_UPDATE_CREATED);
	}
}


static CreateClientClassFn GetWereableCreate()
{
	CBaseClient* pClass = I::Client->GetAllClasses();

	while (pClass->nClassID != EClassIndex::CEconWearable)
		pClass = pClass->pNext;

	return pClass->pCreateFn;
}

CBaseCombatWeapon* CSkinChanger::MakeGlove(int iEntry, int iSerial)
{
	static auto aCreateWereable = GetWereableCreate();
	aCreateWereable(iEntry, iSerial);

	CBaseCombatWeapon* pGlove = static_cast<CBaseCombatWeapon*>(I::ClientEntityList->GetClientEntity(iEntry));

	if (pGlove == nullptr)
	{
		L::Print("couldn't create glove entity, returning");
		return nullptr;
	}

	pGlove->SetAbsOrigin({ MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT });

	return pGlove;
}


void CSkinChanger::Event(IGameEvent* pEvent, const FNV1A_t uNameHash)
{
	if (!I::Engine->IsInGame())
		return;
}

void CSkinChanger::Dump()
{
	const static std::uintptr_t uSigAddress = MEM::FindPattern(CLIENT_DLL, XorStr("E8 ? ? ? ? FF 76 0C 8D 48 04 E8"));

	const int32_t iItemSystemOffset = *reinterpret_cast<std::int32_t*>(uSigAddress + 1);

	const auto pItemSystemFn = reinterpret_cast<CCStrike15ItemSystem * (*)()>(uSigAddress + 5 + iItemSystemOffset);

	const CCStrike15ItemSchema* pItemSchema = reinterpret_cast<CCStrike15ItemSchema*>(std::uintptr_t(pItemSystemFn()) + sizeof(void*));

	{
		const std::int32_t iPaintKitDefinitionOffset = *reinterpret_cast<std::int32_t*>(uSigAddress + 11 + 1);

		const auto pGetPaintKitDefinitionFn = reinterpret_cast<CPaintKit * (__thiscall*)(CCStrike15ItemSchema*, int)>(uSigAddress + 11 + 5 + iPaintKitDefinitionOffset);

		const std::intptr_t pStartElementOffset = *reinterpret_cast<std::intptr_t*>(std::uintptr_t(pGetPaintKitDefinitionFn) + 8 + 2);

		const int iHeadOffset = pStartElementOffset - 12;

		const auto mapHead = reinterpret_cast<Head_t<int, CPaintKit*>*>(std::uintptr_t(pItemSchema) + iHeadOffset);

		for (auto i = 0; i < mapHead->last_element; ++i)
		{
			const auto pPaintKit = mapHead->memory[i].value;

			if (pPaintKit->id == 9001)
				continue;

			const wchar_t* wszName = I::Localize->Find(pPaintKit->item_name.buffer + 1);
			const _bstr_t bstrName(wszName);
			const std::string szName(bstrName);	

			if (pPaintKit->id < 10000)
				vecSkinKits.push_back({ pPaintKit->id, pPaintKit->id != 0 ? fmt::format("{} - {:d}", szName, pPaintKit->id) : szName });
			else
				vecGloveKits.push_back({ pPaintKit->id, szName });
		}

		std::sort(vecSkinKits.begin(), vecSkinKits.end());
		std::sort(vecGloveKits.begin(), vecGloveKits.end());
	}

	for (size_t i = 0; i < vecSkinKits.size(); i++)
		std::transform(vecSkinKits[i].szName.begin(), vecSkinKits[i].szName.end(), vecSkinKits[i].szName.begin(), ::tolower);

	for (size_t i = 0; i < vecGloveKits.size(); i++)
		std::transform(vecGloveKits[i].szName.begin(), vecGloveKits[i].szName.end(), vecGloveKits[i].szName.begin(), ::tolower);
}

int CSkinChanger::GetCorrectSequence(const short nDefinitionIndex, const int iSequence, CBaseEntity* pViewmodelOwner)
{
	if (nDefinitionIndex == WEAPON_KNIFE_BUTTERFLY
		|| nDefinitionIndex == WEAPON_KNIFE_FALCHION
		|| nDefinitionIndex == WEAPON_KNIFE_SURVIVAL_BOWIE
		|| nDefinitionIndex == WEAPON_KNIFE_PUSH
		|| nDefinitionIndex == WEAPON_KNIFE_OUTDOOR)
	{
		if (iSequence == SEQUENCE_DEFAULT_IDLE1 || iSequence == SEQUENCE_DEFAULT_IDLE2)
		{
			pViewmodelOwner->GetCycle() = 0.999f;
			pViewmodelOwner->InvalidatePhysicsRecursive(ANIMATION_CHANGED);
		}
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> distrib(1, 10);

	switch (nDefinitionIndex)
	{
	case WEAPON_KNIFE_BUTTERFLY:
	{
		switch (iSequence)
		{
		case SEQUENCE_DEFAULT_DRAW:
		{
			if (distrib(gen) <= 9)
				return SEQUENCE_BUTTERFLY_DRAW2;
			else
				return SEQUENCE_BUTTERFLY_DRAW;
		}
		case SEQUENCE_DEFAULT_LOOKAT01:
			return RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, SEQUENCE_BUTTERFLY_LOOKAT03);
		default:
			return iSequence + 1;
		}
	}
	case WEAPON_KNIFE_FALCHION:
	{
		switch (iSequence)
		{
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_FALCHION_IDLE1;
		case SEQUENCE_DEFAULT_HEAVY_MISS1:
			return RandomInt(SEQUENCE_FALCHION_HEAVY_MISS1, SEQUENCE_FALCHION_HEAVY_MISS1_NOFLIP);
		case SEQUENCE_DEFAULT_LOOKAT01:
			return RandomInt(SEQUENCE_FALCHION_LOOKAT01, SEQUENCE_FALCHION_LOOKAT02);
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return iSequence;
		default:
			return iSequence - 1;
		}
	}
	case WEAPON_KNIFE_CSS:
	{
		switch (iSequence)
		{
		case SEQUENCE_DEFAULT_LOOKAT01:
			return RandomInt(SEQUENCE_CSS_LOOKAT01, SEQUENCE_CSS_LOOKAT02);
		default:
			return iSequence;
		}
	}
	case WEAPON_KNIFE_PUSH:
	{
		switch (iSequence)
		{
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_DAGGERS_IDLE1;
		case SEQUENCE_DEFAULT_LIGHT_MISS1:
		case SEQUENCE_DEFAULT_LIGHT_MISS2:
		{
			return RandomInt(SEQUENCE_DAGGERS_LIGHT_MISS1, SEQUENCE_DAGGERS_LIGHT_MISS5);
		}
		case SEQUENCE_DEFAULT_HEAVY_MISS1:
			return RandomInt(SEQUENCE_DAGGERS_HEAVY_MISS2, SEQUENCE_DAGGERS_HEAVY_MISS1);
		case SEQUENCE_DEFAULT_HEAVY_HIT1:
		case SEQUENCE_DEFAULT_HEAVY_BACKSTAB:
		case SEQUENCE_DEFAULT_LOOKAT01:
			return iSequence + 3;
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return iSequence;
		default:
			return iSequence + 2;
		}
	}
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
	{
		switch (iSequence)
		{
		case SEQUENCE_DEFAULT_DRAW:
		case SEQUENCE_DEFAULT_IDLE1:
			return iSequence;
		case SEQUENCE_DEFAULT_IDLE2:
			return SEQUENCE_BOWIE_IDLE1;
		default:
			return iSequence - 1;
		}
	}
	case WEAPON_KNIFE_URSUS:
	case WEAPON_KNIFE_CORD:
	case WEAPON_KNIFE_CANIS:
	case WEAPON_KNIFE_OUTDOOR:
	case WEAPON_KNIFE_SKELETON:
	{
		switch (iSequence)
		{
		case SEQUENCE_DEFAULT_DRAW:
			return RandomInt(SEQUENCE_BUTTERFLY_DRAW, SEQUENCE_BUTTERFLY_DRAW2);
		case SEQUENCE_DEFAULT_LOOKAT01:
			return RandomInt(SEQUENCE_BUTTERFLY_LOOKAT01, 14);
		default:
			return iSequence + 1;
		}
	}
	case WEAPON_KNIFE_STILETTO:
	{
		switch (iSequence)
		{
		case SEQUENCE_DEFAULT_LOOKAT01:
			return RandomInt(12, 13);
		default:
			return iSequence;
		}
	}
	case WEAPON_KNIFE_WIDOWMAKER:
	{
		switch (iSequence)
		{
		case SEQUENCE_DEFAULT_LOOKAT01:
			return RandomInt(14, 15);
		default:
			return iSequence;
		}
	}
	default:
		return iSequence;
	}
}

bool CSkinChanger::FixSequences(CRecvProxyData* pData, CBaseViewModel* pViewmodel)
{
	CBaseEntity* pViewmodelOwner = I::ClientEntityList->GetFromHandle<CBaseEntity>(pViewmodel->GetOwnerHandle());
	if (pViewmodelOwner == nullptr || pViewmodelOwner->GetIndex() != I::Engine->GetLocalPlayer())
		return false;

	CBaseCombatWeapon* pViewmodelWeapon = I::ClientEntityList->GetFromHandle<CBaseCombatWeapon>(pViewmodel->GetWeaponHandle());
	if (pViewmodelWeapon == nullptr)
		return false;

	short nDefinitionIndex = pViewmodelWeapon->GetItemDefinitionIndex();

	if (mapItemList.find(nDefinitionIndex) == mapItemList.end() || !pViewmodelWeapon->IsKnife())
		return false;

	long& iSequence = pData->Value.Int;
	iSequence = GetCorrectSequence(nDefinitionIndex, iSequence, pViewmodelOwner);
}