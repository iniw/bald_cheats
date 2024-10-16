﻿// used: std::array
#include <array>

#include "hooks.h"
// used: global variables
#include "../global.h"
// used: cheat variables
#include "variables.h"
// used: input system
#include "../utilities/inputsystem.h"
// used: logging
#include "../utilities/logging.h"
// used: initialize state
#include "../utilities/draw.h"
// used: set localplayer ready
#include "../utilities.h"
// used: render windows
#include "menu.h"
// used: randomint
#include "../utilities/math.h"

/* features */
#include "../features/prediction.h"
#include "../features/legitbot.h"
#include "../features/lagcompensation.h"
#include "../features/triggerbot.h"
#include "../features/visuals.h"
#include "../features/misc.h"
#include "../features/skinchanger.h"

constexpr std::array<const char*, 3U> arrSmokeMaterials =
{
	//"particle/vistasmokev1/vistasmokev1_fire",  // to look cool fresh fashionable yo :sunglasses: (if u wont be cool just uncomment this)
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
};

#pragma region hooks_get
bool H::Setup()
{
	if (MH_Initialize() != MH_OK)
		throw std::runtime_error(XorStr("failed initialize minhook"));

	if (!DTR::Reset.Create(MEM::GetVFunc(I::DirectDevice, VTABLE::RESET), &hkReset))
		return false;

	if (!DTR::EndScene.Create(MEM::GetVFunc(I::DirectDevice, VTABLE::ENDSCENE), &hkEndScene))
		return false;

	if (!DTR::LevelInitPostEntity.Create(MEM::GetVFunc(I::Client, VTABLE::LEVELINITPOSTENTITY), &hkLevelInitPostEntity))
		return false;

	if (!DTR::FrameStageNotify.Create(MEM::GetVFunc(I::Client, VTABLE::FRAMESTAGENOTIFY), &hkFrameStageNotify))
		return false;

	if (!DTR::OverrideView.Create(MEM::GetVFunc(I::ClientMode, VTABLE::OVERRIDEVIEW), &hkOverrideView))
		return false;

	if (!DTR::CreateMove.Create(MEM::GetVFunc(I::ClientMode, VTABLE::CREATEMOVE), &hkCreateMove))
		return false;

	if (!DTR::GetViewModelFOV.Create(MEM::GetVFunc(I::ClientMode, VTABLE::GETVIEWMODELFOV), &hkGetViewModelFOV))
		return false;

	if (!DTR::DoPostScreenEffects.Create(MEM::GetVFunc(I::ClientMode, VTABLE::DOPOSTSCREENEFFECTS), &hkDoPostScreenEffects))
		return false;

	if (!DTR::IsConnected.Create(MEM::GetVFunc(I::Engine, VTABLE::ISCONNECTED), &hkIsConnected))
		return false;

	if (!DTR::IsHLTV.Create(MEM::GetVFunc(I::Engine, VTABLE::ISHLTV), &hkIsHLTV))
		return false;

	if (!DTR::ListLeavesInBox.Create(MEM::GetVFunc(I::Engine->GetBSPTreeQuery(), VTABLE::LISTLEAVESINBOX), &hkListLeavesInBox))
		return false;

	if (!DTR::PaintTraverse.Create(MEM::GetVFunc(I::Panel, VTABLE::PAINTTRAVERSE), &hkPaintTraverse))
		return false;

	if (!DTR::DrawModel.Create(MEM::GetVFunc(I::StudioRender, VTABLE::DRAWMODEL), &hkDrawModel))
		return false;

	if (!DTR::GetLocalViewAngles.Create(MEM::GetVFunc(I::Prediction, VTABLE::GETLOCALVIEWANGLES), &hkGetLocalViewAngles))
		return false;

	if (!DTR::RunCommand.Create(MEM::GetVFunc(I::Prediction, VTABLE::RUNCOMMAND), &hkRunCommand))
		return false;

	if (!DTR::SendMessageGC.Create(MEM::GetVFunc(I::SteamGameCoordinator, VTABLE::SENDMESSAGE), &hkSendMessage))
		return false;

	if (!DTR::RetrieveMessage.Create(MEM::GetVFunc(I::SteamGameCoordinator, VTABLE::RETRIEVEMESSAGE), &hkRetrieveMessage))
		return false;

	if (!DTR::LockCursor.Create(MEM::GetVFunc(I::Surface, VTABLE::LOCKCURSOR), &hkLockCursor))
		return false;

	if (!DTR::PlaySoundSurface.Create(MEM::GetVFunc(I::Surface, VTABLE::PLAYSOUND), &hkPlaySound))
		return false;

	static CConVar* sv_cheats = I::ConVar->FindVar(XorStr("sv_cheats"));
	if (!DTR::sv_cheatsGetBool.Create(MEM::GetVFunc(sv_cheats, VTABLE::GETBOOL), &hksv_cheatsGetBool))
		return false;

	/*
	static CConVar* r_3dsky = I::ConVar->FindVar(XorStr("r_3dsky"));
	if (!DTR::r_3dskyGetInt.Create(MEM::GetVFunc(r_3dsky, VTABLE::GETINT), &hkr_3dskyGetInt))
		return false;

	static CConVar* cl_csm_enabled = I::ConVar->FindVar(XorStr("cl_csm_enabled"));
	if (!DTR::cl_csm_enabledGetBool.Create(MEM::GetVFunc(cl_csm_enabled, VTABLE::GETBOOL), &hkcl_csm_enabledGetBool))
		return false;
	
	static CConVar* mat_postprocess_enable = I::ConVar->FindVar(XorStr("mat_postprocess_enable"));
	if (!DTR::mat_postprocess_enableGetBool.Create(MEM::GetVFunc(mat_postprocess_enable, VTABLE::GETBOOL), &hkmat_postprocess_enableGetBool))
		return false;

	static CConVar* weapon_debug_spread_show = I::ConVar->FindVar(XorStr("weapon_debug_spread_show"));
	if (!DTR::weapon_debug_spread_showGetInt.Create(MEM::GetVFunc(weapon_debug_spread_show, VTABLE::GETINT), &hkweapon_debug_spread_showGetInt))
		return false;
	*/

	static auto pPlayerTable = MEM::GetVTablePointer(CLIENT_DLL, XorStr("C_CSPlayer"));	

	if (!DTR::DoExtraBoneProcessing.Create(MEM::GetVFunc(pPlayerTable, VTABLE::DOEXTRABONEPROCESSING), &hkDoExtraBoneProcessing))
		return false;

	if (!DTR::StandardBlendingRules.Create(MEM::GetVFunc(pPlayerTable, VTABLE::STANDARDBLENDINGRULES), &hkStandardBlendingRules))
		return false;

	return true;
}

void H::Restore()
{
	DTR::Reset.Remove();
	DTR::EndScene.Remove();
	DTR::FrameStageNotify.Remove();
	DTR::OverrideView.Remove();
	DTR::CreateMove.Remove();
	DTR::SendNetMsg.Remove();
	DTR::SendDatagram.Remove();
	DTR::GetViewModelFOV.Remove();
	DTR::DoPostScreenEffects.Remove();
	DTR::IsConnected.Remove();
	DTR::IsHLTV.Remove();
	DTR::ListLeavesInBox.Remove();
	DTR::PaintTraverse.Remove();
	DTR::DrawModel.Remove();
	DTR::RunCommand.Remove();
	DTR::GetLocalViewAngles.Remove();
	DTR::SendMessageGC.Remove();
	DTR::RetrieveMessage.Remove();
	DTR::LockCursor.Remove();
	DTR::PlaySoundSurface.Remove();
	DTR::sv_cheatsGetBool.Remove();
	/*
	DTR::r_3dskyGetInt.Remove();
	DTR::cl_csm_enabledGetBool.Remove();
	DTR::mat_postprocess_enableGetBool.Remove();
	DTR::weapon_debug_spread_showGetInt.Remove();
	*/
	DTR::DoExtraBoneProcessing.Remove();
	DTR::StandardBlendingRules.Remove();

	// @note: also should works but makes it undebuggable
	#if 0
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);
	#endif

	MH_Uninitialize();
}
#pragma endregion

#pragma region hooks_handlers
#pragma region hooks_directx
long D3DAPI H::hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	static auto oReset = DTR::Reset.GetOriginal<decltype(&hkReset)>();

	// check for first initialization
	if (!D::bInitialized)
		return oReset(pDevice, pPresentationParameters);

	// invalidate vertex & index buffer, release fonts texture
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const HRESULT hReset = oReset(pDevice, pPresentationParameters);

	// get directx device and create fonts texture
	if (hReset == D3D_OK)
		ImGui_ImplDX9_CreateDeviceObjects();

	return hReset;
}

long D3DAPI H::hkEndScene(IDirect3DDevice9* pDevice)
{
	static auto oEndScene = DTR::EndScene.GetOriginal<decltype(&hkEndScene)>();
	static void* pUsedAddress = nullptr;

	if (pUsedAddress == nullptr)
	{
		// search for gameoverlay address
		MEMORY_BASIC_INFORMATION memInfo;
		VirtualQuery(_ReturnAddress(), &memInfo, sizeof(MEMORY_BASIC_INFORMATION));

		char chModuleName[MAX_PATH];
		GetModuleFileName(static_cast<HMODULE>(memInfo.AllocationBase), chModuleName, MAX_PATH);

		if (strstr(chModuleName, GAMEOVERLAYRENDERER_DLL) != nullptr)
			pUsedAddress = _ReturnAddress();
	}

	// check for called from gameoverlay and render here to bypass capturing programs
	if (_ReturnAddress() == pUsedAddress)
	{
		// init gui (fonts, sizes, styles, colors) once
		if (!D::bInitialized)
			D::Setup(pDevice);

		DWORD dwColorWriteOld = 0UL, dwSRGBWriteOld = 0UL;

		// save
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwColorWriteOld);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &dwSRGBWriteOld);

		// set
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// render cheat menu & visuals
		W::MainWindow(pDevice);

		ImGui::EndFrame();
		ImGui::Render();

		// render draw lists from draw data
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		// restore
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwColorWriteOld);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, dwSRGBWriteOld);
	}

	return oEndScene(pDevice);
}
#pragma endregion

#pragma region hooks_clientmode
bool FASTCALL H::hkCreateMove(IClientModeShared* thisptr, int edx, float flInputSampleTime, CUserCmd* pCmd)
{
	static auto oCreateMove = DTR::CreateMove.GetOriginal<decltype(&hkCreateMove)>();

	/*
	 * get global localplayer pointer
	 * @note: dont forget check global localplayer for nullptr when using not in createmove
	 * also not recommended use so far
	 */
	CBaseEntity* pLocal = G::pLocal = CBaseEntity::GetLocalPlayer();
	
	// is called from CInput::ExtraMouseSample
	if (pCmd->iCommandNumber == 0)
		return oCreateMove(thisptr, edx, flInputSampleTime, pCmd);

	/*
	 * check is called from CInput::CreateMove
	 * and SetLocalViewAngles for engine/prediction at the same time
	 * cuz SetViewAngles isn't called if return false and can cause frame stuttering
	 */
	if (oCreateMove(thisptr, edx, flInputSampleTime, pCmd))
		I::Prediction->SetLocalViewAngles(pCmd->angViewPoint);

	// save global cmd pointer
	G::pCmd = pCmd;

	if (I::ClientState == nullptr || I::Engine->IsPlayingDemo())
		return oCreateMove(thisptr, edx, flInputSampleTime, pCmd);

	// netchannel pointer
	INetChannel* pNetChannel = I::ClientState->pNetChannel;

	// get stack frame without asm inlines
	// safe and will not break if you omitting frame pointer
	const volatile auto vlBaseAddress = *reinterpret_cast<std::uintptr_t*>(reinterpret_cast<std::uintptr_t>(_AddressOfReturnAddress()) - sizeof(std::uintptr_t));

	/*
	 * get sendpacket pointer from stack frame
	 * if use global sendpacket value then sendpacket applies only on next tick
	 * im not recommend use globals anywhere
	 */
	bool& bSendPacket = *reinterpret_cast<bool*>(vlBaseAddress - 0x1C);

	// save previous view angles for movement correction
	QAngle angOldViewPoint = pCmd->angViewPoint;

	// @note: need do bunnyhop and other movements before prediction
	CMiscellaneous::Get().Run(pCmd, pLocal, bSendPacket);

	int iPreFlags = pLocal->GetFlags();

	/*
	 * CL_RunPrediction
	 * correct prediction when framerate is lower than tickrate
	 * https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/cl_pred.cpp#L41
	 */
	if (I::ClientState->iDeltaTick > 0)
		I::Prediction->Update(I::ClientState->iDeltaTick, I::ClientState->iDeltaTick > 0, I::ClientState->iLastCommandAck, I::ClientState->iLastOutgoingCommand + I::ClientState->nChokedCommands);

	CPrediction::Get().Start(pCmd, pLocal);
	{
		if (C::Get<bool>(Vars.bMiscAutoPistol))
			CMiscellaneous::Get().AutoPistol(pCmd, pLocal);

		if (C::Get<bool>(Vars.bLegit) && pLocal != nullptr && pLocal->IsAlive())
			CLegitBot::Get().Run(pCmd, pLocal, bSendPacket);

		if (C::Get<bool>(Vars.bTrigger))
			CTriggerBot::Get().Run(pCmd, pLocal);

		//if (C::Get<bool>(Vars.bBacktracking))
		//	CBacktracking::Get().Run(pCmd, pLocal);
	}
	CPrediction::Get().End(pCmd, pLocal);

	CBaseEntity* pLocalPostPred = CBaseEntity::GetLocalPlayer();

	int iPostFlags = CBaseEntity::GetLocalPlayer()->GetFlags();

	CMiscellaneous::Get().PostPrediction(pCmd, iPreFlags, pLocalPostPred->GetFlags());

	if (C::Get<int>(Vars.iMiscBlockBotKey) && IPT::IsKeyDown(C::Get<int>(Vars.iMiscBlockBotKey)))
		CMiscellaneous::Get().BlockBot(pCmd, pLocal);

	if(pLocal->IsAlive())
		CMiscellaneous::Get().MovementCorrection(pCmd, pLocalPostPred, angOldViewPoint);

	// clamp & normalize view angles
	if (C::Get<bool>(Vars.bMiscAntiUntrusted))
	{
		pCmd->angViewPoint.Normalize();
		pCmd->angViewPoint.Clamp();
	}

	C::Get<bool>(Vars.bMiscFakeLatency) ? CLagCompensation::Get().UpdateIncomingSequences(pNetChannel) : CLagCompensation::Get().ClearIncomingSequences();

	// @note: doesnt need rehook cuz detours here
	if (pNetChannel != nullptr)
	{
		if (!DTR::SendNetMsg.IsHooked())
			DTR::SendNetMsg.Create(MEM::GetVFunc(pNetChannel, VTABLE::SENDNETMSG), H::hkSendNetMsg);

		if (!DTR::SendDatagram.IsHooked())
			DTR::SendDatagram.Create(MEM::GetVFunc(pNetChannel, VTABLE::SENDDATAGRAM), H::hkSendDatagram);
	}

	// save next view angles state
	G::angRealView = pCmd->angViewPoint;

	// save next global sendpacket state
	G::bSendPacket = bSendPacket;

	// THIS SMELLS LIKE SHIT !!! IT'S FUCKING DIRTY HOLY SHIIIIT
	if (C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_DECALS))
		I::Engine->ExecuteClientCmd(XorStr("r_cleardecals"));

	return false;
}

void FASTCALL H::hkOverrideView(IClientModeShared* thisptr, int edx, CViewSetup* pSetup)
{
	static auto oOverrideView = DTR::OverrideView.GetOriginal<decltype(&hkOverrideView)>();

	if (!I::Engine->IsInGame() || I::Engine->IsTakingScreenshot())
		return oOverrideView(thisptr, edx, pSetup);

	// get camera origin
	G::vecCamera = pSetup->vecOrigin;

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();

	if (pLocal == nullptr || !pLocal->IsAlive())
		return oOverrideView(thisptr, edx, pSetup);

	CBaseCombatWeapon* pWeapon = pLocal->GetWeapon();

	if (pWeapon == nullptr)
		return oOverrideView(thisptr, edx, pSetup);

	if (CCSWeaponData* pWeaponData = I::WeaponSystem->GetWeaponData(pWeapon->GetItemDefinitionIndex());
		pWeaponData != nullptr && std::fpclassify(C::Get<float>(Vars.flScreenCameraFOV)) != FP_ZERO &&
		// check is we not scoped
		(pWeaponData->nWeaponType == WEAPONTYPE_SNIPER ? !pLocal->IsScoped() : true))
		// set camera fov
		pSetup->flFOV += C::Get<float>(Vars.flScreenCameraFOV);

	G::flFov = pSetup->flFOV;

	oOverrideView(thisptr, edx, pSetup);
}

float FASTCALL H::hkGetViewModelFOV(IClientModeShared* thisptr, int edx)
{
	static auto oGetViewModelFOV = DTR::GetViewModelFOV.GetOriginal<decltype(&hkGetViewModelFOV)>();

	if (!I::Engine->IsInGame() || I::Engine->IsTakingScreenshot())
		return oGetViewModelFOV(thisptr, edx);

	if (auto pLocal = CBaseEntity::GetLocalPlayer();
		pLocal != nullptr && pLocal->IsAlive() && std::fpclassify(C::Get<float>(Vars.flScreenViewModelFOV)) != FP_ZERO)
		return oGetViewModelFOV(thisptr, edx) + C::Get<float>(Vars.flScreenViewModelFOV);

	return oGetViewModelFOV(thisptr, edx);
}

int FASTCALL H::hkDoPostScreenEffects(IClientModeShared* thisptr, int edx, CViewSetup* pSetup)
{
	static auto oDoPostScreenEffects = DTR::DoPostScreenEffects.GetOriginal<decltype(&hkDoPostScreenEffects)>();

	if (!I::Engine->IsInGame() || I::Engine->IsTakingScreenshot())
		return oDoPostScreenEffects(thisptr, edx, pSetup);

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();

	if (pLocal != nullptr && I::GlowManager != nullptr && C::Get<bool>(Vars.bEspGlow))
		CVisuals::Get().Glow(pLocal);

	return oDoPostScreenEffects(thisptr, edx, pSetup);
}
#pragma endregion

#pragma region hooks_surface
void FASTCALL H::hkPaintTraverse(ISurface* thisptr, int edx, unsigned int uPanel, bool bForceRepaint, bool bForce)
{
	static auto oPaintTraverse = DTR::PaintTraverse.GetOriginal<decltype(&hkPaintTraverse)>();
	const FNV1A_t uPanelHash = FNV1A::Hash(I::Panel->GetName(uPanel));

	//CMiscellaneous::Get().AutoQueue();

	// remove zoom panel
	if (!I::Engine->IsTakingScreenshot() && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SCOPE) && uPanelHash == FNV1A::HashConst("HudZoom"))
		return;

	oPaintTraverse(thisptr, edx, uPanel, bForceRepaint, bForce);

	if (!D::bInitialized)
		return;

	// @note: we don't render here, only store's data and render it later
	if (uPanelHash == FNV1A::HashConst("FocusOverlayPanel"))
	{
		// clear data from previous call
		D::ClearDrawData();

		// store data to render
		CVisuals::Get().Store();

		// debugging pourposes
		CLegitBot::Get().Draw();
		CBacktracking::Get().Draw();

		// swap given data to safe container
		D::SwapDrawData();
	}
}

void FASTCALL H::hkPlaySound(ISurface* thisptr, int edx, const char* szFileName)
{
	static auto oPlaySound = DTR::PlaySoundSurface.GetOriginal<decltype(&hkPlaySound)>();
	oPlaySound(thisptr, edx, szFileName);
}

void FASTCALL H::hkLockCursor(ISurface* thisptr, int edx)
{
	static auto oLockCursor = DTR::LockCursor.GetOriginal<decltype(&hkLockCursor)>();

	if (W::bMainOpened)
	{
		I::Surface->UnLockCursor();
		return;
	}

	oLockCursor(thisptr, edx);
}
#pragma endregion

#pragma region hooks_baseclient
void FASTCALL H::hkLevelInitPostEntity(IBaseClientDll* thisptr, int edx)
{
	static auto oLevelInitPreEntity = DTR::LevelInitPostEntity.GetOriginal<decltype(&hkLevelInitPostEntity)>();

	//U::EntityListener.Setup();

	return oLevelInitPreEntity(thisptr, edx);
}

void FASTCALL H::hkFrameStageNotify(IBaseClientDll* thisptr, int edx, EClientFrameStage stage)
{
	static auto oFrameStageNotify = DTR::FrameStageNotify.GetOriginal<decltype(&hkFrameStageNotify)>();

	if (!I::Engine->IsInGame())
	{
		CLagCompensation::Get().ClearIncomingSequences(); // clear sequences or we get commands overflow on new map connection
		CVisuals::Get().m_deqHitMarkers.clear(); // clear hitmarker info
		return oFrameStageNotify(thisptr, edx, stage);
	}

	if (I::Engine->IsTakingScreenshot())
		return oFrameStageNotify(thisptr, edx, stage);

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();

	if (pLocal == nullptr)
		return oFrameStageNotify(thisptr, edx, stage);

	CVisuals::Get().SkyChanger(stage);

	static QAngle angAimPunchOld = { }, angViewPunchOld = { };

	switch (stage)
	{
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
	{
		/*
		 * data has been received and we are going to start calling postdataupdate
		 * e.g. resolver or skinchanger and other visuals
		 */

		if(pLocal != nullptr && I::Engine->IsInGame())
			CSkinChanger::Get().Run(pLocal);

		break;
	}
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
	{
		/*
		 * data has been received and called postdataupdate on all data recipients
		 * e.g. now we can modify interpolation, other lagcompensation stuff
		 */

		if (C::Get<bool>(Vars.bBacktracking) && pLocal->IsAlive())
		{
			for (int i = 1; i <= I::Globals->nMaxClients; i++)
			{
				if (i == I::Engine->GetLocalPlayer())
					continue;

				CBaseEntity* pEntity = I::ClientEntityList->Get<CBaseEntity>(i);

				if (pEntity == nullptr || !pEntity->IsAlive() || !pLocal->IsEnemy(pEntity))
					continue;

				VarMapping_t* map = pEntity->VarMapping();

				if (map != nullptr)
				{
					for (int j = 0; j < map->m_nInterpolatedEntries; j++)
					{
						map->m_Entries[j].m_bNeedsToInterpolate = false;
					}
				}
			}
			
		}

		break;
	}
	case FRAME_NET_UPDATE_END:
	{
		/*
		 * received all packets, now do interpolation, prediction, etc
		 * e.g. backtrack stuff
		 */

		CBacktracking::Get().Update(pLocal);

		break;
	}
	case FRAME_RENDER_START:
	{
		/*
		 * start rendering the scene
		 * e.g. remove visual punch, thirdperson, other render/update stuff
		 */

		 // set max flash alpha
		*pLocal->GetFlashMaxAlpha() = C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_FLASHBANG) ? 0.f : 255.f;

		//CVisuals::Get().NightMode();
		CVisuals::Get().Removals();

		// no draw smoke
		for (auto szSmokeMaterial : arrSmokeMaterials)
		{
			IMaterial* pMaterial = I::MaterialSystem->FindMaterial(szSmokeMaterial, TEXTURE_GROUP_OTHER);

			if (pMaterial != nullptr && !pMaterial->IsErrorMaterial())
				pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SMOKE));
		}

		// remove smoke overlay
		static std::uintptr_t uSmokeCount = (MEM::FindPattern(CLIENT_DLL, XorStr("55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0")) + 0x8); // @xref: "effects/overlaysmoke"
		if (C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SMOKE))
			*reinterpret_cast<int*>(*reinterpret_cast<std::uintptr_t*>(uSmokeCount)) = 0;

		// remove visual punch
		if (pLocal->IsAlive())
		{
			// save old values
			angViewPunchOld = pLocal->GetViewPunch();
			angAimPunchOld = pLocal->GetPunch();

			if (C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_PUNCH))
			{
				// change current values
				pLocal->GetViewPunch() = QAngle(0, 0, 0);
				pLocal->GetPunch() = QAngle(0, 0, 0);
			}
		}

		// thirdperson
		if (C::Get<int>(Vars.iWorldThirdPersonKey) > 0)
		{
			static bool bThirdPerson = false;

			if (!I::Engine->IsConsoleVisible() && IPT::IsKeyReleased(C::Get<int>(Vars.iWorldThirdPersonKey)))
				bThirdPerson = !bThirdPerson;

			// my solution is here cuz camera offset is dynamically by standard functions without any garbage in overrideview hook
			I::Input->bCameraInThirdPerson = bThirdPerson && pLocal->IsAlive() && !I::Engine->IsTakingScreenshot();
			I::Input->vecCameraOffset.z = bThirdPerson ? static_cast<float>(C::Get<int>(Vars.iWorldThirdPersonOffset)) : 150.f;
		}

		break;
	}
	case FRAME_RENDER_END:
	{
		/*
		 * finished rendering the scene
		 * here we can restore our modified things
		 */

		// restore original visual punch values
		if (pLocal->IsAlive() && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_PUNCH))
		{
			pLocal->GetViewPunch() = angViewPunchOld;
			pLocal->GetPunch() = angAimPunchOld;
		}

		break;
	}
	default:
		break;
	}

	oFrameStageNotify(thisptr, edx, stage);
}
#pragma endregion

#pragma region hooks_studiorender
void FASTCALL H::hkDrawModel(IStudioRender* thisptr, int edx, DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags)
{
	static auto oDrawModel = DTR::DrawModel.GetOriginal<decltype(&hkDrawModel)>();

	if (!I::Engine->IsInGame() || I::Engine->IsTakingScreenshot())
		return oDrawModel(thisptr, edx, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	bool bClearOverride = false;

	if (pLocal != nullptr && C::Get<bool>(Vars.bEspChams))
		bClearOverride = CVisuals::Get().Chams(pLocal, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

	oDrawModel(thisptr, edx, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);
	
	if (bClearOverride)
		I::StudioRender->ForcedMaterialOverride(nullptr);
}

int FASTCALL H::hkListLeavesInBox(void* thisptr, int edx, const Vector& vecMins, const Vector& vecMaxs, unsigned short* puList, int nListMax)
{
	static auto oListLeavesInBox = DTR::ListLeavesInBox.GetOriginal<decltype(&hkListLeavesInBox)>();

	// @todo: sometimes models doesn't drawn on certain maps (not only me: https://www.unknowncheats.me/forum/counterstrike-global-offensive/330483-disable-model-occulusion-3.html)
	// @test: try to fix z order 11.08.20

	// @credits: soufiw
	// occlusion getting updated on player movement/angle change,
	// in RecomputeRenderableLeaves https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L674
	static std::uintptr_t uInsertIntoTree = (MEM::FindPattern(CLIENT_DLL, XorStr("56 52 FF 50 18")) + 0x5); // @xref: "<unknown renderable>"

	// check for esp state and call from CClientLeafSystem::InsertIntoTree
	if (C::Get<bool>(Vars.bEspChams) && C::Get<bool>(Vars.bEspChamsEnemies) && reinterpret_cast<std::uintptr_t>(_ReturnAddress()) == uInsertIntoTree)
	{
		// get current renderable info from stack https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1470
		if (const auto pInfo = *reinterpret_cast<RenderableInfo_t**>(reinterpret_cast<std::uintptr_t>(_AddressOfReturnAddress()) + 0x14); pInfo != nullptr)
		{
			if (const auto pRenderable = pInfo->pRenderable; pRenderable != nullptr)
			{
				// check if disabling occlusion for players
				if (const auto pEntity = pRenderable->GetIClientUnknown()->GetBaseEntity(); pEntity != nullptr && pEntity->IsPlayer())
				{
					// fix render order, force translucent group (https://www.unknowncheats.me/forum/2429206-post15.html)
					// AddRenderablesToRenderLists: https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L2473
					// @ida addrenderablestorenderlists: 55 8B EC 83 EC 24 53 56 8B 75 08 57 8B 46
					pInfo->uFlags &= ~RENDER_FLAGS_FORCE_OPAQUE_PASS;
					pInfo->uFlags2 |= RENDER_FLAGS_BOUNDS_ALWAYS_RECOMPUTE;

					// extend world space bounds to maximum https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L707
					constexpr Vector vecMapMin(MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT);
					constexpr Vector vecMapMax(MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT);
					return oListLeavesInBox(thisptr, edx, vecMapMin, vecMapMax, puList, nListMax);
				}
			}
		}
	}

	return oListLeavesInBox(thisptr, edx, vecMins, vecMaxs, puList, nListMax);
}
#pragma endregion

#pragma region hooks_engineclient
bool FASTCALL H::hkIsConnected(IEngineClient* thisptr, int edx)
{
	static auto oIsConnected = DTR::IsConnected.GetOriginal<decltype(&hkIsConnected)>();

	// @xref: "IsLoadoutAllowed"
	// sub above the string
	// sub in that function
	// .text : 103A2120 84 C0		test    al, al; Logical Compare
	static std::uintptr_t uLoadoutAllowedReturn = (MEM::FindPattern(CLIENT_DLL, XorStr("75 04 B0 01 5F")) - 0x2);

	// @credits: gavreel
	if (reinterpret_cast<std::uintptr_t>(_ReturnAddress()) == uLoadoutAllowedReturn && C::Get<bool>(Vars.bMiscUnlockInventory))
		return false;

	return oIsConnected(thisptr, edx);
}

bool FASTCALL H::hkIsHLTV(IEngineClient* thisptr, int edx)
{
	static auto oIsHLTV = DTR::IsHLTV.GetOriginal<decltype(&hkIsHLTV)>();

	static auto uSetupVelocity = MEM::FindPattern(CLIENT_DLL, XorStr("84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80"));
	static const auto uAccumulateLayers = MEM::FindPattern(CLIENT_DLL, XorStr("84 C0 75 0D F6 87"));

	if (reinterpret_cast<std::uintptr_t>(_ReturnAddress()) == uSetupVelocity || reinterpret_cast<std::uintptr_t>(_ReturnAddress()) == uAccumulateLayers)
		return true;

	return oIsHLTV(thisptr, edx);
}
#pragma endregion

#pragma region hooks_netchannel
bool FASTCALL H::hkSendNetMsg(INetChannel* thisptr, int edx, INetMessage* pMessage, bool bForceReliable, bool bVoice)
{
	static auto oSendNetMsg = DTR::SendNetMsg.GetOriginal<decltype(&hkSendNetMsg)>();

	/*
	 * @note: disable files crc check (sv_pure)
	 * dont send message if it has FileCRCCheck type
	 */
	if (pMessage->GetType() == 14)
		return false;

	/*
	 * @note: fix lag with chocking packets when voice chat is active
	 * check for voicedata group and enable voice stream
	 * @credits: Flaww
	 */
	if (pMessage->GetGroup() == INetChannelInfo::VOICE)
		bVoice = true;

	return oSendNetMsg(thisptr, edx, pMessage, bForceReliable, bVoice);
}

int FASTCALL H::hkSendDatagram(INetChannel* thisptr, int edx, bf_write* pDatagram)
{
	static auto oSendDatagram = DTR::SendDatagram.GetOriginal<decltype(&hkSendDatagram)>();

	INetChannelInfo* pNetChannelInfo = I::Engine->GetNetChannelInfo();
	static CConVar* sv_maxunlag = I::ConVar->FindVar(XorStr("sv_maxunlag"));

	if (!I::Engine->IsInGame() || !C::Get<bool>(Vars.bMiscFakeLatency) || pDatagram != nullptr || pNetChannelInfo == nullptr || sv_maxunlag == nullptr)
		return oSendDatagram(thisptr, edx, pDatagram);

	int iInReliableStateOld = thisptr->iInReliableState;
	int iInSequenceNrOld = thisptr->iInSequenceNr;

	// calculate max available fake latency with our real ping to keep it w/o real lags or delays
	float flMaxLatency = std::max(0.f, std::clamp(static_cast<float>(C::Get<int>(Vars.iMiscFakeLatencyAmount)) / 1000.f, 0.f, sv_maxunlag->GetFloat()) - pNetChannelInfo->GetLatency(FLOW_OUTGOING));
	CLagCompensation::Get().AddLatencyToNetChannel(thisptr, flMaxLatency);

	int iReturn = oSendDatagram(thisptr, edx, pDatagram);

	thisptr->iInReliableState = iInReliableStateOld;
	thisptr->iInSequenceNr = iInSequenceNrOld;

	return iReturn;
}
#pragma endregion

#pragma region hooks_prediction
void FASTCALL H::hkGetLocalViewAngles(IPrediction* thisptr, int edx, QAngle& ang)
{
	static auto oGetLocalViewAngles = DTR::GetLocalViewAngles.GetOriginal<decltype(&hkGetLocalViewAngles)>();

	if (I::Engine->IsRecordingDemo())
		I::Engine->GetViewAngles(ang); // pov demo psilent
	else
		oGetLocalViewAngles(thisptr, edx, ang);
}

void FASTCALL H::hkRunCommand(IPrediction* thisptr, int edx, CBaseEntity* pEntity, CUserCmd* pCmd, IMoveHelper* pMoveHelper)
{
	static auto oRunCommand = DTR::RunCommand.GetOriginal<decltype(&hkRunCommand)>();

	/* there is tickbase corrections / velocity modifier fix */

	oRunCommand(thisptr, edx, pEntity, pCmd, pMoveHelper);

	// get movehelper interface pointer
	I::MoveHelper = pMoveHelper;
}
#pragma endregion

#pragma region hooks_steamgamecoordinator
int FASTCALL H::hkSendMessage(ISteamGameCoordinator* thisptr, int edx, std::uint32_t uMsgType, const void* pData, std::uint32_t uData)
{
	static auto oSendMessage = DTR::SendMessageGC.GetOriginal<decltype(&hkSendMessage)>();

	std::uint32_t uMessageType = uMsgType & 0x7FFFFFFF;
	void* pDataMutable = const_cast<void*>(pData);

	const int iStatus = oSendMessage(thisptr, edx, uMsgType, pDataMutable, uData);

	if (iStatus != EGCResultOK)
		return iStatus;

	#ifdef DEBUG_CONSOLE
	L::PushConsoleColor(FOREGROUND_INTENSE_GREEN | FOREGROUND_RED);
	L::Print(fmt::format(XorStr("[<-] Message sent to GC {:d}!"), uMessageType));
	L::PopConsoleColor();
	#endif

	return iStatus;
}

int FASTCALL H::hkRetrieveMessage(ISteamGameCoordinator* thisptr, int edx, std::uint32_t* puMsgType, void* pDest, std::uint32_t uDest, std::uint32_t* puMsgSize)
{
	static auto oRetrieveMessage = DTR::RetrieveMessage.GetOriginal<decltype(&hkRetrieveMessage)>();
	const int iStatus = oRetrieveMessage(thisptr, edx, puMsgType, pDest, uDest, puMsgSize);

	if (iStatus != EGCResultOK)
		return iStatus;

	std::uint32_t uMessageType = *puMsgType & 0x7FFFFFFF;

	#ifdef DEBUG_CONSOLE
	L::PushConsoleColor(FOREGROUND_INTENSE_GREEN | FOREGROUND_RED);
	L::Print(fmt::format(XorStr("[->] Message received from GC {:d}!"), uMessageType));
	L::PopConsoleColor();
	#endif

	// check for k_EMsgGCCStrike15_v2_GCToClientSteamdatagramTicket message when we can accept the game
	if (C::Get<bool>(Vars.bMiscAutoAccept) && uMessageType == 9177)
	{
		U::SetLocalPlayerReady();
		U::FlashWindow(IPT::hWindow);
	}

	return iStatus;
}
#pragma endregion

#pragma region hooks_csplayer_table
void FASTCALL H::hkDoExtraBoneProcessing(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, matrix3x4_t* matrix, CBoneBitList& bone_list, CIKContext* context)
{
	static auto oDoExtraBoneProcessing = DTR::DoExtraBoneProcessing.GetOriginal<decltype(&hkDoExtraBoneProcessing)>();

	CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(ecx);
	if (pEntity == nullptr)
		return oDoExtraBoneProcessing(ecx, edx, hdr, pos, q, matrix, bone_list, context);

	if (pEntity->IsPlayer() && pEntity->IsAlive())
	{
		int* pAnimLayersOwner = reinterpret_cast<int*>(*reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(pEntity) + 0x2990) + 0x30);
		for (int i = 13; i; --i) {
			if (reinterpret_cast<CBaseEntity*>(pAnimLayersOwner) != pEntity) {
				*pAnimLayersOwner = reinterpret_cast<uintptr_t>(pEntity);
			}

			pAnimLayersOwner += 14;
		}
	}
}

void FASTCALL H::hkStandardBlendingRules(void* ecx, void* edx, CStudioHdr* pStudioHdr, Vector pos[], Quaternion q[], float currentTime, int boneMask)
{
	static auto oStandardBlendingRules = DTR::StandardBlendingRules.GetOriginal<decltype(&hkStandardBlendingRules)>();

	CBaseEntity* pEntity = reinterpret_cast<CBaseEntity*>(ecx);

	if (pEntity == nullptr || (pEntity->GetIndex() - 1) > 63)
		return oStandardBlendingRules(ecx, edx, pStudioHdr, pos, q, currentTime, boneMask);

	// disable interpolation.
	if (!(pEntity->GetEffects() & EF_NOINTERP))
		pEntity->GetEffects() |= EF_NOINTERP;

	oStandardBlendingRules(ecx, edx, pStudioHdr, pos, q, currentTime, boneMask);

	// restore interpolation.
	pEntity->GetEffects() &= ~EF_NOINTERP;	
}
#pragma endregion

#pragma region hooks_convar
bool FASTCALL H::hksv_cheatsGetBool(CConVar* thisptr, int edx)
{
	static auto osv_cheatsGetBool = DTR::sv_cheatsGetBool.GetOriginal<decltype(&hksv_cheatsGetBool)>();
	static std::uintptr_t uCAM_ThinkReturn = MEM::FindPattern(CLIENT_DLL, XorStr("85 C0 75 30 38 86")); // @xref: "Pitch: %6.1f   Yaw: %6.1f   Dist: %6.1f %16s"
	static CConVar* sv_cheats = I::ConVar->FindVar(XorStr("sv_cheats"));

	if (reinterpret_cast<std::uintptr_t>(_ReturnAddress()) == uCAM_ThinkReturn && C::Get<int>(Vars.iWorldThirdPersonKey) > 0 || thisptr == sv_cheats)
		return true;

	return osv_cheatsGetBool(thisptr, edx);
}

int FASTCALL H::hkr_3dskyGetInt(CConVar* thisptr, int edx)
{
	static auto or_3dskyGetInt = DTR::r_3dskyGetInt.GetOriginal<decltype(&hkr_3dskyGetInt)>();
	static CConVar* r_3dsky = I::ConVar->FindVar(XorStr("r_3dsky"));

	if (thisptr == r_3dsky && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_3DSKY))
		return 0;

	return or_3dskyGetInt(thisptr, edx);
}

bool FASTCALL H::hkcl_csm_enabledGetBool(CConVar* thisptr, int edx)
{
	static auto ocl_csm_enabledGetBool = DTR::cl_csm_enabledGetBool.GetOriginal<decltype(&hkcl_csm_enabledGetBool)>();
	static CConVar* cl_csm_enabled = I::ConVar->FindVar(XorStr("cl_csm_enabled"));

	if (thisptr == cl_csm_enabled && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SHADOWS))
		return 0;

	return ocl_csm_enabledGetBool(thisptr, edx);
}

bool FASTCALL H::hkmat_postprocess_enableGetBool(CConVar* thisptr, int edx)
{
	static auto omat_postprocess_enableGetBool = DTR::mat_postprocess_enableGetBool.GetOriginal<decltype(&hkmat_postprocess_enableGetBool)>();
	static CConVar* mat_postprocess_enable = I::ConVar->FindVar(XorStr("mat_postprocess_enable"));

	if (thisptr == mat_postprocess_enable && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_POSTPROCESSING))
		return 0;

	return omat_postprocess_enableGetBool(thisptr, edx);
}

int FASTCALL H::hkweapon_debug_spread_showGetInt(CConVar* thisptr, int edx)
{
	static auto oweapon_debug_spread_showGetInt = DTR::weapon_debug_spread_showGetInt.GetOriginal<decltype(&hkweapon_debug_spread_showGetInt)>();
	static CConVar* weapon_debug_spread_show = I::ConVar->FindVar(XorStr("weapon_debug_spread_show"));

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();

	if (thisptr == weapon_debug_spread_show && C::Get<bool>(Vars.bScreenSniperCrosshair) && pLocal->IsAlive() && !pLocal->IsScoped())
		return 3;

	return oweapon_debug_spread_showGetInt(thisptr, edx);
}
#pragma endregion

long CALLBACK H::hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// process keys
	IPT::Process(uMsg, wParam, lParam);

	// switch window state on key click
	if (C::Get<int>(Vars.iMenuKey) > 0 && IPT::IsKeyReleased(C::Get<int>(Vars.iMenuKey)))
		W::bMainOpened = !W::bMainOpened;

	// disable game input when menu is opened
	I::InputSystem->EnableInput(!W::bMainOpened);

	/*
	 * @note: we can use imgui input handler to our binds if remove menu state check
	 * with ImGui::IsKeyDown, ImGui::IsKeyPressed, etc functions
	 * but imgui api's keys down durations doesnt have forward compatibility
	 * and i dont want spend a lot of time on recode it
	 */
	if (D::bInitialized && W::bMainOpened && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return 1L;

	// return input controls to the game
	return CallWindowProcW(IPT::pOldWndProc, hWnd, uMsg, wParam, lParam);
}
#pragma endregion

#pragma region proxies_get
bool P::Setup()
{
	// @note: as example
	#if 0
	RecvProp_t* pSmokeEffectTickBegin = CNetvarManager::Get().mapProps[FNV1A::HashConst("CSmokeGrenadeProjectile->m_nSmokeEffectTickBegin")].pRecvProp;
	if (pSmokeEffectTickBegin == nullptr)
		return false;

	RVP::SmokeEffectTickBegin = std::make_shared<CRecvPropHook>(pSmokeEffectTickBegin, P::SmokeEffectTickBegin);
	#endif

	RecvProp_t* pSequence = CNetvarManager::Get().mapProps[FNV1A::HashConst("CBaseViewModel->m_nSequence")].pRecvProp;
	if (pSequence == nullptr)
		return false;

	RVP::Sequence = std::make_shared<CRecvPropHook>(pSequence, P::Sequence);

	return true;
}

void P::Restore()
{
	// @note: as example
	#if 0
	// restore smoke effect
	RVP::SmokeEffectTickBegin->Restore();
	#endif

	RVP::Sequence->Restore();
}
#pragma endregion

#pragma region proxies_handlers
void P::SmokeEffectTickBegin(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	static auto oSmokeEffectTickBegin = RVP::SmokeEffectTickBegin->GetOriginal();

	if (C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SMOKE))
	{
		if (auto pEntity = static_cast<CBaseEntity*>(pStruct); pEntity != nullptr)
			pEntity->GetOrigin() = Vector(MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT);
	}

	oSmokeEffectTickBegin(pData, pStruct, pOut);
}

void P::Sequence(const CRecvProxyData* pData, void* pStruct, void* pOut)
{
	static auto oSequence = RVP::Sequence->GetOriginal();

	CBaseEntity* pLocal = CBaseEntity::GetLocalPlayer();
	if (pLocal == nullptr || !pLocal->IsAlive())
		return oSequence(pData, pStruct, pOut);

	CBaseViewModel* pViewmodel = static_cast<CBaseViewModel*>(pStruct);
	if (pViewmodel == nullptr)
		return oSequence(pData, pStruct, pOut);

	CRecvProxyData* pModifiableData = const_cast<CRecvProxyData*>(pData);
	if (pModifiableData == nullptr)
		return oSequence(pData, pStruct, pOut);

	if (!CSkinChanger::Get().FixSequences(pModifiableData, pViewmodel))
		return oSequence(pData, pStruct, pOut);

	oSequence(pData, pStruct, pOut);
}
#pragma endregion
