/*
Plugin-SDK (Grand Theft Auto San Andreas) header file
Authors: GTA Community. See more here
https://github.com/DK22Pac/plugin-sdk
Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "PluginBase.h"
#include <array>

class PLUGIN_API CTimer
{
public:
    using UpdateTimeHistory_t = std::array<unsigned int, 4>;

    typedef std::uint64_t(__cdecl* TimerFunction_t)();
    static TimerFunction_t& ms_fnTimerFunction;

    // class variables
    static bool& ms_bEnableTimeDebug;
    static bool& ms_bSkipProcessThisFrame;
    static bool& ms_bSlowMotionActive;
    static float& ms_fGameFPS;

    static bool& m_CodePause;
    static bool& m_UserPause;
    static unsigned int& m_FrameCounter;
    static unsigned int& ms_nRenderTimerPauseCount;
    static float& ms_fTimeStepNonClipped;
    static float& ms_fTimeStep;
    static unsigned int& ms_nTimerDivider;

    static float& ms_fOldTimeStep;
    static float& ms_fSlowMotionScale;

    // game speed
    static float& ms_fTimeScale;
    static unsigned int& m_snTimeInMillisecondsPauseMode;
    static unsigned int& m_snTimeInMillisecondsNonClipped;
    static unsigned int& ms_nPreviousTimeInMillisecondsNonClipped;
    static unsigned int& m_snTimeInMilliseconds;
    static std::uint64_t& m_snRenderStartTime;

    // Array of update time history with the last item being from before the most recent frame
    static UpdateTimeHistory_t& m_UpdateTimeMsHistory;

    static void InjectHooks();

    // class functions

    static void Initialise();
    static void Shutdown();
    static void UpdateVariables(float timeStep);
    static void Suspend();
    static void Resume();
    static int GetCyclesPerMillisecond();
    // cycles per ms * 20
    static int GetCyclesPerFrame();
    static unsigned int GetCurrentTimeInCycles();
    static void Stop();
    static bool GetIsSlowMotionActive();
    static void StartUserPause();
    static void EndUserPause();
    static bool GetIsPaused() { return m_UserPause || m_CodePause; }
    static void Update();
    static void UpdateTimeStep(float fTimeStep);
};
