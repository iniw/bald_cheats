// used: std::call_once
#include <mutex>

#include "visuals.h"
// used: camera origin global variable
#include "../global.h"
// used: keyvalues for materials
#include "../sdk/datatypes/keyvalues.h"
// used: cheat variables
#include "../core/variables.h"
// used: main window open state
#include "../core/menu.h"
// used: drawmodelexecute original for chams
#include "../core/hooks.h"
// used: render functions
#include "../utilities/draw.h"
// used: vectortransformer to get bounding box
#include "../utilities/math.h"
// used: get weapon icon
#include "../utilities.h"
// used: _bstr_t
#include <comdef.h>
#include "../features/lagcompensation.h"

// @note: avoid store imcolor, store either u32 of imvec4
void CVisuals::Store()
{
	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();

	if (pLocal == nullptr)
		return;

	const float flServerTime = TICKS_TO_TIME(pLocal->GetTickBase());

	// sniper crosshair
	static CConVar* weapon_debug_spread_show = I::ConVar->FindVar(XorStr("weapon_debug_spread_show"));
	weapon_debug_spread_show->fnChangeCallbacks.Size() = NULL;
	weapon_debug_spread_show->SetValue((C::Get<bool>(Vars.bScreenSniperCrosshair) && pLocal->IsAlive() && !pLocal->IsScoped()) ? 3 : 0);

	// check is render initialized
	if (!D::bInitialized)
		return;

	const ImVec2 vecScreenSize = ImGui::GetIO().DisplaySize;

	#pragma region visuals_store_world
	// render scope lines
	if (auto pWeapon = pLocal->GetWeapon(); pWeapon != nullptr && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SCOPE))
	{
		CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(pWeapon->GetItemDefinitionIndex());

		// is sniper and scoped
		if (pWeaponData != nullptr && pWeaponData->nWeaponType == WEAPONTYPE_SNIPER && pLocal->IsScoped())
		{
			float flWidth = pWeapon->GetInaccuracy() * 300.f;
			int iAlpha = std::min(255, static_cast<int>(255.f * pWeapon->GetInaccuracy()));

			D::AddLine(ImVec2(0.f, vecScreenSize.y * 0.5f), ImVec2(vecScreenSize.x, vecScreenSize.y * 0.5f), Color(0, 0, 0, 200));
			D::AddLine(ImVec2(vecScreenSize.x * 0.5f, 0.f), ImVec2(vecScreenSize.x * 0.5f, vecScreenSize.y), Color(0, 0, 0, 200));

			// horizontal
			D::AddRectMultiColor(ImVec2(0.f, vecScreenSize.y * 0.5f), ImVec2(vecScreenSize.x, vecScreenSize.y * 0.5f + flWidth), Color(0, 0, 0, iAlpha), Color(0, 0, 0, iAlpha), Color(0, 0, 0, 0), Color(0, 0, 0, 0));
			D::AddRectMultiColor(ImVec2(0.f, vecScreenSize.y * 0.5f - flWidth), ImVec2(vecScreenSize.x, vecScreenSize.y * 0.5f), Color(0, 0, 0, 0), Color(0, 0, 0, 0), Color(0, 0, 0, iAlpha), Color(0, 0, 0, iAlpha));
			// vertical
			D::AddRectMultiColor(ImVec2(vecScreenSize.x * 0.5f, 0.f), ImVec2(vecScreenSize.x * 0.5f + flWidth, vecScreenSize.y), Color(0, 0, 0, iAlpha), Color(0, 0, 0, 0), Color(0, 0, 0, 0), Color(0, 0, 0, iAlpha));
			D::AddRectMultiColor(ImVec2(vecScreenSize.x * 0.5f - flWidth, 0.f), ImVec2(vecScreenSize.x * 0.5f, vecScreenSize.y), Color(0, 0, 0, 0), Color(0, 0, 0, iAlpha), Color(0, 0, 0, iAlpha), Color(0, 0, 0, 0));
		}
	}
	#pragma endregion

	#pragma region visuals_store_screen

	if (C::Get<bool>(Vars.bScreenHitMarker))
		HitMarker(vecScreenSize, flServerTime, C::Get<Color>(Vars.colScreenHitMarker), C::Get<Color>(Vars.colScreenHitMarkerDamage));

	if (C::Get<bool>(Vars.bSpectatorList))
		SpectatorList(vecScreenSize, pLocal);

	if (C::Get<bool>(Vars.bMiscVeloIndicator) && pLocal->IsAlive())
		VelocityIndicator(vecScreenSize, pLocal);

	#pragma endregion

	#pragma region visuals_store_esp
	std::vector<std::pair<CBaseEntity*, float>> vecOrder = { };

	for (int i = 1; i < I::ClientEntityList->GetMaxEntities(); i++)
	{
		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);

		if (pEntity == nullptr || pEntity->IsDormant())
			continue;

		const Vector vecOrigin = pEntity->GetOrigin();

		// save entities and calculated distance for sort
		vecOrder.emplace_back(std::make_pair(pEntity, (pEntity->GetRenderOrigin() - G::vecCamera).Length()));
	}

	// sort entities by distance to make closest entity drawn last to make it easy readable and look nicer
	std::sort(vecOrder.begin(), vecOrder.end(), [](const std::pair<CBaseEntity*, float>& a, const std::pair<CBaseEntity*, float>& b)
		{
			return a.second > b.second;
		});


	for (const auto& [pEntity, flDistance] : vecOrder)
	{
		CBaseClient* pClientClass = pEntity->GetClientClass();

		if (pClientClass == nullptr)
			continue;

		const EClassIndex nIndex = pClientClass->nClassID;

		switch (nIndex)
		{
		case EClassIndex::CC4:
		{
			if (!C::Get<bool>(Vars.bEspMain) || !C::Get<bool>(Vars.bEspMainBomb))
				break;

			// get bomb owner
			const CBaseEntity* pOwner = I::ClientEntityList->Get<CBaseEntity>(pEntity->GetOwnerEntityHandle());

			// check only for dropped bomb, for bomb carrier need another way
			if (pOwner != nullptr)
				break;

			const Vector vecOrigin = pEntity->GetOrigin();
			Vector2D vecScreen = { };

			// get bomb on-screen position
			if (!D::WorldToScreen(vecOrigin, vecScreen))
				break;

			// create bomb context
			Context_t ctx = { };

			Bomb(vecScreen, ctx, Color(40, 40, 40, 200));
			break;
		}
		case EClassIndex::CPlantedC4:
		{
			if (!C::Get<bool>(Vars.bEspMain) || !C::Get<bool>(Vars.bEspMainBomb))
				break;

			// cast to planted bomb entity
			CPlantedC4* pBomb = reinterpret_cast<CPlantedC4*>(pEntity);

			if (!pBomb->IsPlanted())
				break;

			const Vector vecOrigin = pEntity->GetOrigin();
			Vector2D vecScreen = { };

			// get planted bomb on-screen position
			if (!D::WorldToScreen(vecOrigin, vecScreen))
				break;

			// setup planted bomb context
			Context_t ctx = { };

			PlantedBomb(pBomb, flServerTime, vecScreen, ctx, Color(20, 20, 20, 150), Color(80, 180, 200, 200), Color(255, 100, 100));
			break;
		}
		case EClassIndex::CCSPlayer:
		{
			if (!C::Get<bool>(Vars.bEspMain) || !pEntity->IsAlive())
				break;

			if (!pLocal->IsAlive())
			{
				// check is not spectating current entity
				if (const auto pObserverEntity = I::ClientEntityList->Get<CBaseEntity>(pLocal->GetObserverTargetHandle()); pObserverEntity != nullptr && pObserverEntity == pEntity && *pLocal->GetObserverMode() == OBS_MODE_IN_EYE)
					break;
			}

			if (pEntity != pLocal && (pLocal->IsEnemy(pEntity) && C::Get<bool>(Vars.bEspMainEnemies)))
			{
				// set entity spotted in-game radar
				if (C::Get<bool>(Vars.bEspMainPlayerRadar))
					*pEntity->IsSpotted() = true;

				// setup player context
				Context_t ctx = { };

				// get player bounding box
				if (!GetBoundingBox(pEntity, &ctx.box))
					break;

				Player(pLocal, pEntity, ctx, flDistance, Color(255, 255, 255, 255), Color(20, 20, 20, 150), Color(0, 0, 0, 220));			
			}

			break;
		}
		case EClassIndex::CEnvTonemapController:
		{
			NightMode(reinterpret_cast<CEnvTonemapController*>(pEntity));
			break;
		}
		case EClassIndex::CFogController:
		{
			Fog(reinterpret_cast<CFogController*>(pEntity));
			break;
		}
		case EClassIndex::CFish:
		{
			if (C::Get<bool>(Vars.bWorldFish))
				Fishes(reinterpret_cast<CFish*>(pEntity), pLocal);
			break;
		}
		case EClassIndex::CBaseCSGrenadeProjectile:
		case EClassIndex::CDecoyProjectile:
		case EClassIndex::CSmokeGrenadeProjectile:
		case EClassIndex::CMolotovProjectile:
		case EClassIndex::CInferno:
		{
			if (!C::Get<bool>(Vars.bEspMain) || !C::Get<bool>(Vars.bEspMainGrenades))
				break;

			const Vector vecOrigin = pEntity->GetOrigin();
			Vector2D vecScreen = { };

			// get grenade on-screen position
			if (!D::WorldToScreen(vecOrigin, vecScreen))
				break;

			// setup grenade context
			Context_t ctx = { };

			Grenade(pEntity, nIndex, flServerTime, flDistance, vecScreen, ctx, Color(20, 20, 20, 150), Color(40, 40, 40, 100), Color(0, 0, 0, 100));
			break;
		}
		default:
		{
			// check for esp state and skip weapon in hands
			if (!C::Get<bool>(Vars.bEspMain) || !C::Get<bool>(Vars.bEspMainWeapons) || nIndex == EClassIndex::CBaseWeaponWorldModel)
				break;

			// world weapons check
			if (pEntity->IsWeapon())
			{
				// cast entity to weapon
				CBaseCombatWeapon* pWeapon = reinterpret_cast<CBaseCombatWeapon*>(pEntity);

				if (pWeapon == nullptr)
					break;

				const short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();
				CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);

				if (pWeaponData == nullptr || !pWeaponData->IsGun())
					break;

				// get weapon owner
				const CBaseEntity* pOwner = I::ClientEntityList->Get<CBaseEntity>(pEntity->GetOwnerEntityHandle());

				// check only dropped weapons for active weapons we using another way
				if (pOwner != nullptr)
					break;

				// create weapon context
				Context_t ctx = { };

				// get weapon bounding box
				if (!GetBoundingBox(pEntity, &ctx.box))
					break;

				DroppedWeapons(pWeapon, nDefinitionIndex, ctx, flDistance, Color(255, 255, 255, 255), Color(80, 180, 200, 200), Color(40, 40, 40, 50), Color(0, 0, 0, 150));
			}

			break;
		}
		}
	}
	#pragma endregion
}

void CVisuals::Event(IGameEvent* pEvent, const FNV1A_t uNameHash)
{
	if (!I::Engine->IsInGame())
		return;

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (pLocal == nullptr || !pLocal->IsAlive())
		return;

	float flServerTime = TICKS_TO_TIME(pLocal->GetTickBase());

	/* get hitmarker info */
	if (C::Get<bool>(Vars.bScreenHitMarker))
	{
		CBaseEntity* pAttacker = I::ClientEntityList->Get<CBaseEntity>(I::Engine->GetPlayerForUserID(pEvent->GetInt(XorStr("attacker"))));

		if (pAttacker == pLocal)
		{
			CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(I::Engine->GetPlayerForUserID(pEvent->GetInt(XorStr("userid"))));

			if (pEntity != nullptr && pEntity != pLocal)
			{
				static CConVar* volume = I::ConVar->FindVar(XorStr("volume"));

				// play hit sound
				if (C::Get<bool>(Vars.bScreenHitMarkerSound))
					I::Engine->ExecuteClientCmd(fmt::format(XorStr("playvol survival\\money_collect_01.wav {:f}"), std::clamp(volume->GetFloat() * 5, 0.f, 1.f)).c_str());

				const auto vecPosition = pEntity->GetHitGroupPosition(pEvent->GetInt(XorStr("hitgroup")));

				if (!vecPosition.has_value())
					return;

				// add hit info
				m_deqHitMarkers.emplace_back(HitMarkerObject_t{ vecPosition.value(), pEvent->GetInt(XorStr("dmg_health")), flServerTime + C::Get<float>(Vars.flScreenHitMarkerTime) });
			}
		}
	}
}

bool CVisuals::Chams(CBaseEntity* pLocal, DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags)
{
	static auto oDrawModel = DTR::DrawModel.GetOriginal<decltype(&H::hkDrawModel)>();
	IClientRenderable* pRenderable = info.pClientEntity;

	if (pRenderable == nullptr)
		return false;

	// get entity from renderable
	CBaseEntity* pEntity = pRenderable->GetIClientUnknown()->GetBaseEntity();

	if (pEntity == nullptr)
		return false;

	std::string_view szModelName = info.pStudioHdr->szName;

	// check for players
	if (pEntity->IsPlayer() && pEntity->IsAlive() && pLocal->IsEnemy(pEntity))
	{
		// skip glow models
		if (nFlags & (STUDIO_RENDER | STUDIO_SKIP_FLEXES | STUDIO_DONOTMODIFYSTENCILSTATE | STUDIO_NOLIGHTING_OR_CUBEMAP | STUDIO_SKIP_DECALS))
			return false;

		static IMaterial* pMaterial = nullptr;
		static IMaterial* pMaterialWall = nullptr;
		static IMaterial* pMaterialBacktrack = nullptr;

		// set players material
		// set visible players material
		switch (C::Get<int>(Vars.iEspChamsEnemiesVisible))
		{
		case (int)EVisualsEnemiesChams::FLAT:
			pMaterial = m_arrMaterials.at(0).second;
			break;
		case (int)EVisualsEnemiesChams::REFLECTIVE:
			pMaterial = m_arrMaterials.at(2).first;
			break;
		default:
			pMaterial = m_arrMaterials.at(0).first;
			break;
		}

		// set invisible players material
		switch (C::Get<int>(Vars.iEspChamsEnemiesWall))
		{
		case (int)EVisualsEnemiesChams::FLAT:
			pMaterialWall = m_arrMaterials.at(0).second;
			break;
		case (int)EVisualsEnemiesChams::REFLECTIVE:
			pMaterialWall = m_arrMaterials.at(2).first;
			break;
		default:
			pMaterialWall = m_arrMaterials.at(0).first;
			break;
		}

		switch (C::Get<int>(Vars.iEspChamsEnemiesBacktrack))
		{
		case (int)EVisualsEnemiesChams::FLAT:
			pMaterialBacktrack = m_arrMaterials.at(0).second;
			break;
		case (int)EVisualsEnemiesChams::REFLECTIVE:
			pMaterialBacktrack = m_arrMaterials.at(2).first;
			break;
		default:
			pMaterialBacktrack = m_arrMaterials.at(0).first;
			break;
		}

		// check is valid material
		if (pMaterial == nullptr || pMaterial->IsErrorMaterial() || pMaterialWall == nullptr || pMaterialWall->IsErrorMaterial() || pMaterialBacktrack == nullptr || pMaterialBacktrack->IsErrorMaterial())
			return false;

		// get colors
		const Color colVisible = C::Get<Color>(Vars.colEspChamsEnemiesVisible);
		const Color colWall = C::Get<Color>(Vars.colEspChamsEnemiesWall);
		const Color colBacktrack = C::Get<Color>(Vars.colEspChamsEnemiesBacktrack);
		
		/* backtrack chams*/
		if (C::Get<bool>(Vars.bEspChamsBacktrack) && C::Get<bool>(Vars.bEspChamsEnemiesBacktrack))
		{
			std::deque deqRecords = CBacktracking::Get().GetPlayerRecord(pEntity->GetIndex());
			if (!deqRecords.empty())
			{
				if (C::Get<int>(Vars.iEspChamsBacktrackType) == 0)
				{
					for (auto& record : deqRecords)
					{
						if (!CBacktracking::Get().IsValid(record.flSimtime))
							continue;

						if (record.vecOrigin == pEntity->GetOrigin()) // if the record is going to overlay the player
						{
							//restore material
							I::StudioRender->ForcedMaterialOverride(nullptr);

							// draw original player
							oDrawModel(I::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

							// skip this record
							continue;
						}

						// set color
						pMaterialBacktrack->ColorModulate(colBacktrack.rBase(), colBacktrack.gBase(), colBacktrack.bBase());

						// set alpha
						pMaterialBacktrack->AlphaModulate(colBacktrack.aBase());

						// disable "$ignorez" flag
						pMaterialBacktrack->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

						// set wireframe
						pMaterialBacktrack->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, C::Get<int>(Vars.iEspChamsEnemiesBacktrack) == (int)EVisualsEnemiesChams::WIREFRAME ? true : false);

						// override customized material
						I::StudioRender->ForcedMaterialOverride(pMaterialBacktrack);

						// draw material
						oDrawModel(I::StudioRender, 0, pResults, info, record.arrMatrix.data(), flFlexWeights, flFlexDelayedWeights, record.vecOrigin, nFlags);
					}
				}
				else
				{
					auto record = deqRecords[deqRecords.size() - 1];
					
					// set color
					pMaterialBacktrack->ColorModulate(colBacktrack.rBase(), colBacktrack.gBase(), colBacktrack.bBase());

					// set alpha
					pMaterialBacktrack->AlphaModulate(colBacktrack.aBase());

					// disable "$ignorez" flag
					pMaterialBacktrack->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

					// set wireframe
					pMaterialBacktrack->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, C::Get<int>(Vars.iEspChamsEnemiesBacktrack) == (int)EVisualsEnemiesChams::WIREFRAME ? true : false);

					// override customized material
					I::StudioRender->ForcedMaterialOverride(pMaterialBacktrack);

					// draw material
					oDrawModel(I::StudioRender, 0, pResults, info, record.arrMatrix.data(), flFlexWeights, flFlexDelayedWeights, record.vecOrigin, nFlags);

					// restore material
					I::StudioRender->ForcedMaterialOverride(nullptr);

					// draw original player
					oDrawModel(I::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
				}
			}
		}

		if (C::Get<bool>(Vars.bEspChamsEnemies))
		{
			/* xqz chams */
			if (C::Get<bool>(Vars.bEspChamsEnemiesWall))
			{
				// set xqz color
				pMaterialWall->ColorModulate(colWall.rBase(), colWall.gBase(), colWall.bBase());

				// set xqz alpha
				pMaterialWall->AlphaModulate(colWall.aBase());

				// enable "$ignorez" flag and it enables ignore the z axis
				pMaterialWall->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);

				// set xqz wireframe
				pMaterialWall->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, C::Get<int>(Vars.iEspChamsEnemiesWall) == (int)EVisualsEnemiesChams::WIREFRAME ? true : false);

				// override ignorez material
				I::StudioRender->ForcedMaterialOverride(pMaterialWall);

				// draw model with xqz material
				oDrawModel(I::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

				// clear override
				I::StudioRender->ForcedMaterialOverride(nullptr);

				// fix for xqz chams
				pMaterialWall->ColorModulate(1.0f, 1.0f, 1.0f);
			}

			/* visible chams */
			if (C::Get<bool>(Vars.bEspChamsEnemiesVisible))
			{
				// set color
				pMaterial->ColorModulate(colVisible.rBase(), colVisible.gBase(), colVisible.bBase());

				// set alpha
				pMaterial->AlphaModulate(colVisible.aBase());

				// disable "$ignorez" flag
				pMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

				// set wireframe
				pMaterial->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, C::Get<int>(Vars.iEspChamsEnemiesVisible) == (int)EVisualsEnemiesChams::WIREFRAME ? true : false);

				// override customized material
				I::StudioRender->ForcedMaterialOverride(pMaterial);

				// draw material
				oDrawModel(I::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
			}
		}

		return true;
	}

	// check for viewmodel sleeves
	else if (szModelName.find(XorStr("sleeve")) != std::string_view::npos && C::Get<bool>(Vars.bEspChamsViewModel) && C::Get<int>(Vars.iEspChamsViewModel) == (int)EVisualsViewModelChams::NO_DRAW)
	{
		// get original sleeves material
		IMaterial* pSleeveMaterial = I::MaterialSystem->FindMaterial(szModelName.data(), XorStr(TEXTURE_GROUP_MODEL));

		// check is valid material
		if (pSleeveMaterial == nullptr)
			return false;

		pSleeveMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		I::StudioRender->ForcedMaterialOverride(pSleeveMaterial);

		// we need to clear override
		return true;
	}
	// check for viewmodel @note: u can separate this
	// check for viewmodel @note: u can separate this
	else if ((szModelName.find(XorStr("weapons\\v_")) != std::string_view::npos || szModelName.find(XorStr("arms")) != std::string_view::npos) && C::Get<bool>(Vars.bEspChamsViewModel))
	{
		// get original viewmodel material
		IMaterial* pViewModelMaterial = I::MaterialSystem->FindMaterial(szModelName.data(), XorStr(TEXTURE_GROUP_MODEL));

		// check is valid material
		if (pViewModelMaterial == nullptr)
			return false;

		if (C::Get<int>(Vars.iEspChamsViewModel) == (int)EVisualsViewModelChams::NO_DRAW)
		{
			pViewModelMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			I::StudioRender->ForcedMaterialOverride(pViewModelMaterial);

			// we need to clear override
			return true;
		}

		static IMaterial* pMaterial = nullptr;

		// set viewmodel material
		switch (C::Get<int>(Vars.iEspChamsViewModel))
		{
			case (int)EVisualsViewModelChams::FLAT:
				pMaterial = m_arrMaterials.at(1).second;
				break;
			case (int)EVisualsViewModelChams::GLOW:
				pMaterial = m_arrMaterials.at(2).second;
				break;
			case (int)EVisualsViewModelChams::SCROLL:
				pMaterial = m_arrMaterials.at(3).first;
				break;
			case (int)EVisualsViewModelChams::CHROME:
				pMaterial = m_arrMaterials.at(3).second;
				break;
			default:
				pMaterial = m_arrMaterials.at(1).first;
				break;
		}

		// check is valid material
		if (pMaterial == nullptr || pMaterial->IsErrorMaterial())
			return false;

		// get color
		const Color colAdditional = C::Get<Color>(Vars.colEspChamsViewModelAdditional);
		const Color colViewModel = C::Get<Color>(Vars.colEspChamsViewModel);

		// change material variables
		if (C::Get<int>(Vars.iEspChamsViewModel) == (int)EVisualsViewModelChams::GLOW)
		{
			static bool bEnvMapFresnelFound = false;
			IMaterialVar* pEnvMapFresnel = pMaterial->FindVar(XorStr("$envmapfresnel"), &bEnvMapFresnelFound);

			// add fresnel effect for glow
			if (bEnvMapFresnelFound)
				pEnvMapFresnel->SetInt(1);

			static bool bFoundEnvMapTint = false;
			IMaterialVar* pEnvMapTint = pMaterial->FindVar(XorStr("$envmaptint"), &bFoundEnvMapTint);

			// set additional color
			if (bFoundEnvMapTint)
				pEnvMapTint->SetVector(colAdditional.rBase(), colAdditional.gBase(), colAdditional.bBase());

			// disable color fusion for glow
			pMaterial->SetMaterialVarFlag(MATERIAL_VAR_ADDITIVE, true);

			// set "$ignorez" flag to 0 and disable it
			pMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

			// set wireframe
			pMaterial->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, C::Get<int>(Vars.iEspChamsViewModel) == (int)EVisualsViewModelChams::WIREFRAME ? true : false);

			// override customized material
			I::StudioRender->ForcedMaterialOverride(pMaterial);

			// then draw original with our material
			oDrawModel(I::StudioRender, 0, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

			// clear overrides
			I::StudioRender->ForcedMaterialOverride(nullptr);
		}

		// set color
		I::StudioRender->SetColorModulation(colViewModel.Base().data());

		// set alpha
		I::StudioRender->SetAlphaModulation(colViewModel.aBase());

		// disable color fusion
		pMaterial->SetMaterialVarFlag(MATERIAL_VAR_ADDITIVE, false);

		// set wireframe
		pMaterial->SetMaterialVarFlag(MATERIAL_VAR_WIREFRAME, C::Get<int>(Vars.iEspChamsViewModel) == (int)EVisualsViewModelChams::WIREFRAME ? true : false);

		// set "$ignorez" flag to 0 and disable it
		pMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, false);

		// override customized material
		I::StudioRender->ForcedMaterialOverride(pMaterial);

		// we need to clear override
		return true;
	}

	return false;
}

void CVisuals::Glow(CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive())
	{
		CBaseEntity* pObserverEntity = I::ClientEntityList->Get<CBaseEntity>(pLocal->GetObserverTargetHandle());
		if (pObserverEntity != nullptr)
			pLocal = pObserverEntity;
	}

	for (int i = 0; i < I::GlowManager->vecGlowObjectDefinitions.Count(); i++)
	{
		IGlowObjectManager::GlowObject_t& hGlowObject = I::GlowManager->vecGlowObjectDefinitions[i];

		// is current object not used
		if (hGlowObject.IsEmpty())
			continue;

		hGlowObject.Clear();

		// get current entity from object handle
		CBaseEntity* pEntity = hGlowObject.pEntity;

		if (pEntity == nullptr)
			continue;

		const Vector vecOrigin = pEntity->GetOrigin();
		Vector2D vecScreen = { };

		if (!D::WorldToScreen(vecOrigin, vecScreen))
			continue;

		CBaseClient* pClientClass = pEntity->GetClientClass();

		if (pClientClass == nullptr)
			continue;

		// get class id
		const EClassIndex nIndex = pClientClass->nClassID;

		bool bIsVisible = pEntity->IsPlayer() ? pLocal->IsVisible(pEntity, pEntity->GetEyePosition()) : pLocal->IsVisible(pEntity, vecOrigin);

		switch (nIndex)
		{
		case EClassIndex::CC4:
			if(C::Get<bool>(Vars.bEspGlowBomb))
				hGlowObject.Set(bIsVisible ? C::Get<Color>(Vars.colEspGlowBombVisible) : C::Get<Color>(Vars.colEspGlowBombWall));
			break;
		case EClassIndex::CPlantedC4:
			if (C::Get<bool>(Vars.bEspGlowBomb))
				hGlowObject.Set(bIsVisible ? C::Get<Color>(Vars.colEspGlowPlantedBombVisible) : C::Get<Color>(Vars.colEspGlowPlantedBombWall));
			break;
		case EClassIndex::CCSPlayer:
		{
			if (pEntity->IsDormant() || !pEntity->IsAlive() || !C::Get<bool>(Vars.bEspGlowEnemies) || !pLocal->IsEnemy(pEntity))
				break;

			if (bIsVisible ? C::Get<bool>(Vars.bEspGlowEnemiesVisible) : C::Get<bool>(Vars.bEspGlowEnemiesWall))
				hGlowObject.Set(bIsVisible ? C::Get<Color>(Vars.colEspGlowEnemiesVisible) : (C::Get<Color>(Vars.colEspGlowEnemiesWall)));
			break;
		}
		case EClassIndex::CBaseCSGrenadeProjectile:
		case EClassIndex::CDecoyProjectile:
		case EClassIndex::CMolotovProjectile:
		case EClassIndex::CSensorGrenadeProjectile:
		case EClassIndex::CSmokeGrenadeProjectile:
			if (C::Get<bool>(Vars.bEspGlowGrenades) && bIsVisible ? C::Get<bool>(Vars.bEspGlowGrenadesVisible) : C::Get<bool>(Vars.bEspGlowGrenadesWall))
				hGlowObject.Set(bIsVisible ? C::Get<Color>(Vars.colEspGlowGrenadesVisible) : C::Get<Color>(Vars.colEspGlowGrenadesWall));
			break;
		default:
			if (C::Get<bool>(Vars.bEspGlowWeapons) && pEntity->IsWeapon() && bIsVisible ? C::Get<bool>(Vars.bEspGlowWeaponsVisible) : C::Get<bool>(Vars.bEspGlowWeaponsWall))
				hGlowObject.Set(bIsVisible ? C::Get<Color>(Vars.colEspGlowWeaponsVisible) : C::Get<Color>(Vars.colEspGlowWeaponsWall));
			break;
		}
	}
}

void CVisuals::Removals()
{
	// disable 3d skybox
	static CConVar* r_3dsky = I::ConVar->FindVar("r_3dsky");
	r_3dsky->fnChangeCallbacks.Size() = NULL;
	r_3dsky->SetValue(!C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_3DSKY));

	// disable shadows
	static CConVar* cl_csm_enabled = I::ConVar->FindVar("cl_csm_enabled");
	cl_csm_enabled->fnChangeCallbacks.Size() = NULL;
	cl_csm_enabled->SetValue(!C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SHADOWS));

	// disable post-proccesing
	static CConVar* mat_postprocess_enable = I::ConVar->FindVar(XorStr("mat_postprocess_enable"));
	mat_postprocess_enable->fnChangeCallbacks.Size() = NULL;
	mat_postprocess_enable->SetValue(!C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_POSTPROCESSING));

	/* disable decals @note doesn't work for some fucking reason
	static CConVar* r_drawdecals = I::ConVar->FindVar(XorStr("r_drawdecals"));
	r_drawdecals->fnChangeCallbacks.Size() = NULL;
	r_drawdecals->SetValue(!C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_DECALS));
	*/
}

bool CVisuals::GetBoundingBox(CBaseEntity* pEntity, Box_t* pBox) const
{
	const ICollideable* pCollideable = pEntity->GetCollideable();

	if (pCollideable == nullptr)
		return false;

	// get mins/maxs
	const Vector vecMin = pCollideable->OBBMins();
	const Vector vecMax = pCollideable->OBBMaxs();

	/*
	 * build AABB points
	 *
	 * points navigation:
	 * [N] [back/front][left/right][bottom/top]
	 *	0 - blb
	 *	1 - brb
	 *	2 - frb
	 *	3 - flb
	 *	4 - frt
	 *	5 - brt
	 *	6 - blt
	 *	7 - flt
	 */
	std::array<Vector, 8U> arrPoints =
	{
		Vector(vecMin.x, vecMin.y, vecMin.z),
		Vector(vecMin.x, vecMax.y, vecMin.z),
		Vector(vecMax.x, vecMax.y, vecMin.z),
		Vector(vecMax.x, vecMin.y, vecMin.z),
		Vector(vecMax.x, vecMax.y, vecMax.z),
		Vector(vecMin.x, vecMax.y, vecMax.z),
		Vector(vecMin.x, vecMin.y, vecMax.z),
		Vector(vecMax.x, vecMin.y, vecMax.z)
	};

	if (arrPoints.data() == nullptr)
		return false;

	// get transformation matrix
	const matrix3x4_t& matTransformed = pEntity->GetCoordinateFrame();

	float flLeft = std::numeric_limits<float>::max();
	float flTop = std::numeric_limits<float>::max();
	float flRight = -std::numeric_limits<float>::max();
	float flBottom = -std::numeric_limits<float>::max();

	// get screen points position
	std::array<Vector2D, 8U> arrScreen = { };
	for (std::size_t i = 0U; i < 8U; i++)
	{
		if (!D::WorldToScreen(M::VectorTransform(arrPoints.at(i), matTransformed), arrScreen.at(i)))
			return false;

		/*
		 * transformed points navigation:
		 * [N] [back/front][left/right][bottom/top]
		 *	0 - frb
		 *	1 - flb
		 *	2 - blb
		 *	3 - brb
		 *	4 - frt
		 *	5 - blt
		 *	6 - brt
		 *	7 - flt
		 */
		flLeft = std::min(flLeft, arrScreen.at(i).x);
		flTop = std::min(flTop, arrScreen.at(i).y);
		flRight = std::max(flRight, arrScreen.at(i).x);
		flBottom = std::max(flBottom, arrScreen.at(i).y);
	}

	// set calculated box
	pBox->left = flLeft;
	pBox->top = flTop;
	pBox->right = flRight;
	pBox->bottom = flBottom;
	pBox->width = flRight - flLeft;
	pBox->height = flBottom - flTop;
	return true;
}

IMaterial* CVisuals::CreateMaterial(std::string_view szName, std::string_view szShader, std::string_view szBaseTexture, std::string_view szEnvMap, bool bIgnorez, bool bWireframe, std::string_view szProxies) const
{
	/*
	 * @note: materials info:
	 1	shaders: https://developer.valvesoftware.com/wiki/Category:Shaders
	 -		parameters: https://developer.valvesoftware.com/wiki/Category:List_of_Shader_Parameters
	 2	proxies: https://developer.valvesoftware.com/wiki/Material_proxies
	 -		list: https://developer.valvesoftware.com/wiki/List_Of_Material_Proxies
	 *
	 * use "mat_texture_list 1" command to see full materials list
	 */

	const std::string szMaterial = fmt::format(XorStr(R"#("{shader}"
	{{
		"$basetexture"		"{texture}"
		"$envmap"			"{envmap}"
		"$envmapfresnel"	"0"
		"$model"			"1"
		"$translucent"		"0"
		"$ignorez"			"{ignorez}"
		"$selfillum"		"1"
		"$halflambert"		"1"
		"$wireframe"		"{wireframe}"
		"$nofog"			"1"
		"proxies"
		{{
			{proxies}
		}}
	}})#"), fmt::arg(XorStr("shader"), szShader), fmt::arg(XorStr("texture"), szBaseTexture), fmt::arg(XorStr("envmap"), szEnvMap), fmt::arg(XorStr("ignorez"), bIgnorez ? 1 : 0), fmt::arg(XorStr("wireframe"), bWireframe ? 1 : 0), fmt::arg(XorStr("proxies"), szProxies));

	// load to memory
	CKeyValues* pKeyValues = static_cast<CKeyValues*>(CKeyValues::operator new(sizeof(CKeyValues)));
	pKeyValues->Init(szShader.data());
	pKeyValues->LoadFromBuffer(szName.data(), szMaterial.c_str());

	// create from buffer
	return I::MaterialSystem->CreateMaterial(szName.data(), pKeyValues);
}

void CVisuals::HitMarker(const ImVec2& vecScreenSize, float flServerTime, Color colLines, Color colDamage)
{
	if (m_deqHitMarkers.empty())
		return;

	// get last time delta for lines
	const float flLastDelta = m_deqHitMarkers.back().flTime - flServerTime;

	if (flLastDelta <= 0.f)
		return;

	const float flMaxLinesAlpha = colLines.aBase();
	constexpr int arrSides[4][2] = { { -1, -1 }, { 1, 1 }, { -1, 1 }, { 1, -1 } };
	for (const auto& iSide : arrSides)
	{
		// set fade out alpha
		colLines.arrColor.at(3) = static_cast<std::uint8_t>(std::min(flMaxLinesAlpha, flLastDelta / C::Get<float>(Vars.flScreenHitMarkerTime)) * 255.f);
		// draw mark cross
		D::AddLine(ImVec2(vecScreenSize.x * 0.5f + C::Get<int>(Vars.iScreenHitMarkerGap) * iSide[0], vecScreenSize.y * 0.5f + C::Get<int>(Vars.iScreenHitMarkerGap) * iSide[1]), ImVec2(vecScreenSize.x * 0.5f + C::Get<int>(Vars.iScreenHitMarkerLenght) * iSide[0], vecScreenSize.y * 0.5f + C::Get<int>(Vars.iScreenHitMarkerLenght) * iSide[1]), colLines);
	}

	if (!C::Get<bool>(Vars.bScreenHitMarkerDamage))
		return;

	const float flMaxDamageAlpha = colDamage.aBase();
	for (std::size_t i = 0U; i < m_deqHitMarkers.size(); i++)
	{
		const float flDelta = m_deqHitMarkers.at(i).flTime - flServerTime;

		if (flDelta <= 0.f)
		{
			m_deqHitMarkers.erase(m_deqHitMarkers.cbegin() + i);
			continue;
		}

		Vector2D vecScreen = { };
		if (D::WorldToScreen(m_deqHitMarkers.at(i).vecPosition, vecScreen))
		{
			// max distance for floating damage
			constexpr float flDistance = 40.f;
			const float flRatio = 1.0f - (flDelta / C::Get<float>(Vars.flScreenHitMarkerTime));

			// set fade out alpha
			const int iAlpha = static_cast<int>(std::min(flMaxDamageAlpha, flDelta / C::Get<float>(Vars.flScreenHitMarkerTime)) * 255.f);
			colDamage.arrColor.at(3) = static_cast<std::uint8_t>(iAlpha);

			// draw dealt damage
			D::AddText(F::SmallestPixel, 15.f, ImVec2(vecScreen.x, vecScreen.y - flRatio * flDistance), std::to_string(m_deqHitMarkers.at(i).iDamage), colDamage, IMGUI_TEXT_OUTLINE, Color(0, 0, 0, iAlpha));
		}
	}
}

void CVisuals::SpectatorList(const ImVec2& vecScreenSize, CBaseEntity* pLocal)
{
	if (!pLocal->IsAlive())
	{
		CBaseEntity* pObserverTarget = I::ClientEntityList->Get<CBaseEntity>(pLocal->GetObserverTargetHandle());

		if (pObserverTarget == nullptr)
			return;

		pLocal = pObserverTarget;
	}

	float flOffset = 0.f;

	for (int i = 1; i <= I::Globals->nMaxClients; i++)
	{
		if (i == I::Engine->GetLocalPlayer())
			continue;

		CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);

		if (pEntity == nullptr || !pEntity->IsPlayer() || pEntity->IsAlive())
			continue;

		CBaseEntity* pObserverTarget = I::ClientEntityList->Get<CBaseEntity>(pEntity->GetObserverTargetHandle());

		if (pObserverTarget == nullptr || pObserverTarget != pLocal)
			continue;

		PlayerInfo_t pInfo;
		if (!I::Engine->GetPlayerInfo(i, &pInfo))
			continue;

		const char* szName = pInfo.szName;

		ImVec2 vecNameSize = F::SegoeUI->CalcTextSizeA(12.f, FLT_MAX, 0, szName);

		D::AddText(F::SegoeUI, 12.f, ImVec2(vecScreenSize.x - vecNameSize.x - 5.f, 0 + flOffset), szName, Color(255, 255, 255), IMGUI_TEXT_DROPSHADOW);

		flOffset += vecNameSize.y;
	}
}

void CVisuals::VelocityIndicator(const ImVec2& vecScreenSize, CBaseEntity* pLocal)
{
	int iVelocity = std::floorf(pLocal->GetVelocity().Length2D() + 0.5f);
	int iMaxVelocity = 320;

	std::string szText = std::to_string(iVelocity);
	ImVec2 vecTextSize = F::SegoeUI40->CalcTextSizeA(40.f, FLT_MAX, 0, szText.data());

	float flHue = ((static_cast<float>(iVelocity) / static_cast<float>(iMaxVelocity)) * 150.f) / 360.f;

	D::AddText(F::SegoeUI40, 40.f, ImVec2(vecScreenSize.x / 2.f - vecTextSize.x / 2.f, vecScreenSize.y - vecScreenSize.y / 6.f), szText.data(), Color::FromHSB(flHue, 0.5f, 1.f), IMGUI_TEXT_DROPSHADOW);
}

void CVisuals::NightMode(CEnvTonemapController* pController) const
{ 
	static bool bSwitch = false;

	// prevent change every tick
	if (bSwitch)
	{
		pController->IsUseCustomAutoExposureMin() = true;
		pController->IsUseCustomAutoExposureMax() = true;
		pController->GetCustomAutoExposureMin() = (C::Get<bool>(Vars.bWorldNightMode)) ? 0.2f : 1.0f;
		pController->GetCustomAutoExposureMax() = (C::Get<bool>(Vars.bWorldNightMode)) ? 0.2f : 1.0f;
	}

	if (bSwitch != (C::Get<bool>(Vars.bWorldNightMode)))
		bSwitch = (C::Get<bool>(Vars.bWorldNightMode));
}

void CVisuals::NightMode()
{
	static float flColorR = 0.f;
	static float flColorG = 0.f;
	static float flColorB = 0.f;
	static float flColorA = 0.f;

	static CConVar* sv_skyname = I::ConVar->FindVar("sv_skyname");
	static CConVar* r_drawspecificstaticprop = I::ConVar->FindVar("r_drawspecificstaticprop");
	static bool bPerformed = false;


	if (C::Get<bool>(Vars.bWorldNightMode))
	{
		sv_skyname->SetValue("sky_csgo_night02");
		r_drawspecificstaticprop->SetValue(0);

		for (MaterialHandle_t i = I::MaterialSystem->FirstMaterial(); i != I::MaterialSystem->InvalidMaterial(); i = I::MaterialSystem->NextMaterial(i))
		{
			IMaterial* pMaterial = I::MaterialSystem->GetMaterial(i);

			if (pMaterial == nullptr)
				continue;

			const char* szMatGroup = pMaterial->GetTextureGroupName();
			const char* szMatName = pMaterial->GetName();

			if (strstr(szMatGroup, "World textures"))
			{
				pMaterial->ColorModulate(0.25f, 0.25f, 0.25f);
			}
			else if (strstr(szMatGroup, "StaticProp"))
			{
				pMaterial->ColorModulate(0.30, 0.30, 0.30);
			}
			else if (strstr(szMatName, "models/props/de_dust/palace_bigdome"))
			{
				pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			}
			else if (strstr(szMatName, "models/props/de_dust/palace_pillars"))
			{
				pMaterial->ColorModulate(0.30, 0.30, 0.30);
			}
			else if (strstr(szMatGroup, "Particle textures"))
			{
				pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			}

			bPerformed = true;
		}
	}
	else
	{
		if (bPerformed)
		{
			r_drawspecificstaticprop->SetValue(1);

			for (MaterialHandle_t i = I::MaterialSystem->FirstMaterial(); i != I::MaterialSystem->InvalidMaterial(); i = I::MaterialSystem->NextMaterial(i))
			{
				IMaterial* pMaterial = I::MaterialSystem->GetMaterial(i);

				if (pMaterial == nullptr)
					continue;

				const char* szMatGroup = pMaterial->GetTextureGroupName();
				const char* szMatName = pMaterial->GetName();

				if (strstr(szMatGroup, "World textures"))
				{
					pMaterial->ColorModulate(1.f, 1.f, 1.f);
				}
				else if (strstr(szMatGroup, "StaticProp"))
				{
					pMaterial->ColorModulate(1.f, 1.f, 1.f);
				}
				else if (strstr(szMatName, "models/props/de_dust/palace_bigdome"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				}
				else if (strstr(szMatName, "models/props/de_dust/palace_pillars"))
				{
					pMaterial->ColorModulate(1.f, 1.f, 1.f);
				}
				else if (strstr(szMatGroup, "Particle textures"))
				{
					pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, false);
				}
			}

			bPerformed = false;
		}
	}
}

void CVisuals::Fog(CFogController* pController) const
{
	pController->Enable() = C::Get<bool>(Vars.bWorldFog);
	pController->ColorPrimary() = C::Get<Color>(Vars.colWorldFogPrimary).GetU32();
	pController->Blend() = false;
	pController->Start() = C::Get<int>(Vars.iWorldFogStart);
	pController->End() = C::Get<int>(Vars.iWorldFogEnd);
	pController->MaxDensity() = 1.f;
}

void CVisuals::Fishes(CFish* pFish, CBaseEntity* pLocal) const
{
	pFish->X() = pLocal->GetAbsOrigin().x;
	pFish->Y() = pLocal->GetAbsOrigin().y;
	pFish->Z() = pLocal->GetAbsOrigin().z + 120.f;
}

void CVisuals::Bomb(const Vector2D& vecScreen, Context_t& ctx, const Color& colFrame)
{
	const char* szIcon = U::GetWeaponIcon(WEAPON_C4);
	const ImVec2 vecIconSize = F::Icons->CalcTextSizeA(12.f, FLT_MAX, 0.f, szIcon);

	const char* szName = XorStr("C4");
	const ImVec2 vecNameSize = F::SegoeUI->CalcTextSizeA(12.f, FLT_MAX, 0.f, szName);

	static ImVec2 vecSize(vecIconSize.x + vecNameSize.x, vecNameSize.y);

	// set custom box
	ctx.box = { vecScreen.x - vecSize.x * 0.5f, vecScreen.y - vecSize.y * 0.5f, vecScreen.x + vecSize.x * 0.5f, vecScreen.y + vecSize.y * 0.5f, vecSize.x, vecSize.y };

	// frame
	D::AddRect(ImVec2(ctx.box.left - 2.f, ctx.box.top), ImVec2(ctx.box.right + 2.f, ctx.box.bottom + 1.f), colFrame, IMGUI_RECT_FILLED, colFrame);
	// icon
	D::AddText(F::Icons, 12.f, ImVec2(ctx.box.left, ctx.box.top), szIcon, Color(255, 255, 255));
	// text
	D::AddText(F::SegoeUI, 12.f, ImVec2(ctx.box.left + vecIconSize.x, ctx.box.top), szName, Color(255, 255, 255));
}

void CVisuals::PlantedBomb(CPlantedC4* pBomb, float flServerTime, const Vector2D& vecScreen, Context_t& ctx, const Color& colFrame, const Color& colDefuse, const Color& colFailDefuse)
{
	const char* szIcon = U::GetWeaponIcon(WEAPON_C4);
	static ImVec2 vecIconSize = F::Icons->CalcTextSizeA(12.f, FLT_MAX, 0.f, szIcon);

	const char* szName = XorStr("PLANTED C4");
	static ImVec2 vecNameSize = F::SegoeUI->CalcTextSizeA(12.f, FLT_MAX, 0.f, szName);

	static ImVec2 vecSize(vecIconSize.x + vecNameSize.x, vecNameSize.y);

	// set custom box
	ctx.box = { vecScreen.x - vecSize.x * 0.5f - 2.f, vecScreen.y - vecSize.y * 0.5f, vecScreen.x + vecSize.x * 0.5f + 2.f, vecScreen.y + vecSize.y * 0.5f + 1.f, vecSize.x + 4.f, vecSize.y + 1.f};

	// get defuser entity
	const CBaseEntity* pDefuser = I::ClientEntityList->Get<CBaseEntity>(pBomb->GetDefuserHandle());

	/* info */
	// frame
	D::AddRect(ImVec2(ctx.box.left, ctx.box.top), ImVec2(ctx.box.right, ctx.box.bottom), colFrame, IMGUI_RECT_FILLED, Color(0, 0, 0, 255));
	// icon
	D::AddText(F::Icons, 12.f, ImVec2(ctx.box.left + 2.f, ctx.box.top), szIcon, Color(255, 255, 255));
	// text
	D::AddText(F::SegoeUI, 12.f, ImVec2(ctx.box.left + 2.f + vecIconSize.x, ctx.box.top), szName, Color(255, 255, 255));

	/* bar */
	// get timer values
	const float flCurrentTime = pBomb->GetTimer(flServerTime);
	const float flMaxTime = pBomb->GetTimerLength();

	// calculate bomb timer-based width factor
	const float flFactor = flCurrentTime / flMaxTime;
	// calculate bomb timer-based color
	const float flHue = (flFactor * 120.f) / 360.f;

	/* timer bar */
	if (flFactor > 0.f)
	{
		// background
		D::AddRect(ImVec2(ctx.box.left, ctx.box.bottom), ImVec2(ctx.box.right, ctx.box.bottom + 4.f), colFrame, IMGUI_RECT_FILLED);
		// bar
		D::AddRect(ImVec2(ctx.box.left + 1.f, ctx.box.bottom + 1.f), ImVec2((ctx.box.left + 1.f) + (ctx.box.width - 1.f) * flFactor, ctx.box.bottom + 3.f), Color::FromHSB(flHue, 1.f, 1.f), IMGUI_RECT_FILLED);
		ctx.arrPadding.at(DIR_BOTTOM) += 4.f;
	}

	// check for defuser to update defusing time
	if (pDefuser != nullptr)
	{
		// get defuse timer value
		const float flDefuseTime = pBomb->GetDefuseTimer(flServerTime);
		const float flMaxDefuseTime = pBomb->GetDefuseLength();

		// calculate bomb defuse timer-based width factor
		const float flDefuseFactor = flDefuseTime / flMaxDefuseTime;

		/* defusing bar */

		if (flDefuseFactor > 0.f)
		{
			// background
			D::AddRect(ImVec2(ctx.box.left, ctx.box.bottom + ctx.arrPadding.at(DIR_BOTTOM)), ImVec2(ctx.box.right, ctx.box.bottom + 4.f + ctx.arrPadding.at(DIR_BOTTOM)), colFrame, IMGUI_RECT_FILLED);
			// bar
			D::AddRect(ImVec2(ctx.box.left + 1.f, ctx.box.bottom + 1.f + ctx.arrPadding.at(DIR_BOTTOM)), ImVec2((ctx.box.left + 1.f) + (ctx.box.width - 1.f) * flDefuseFactor, ctx.box.bottom + 3.f + ctx.arrPadding.at(DIR_BOTTOM)), (flDefuseTime < flCurrentTime) ? colDefuse : colFailDefuse, IMGUI_RECT_FILLED);
		}
	}
}

void CVisuals::Grenade(CBaseEntity* pGrenade, EClassIndex nIndex, float flServerTime, float flDistance, const Vector2D& vecScreen, Context_t& ctx, const Color& colFrame, const Color& colBackground, const Color& colOutline)
{
	// setup temporary values
	const char* szName = XorStr("NONE");
	float flFactor = 0.f;
	Color colGrenade(255, 255, 255);

	// get grenade model name
	if (std::string_view szModelName = I::ModelInfo->GetModelName(pGrenade->GetModel()); !szModelName.empty())
	{
		// get grenade info
		switch (nIndex)
		{
		case EClassIndex::CBaseCSGrenadeProjectile:
		{
			// separate greandes by model name
			if (szModelName.find(XorStr("fraggrenade")) != std::string_view::npos)
				szName = XorStr("HE");
			else if (szModelName.find(XorStr("flashbang")) != std::string_view::npos)
				szName = XorStr("FLASH");
			break;
		}
		case EClassIndex::CDecoyProjectile:
		{
			szName = XorStr("DECOY");
			break;
		}
		case EClassIndex::CSmokeGrenadeProjectile:
		{
			// cast to smoke grenade
			CSmokeGrenade* pSmoke = reinterpret_cast<CSmokeGrenade*>(pGrenade);
			flFactor = ((TICKS_TO_TIME(pSmoke->GetEffectTickBegin()) + pSmoke->GetMaxTime()) - flServerTime) / pSmoke->GetMaxTime();
			//colGrenade = Color(230, 130, 0);
			colGrenade = Color(255, 255, 255);
			szName = XorStr("SMOKE");
			break;
		}
		case EClassIndex::CMolotovProjectile:
		case EClassIndex::CInferno:
		{
			// cast to inferno grenade
			CInferno* pInferno = reinterpret_cast<CInferno*>(pGrenade);
			flFactor = ((TICKS_TO_TIME(pInferno->GetEffectTickBegin()) + pInferno->GetMaxTime()) - flServerTime) / pInferno->GetMaxTime();
			colGrenade = Color(255, 100, 100);

			// separate grenades by model name
			if (szModelName.find(XorStr("molotov")) != std::string_view::npos)
				szName = XorStr("MOLOTOV");
			else if (szModelName.find(XorStr("incendiary")) != std::string_view::npos)
				szName = XorStr("INCENDIARY");
			else
				szName = XorStr("FIRE");
			break;
		}
		default:
			break;
		}
	}

	const ImVec2 vecNameSize = F::SegoeUI->CalcTextSizeA(12.f, FLT_MAX, 0.0f, szName);
	const ImVec2 vecSize(vecNameSize.x, vecNameSize.y);

	// set custom box
	ctx.box = { vecScreen.x - vecSize.x * 0.5f - 2.f, vecScreen.y - vecSize.y * 0.5f, vecScreen.x + vecSize.x * 0.5f + 2.f, vecScreen.y + vecSize.y * 0.5f + 1.f, vecSize.x + 4.f, vecSize.y + 1.f};

	/* info */
	// frame
	if(C::Get<bool>(Vars.bEspMainGrenadesFrame))
		D::AddRect(ImVec2(ctx.box.left, ctx.box.top), ImVec2(ctx.box.right, ctx.box.bottom), colFrame, IMGUI_RECT_FILLED, Color(0, 0, 0, 255));
	// text
	if (C::Get<bool>(Vars.bEspMainGrenadesName))
		D::AddText(F::SegoeUI, 12.f, ImVec2(ctx.box.left + 2.f, ctx.box.top), szName, C::Get<Color>(Vars.colEspMainGrenadesName), IMGUI_TEXT_DROPSHADOW, Color(0, 0, 0, 255));

	const int iDistance = static_cast<int>(M_INCH2METRE(flDistance));
	std::string szDistance = std::to_string(iDistance).append(XorStr("M"));
	const ImVec2 vecDistanceSize = F::Roboto->CalcTextSizeA(10.f, FLT_MAX, 0.0f, szDistance.data());

	if (C::Get<bool>(Vars.bEspMainGrenadesDistance))
		D::AddText(F::Roboto, 10.f, ImVec2(ctx.box.left + ctx.box.width * 0.5f - vecDistanceSize.x * 0.5f, ctx.box.top - 2 - vecDistanceSize.y), szDistance, C::Get<Color>(Vars.colEspMainWeaponDistance), IMGUI_TEXT_DROPSHADOW, Color(0, 0, 0, 255));

	if (flFactor > 0.f && C::Get<bool>(Vars.bEspMainGrenadesDuration))
	{
		/* bar */
		// background
		D::AddRect(ImVec2(ctx.box.left, ctx.box.bottom), ImVec2(ctx.box.right, ctx.box.bottom + 4.f), colFrame, IMGUI_RECT_FILLED);
		// bar
		D::AddRect(ImVec2(ctx.box.left + 1.f, ctx.box.bottom + 1.f), ImVec2((ctx.box.left + 1.f) + (ctx.box.width - 1.f) * flFactor, ctx.box.bottom + 3.f), colGrenade, IMGUI_RECT_FILLED);
	}
}

void CVisuals::DroppedWeapons(CBaseCombatWeapon* pWeapon, short nItemDefinitionIndex, Context_t& ctx, const float flDistance, const Color& colPrimary, const Color& colAmmo, const Color& colBackground, const Color& colOutline)
{
	if (C::Get<bool>(Vars.bEspMainWeaponBox))
		Box(ctx.box, C::Get<Color>(Vars.colEspMainWeaponBox), Color(0, 0, 0, 150));

	// @note: for text weapon names
	// const char* szHudName = pWeaponData->szHudName;
	// wchar_t* wszLocalizeName = I::Localize->Find(szHudName);

	const short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();
	CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);

	// ammo bar
	if (C::Get<bool>(Vars.bEspMainWeaponAmmo))
		AmmoBar(nullptr, pWeapon, ctx, C::Get<Color>(Vars.colEspMainWeaponAmmo), Color(0, 0, 0, 150), Color(0, 0, 0, 150));

	// weapon text
	if (C::Get<bool>(Vars.bEspMainWeaponText))
	{
		wchar_t* wszLocalizeName = I::Localize->Find(pWeaponData->szHudName);
		_bstr_t bstrWeaponName(wszLocalizeName);
		const char* szWeaponName = static_cast<const char*>(bstrWeaponName);

		const ImVec2 vecNameSize = F::Roboto->CalcTextSizeA(10.f, FLT_MAX, 0.f, szWeaponName);

		D::AddText(F::Roboto, 10.f, ImVec2(ctx.box.left + ctx.box.width * 0.5f - vecNameSize.x * 0.5f, ctx.box.bottom + 2 + ctx.arrPadding.at(DIR_BOTTOM)), szWeaponName, C::Get<Color>(Vars.colEspMainWeaponText), IMGUI_TEXT_DROPSHADOW, colOutline);
		ctx.arrPadding.at(DIR_BOTTOM) += vecNameSize.y;
	}

	// weapon icon
	if (C::Get<bool>(Vars.bEspMainWeaponIcon))
	{
		// get dropped weapon icon
		const char* szIcon = U::GetWeaponIcon(nItemDefinitionIndex);
		const ImVec2 vecIconSize = F::Icons->CalcTextSizeA(12.f, FLT_MAX, 0.f, szIcon);

		D::AddText(F::Icons, 12.f, ImVec2(ctx.box.left + ctx.box.width * 0.5f - vecIconSize.x * 0.5f, ctx.box.bottom + 2 + ctx.arrPadding.at(DIR_BOTTOM)), szIcon, C::Get<Color>(Vars.colEspMainWeaponIcon), IMGUI_TEXT_DROPSHADOW, colOutline);
		ctx.arrPadding.at(DIR_BOTTOM) += vecIconSize.y;
	}

	// distance
	if (C::Get<bool>(Vars.bEspMainWeaponDistance))
	{
		const int iDistance = static_cast<int>(M_INCH2METRE(flDistance));
		std::string szDistance = std::to_string(iDistance).append(XorStr("M"));
		const ImVec2 vecDistanceSize = F::Roboto->CalcTextSizeA(10.f, FLT_MAX, 0.0f, szDistance.data());

		D::AddText(F::Roboto, 10.f, ImVec2(ctx.box.left + ctx.box.width * 0.5f - vecDistanceSize.x * 0.5f, ctx.box.top - 2 - vecDistanceSize.y - ctx.arrPadding.at(DIR_TOP)), szDistance, C::Get<Color>(Vars.colEspMainWeaponDistance), IMGUI_TEXT_DROPSHADOW, colOutline);
		ctx.arrPadding.at(DIR_TOP) += vecDistanceSize.y;
	}
}

void CVisuals::Player(CBaseEntity* pLocal, CBaseEntity* pEntity, Context_t& ctx, const float flDistance, const Color& colInfo, const Color& colFrame, const Color& colOutline)
{
	PlayerInfo_t pInfo;
	if (!I::Engine->GetPlayerInfo(pEntity->GetIndex(), &pInfo))
		return;

	if (C::Get<bool>(Vars.bEspMainPlayerBox))
		Box(ctx.box, C::Get<Color>(Vars.colEspMainPlayerBox), Color(0, 0, 0, 150));

	if (C::Get<bool>(Vars.bEspMainPlayerSkeleton))
		Skeleton(pEntity);

	if (C::Get<bool>(Vars.bEspMainPlayerBacktrack))
	{
		std::deque deqRecords = CBacktracking::Get().GetPlayerRecord(pEntity->GetIndex());
		if (!deqRecords.empty())
		{
			for (auto& record : deqRecords)
			{
				if (!CBacktracking::Get().IsValid(record.flSimtime)) // only draw valid records
					continue;

				Vector2D vecScreen = { };
				if (!D::WorldToScreen(record.vecHeadPos, vecScreen))
					return;

				D::AddRect(ImVec2(vecScreen.x - 0.5f, vecScreen.y - 0.5f), ImVec2(vecScreen.x + 0.5f, vecScreen.y + 0.5f), C::Get<Color>(Vars.colEspMainPlayerBacktrack), IMGUI_RECT_FILLED);
			}
		}
	}

	const float flMainFontSize = 12.f;
	const float flOtherFontSize = 10.f;
	const float flFlagsFontSize = 8.f;

	#pragma region visuals_player_top
	if (C::Get<bool>(Vars.bEspMainPlayerFlash) && pEntity->GetFlashDuration() > 0.2f)
		FlashBar(pEntity, ctx, C::Get<Color>(Vars.colEspMainPlayerFlash), Color(0, 0, 0, 150), Color(0, 0, 0, 150));

	if (C::Get<bool>(Vars.bEspMainPlayerName))
	{
		// get player name
		std::string szName = pInfo.szName;

		// truncate name
		if (szName.length() > 15U)
			szName = szName.substr(0U, 15U).append(XorStr("..."));

		const ImVec2 vecNameSize = F::SegoeUI->CalcTextSizeA(flMainFontSize, FLT_MAX, 0.0f, szName.c_str());

		D::AddText(F::SegoeUI, flMainFontSize, ImVec2(ctx.box.left + ctx.box.width * 0.5f - vecNameSize.x * 0.5f, ctx.box.top - 2 - vecNameSize.y - ctx.arrPadding.at(DIR_TOP)), szName, C::Get<Color>(Vars.colEspMainPlayerName), IMGUI_TEXT_DROPSHADOW, colOutline);
		ctx.arrPadding.at(DIR_TOP) += vecNameSize.y;
	}

	if (C::Get<bool>(Vars.bEspMainPlayerNadeKill) && pEntity->GetHealth() < 5)
	{
		std::string szText = "nadekill";
		const ImVec2 vecTextSize = F::Roboto->CalcTextSizeA(flOtherFontSize, FLT_MAX, 0.0f, szText.c_str());

		D::AddText(F::Roboto, flOtherFontSize, ImVec2(ctx.box.left + ctx.box.width * 0.5f - vecTextSize.x * 0.5f, ctx.box.top - 2 - vecTextSize.y - ctx.arrPadding.at(DIR_TOP)), szText, C::Get<Color>(Vars.colEspMainPlayerNadeKill), IMGUI_TEXT_DROPSHADOW, colOutline);
		ctx.arrPadding.at(DIR_TOP) += vecTextSize.y;
	}
	#pragma endregion

	#pragma region visuals_player_bottom
	// get active weapon
	if (CBaseCombatWeapon* pActiveWeapon = pEntity->GetWeapon(); pActiveWeapon != nullptr)
	{
		// ammo bar
		if (C::Get<bool>(Vars.bEspMainPlayerAmmo))
			AmmoBar(pEntity, pActiveWeapon, ctx, C::Get<Color>(Vars.colEspMainPlayerAmmo), Color(0, 0, 0, 150), Color(0, 0, 0, 150));

		// get all other weapons
		if (C::Get<bool>(Vars.bEspMainPlayerWeaponIcon) || C::Get<bool>(Vars.bEspMainPlayerWeaponText))
		{
			if (const auto pWeapon = pEntity->GetWeapon(); pWeapon != nullptr)
			{
				const short nDefinitionIndex = pWeapon->GetItemDefinitionIndex();
				CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(nDefinitionIndex);

				if (C::Get<bool>(Vars.bEspMainPlayerWeaponText))
				{
					wchar_t* wszLocalizeName = I::Localize->Find(pWeaponData->szHudName);
					_bstr_t bstrWeaponName(wszLocalizeName);
					std::string szWeaponName = static_cast<const char*>(bstrWeaponName);

					const ImVec2 vecTextSize = F::Roboto->CalcTextSizeA(10.f, FLT_MAX, 0.0f, szWeaponName.data());
					D::AddText(F::Roboto, 10.f, ImVec2(ctx.box.left + ctx.box.width * 0.5f - vecTextSize.x * 0.5f, ctx.box.bottom + 2 + ctx.arrPadding.at(DIR_BOTTOM)), szWeaponName, C::Get<Color>(Vars.colEspMainPlayerWeaponText), IMGUI_TEXT_DROPSHADOW, colOutline);
					ctx.arrPadding.at(DIR_BOTTOM) += vecTextSize.y;
				}

				// do not draw some useless dangerzone weapons (lmao i just dont add icons)
				if (C::Get<bool>(Vars.bEspMainPlayerWeaponIcon) && nDefinitionIndex != WEAPON_SHIELD && nDefinitionIndex != WEAPON_BREACHCHARGE && nDefinitionIndex != WEAPON_BUMPMINE)
				{
					// pass only active grenades/fists/tablet // @todo: or make it on right/left bottom side
					if (pWeaponData != nullptr && (pWeaponData->nWeaponType != WEAPONTYPE_GRENADE && nDefinitionIndex != WEAPON_FISTS && nDefinitionIndex != WEAPON_TABLET))
					{
						// draw weapons list
						const char* szIcon = U::GetWeaponIcon(nDefinitionIndex);
						const ImVec2 vecIconSize = F::Icons->CalcTextSizeA(flMainFontSize, FLT_MAX, 0.0f, szIcon);
						D::AddText(F::Icons, flMainFontSize, ImVec2(ctx.box.left + ctx.box.width * 0.5f - vecIconSize.x * 0.5f, ctx.box.bottom + 2 + ctx.arrPadding.at(DIR_BOTTOM)), szIcon, C::Get<Color>(Vars.colEspMainPlayerWeaponIcon), IMGUI_TEXT_DROPSHADOW, colOutline);
						ctx.arrPadding.at(DIR_BOTTOM) += vecIconSize.y;
					}
				}
			}
		}
	}
	#pragma endregion

	#pragma region visuals_player_left
	if (C::Get<bool>(Vars.bEspMainPlayerHealth))
	{
		// calculate hp-based color
		const float flFactor = static_cast<float>(pEntity->GetHealth()) / static_cast<float>(pEntity->GetMaxHealth());
		const float flHue = (flFactor * 120.f) / 360.f;
		HealthBar(ctx, flFactor, pEntity->GetHealth(), pEntity->GetMaxHealth(), Color::FromHSB(flHue, 1.f, 1.f), Color(0, 0, 0, 150), Color(0, 0, 0, 150));
	}

	#pragma region visuals_player_right
	if (C::Get<std::vector<bool>>(Vars.vecEspMainPlayerFlags).at(INFO_FLAG_ARMOR) && pEntity->GetArmor() > 0)
	{
		const char* szArmorText = pEntity->HasHelmet() ? "hk" : "k";
		const ImVec2 vecArmorSize = F::Roboto->CalcTextSizeA(flOtherFontSize, FLT_MAX, 0.0f, szArmorText);
		D::AddText(F::Roboto, flOtherFontSize, ImVec2(ctx.box.right + 2, ctx.box.top + ctx.arrPadding.at(DIR_RIGHT)), szArmorText, Color(255, 255, 255), IMGUI_TEXT_DROPSHADOW, colOutline);
		ctx.arrPadding.at(DIR_RIGHT) += vecArmorSize.y;
	}

	if (C::Get<std::vector<bool>>(Vars.vecEspMainPlayerFlags).at(INFO_FLAG_KIT) && pEntity->HasDefuser())
	{
		constexpr const char* szKitText = "kit";
		const ImVec2 vecKitSize = F::Roboto->CalcTextSizeA(flOtherFontSize, FLT_MAX, 0.0f, szKitText);
		D::AddText(F::Roboto, flOtherFontSize, ImVec2(ctx.box.right + 2, ctx.box.top + ctx.arrPadding.at(DIR_RIGHT)), szKitText, Color(255, 255, 255), IMGUI_TEXT_DROPSHADOW, colOutline);
		ctx.arrPadding.at(DIR_RIGHT) += vecKitSize.y;
	}

	if (C::Get<std::vector<bool>>(Vars.vecEspMainPlayerFlags).at(INFO_FLAG_ZOOM) && pEntity->IsScoped())
	{
		constexpr const char* szScopedText = "scoped";
		const ImVec2 vecScopedSize = F::Roboto->CalcTextSizeA(flOtherFontSize, FLT_MAX, 0.0f, szScopedText);
		D::AddText(F::Roboto, flOtherFontSize, ImVec2(ctx.box.right + 2, ctx.box.top + ctx.arrPadding.at(DIR_RIGHT)), szScopedText, Color(255, 255, 255), IMGUI_TEXT_DROPSHADOW, colOutline);
		ctx.arrPadding.at(DIR_RIGHT) += vecScopedSize.y;
	}
	#pragma endregion
}

void CVisuals::Box(const Box_t& box, const Color& colPrimary, const Color& colOutline)
{
	D::AddRect(ImVec2(box.left, box.top), ImVec2(box.right, box.bottom), colPrimary, IMGUI_RECT_OUTLINE | IMGUI_RECT_BORDER, colOutline);
}

void CVisuals::HealthBar(Context_t& ctx, const float flFactor, const int iHealth, const int iMaxHealth, const Color& colPrimary, const Color& colBackground, const Color& colOutline)
{
	// background
	D::AddRect(ImVec2(ctx.box.left - 5 - ctx.arrPadding.at(DIR_LEFT), ctx.box.top), ImVec2(ctx.box.left - 3 - ctx.arrPadding.at(DIR_LEFT), ctx.box.bottom), colBackground, IMGUI_RECT_FILLED | IMGUI_RECT_OUTLINE, colOutline);
	// bar
	D::AddRect(ImVec2(ctx.box.left - 5 - ctx.arrPadding.at(DIR_LEFT), ctx.box.bottom - (ctx.box.height * flFactor)), ImVec2(ctx.box.left - 3 - ctx.arrPadding.at(DIR_LEFT), ctx.box.bottom), C::Get<bool>(Vars.bEspMainPlayerOverrideHealthColor) ? C::Get<Color>(Vars.colEspMainPlayerHealth) : colPrimary, IMGUI_RECT_FILLED);
	// text
	if (iHealth < iMaxHealth)
	{  
		std::string szHealthText = std::to_string(iHealth);
		ImVec2 vecTextSize = F::Tahoma->CalcTextSizeA(9.0f, FLT_MAX, 0.0f, szHealthText.data());
		D::AddText(F::Tahoma, 9.0f, ImVec2(ctx.box.left - 4 - ctx.arrPadding.at(DIR_LEFT) - vecTextSize.x / 2.f, ctx.box.bottom - (ctx.box.height * flFactor) - vecTextSize.y / 2.f), szHealthText, Color(255, 255, 255, 255), IMGUI_TEXT_OUTLINE, Color(0, 0, 0, 200));
	}

	ctx.arrPadding.at(DIR_LEFT) += 6.0f;
}

void CVisuals::AmmoBar(CBaseEntity* pEntity, CBaseCombatWeapon* pWeapon, Context_t& ctx, const Color& colPrimary, const Color& colBackground, const Color& colOutline)
{
	CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(pWeapon->GetItemDefinitionIndex());
	if (pWeaponData == nullptr)
		return;

	// shown only for guns
	if (!pWeaponData->IsGun())
		return;

	// get ammo count
	const int iAmmo = pWeapon->GetAmmo();
	const int iMaxAmmo = pWeaponData->iMaxClip1;

	// add modifiable width factor
	float flFactor = 0.f;

	CAnimationLayer* pLayer = nullptr;
	int nActivity = 0;

	// get entity animation activity
	if (pEntity != nullptr)
	{
		pLayer = pEntity->GetAnimationLayer(1);
		nActivity = pEntity->GetSequenceActivity(pLayer->nSequence);
	}

	// calculate ammo-based width factor
		// check for reloading animation
	if (pEntity != nullptr && pLayer != nullptr && nActivity == ACT_CSGO_RELOAD && pLayer->flWeight != 0.0f)
		// reloading width
		flFactor = pLayer->flCycle;
	else
		// if not reloading
		flFactor = static_cast<float>(iAmmo) / static_cast<float>(iMaxAmmo);

	// background
	D::AddRect(ImVec2(ctx.box.left, ctx.box.bottom + 3 + ctx.arrPadding.at(DIR_BOTTOM)), ImVec2(ctx.box.right, ctx.box.bottom + 5 + ctx.arrPadding.at(DIR_BOTTOM)), colBackground, IMGUI_RECT_FILLED | IMGUI_RECT_OUTLINE, colOutline);
	// bar
	D::AddRect(ImVec2(ctx.box.left, ctx.box.bottom + 3 + ctx.arrPadding.at(DIR_BOTTOM)), ImVec2(ctx.box.left + ctx.box.width * flFactor, ctx.box.bottom + 5 + ctx.arrPadding.at(DIR_BOTTOM)), colPrimary, IMGUI_RECT_FILLED);
	ctx.arrPadding.at(DIR_BOTTOM) += 6.0f;
}

void CVisuals::FlashBar(CBaseEntity* pEntity, Context_t& ctx, const Color& colPrimary, const Color& colBackground, const Color& colOutline)
{
	// calculate flash alpha-based width factor
	float flFactor = pEntity->GetFlashAlpha() / *pEntity->GetFlashMaxAlpha();
	// background
	D::AddRect(ImVec2(ctx.box.left, ctx.box.top - 5 - ctx.arrPadding.at(DIR_TOP)), ImVec2(ctx.box.right, ctx.box.top - 3 - ctx.arrPadding.at(DIR_TOP)), colBackground, IMGUI_RECT_FILLED | IMGUI_RECT_OUTLINE, colOutline);
	// bar
	D::AddRect(ImVec2(ctx.box.left, ctx.box.top - 5 - ctx.arrPadding.at(DIR_TOP)), ImVec2(ctx.box.left + ctx.box.width * flFactor, ctx.box.top - 3 - ctx.arrPadding.at(DIR_TOP)), colPrimary, IMGUI_RECT_FILLED);
	ctx.arrPadding.at(DIR_TOP) += 6.0f;
}

void CVisuals::Skeleton(CBaseEntity* pEntity)
{
	studiohdr_t* pStudioModel = I::ModelInfo->GetStudioModel(pEntity->GetModel());
	if (pStudioModel == nullptr)
		return;

	Vector2D vecScreenParent, vecScreenChild;
	Vector vecParent, vecChild;

	for (int i = 0; i < pStudioModel->nBones; i++)
	{
		mstudiobone_t* pBone = pStudioModel->GetBone(i);

		if (pBone && pBone->iFlags & BONE_USED_BY_HITBOX && pBone->iParent != -1)
		{
			vecParent = pEntity->GetBonePosition(pBone->iParent).value();
			vecChild = pEntity->GetBonePosition(i).value();

			int iChestBone = BONE_SPINE_3;

			Vector vecUpperDirection = pEntity->GetBonePosition(iChestBone + 1).value() - pEntity->GetBonePosition(iChestBone).value();
			Vector vecBreastBone = pEntity->GetBonePosition(iChestBone).value() + vecUpperDirection / 2.f;
			
			Vector vecDeltaParent = vecParent - vecBreastBone;
			Vector vecDeltaChild = vecChild - vecBreastBone;

			if ((vecDeltaParent.Length() < 9 && vecDeltaChild.Length() < 9))
				vecParent = vecBreastBone;

			if (i == iChestBone - 1)
				vecChild = vecBreastBone;

			if (std::abs(vecDeltaChild.z) < 5 && (vecDeltaParent.Length() < 5 && vecDeltaChild.Length() < 5) || i == iChestBone)
				continue;

			if (D::WorldToScreen(vecParent, vecScreenParent) && D::WorldToScreen(vecChild, vecScreenChild))
				D::AddLine(ImVec2(vecScreenParent.x, vecScreenParent.y), ImVec2(vecScreenChild.x, vecScreenChild.y), C::Get<Color>(Vars.colEspMainPlayerSkeleton));
		}
	}

}
