#pragma once
// used: winapi, directx, fmt includes
#include "../common.h"
// used: hook setup/destroy
#include "../utilities/detourhook.h"
// used: recvprop hook setup/destroy, recvproxydata
#include "netvar.h"
// used: baseclasses
#include "interfaces.h"

#ifndef FASTCALL
#define FASTCALL __fastcall
#endif

/*
* VTABLE INDEXES
* functions indexes in their virtual tables
*/
namespace VTABLE
{
	// work with namespace cuz if use enum class need additional convert to int
	enum
	{
		/* directx table */
		RESET = 16,
		ENDSCENE = 42,
		RESETEX = 132,

		/* client table */
		LEVELINITPOSTENTITY = 6,
		FRAMESTAGENOTIFY = 37,

		/* panel table */
		PAINTTRAVERSE = 41,

		/* clientmode table */
		OVERRIDEVIEW = 18,
		OVERRIDEMOUSEINPUT = 23,
		CREATEMOVE = 24,
		GETVIEWMODELFOV = 35,
		DOPOSTSCREENEFFECTS = 44,

		/* modelrender table */
		DRAWMODELEXECUTE = 21,

		/* studiorender table */
		DRAWMODEL = 29,

		/* engine table */
		ISCONNECTED = 27,
		ISHLTV = 93,

		/* bsp query table */
		LISTLEAVESINBOX = 6,

		/* prediction table */
		GETLOCALVIEWANGLES = 12,
		RUNCOMMAND = 19,

		/* steamgamecoordinator table */
		SENDMESSAGE = 0,
		RETRIEVEMESSAGE = 2,

		/* sound table */
		EMITSOUND = 5,

		/* materialsystem table */
		OVERRIDECONFIG = 21,

		/* renderview table */
		SCENEEND = 9,

		/* surface table */
		LOCKCURSOR = 67,
		PLAYSOUND = 82,

		/* gameevent table */
		FIREEVENT = 9,

		/* convar table */
		GETBOOL = 13,
		GETINT = 12,

		/* netchannel table */
		SENDNETMSG = 40,
		SENDDATAGRAM = 46,

		/* filesystem table */
		GETUNVERIFIEDFILHEHASHES = 101,
		LOOSEFILEALLOWED = 128,

		/* ccsplayer table */
		DOEXTRABONEPROCESSING = 197,
		STANDARDBLENDINGRULES = 205,
		UPDATECLIENTSIDEANIMATION = 223,
	};
}

/*
 * DETOURS
 * detour hook managers
 */
namespace DTR
{
	inline CDetourHook Reset;
	inline CDetourHook EndScene;
	inline CDetourHook LevelInitPostEntity;
	inline CDetourHook FrameStageNotify;
	inline CDetourHook OverrideView;
	inline CDetourHook CreateMove;
	inline CDetourHook SendNetMsg;
	inline CDetourHook SendDatagram;
	inline CDetourHook GetViewModelFOV;
	inline CDetourHook DoPostScreenEffects;
	inline CDetourHook IsConnected;
	inline CDetourHook IsHLTV;
	inline CDetourHook ListLeavesInBox;
	inline CDetourHook PaintTraverse;
	inline CDetourHook DrawModel;
	inline CDetourHook GetLocalViewAngles;
	inline CDetourHook RunCommand;
	inline CDetourHook SendMessageGC;
	inline CDetourHook RetrieveMessage;
	inline CDetourHook LockCursor;
	inline CDetourHook PlaySoundSurface;
	inline CDetourHook sv_cheatsGetBool;
	inline CDetourHook r_3dskyGetInt;
	inline CDetourHook cl_csm_enabledGetBool;
	inline CDetourHook mat_postprocess_enableGetBool;
	inline CDetourHook weapon_debug_spread_showGetInt;
	inline CDetourHook DoExtraBoneProcessing;
	inline CDetourHook StandardBlendingRules;
}

/*
 * HOOKS
 * swap functions with given pointers
 */
namespace H
{
	// Get
	bool	Setup();
	void	Restore();

	// Handlers
	/* [type][call]		hk[name] (args...) */
	long	D3DAPI		hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	long	D3DAPI		hkEndScene(IDirect3DDevice9* pDevice);
	bool	FASTCALL	hkCreateMove(IClientModeShared* thisptr, int edx, float flInputSampleTime, CUserCmd* pCmd);
	void	FASTCALL	hkPaintTraverse(ISurface* thisptr, int edx, unsigned int uPanel, bool bForceRepaint, bool bForce);
	void	FASTCALL	hkPlaySound(ISurface* thisptr, int edx, const char* szFileName);
	void	FASTCALL	hkLockCursor(ISurface* thisptr, int edx);
	void	FASTCALL	hkLevelInitPostEntity(IBaseClientDll* thisptr, int edx);
	void	FASTCALL	hkFrameStageNotify(IBaseClientDll* thisptr, int edx, EClientFrameStage stage);
	void	FASTCALL	hkDrawModel(IStudioRender* thisptr, int edx, DrawModelResults_t* pResults, const DrawModelInfo_t& info, matrix3x4_t* pBoneToWorld, float* flFlexWeights, float* flFlexDelayedWeights, const Vector& vecModelOrigin, int nFlags);
	int		FASTCALL	hkListLeavesInBox(void* thisptr, int edx, const Vector& vecMins, const Vector& vecMaxs, unsigned short* puList, int nListMax);
	bool	FASTCALL	hkIsConnected(IEngineClient* thisptr, int edx);
	bool	FASTCALL	hkIsHLTV(IEngineClient* thisptr, int edx);
	bool	FASTCALL	hkSendNetMsg(INetChannel* thisptr, int edx, INetMessage* pMessage, bool bForceReliable, bool bVoice);
	int		FASTCALL	hkSendDatagram(INetChannel* thisptr, int edx, bf_write* pDatagram);
	void	FASTCALL	hkOverrideView(IClientModeShared* thisptr, int edx, CViewSetup* pSetup);
	float	FASTCALL	hkGetViewModelFOV(IClientModeShared* thisptr, int edx);
	int		FASTCALL	hkDoPostScreenEffects(IClientModeShared* thisptr, int edx, CViewSetup* pSetup);
	void	FASTCALL	hkGetLocalViewAngles(IPrediction* thisptr, int edx, QAngle& ang);
	void	FASTCALL	hkRunCommand(IPrediction* thisptr, int edx, CBaseEntity* pEntity, CUserCmd* pCmd, IMoveHelper* pMoveHelper);
	int		FASTCALL	hkSendMessage(ISteamGameCoordinator* thisptr, int edx, std::uint32_t uMsgType, const void* pData, std::uint32_t uData);
	int		FASTCALL	hkRetrieveMessage(ISteamGameCoordinator* thisptr, int edx, std::uint32_t* puMsgType, void* pDest, std::uint32_t uDest, std::uint32_t* puMsgSize);
	void	FASTCALL	hkDoExtraBoneProcessing(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, matrix3x4_t* matrix, CBoneBitList& bone_list, CIKContext* context);
	void	FASTCALL	hkStandardBlendingRules(void* ecx, void* edx, CStudioHdr* pStudioHdr, Vector pos[], Quaternion q[], float currentTime, int boneMask);
	bool	FASTCALL	hksv_cheatsGetBool(CConVar* thisptr, int edx);
	int		FASTCALL	hkr_3dskyGetInt(CConVar* thisptr, int edx);
	bool	FASTCALL	hkcl_csm_enabledGetBool(CConVar* thisptr, int edx);
	bool	FASTCALL	hkmat_postprocess_enableGetBool(CConVar* thisptr, int edx);
	int		FASTCALL	hkweapon_debug_spread_showGetInt(CConVar* thisptr, int edx);
	long	CALLBACK	hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

/*
 * RECV VAR PROXY MANAGERS
 * proxy property hook managers
 */
namespace RVP
{
	inline std::shared_ptr<CRecvPropHook> SmokeEffectTickBegin;
	inline std::shared_ptr<CRecvPropHook> Sequence;
}

/*
 * PROXIES
 * networkable property proxy swap functions
 */
namespace P
{
	// Get
	bool	Setup();
	void	Restore();

	// Handlers
	void	SmokeEffectTickBegin(const CRecvProxyData* pData, void* pStruct, void* pOut);
	void	Sequence(const CRecvProxyData* pData, void* pStruct, void* pOut);
}
