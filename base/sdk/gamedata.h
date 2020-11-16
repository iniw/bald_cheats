#pragma once
#include <map>
#include <string>
#include <unordered_map>

// used: itemdefinitionindex; baseentity, baseweapon classes
#include "../sdk/entity.h"

namespace GD
{
	struct PaintKit_t
	{
		int iId;
		std::string szName;

		auto operator < (const PaintKit_t& other) const -> bool
		{
			return szName < other.szName;
		}
	};

	struct WeaponInfo_t
	{
		constexpr WeaponInfo_t(const char* szModel, const char* szIcon = nullptr) :
			szModel(szModel),
			szIcon(szIcon)
		{}

		const char* szModel;
		const char* szIcon;
	};

	struct WeaponName_t
	{
		constexpr WeaponName_t(const int iDefinitionIndex, const char* szName) :
			iDefinitionIndex(iDefinitionIndex),
			szName(szName)
		{}

		int iDefinitionIndex = 0;
		const char* szName = nullptr;
	};

	struct QualityName_t
	{
		constexpr QualityName_t(const int iIndex, const char* szName) :
			iIndex(iIndex),
			szName(szName)
		{}

		int iIndex = 0;
		const char* szName = nullptr;
	};
}

static constexpr std::pair<int, const char*>  arrKnifeNames[]
{
	{WEAPON_NONE, "default"},
	{WEAPON_KNIFE, "default ct"},
	{WEAPON_KNIFE_T, "default t"},
	{WEAPON_KNIFE_BAYONET, "bayonet"},
	{WEAPON_KNIFE_SURVIVAL_BOWIE, "bowie knife"},
	{WEAPON_KNIFE_BUTTERFLY, "butterfly knife"},
	{WEAPON_KNIFE_CSS, "classic knife"},
	{WEAPON_KNIFE_FALCHION, "falchion knife"},
	{WEAPON_KNIFE_FLIP, "flip knife"},
	{WEAPON_KNIFE_GUT, "gut knife"},
	{WEAPON_KNIFE_TACTICAL, "huntsman knife"},
	{WEAPON_KNIFE_KARAMBIT, "karambit"},
	{WEAPON_KNIFE_M9_BAYONET, "m9 bayonet"},
	{WEAPON_KNIFE_GYPSY_JACKKNIFE, "navaja knife"},
	{WEAPON_KNIFE_OUTDOOR, "nomad knife"},
	{WEAPON_KNIFE_CORD, "paracord knife"},
	{WEAPON_KNIFE_PUSH, "shadow daggers"},
	{WEAPON_KNIFE_SKELETON, "skeleton knife"},
	{WEAPON_KNIFE_STILETTO, "stiletto knife"},
	{WEAPON_KNIFE_CANIS, "survival knife"},
	{WEAPON_KNIFE_WIDOWMAKER, "talon knife"},
	{WEAPON_KNIFE_URSUS, "ursus knife"}
};

static constexpr std::pair<int, const char*> arrGlovesNames[] =
{
	{WEAPON_NONE, "default"},
	{GLOVE_STUDDED_BLOODHOUND, "bloodhound"},
	{GLOVE_LEATHER_HANDWRAPS, "handwraps"},
	{GLOVE_HYDRA, "hydra"},
	{GLOVE_SLICK, "driver"},
	{GLOVE_SPORTY, "sport"},
	{GLOVE_MOTORCYCLE, "motorcycle"},
	{GLOVE_SPECIALIST, "specialist"}
};

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

// @note: u can find viewmodel indexes with "sv_precacheinfo"
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
	{ WEAPON_KNIFE, { "Knife (Counter-Terrorists)", "models/weapons/v_knife_default_ct.mdl", "knife" } },
	{ WEAPON_KNIFE_T, { "Knife (Terrorists)", "models/weapons/v_knife_default_t.mdl", "knife_t" } },
	{ WEAPON_M4A1_SILENCER, { "M4A1-S", "models/weapons/v_rif_m4a1_s.mdl", "m4a1_silencer" } },
	{ WEAPON_USP_SILENCER, { "USP-S", "models/weapons/v_pist_223.mdl", "usp_silencer" } },
	{ WEAPON_CZ75A, { "CZ75 Auto", "models/weapons/v_pist_cz_75.mdl", "cz75a" } },
	{ WEAPON_REVOLVER, { "R8 Revolver", "models/weapons/v_pist_revolver.mdl", "revolver" } },
	{ WEAPON_KNIFE_BAYONET, { "Bayonet", "models/weapons/v_knife_bayonet.mdl", "bayonet" } },
	{ WEAPON_KNIFE_CSS, { "Classic Knife",  "models/weapons/v_knife_css.mdl", "knife_css"} },
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

static const std::pair<std::size_t, std::string> arrItemQuality[] =
{
	{ 0, "Normal" },
	{ 1, "Genuine" },
	{ 2, "Vintage" },
	{ 3, "Unusual" },
	{ 5, "Community" },
	{ 6, "Developer" },
	{ 7, "Self-Made" },
	{ 8, "Customized" },
	{ 9, "Strange" },
	{ 10, "Completed" },
	{ 12, "Tournament" }
};