#pragma once
// used: winapi, fmt includes
#include "common.h"
// used: vector
#include "sdk/datatypes/vector.h"
// used: keyvalues
#include "sdk/datatypes/keyvalues.h"
// used: baseentity, handleentity
#include "sdk/entity.h"
// used: material
#include "sdk/interfaces/imaterialsystem.h"
// used: events listerner setup
#include "utilities/eventlistener.h"
// used: entity listerner setup
#include "utilities/entitylistener.h"

/* internal implementation for measuring specific time intervals */
class CTimer
{
public:
	CTimer(bool bStart = false)
	{
		if (bStart)
			Reset();
	}

	/* set time point to now */
	void Reset()
	{
		timePoint = std::chrono::high_resolution_clock::now();
	}

	/* returns elapsed time between last time point and now in given duration type (default: milliseconds) */
	template <class C = std::chrono::milliseconds>
	long long Elapsed() const
	{
		return std::chrono::duration_cast<C>(std::chrono::high_resolution_clock::now() - timePoint).count();
	}

private:
	std::chrono::high_resolution_clock::time_point timePoint = { };
};

/*
 * UTILITIES
 * third-party functions-helpers
 */
namespace U
{
	struct HudWeapons_t {
		std::int32_t* GetWeaponCount() {
			return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
		}
	};

	// Get
	/* returns class of hud element with given name */
	template <class C>
	C* FindHudElement(const char* szName);

	// Game
	/* updates hud, clears delta tick */
	void ForceFullUpdate();
	/* return true if ray goes through smoke */
	bool LineGoesThroughSmoke(const Vector& vecStart, const Vector& vecEnd, const bool bGrenadeBloat = true);
	/* set localplayer accepted competitive game */
	void SetLocalPlayerReady(); // @credits: oneshotgh
	/* set given string to nickname */
	void SendName(const char* szName);
	/* set given string to clantag */
	void SendClanTag(const char* szClanTag, const char* szIdentifier);
	/* load specified model before you will be in-game */
	bool PrecacheModel(const char* szModelName);
	/* loads the skybox with that name */
	void LoadSkybox(const char* szSkyName);
	/* allocate client dll data for the object */
	IClientNetworkable* CreateDLLEntity(int iEntity, EClassIndex nClassID, int nSerial);
	/* returns icon of given index for custom font */
	const char* GetWeaponIcon(short nItemDefinitionIndex);
	/* lenght of a utf-8 encoded string */
	int UTF8Len(char szFirstByte);

	// Extra
	/* flashes the specified window */
	void FlashWindow(HWND pWindow);

	// String
	/* converts from unicode to ascii string */
	std::string UnicodeAscii(const std::wstring_view wszUnicode);
	/* converts from ascii to unicode string */
	std::wstring AsciiUnicode(const std::string_view szAscii);

	// Links
	/* event listener implementation */
	inline CEventListener EventListener;
	/* entity listener implementation */
	inline CEntityListener EntityListener;
}
