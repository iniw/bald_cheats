#pragma once
// used: std::vector for getfilledvector
#include <vector>
// used: std::map for getfilledmap
#include <map>
// used: winapi, fmt includes
#include "../utilities/logging.h"
#include "../common.h"

#pragma region memory_modules_definitions
#define ENGINE_DLL				XorStr("engine.dll")
#define CLIENT_DLL				XorStr("client.dll")
#define LOCALIZE_DLL			XorStr("localize.dll")
#define MATERIALSYSTEM_DLL		XorStr("materialsystem.dll")
#define VGUI_DLL				XorStr("vguimatsurface.dll")
#define VGUI2_DLL				XorStr("vgui2.dll")
#define SHADERPIDX9_DLL			XorStr("shaderapidx9.dll")
#define GAMEOVERLAYRENDERER_DLL XorStr("gameoverlayrenderer.dll")
#define PHYSICS_DLL				XorStr("vphysics.dll")
#define VSTDLIB_DLL				XorStr("vstdlib.dll")
#define TIER0_DLL				XorStr("tier0.dll")
#define INPUTSYSTEM_DLL			XorStr("inputsystem.dll")
#define STUDIORENDER_DLL		XorStr("studiorender.dll")
#define DATACACHE_DLL			XorStr("datacache.dll")
#define STEAM_API_DLL			XorStr("steam_api.dll")
#define MATCHMAKING_DLL			XorStr("matchmaking.dll")
#define SERVER_DLL				XorStr("server.dll")
#define SERVERBROWSER_DLL		XorStr("serverbrowser.dll")
#pragma endregion

#pragma region memory_pattern_convert
#define INRANGE( x, min, max ) (x >= min && x <= max) 
#define GETBITS( x ) ( INRANGE( ( x&( ~0x20 ) ), 'A', 'F' ) ? ( ( x&( ~0x20 ) ) - 'A' + 0xA) : ( INRANGE( x, '0', '9' ) ? x - '0' : 0 ) )
#define GETBYTE( x ) ( GETBITS( x[0] ) << 4 | GETBITS( x[1] ) )
#pragma endregion

/*
 * singleton implementation
 * restricts the instantiation of a class to one single class instance
 */
template <typename T>
class CSingleton
{
protected:
	CSingleton() { }
	~CSingleton() { }

	CSingleton(const CSingleton&) = delete;
	CSingleton& operator=(const CSingleton&) = delete;

	CSingleton(CSingleton&&) = delete;
	CSingleton& operator=(CSingleton&&) = delete;
public:
	static T& Get()
	{
		static T pInstance{ };
		return pInstance;
	}
};

/* virtualprotect raii wrapper */
class CWrappedProtect
{
public:
	CWrappedProtect(void* pBaseAddress, std::size_t uLength, DWORD dwFlags)
	{
		this->pBaseAddress = pBaseAddress;
		this->uLength = uLength;

		if (!VirtualProtect(pBaseAddress, uLength, dwFlags, &dwOldFlags))
			throw std::system_error(GetLastError(), std::system_category(), XorStr("failed to protect region"));
	}

	~CWrappedProtect()
	{
		VirtualProtect(pBaseAddress, uLength, dwOldFlags, &dwOldFlags);
	}
private:
	void*			pBaseAddress;
	std::size_t		uLength;
	DWORD			dwOldFlags;
};

/*
 * MEMORY
 * memory management functions
 */
namespace MEM
{
	// Get
	/* ida style byte pattern comparison */
	std::uintptr_t	FindPattern(const char* szModuleName, const char* szPattern);
	/* ida style byte pattern comparison with native given address */
	std::uintptr_t	FindPattern(std::uint8_t* uRegionStart, std::uintptr_t uRegionSize, const char* szPattern);
	/* returns pointer to given vtable name @credits: hinnie */
	std::uintptr_t* GetVTablePointer(std::string_view szModuleName, std::string_view szTableName);
	/* checks is we have given section in given address */
	bool			GetSectionInfo(std::uintptr_t uBaseAddress, const std::string& szSectionName, std::uintptr_t& uSectionStart, std::uintptr_t& uSectionSize);
	/* returns xrefs addresses for given address */
	std::vector<std::uintptr_t> GetXrefs(std::uintptr_t uAddress, std::uintptr_t uStart, std::size_t uSize);

	// Convert
	std::vector<int> PatternToBytes(const char* szPattern);
	std::string		BytesToPattern(std::uint8_t* arrBytes, std::size_t uSize);

	// Check
	/* can we read/readwrite given memory region */
	bool			IsValidCodePtr(std::uintptr_t uAddress);

	/* returns vector filled with given value */
	template <typename T, std::size_t S>
	std::vector<T> GetFilledVector(const T& fill)
	{
		std::vector<T> vecTemp(S);
		std::fill(vecTemp.begin(), vecTemp.begin() + S, fill);
		return vecTemp;
	}

	template <typename T1, typename T2>
	std::map<T1, T2> GetFilledMap(const T1& fill1, const T2& fill2)
	{
		std::map<T1, T2> mapTemp;

		static constexpr std::pair<int, const char*> arrWeaponNames[]
		{
				{WEAPON_KNIFE, "knife"},
				{GLOVE_T, "glove"},
				{WEAPON_AK47, "ak-47"},
				{WEAPON_AUG, "aug"},
				{WEAPON_AWP, "awp"},
				{WEAPON_CZ75A, "cz75 auto"},
				{WEAPON_DEAGLE, "desert eagle"},
				{WEAPON_ELITE, "dual berettas"},
				{WEAPON_FAMAS, "famas"},
				{WEAPON_FIVESEVEN, "five-seven"},
				{WEAPON_G3SG1, "g3sg1"},
				{WEAPON_GALILAR, "galil ar"},
				{WEAPON_GLOCK, "glock-18"},
				{WEAPON_M249, "m249"},
				{WEAPON_M4A1_SILENCER, "m4a1-s"},
				{WEAPON_M4A1, "m4a4"},
				{WEAPON_MAC10, "mac-10"},
				{WEAPON_MAG7, "mag-7"},
				{WEAPON_MP5SD, "mp5-sd"},
				{WEAPON_MP7, "mp7"},
				{WEAPON_MP9, "mp9"},
				{WEAPON_NEGEV, "negev"},
				{WEAPON_NOVA, "nova"},
				{WEAPON_HKP2000, "p2000"},
				{WEAPON_P250, "p250"},
				{WEAPON_P90, "p90"},
				{WEAPON_BIZON, "pp-bizon"},
				{WEAPON_REVOLVER, "r8 revolver"},
				{WEAPON_SAWEDOFF, "sawed-off"},
				{WEAPON_SCAR20, "scar-20"},
				{WEAPON_SSG08, "ssg 08"},
				{WEAPON_SG556, "sg 553"},
				{WEAPON_TEC9, "tec-9"},
				{WEAPON_UMP45, "ump-45"},
				{WEAPON_USP_SILENCER, "usp-s"},
				{WEAPON_XM1014, "xm1014"},
		};

		for(int i = 0; i < 36; i++)
			mapTemp[arrWeaponNames[i].first] = fill2;

		return mapTemp;
	}

	/*
	 * virtual function implementation
	 * returns native function of specified class at given index
	 */
	template <typename T = void*>
	constexpr T GetVFunc(void* thisptr, std::size_t nIndex)
	{
		return (*static_cast<T**>(thisptr))[nIndex];
	}

	/*
	 * virtual function call implementation
	 * calls function of specified class at given index
	 * @note: doesnt adding references automatic and needs to add it manually!
	 */
	template <typename T, typename ... args_t>
	constexpr T CallVFunc(void* thisptr, std::size_t nIndex, args_t... argList)
	{
		using VirtualFn = T(__thiscall*)(void*, decltype(argList)...);
		return (*static_cast<VirtualFn**>(thisptr))[nIndex](thisptr, argList...);
	}
}
