/*
Plugin-SDK (Grand Theft Auto San Andreas) source file
Authors: GTA Community. See more here
https://github.com/DK22Pac/plugin-sdk
Do not delete this comment block. Respect others' work!
*/
#include <profileapi.h> // QueryPerformanceFrequency

#include "StdInc.h"
#include <ReversibleHooks.h>

CTimer::TimerFunction_t& CTimer::ms_fnTimerFunction = *(TimerFunction_t*)0xB7CB28;

unsigned int& CTimer::renderTimerPauseCount = *(unsigned int*)0xB7CB44;
bool& CTimer::m_sbEnableTimeDebug = *(bool*)0xB7CB40;
bool& CTimer::bSkipProcessThisFrame = *(bool*)0xB7CB89;
bool& CTimer::bSlowMotionActive = *(bool*)0xB7CB88;
float& CTimer::game_FPS = *(float*)0xB7CB50;

bool& CTimer::m_CodePause = *(bool*)0xB7CB48;
bool& CTimer::m_UserPause = *(bool*)0xB7CB49;

unsigned int& CTimer::m_FrameCounter = *(unsigned int*)0xB7CB4C;
unsigned int& CTimer::m_snTimerDivider = *(unsigned int*)0xB7CB2C;

float& CTimer::ms_fTimeStepNonClipped = *(float*)0xB7CB58;
float& CTimer::ms_fTimeStep = *(float*)0xB7CB5C;
float& CTimer::ms_fTimeScale = *(float*)0xB7CB64;

unsigned int& CTimer::m_snTimeInMillisecondsPauseMode = *(unsigned int*)0xB7CB7C;
unsigned int& CTimer::m_snTimeInMillisecondsNonClipped = *(unsigned int*)0xB7CB80;
unsigned int& CTimer::m_snTimeInMilliseconds = *(unsigned int*)0xB7CB84;
unsigned int& CTimer::ms_nPreviousTimeInMillisecondsNonClipped = *(unsigned int*)0xB7CB68;
std::uint64_t& CTimer::m_snRenderStartTime = *(std::uint64_t*)0xB7CB38;

unsigned int& CTimer::m_snPPPPreviousTimeInMilliseconds = *(unsigned int*)0xB7CB6C;
unsigned int& CTimer::m_snPPPreviousTimeInMilliseconds = *(unsigned int*)0xB7CB70;
unsigned int& CTimer::m_snPPreviousTimeInMilliseconds = *(unsigned int*)0xB7CB74;
unsigned int& CTimer::m_snPreviousTimeInMilliseconds = *(unsigned int*)0xB7CB78;

float& CTimer::ms_fOldTimeStep = *(float*)0xB7CB54;
float& CTimer::ms_fSlowMotionScale = *(float*)0xB7CB60;

void CTimer::InjectHooks()
{
    ReversibleHooks::Install("CTimer", "Initialise", 0x5617E0, &Initialise);
    ReversibleHooks::Install("CTimer", "Shutdown", 0x5618C0, &Shutdown);
    ReversibleHooks::Install("CTimer", "UpdateVariables", 0x5618D0, &UpdateVariables);
    ReversibleHooks::Install("CTimer", "Suspend", 0x5619D0, &Suspend);
    ReversibleHooks::Install("CTimer", "Resume", 0x561A00, &Resume);
    ReversibleHooks::Install("CTimer", "GetCyclesPerMillisecond", 0x561A40, &GetCyclesPerMillisecond);
    ReversibleHooks::Install("CTimer", "GetCyclesPerFrame", 0x561A50, &GetCyclesPerFrame);
    ReversibleHooks::Install("CTimer", "GetCurrentTimeInCycles", 0x561A80, &GetCurrentTimeInCycles);
    ReversibleHooks::Install("CTimer", "Stop", 0x561AA0, &Stop);
    ReversibleHooks::Install("CTimer", "GetIsSlowMotionActive", 0x561AD0, &GetIsSlowMotionActive);
    ReversibleHooks::Install("CTimer", "StartUserPause", 0x561AF0, &StartUserPause);
    ReversibleHooks::Install("CTimer", "EndUserPause", 0x561B00, &EndUserPause);
    ReversibleHooks::Install("CTimer", "Update", 0x561B10, &Update);
}

/*
* TODO: Have to include <timeapi.h> and link winmm.lib (in the premake presumeably)
* See: https://stackoverflow.com/questions/26762013
uint64_t __cdecl gtaTimerFunction()
{
    DWORD time; 
    TIMECAPS ptc;
    timeGetDevCaps(&ptc, 8u);
    timeBeginPeriod(ptc.wPeriodMin);
    time = timeGetTime();
    timeEndPeriod(ptc.wPeriodMin);
    return time;
}*/

uint64_t __cdecl getPerformaceCounter()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (uint64_t)counter.QuadPart;
}

// 0x5617E0
void CTimer::Initialise()
{
    m_UserPause = false;
    m_CodePause = false;
    bSlowMotionActive = false;
    bSkipProcessThisFrame = false;
    m_snPPPPreviousTimeInMilliseconds = 0;
    m_snPPPreviousTimeInMilliseconds = 0;
    m_snPPreviousTimeInMilliseconds = 0;
    m_snPreviousTimeInMilliseconds = 0;
    ms_nPreviousTimeInMillisecondsNonClipped = 0;
    m_snTimeInMilliseconds = 0;
    m_FrameCounter = 0;
    renderTimerPauseCount = 0;
    m_sbEnableTimeDebug = false;
    game_FPS = 0.0;
    m_snTimeInMillisecondsNonClipped = 1;
    m_snTimeInMillisecondsPauseMode = 1;
    ms_fTimeScale = 1.0;
    ms_fTimeStep = 1.0;
    ms_fOldTimeStep = 1.0;
    ms_fSlowMotionScale = -1.0;

    LARGE_INTEGER freq = GetOSWPerformanceFrequency();
    if (QueryPerformanceFrequency(&freq))
    {
        ms_fnTimerFunction = &getPerformaceCounter;
        m_snTimerDivider = freq.u.LowPart / 1000;
    }
    else
    {
        //ms_fnTimerFunction = &gtaTimerFunction;
        ms_fnTimerFunction = (TimerFunction_t)0x56188F;
        m_snTimerDivider = 1;
    }
    m_snRenderStartTime = ms_fnTimerFunction();
}

// 0x5618C0
void CTimer::Shutdown()
{
    m_sbEnableTimeDebug = false;
}

// 0x5618D0
void CTimer::UpdateVariables(float timeStep)
{
    const float realStep = (float)timeStep / (float)m_snTimerDivider;
    m_snTimeInMillisecondsNonClipped += (unsigned int)realStep;
    ms_fTimeStepNonClipped = 0.05f * realStep;

    const auto timeToAdd = (unsigned int)std::min<float>(realStep, 300.0f); // Clamp to max 300
    m_snTimeInMilliseconds += timeToAdd;

    if (ms_fTimeStepNonClipped < 0.01f && !m_UserPause && !m_CodePause && !CSpecialFX::bSnapShotActive) {
        ms_fTimeStepNonClipped = 0.01f;
    }

    ms_fOldTimeStep = ms_fTimeStep;
    ms_fTimeStep = clamp<float>(ms_fTimeStepNonClipped, 0.00001f, 3.0f);
}

// 0x5619D0
void CTimer::Suspend()
{
    if (!m_sbEnableTimeDebug) {
        return;
    }

    if (m_snRenderTimerPauseCount == 0 || m_snRenderTimerPauseCount >= 0xFFFFFFFF) {
        m_snRenderPauseTime = ms_fnTimerFunction();
    }
    m_snRenderTimerPauseCount++;
}

// 0x561A00
bool CTimer::Resume()
{
    if (m_sbEnableTimeDebug) {
        if (!--renderTimerPauseCount) {
            m_snRenderStartTime = ms_fnTimerFunction() - renderTimerPauseTime + m_snRenderStartTime;
            return (bool)m_snRenderStartTime;
        }
    }
    return m_sbEnableTimeDebug;
}

// 0x561A40
int CTimer::GetCyclesPerMillisecond()
{
    return m_snTimerDivider;
}

// 0x561A50
int CTimer::GetCyclesPerFrame()
{
    return GetCyclesPerMillisecond() * 20;
}

// 0x561A80
unsigned int CTimer::GetCurrentTimeInCycles()
{
    LARGE_INTEGER PerformanceCount;
    QueryPerformanceCounter(&PerformanceCount);
    return (unsigned int)(PerformanceCount.QuadPart - m_snRenderStartTime);
}

// 0x561AA0
void CTimer::Stop()
{
    m_snPPPPreviousTimeInMilliseconds = m_snTimeInMilliseconds;
    m_snPPPreviousTimeInMilliseconds = m_snTimeInMilliseconds;
    m_snPPreviousTimeInMilliseconds = m_snTimeInMilliseconds;
    m_snPreviousTimeInMilliseconds = m_snTimeInMilliseconds;
    m_sbEnableTimeDebug = 0;
    ms_nPreviousTimeInMillisecondsNonClipped = m_snTimeInMillisecondsNonClipped;
}

// 0x561AD0
bool CTimer::GetIsSlowMotionActive()
{
    return ms_fTimeScale < 1.0;
}

// 0x561AF0
void CTimer::StartUserPause()
{
    m_UserPause = true;
}

// 0x561B00
void CTimer::EndUserPause()
{
    m_UserPause = false;
}

// 0x561B10
void CTimer::Update()
{
#ifdef USE_DEFAULT_FUNCTIONS
    ((void(__cdecl *)()) 0x561B10)();
#else
    if (!ms_fnTimerFunction)
        return;
    m_sbEnableTimeDebug = true;
    game_FPS = float(1000.0f / (m_snTimeInMillisecondsNonClipped - ms_nPreviousTimeInMillisecondsNonClipped));

    // Update history
    m_snPPPPreviousTimeInMilliseconds = m_snPPPreviousTimeInMilliseconds;
    m_snPPPreviousTimeInMilliseconds = m_snPPreviousTimeInMilliseconds;
    m_snPPreviousTimeInMilliseconds = m_snPreviousTimeInMilliseconds;
    m_snPreviousTimeInMilliseconds = m_snTimeInMilliseconds;
    ms_nPreviousTimeInMillisecondsNonClipped = m_snTimeInMillisecondsNonClipped;

    const uint64_t nRenderTimeBefore = m_snRenderStartTime;
    m_snRenderStartTime = ms_fnTimerFunction();
    auto fTimeDelta = float(m_snRenderStartTime - nRenderTimeBefore);
    if (!GetIsPaused())
        fTimeDelta *= ms_fTimeScale;
    m_snTimeInMillisecondsPauseMode += unsigned int(fTimeDelta / m_snTimerDivider);
    if (GetIsPaused())
        fTimeDelta = 0.0f;
    UpdateVariables(fTimeDelta);
    m_FrameCounter++;
#endif
}

void CTimer::UpdateTimeStep(float fTimeStep)
{
    ms_fTimeStep = std::max(fTimeStep, 0.00001f);
}
