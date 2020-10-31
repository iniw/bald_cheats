#pragma once
// used: std::deque
#include <deque>
#include <array>

// used: winapi definitions
#include "../common.h"
// used: usercmd
#include "../sdk/datatypes/usercmd.h"
// used: netchannel
#include "../sdk/interfaces/inetchannel.h"
// used: ConVar interface
#include "../core/interfaces.h"

#pragma region lagcompensation_definitions
#define LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR ( 64.0f * 64.0f )
#define LAG_COMPENSATION_EPS_SQR ( 0.1f * 0.1f )
#define LAG_COMPENSATION_ERROR_EPS_SQR ( 4.0f * 4.0f )
#pragma endregion

#pragma region lagcompensation_enumerations
enum ELagCompensationState
{
	LC_NO =					0,
	LC_ALIVE =				(1 << 0),
	LC_ORIGIN_CHANGED =		(1 << 8),
	LC_ANGLES_CHANGED =		(1 << 9),
	LC_SIZE_CHANGED =		(1 << 10),
	LC_ANIMATION_CHANGED =	(1 << 11)
};
#pragma endregion

struct SequenceObject_t
{
	SequenceObject_t(int iInReliableState, int iOutReliableState, int iSequenceNr, float flCurrentTime)
		: iInReliableState(iInReliableState), iOutReliableState(iOutReliableState), iSequenceNr(iSequenceNr), flCurrentTime(flCurrentTime) { }

	int iInReliableState;
	int iOutReliableState;
	int iSequenceNr;
	float flCurrentTime;
};

// @note: FYI - https://www.unknowncheats.me/forum/counterstrike-global-offensive/280912-road-perfect-aimbot-1-interpolation.html

class CLagCompensation : public CSingleton<CLagCompensation>
{
public:
	// Get
	void Run(CUserCmd* pCmd);

	// Main
	void UpdateIncomingSequences(INetChannel* pNetChannel);
	void ClearIncomingSequences();
	void AddLatencyToNetChannel(INetChannel* pNetChannel, float flLatency);

private:
	// Values
	std::deque<SequenceObject_t> vecSequences = { };
	/* our real incoming sequences count */
	int nRealIncomingSequence = 0;
	/* count of incoming sequences what we can spike */
	int nLastIncomingSequence = 0;
};

struct Record_t
{
	float flSimtime;
	Vector vecHitboxPos;
	Vector vecHeadPos;
	Vector vecOrigin;
	Vector vecAbsOrigin;
	Vector vecMins;
	Vector vecMaxs;
	Vector vecViewOffset;
	//QAngle angAbsAngles;
	std::array<matrix3x4_t, MAXSTUDIOBONES> arrMatrix;
};

struct OriginalData_t
{
	float flSimtime;
	Vector vecOrigin;
	Vector vecAbsOrigin;
	Vector vecMins;
	Vector vecMaxs;
	Vector vecViewOffset;
	//QAngle angAbsAngles;
};

class CBacktracking : public CSingleton<CBacktracking>
{
public:
	void Run(CUserCmd* pCmd, CBaseEntity* pLocal);
	void Update(CBaseEntity* pLocal);
	std::deque<Record_t> GetPlayerRecord(int iIndex);
	void ApplyData(Record_t record, CBaseEntity* pEntity);
	void RestoreData(CBaseEntity* pEntity);
	float GetLerp();
	bool IsValid(float flSimtime, CBaseEntity* pLocal);
	std::array<std::deque<Record_t>, 65> m_arrRecords;

private:
	bool IsValid(CBaseEntity* pLocal, CBaseEntity* pEntity);
	CBaseEntity* GetBestEntity(CBaseEntity* pLocal);
	Vector GetBestHitbox(CBaseEntity* pLocal, CBaseEntity* pEntity, std::array<matrix3x4_t, MAXSTUDIOBONES> arrCustomMatrix);

	OriginalData_t m_orgData;
	CConVar* m_cl_updaterate;
	CConVar* m_sv_minupdaterate;
	CConVar* m_sv_maxupdaterate;
	CConVar* m_cl_interp;
	CConVar* m_cl_interp_ratio;
	CConVar* m_sv_client_min_interp_ratio;
	CConVar* m_sv_client_max_interp_ratio;
	CConVar* m_sv_maxunlag;
};