#include "entitylistener.h"

// used: cliententitylist interface
#include "../core/interfaces.h"
// used: IsValid()	
#include "../features/legitbot.h"

void CEntityListener::Setup()
{
	vecLegitBotEntities.clear();
	vecEntities.clear();

	for (int i = 1; i <= I::Globals->nMaxClients; i++)
	{
		if (i == I::Engine->GetLocalPlayer())
			continue;

		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);
		if (pEntity == nullptr)
			continue;

		CBaseClient* pClientClass = pEntity->GetClientClass();
		if (pClientClass == nullptr)
			return;

		switch (pClientClass->nClassID)
		{
		case EClassIndex::CCSPlayer:
			vecEntities.push_back(EntityObject_t(pEntity, i));

			if (CLegitBot::Get().IsValid(pEntity))
				vecLegitBotEntities.push_back(EntityObject_t(pEntity, i));

			break;
		default:
			break;
		}
	}

	I::ClientEntityList->AddListenerEntity(this);
	L::Print(XorStr("entity listener created"));
}

void CEntityListener::Destroy()
{
	I::ClientEntityList->RemoveListenerEntity(this);
}

void CEntityListener::OnEntityCreated(CBaseEntity* pEntity)
{
	if (pEntity == nullptr)
		return;

	const int nIndex = pEntity->GetIndex();
	if (nIndex < 0)
		return;

	CBaseClient* pClientClass = pEntity->GetClientClass();
	if (pClientClass == nullptr)
		return;

	PlayerInfo_t pInfo;
	if (!I::Engine->GetPlayerInfo(pEntity->GetIndex(), &pInfo))
		return;

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (pLocal == nullptr)
		return;

	// entity class check
	switch (pClientClass->nClassID)
	{
	case EClassIndex::CCSPlayer:
		vecEntities.push_back(EntityObject_t(pEntity, nIndex));

		if (CLegitBot::Get().IsValid(pEntity))
			vecLegitBotEntities.push_back(EntityObject_t(pEntity, nIndex));

		break;
	default:
		break;
	}
}

void CEntityListener::OnEntityDeleted(CBaseEntity* pEntity)
{
	if (pEntity == nullptr)
		return;

	const int nIndex = pEntity->GetIndex();
	if (nIndex < 0)
		return;

	PlayerInfo_t pInfo;
	if (!I::Engine->GetPlayerInfo(pEntity->GetIndex(), &pInfo))
		return;

	// get current player entry
	const auto current = std::find_if(vecEntities.begin(), vecEntities.end(),
		// compare entry index
		[&](const EntityObject_t& entry)
		{
			return entry.nIndex == nIndex;
		});

	if (current != vecEntities.end())
		vecEntities.erase(current);

	if (current != vecLegitBotEntities.end())
		vecLegitBotEntities.erase(current);
}
