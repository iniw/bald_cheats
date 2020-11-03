#include "eventlistener.h"

// used: gameevent interface
#include "../core/interfaces.h"

/* features event */
#include "../features/visuals.h"
#include "../features/skinchanger.h"
#include "../features/misc.h"

void CEventListener::Setup(const std::deque<const char*> arrEvents)
{
	if (arrEvents.empty())
		return;

	for (auto szEvent : arrEvents)
	{
		I::GameEvent->AddListener(this, szEvent, false);

		if (!I::GameEvent->FindListener(this, szEvent))
			throw std::runtime_error(XorStr("failed to add event listener"));
	}
}

void CEventListener::Destroy()
{
	I::GameEvent->RemoveListener(this);
}

void CEventListener::FireGameEvent(IGameEvent* pEvent)
{
	if (pEvent == nullptr)
		return;

	// get hash of event name
	const FNV1A_t uNameHash = FNV1A::Hash(pEvent->GetName());

	#pragma region eventlistener_handlers
	switch (uNameHash)
	{
	case FNV1A::HashConst("player_death"):
	{
		CSkinChanger::Get().Event(pEvent, uNameHash);
		break;
	}
	case FNV1A::HashConst("player_hurt"):
	{
		CVisuals::Get().Event(pEvent, uNameHash);
		break;
	}
	default:
		break;
	}
	#pragma endregion
}
