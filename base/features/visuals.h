#pragma once
// using: std::deque for visuals objects
#include <deque>

// used: winapi, directx, imgui, fmt includes
#include "../common.h"
// used: vector
#include "../sdk/datatypes/vector.h"
// used: color
#include "../sdk/datatypes/color.h"
// used: baseentity, baseweapon, envtonemapcontroller classes
#include "../sdk/entity.h"
// used: listener event function
#include "../sdk/interfaces/igameeventmanager.h"
// used: convar, engine, cliententitylist, globals, input, materialsystem, model/view render, gameevent interfaces
#include "../core/interfaces.h"

struct Box_t
{
	float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f, width = 0.f, height = 0.f;
};

struct HitMarkerObject_t
{
	Vector	vecPosition;
	int		iDamage;
	float	flTime;
};

static const char* arrVisualsSkyboxes[] =
{
	"default",
	"cs_baggage_skybox",
	"cs_tibet",
	"embassy",
	"italy",
	"jungle",
	"nukeblank",
	"office",
	"sky_cs15_daylight01_hdr",
	"sky_cs15_daylight02_hdr",
	"sky_cs15_daylight03_hdr",
	"sky_cs15_daylight04_hdr",
	"sky_csgo_cloudy01",
	"sky_csgo_night_flat",
	"sky_csgo_night02",
	"sky_day02_05_hdr",
	"sky_day02_05",
	"sky_dust",
	"sky_l4d_rural02_ldr",
	"sky_venice",
	"vertigo_hdr",
	"vertigo",
	"vertigoblue_hdr",
	"vietnam",
	"sky_lunacy"
};

class CEnvTonemapController;
class CVisuals : public CSingleton<CVisuals>
{
public:
	CVisuals()
	{
		constexpr std::string_view szScrollProxies = R"#("texturescroll"
		{
			"texturescrollvar"		"$basetexturetransform"
			"texturescrollrate"		"0.2"
			"texturescrollangle"	"90"
		})#";

		/*
		 * materials navigation:
		 * [N]	[group]		[lit][proxy]
		 *					[1/2] [1/2]
		 *	0 - players		[+/-] [-/-]
		 *	1 - viewmodel	[+/-] [-/-]
		 *	2 - reflects	[+/+] [-/-]
		 *	3 - custom		[+/+] [+/-]
		 */
		m_arrMaterials =
		{
			std::make_pair(CreateMaterial(XorStr("bald_players"), XorStr("VertexLitGeneric")),
			CreateMaterial(XorStr("bald_players_flat"), XorStr("UnlitGeneric"))),

			std::make_pair(CreateMaterial(XorStr("bald_viewmodel"), XorStr("VertexLitGeneric")),
			CreateMaterial(XorStr("bald_viewmodel_flat"), XorStr("UnlitGeneric"))),

			std::make_pair(CreateMaterial(XorStr("bald_reflective"), XorStr("VertexLitGeneric"), XorStr("vgui/white"), XorStr("env_cubemap")),
			CreateMaterial(XorStr("bald_glow"), XorStr("VertexLitGeneric"), XorStr("vgui/white"), XorStr("models/effects/cube_white"))),

			std::make_pair(CreateMaterial(XorStr("bald_scroll"), XorStr("VertexLitGeneric"), XorStr("dev/screenhighlight_pulse"), "", false, false, szScrollProxies),
			I::MaterialSystem->FindMaterial(XorStr("models/inventory_items/hydra_crystal/hydra_crystal_detail"), TEXTURE_GROUP_OTHER))
		};

		// increment references for every material only once
		for (auto& [pFirstMaterial, pSecondMaterial] : m_arrMaterials)
		{
			if (pFirstMaterial != nullptr && !pFirstMaterial->IsErrorMaterial())
				pFirstMaterial->IncrementReferenceCount();

			if (pSecondMaterial != nullptr && !pSecondMaterial->IsErrorMaterial())
				pSecondMaterial->IncrementReferenceCount();
		}
	}
	// Get
	/* sort entities and save data to draw */
	void Store();
	/* get info for hitmarker or e.g. bullettracer */
	void Event(IGameEvent* pEvent, const FNV1A_t uNameHash);
	// Other
	/* color player models, returns true when need clear overrides */
	bool Chams(CBaseEntity* pLocal, DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags);
	/* glow effect on entities like while spectating */
	void Glow(CBaseEntity* pLocal);
	/* modulates materials and changes the skybox to make it look like night */
	void NightMode();
	/* changes the skybox */
	void SkyChanger(EClientFrameStage stage);
	/* removes a few particles n stuff for "better" fps */
	void Removals();

	/* saved hitmarker info's */
	std::deque<HitMarkerObject_t> m_deqHitMarkers = { };

private:
	enum EPaddingDirection : unsigned int
	{
		DIR_LEFT = 0,
		DIR_TOP,
		DIR_RIGHT,
		DIR_BOTTOM,
		DIR_MAX
	};

	struct Context_t
	{
		// bounding box
		Box_t box = { };
		// dormant alpha
		float iAlpha = 0;
		// summary of all directions paddings
		std::array<float, DIR_MAX> arrPadding = { 0, 0, 0, 0 };
	};

	// Extra
	/* get bounding box points of given entity */
	bool GetBoundingBox(CBaseEntity* pEntity, Box_t* pBox) const;
	/* create .vmt materials with customized parameters for chams */
	IMaterial* CreateMaterial(std::string_view szName, std::string_view szShader, std::string_view szBaseTexture = XorStr("vgui/white"), std::string_view szEnvMap = "", bool bIgnorez = false, bool bWireframe = false, std::string_view szProxies = "") const;

	// On-Screen
	void HitMarker(const ImVec2& vecScreenSize, float flServerTime, Color colLines, Color colDamage);
	void SpectatorList(const ImVec2& vecScreenSize, CBaseEntity* pLocal);
	void VelocityIndicator(const ImVec2& vecScreenSize, CBaseEntity* pLocal);

	// World
	/* changes the exposure to make the world looks like at night or fullbright */
	void NightMode(CEnvTonemapController* pController) const; // @credits: sapphyrus
	/* custom fog waw */
	void Fog(CFogController* pController) const;
	/* #savedafishes */
	void Fishes(CFish* pFish, CBaseEntity* pLocal) const;
	/* draw frame with title at bomb position */
	void Bomb(const Vector2D& vecScreen, Context_t& ctx, const Color& colFrame);
	/* draw frame with title, timer and defuse bars at planted bomb position */
	void PlantedBomb(CPlantedC4* pBomb, float flServerTime, const Vector2D& vecScreen, Context_t& ctx, const Color& colFrame, const Color& colDefuse, const Color& colFailDefuse);
	/* draw frame with name and timer bar at grenade position */
	void Grenade(CBaseEntity* pGrenade, EClassIndex nIndex, float flServerTime, float flDistance, const Vector2D& vecScreen, Context_t& ctx, const Color& colFrame, const Color& colBackground, const Color& colOutline);
	/* draw weapon icon, ammo bar, distance at dropped weapons positions */
	void DroppedWeapons(CBaseCombatWeapon* pWeapon, short nItemDefinitionIndex, Context_t& ctx, const float flDistance, const Color& colPrimary, const Color& colAmmo, const Color& colBackground, const Color& colOutline);
	// Player
	/* draw box, bars, text info's, etc for player */
	void Player(CBaseEntity* pLocal, CBaseEntity* pEntity, Context_t& ctx, const float flDistance, const Color& colInfo, const Color& colFrame, const Color& colOutline);

	// Entities
	/* draw entity bounding box */
	void Box(const Box_t& box, const Color& colPrimary, const Color& colOutline);
	/* draw vertical line with health-based height */
	void HealthBar(Context_t& ctx, const float flHealth, const int iMaxHealth, const int iFactor, const Color& colPrimary, const Color& colBackground, const Color& colOutline);
	/* draw horizontal line with ammo-based width */
	void AmmoBar(CBaseEntity* pEntity, CBaseCombatWeapon* pWeapon, Context_t& ctx, const Color& colPrimary, const Color& colBackground, const Color& colOutline);
	/* draw horizontal line with flashed duration-based width */
	void FlashBar(CBaseEntity* pEntity, Context_t& ctx, const Color& colPrimary, const Color& colBackground, const Color& colOutline);
	/* draw lines on some of the entitie's bones */
	void Skeleton(CBaseEntity* pEntity);
	/* draws dots on the head of every record for that entity */
	void VisualizeBacktrack(int iEntityIndex);

	/* chams materials */
	std::array<std::pair<IMaterial*, IMaterial*>, 4U> m_arrMaterials = { };
};
