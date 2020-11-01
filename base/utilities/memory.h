#pragma once
// used: std::vector for getfilledvector
#include <vector>
// used: winapi, fmt includes
#include "../common.h"
#include <map>
#include <unordered_map>

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

	template <typename T1, typename T2, std::size_t S>
	std::map<T1, T2> GetFilledMap(const T1& fill1, const T2& fill2)
	{
		std::map<T1, T2> mapTemp;

		struct SkinObject_t
		{
			const char* szName = "";
			const char* szModel = "";
			const char* szKillIcon = "";

			SkinObject_t(const char* _szName, const char* _szModel, const char* _szKillIcon)
			{
				szName = _szName;
				szModel = _szModel;
				szKillIcon = _szKillIcon;
			}

			SkinObject_t()
			{
				szName = nullptr;
				szModel = nullptr;
				szKillIcon = nullptr;
			}
		};

		static std::unordered_map<int, SkinObject_t> mapItemList =
		{
			{ WEAPON_DEAGLE, { "Desert Eagle", "models/weapons/v_pist_deagle.mdl", "deagle" } },
			{ WEAPON_ELITE, { "Dual Berettas", "models/weapons/v_pist_elite.mdl", "elite" } },
			{ WEAPON_FIVESEVEN, { "Five-SeveN", "models/weapons/v_pist_fiveseven.mdl", "fiveseven" } },
			{ WEAPON_GLOCK, { "Glock-18", "models/weapons/v_pist_glock18.mdl", "glock" } },
			{ WEAPON_AK47, { "AK-47", "models/weapons/v_rif_ak47.mdl", "ak47" } },
			{ WEAPON_AUG, { "AUG", "models/weapons/v_rif_aug.mdl", "aug" } },
			{ WEAPON_AWP, { "AWP", "models/weapons/v_snip_awp.mdl", "awp" } },
			{ WEAPON_FAMAS, { "FAMAS", "models/weapons/v_rif_famas.mdl", "famas" } },
			{ WEAPON_G3SG1, { "G3SG1", "models/weapons/v_snip_g3sg1.mdl", "g3sg1" } },
			{ WEAPON_GALILAR, { "Galil AR", "models/weapons/v_rif_galilar.mdl", "galilar" } },
			{ WEAPON_M249, { "M249", "models/weapons/v_mach_m249para.mdl", "m249" } },
			{ WEAPON_M4A1, { "M4A4", "models/weapons/v_rif_m4a1.mdl", "m4a1" } },
			{ WEAPON_MAC10, { "MAC-10", "models/weapons/v_smg_mac10.mdl", "mac10" } },
			{ WEAPON_P90, { "P90", "models/weapons/v_smg_p90.mdl", "p90" } },
			{ WEAPON_MP5SD, { "MP5-SD", "models/weapons/v_smg_mp5sd.mdl", "mp5sd" } },
			{ WEAPON_UMP45, { "UMP-45", "models/weapons/v_smg_ump45.mdl", "ump45" } },
			{ WEAPON_XM1014, { "XM1014", "models/weapons/v_shot_xm1014.mdl", "xm1014" } },
			{ WEAPON_BIZON, { "PP-Bizon", "models/weapons/v_smg_bizon.mdl", "bizon" } },
			{ WEAPON_MAG7, { "MAG-7", "models/weapons/v_shot_mag7.mdl", "mag7" } },
			{ WEAPON_NEGEV, { "Negev", "models/weapons/v_mach_negev.mdl", "negev" } },
			{ WEAPON_SAWEDOFF, { "Sawed-Off", "models/weapons/v_shot_sawedoff.mdl", "sawedoff" } },
			{ WEAPON_TEC9, { "Tec-9", "models/weapons/v_pist_tec9.mdl", "tec9" } },
			{ WEAPON_HKP2000, { "P2000", "models/weapons/v_pist_hkp2000.mdl", "hkp2000" } },
			{ WEAPON_MP7, { "MP7", "models/weapons/v_smg_mp7.mdl", "mp7" } },
			{ WEAPON_MP9, { "MP9", "models/weapons/v_smg_mp9.mdl", "mp9" } },
			{ WEAPON_NOVA, { "Nova", "models/weapons/v_shot_nova.mdl", "nova" } },
			{ WEAPON_P250, { "P250", "models/weapons/v_pist_p250.mdl", "p250" } },
			{ WEAPON_SCAR20, { "SCAR-20", "models/weapons/v_snip_scar20.mdl", "scar20" } },
			{ WEAPON_SG556, { "SG 553", "models/weapons/v_rif_sg556.mdl", "sg556" } },
			{ WEAPON_SSG08, { "SSG 08", "models/weapons/v_snip_ssg08.mdl", "ssg08" } },
			{ WEAPON_KNIFE, { "Knife (Counter-Terrorists)", "models/weapons/v_knife_default_ct.mdl", "knife_default_ct" } },
			{ WEAPON_KNIFE_T, { "Knife (Terrorists)", "models/weapons/v_knife_default_t.mdl", "knife_t" } },
			{ WEAPON_M4A1_SILENCER, { "M4A1-S", "models/weapons/v_rif_m4a1_s.mdl", "m4a1_silencer" } },
			{ WEAPON_USP_SILENCER, { "USP-S", "models/weapons/v_pist_223.mdl", "usp_silencer" } },
			{ WEAPON_CZ75A, { "CZ75 Auto", "models/weapons/v_pist_cz_75.mdl", "cz75a" } },
			{ WEAPON_REVOLVER, { "R8 Revolver", "models/weapons/v_pist_revolver.mdl", "revolver" } },
			{ WEAPON_KNIFE_BAYONET, { "Bayonet", "models/weapons/v_knife_bayonet.mdl", "bayonet" } },
			{ WEAPON_KNIFE_FLIP, { "Flip Knife", "models/weapons/v_knife_flip.mdl", "knife_flip" } },
			{ WEAPON_KNIFE_GUT, { "Gut Knife", "models/weapons/v_knife_gut.mdl", "knife_gut" } },
			{ WEAPON_KNIFE_KARAMBIT, { "Karambit", "models/weapons/v_knife_karam.mdl", "knife_karambit" } },
			{ WEAPON_KNIFE_M9_BAYONET, { "M9 Bayonet", "models/weapons/v_knife_m9_bay.mdl", "knife_m9_bayonet" } },
			{ WEAPON_KNIFE_TACTICAL, { "Huntsman Knife", "models/weapons/v_knife_tactical.mdl", "knife_tactical" } },
			{ WEAPON_KNIFE_FALCHION, { "Falchion Knife", "models/weapons/v_knife_falchion_advanced.mdl", "knife_falchion" } },
			{ WEAPON_KNIFE_SURVIVAL_BOWIE, { "Bowie Knife", "models/weapons/v_knife_survival_bowie.mdl", "knife_survival_bowie" } },
			{ WEAPON_KNIFE_BUTTERFLY, { "Butterfly Knife", "models/weapons/v_knife_butterfly.mdl", "knife_butterfly" } },
			{ WEAPON_KNIFE_PUSH, { "Shadow Daggers", "models/weapons/v_knife_push.mdl", "knife_push" } },
			{ WEAPON_KNIFE_CORD, { "Paracord Knife", "models/weapons/v_knife_cord.mdl", "knife_cord" } },
			{ WEAPON_KNIFE_CANIS, { "Survival Knife", "models/weapons/v_knife_canis.mdl", "knife_canis" } },
			{ WEAPON_KNIFE_URSUS, { "Ursus Knife", "models/weapons/v_knife_ursus.mdl", "knife_ursus" } },
			{ WEAPON_KNIFE_GYPSY_JACKKNIFE, { "Navaja Knife", "models/weapons/v_knife_gypsy_jackknife.mdl", "knife_gypsy_jackknife" } },
			{ WEAPON_KNIFE_OUTDOOR, { "Nomad Knife", "models/weapons/v_knife_outdoor.mdl", "knife_outdoor" } },
			{ WEAPON_KNIFE_STILETTO, { "Stiletto Knife", "models/weapons/v_knife_stiletto.mdl", "knife_stiletto" } },
			{ WEAPON_KNIFE_WIDOWMAKER, { "Talon Knife", "models/weapons/v_knife_widowmaker.mdl", "knife_widowmaker" } },
			{ WEAPON_KNIFE_SKELETON, { "Skeleton Knife", "models/weapons/v_knife_skeleton.mdl", "knife_skeleton" } },
			{ GLOVE_STUDDED_BLOODHOUND, { "Bloodhound Gloves", "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl", "" } },
			{ GLOVE_T, { "Gloves (Terrorists)", "models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl", "" } },
			{ GLOVE_CT, {  "Gloves (Counter-Terrorists)", "models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl", "" } },
			{ GLOVE_SPORTY, {  "Vice Gloves", "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl", "" } },
			{ GLOVE_SLICK, {  "Driver gloves", "models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl", "" } },
			{ GLOVE_LEATHER_HANDWRAPS, {  "Hand Wraps Gloves", "models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl", "" } },
			{ GLOVE_MOTORCYCLE, {  "Moto Gloves", "models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl", "" } },
			{ GLOVE_SPECIALIST, {  "Specialist Gloves", "models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl", "" } },
			{ GLOVE_HYDRA, {  "Hydra Gloves", "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_hydra.mdl", "" } }
		};
		
		for (auto& value : mapItemList)
		{
			mapTemp[value.first] = fill2;
		}

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
