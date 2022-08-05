#pragma once

#include "Vector.h"

class CRestart {
public:
    static bool& bOverrideRespawnBasePointForMission;
    static CVector& OverrideRespawnBasePointForMission;
    static float& OverrideHeading;
    static bool& bOverrideRestart;
    static CVector& OverridePosition;

    static constexpr uint32 MAX_RESTART_POINTS{ 10u };

    static inline bool& bFadeInAfterNextArrest = *(bool*)0xA4325C;
    static inline bool& bFadeInAfterNextDeath = *(bool*)0xA4325D;

    static int16& NumberOfPoliceRestarts;
    static inline int32(&PoliceRestartWhenToUse)[MAX_RESTART_POINTS] = *(int32(*)[MAX_RESTART_POINTS])0xA43270;
    static inline float(&PoliceRestartHeadings)[MAX_RESTART_POINTS] = *(float(*)[MAX_RESTART_POINTS])0xA43298;
    static inline CVector(&PoliceRestartPoints)[MAX_RESTART_POINTS] = *(CVector(*)[MAX_RESTART_POINTS])0xA43390;

    static int16& NumberOfHospitalRestarts;
    static inline int32(&HospitalRestartWhenToUse)[MAX_RESTART_POINTS] = *(int32(*)[MAX_RESTART_POINTS])0xA432C0;
    static inline float(&HospitalRestartHeadings)[MAX_RESTART_POINTS] = *(float(*)[MAX_RESTART_POINTS])0xA432E8;
    static inline CVector(&HospitalRestartPoints)[MAX_RESTART_POINTS] = *(CVector(*)[MAX_RESTART_POINTS])0xA43318;

    // Script command 2271 (COMMAND_SET_EXTRA_HOSPITAL_RESTART_POINT) arguments
    static inline float& ScriptExtraHospitalRestartPoint_Angle = *(float*)0xA43254;
    static inline float& ScriptExtraHospitalRestartPoint_Radius = *(float*)0xA43258;
    static inline CVector& ScriptExtraHospitalRestartPoint_Pos = *(CVector*)0xA43414;

    // Script command 2272 (COMMAND_SET_EXTRA_POLICE_RESTART_POINT) arguments
    static inline float& ScriptExtraPoliceRestartPoint_Radius = *(float*)0xA43250;
    static inline float& ScriptExtraPoliceRestartPoint_Angle = *(float*)0xA4324C;
    static inline CVector& ScriptExtraPoliceRestartPoint_Pos = *(CVector*)0xA43420;

public:
    static void InjectHooks();

    static void Initialise();
    static void AddHospitalRestartPoint(const CVector& point, float angle, int32 townId);
    static void AddPoliceRestartPoint(const CVector& point, float angle, int32 townId);
    static void OverrideNextRestart(CVector const& point, float angle);
    static void CancelOverrideRestart();
    static void SetRespawnPointForDurationOfMission(CVector point);
    static void ClearRespawnPointForDurationOfMission();
    static void FindClosestHospitalRestartPoint(CVector point, CVector& outPos, float& outAngle);
    static void FindClosestPoliceRestartPoint(CVector point, CVector& storedPoint, float& outAngle);
    static void Load();
    static void Save();
};
