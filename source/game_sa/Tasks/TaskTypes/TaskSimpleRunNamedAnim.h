/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "TaskSimpleAnim.h"
#include "AnimBlendHierarchy.h"
#include "TaskTimer.h"
#include "Vector.h"

class CTaskSimpleRunNamedAnim : public CTaskSimpleAnim {
public:
    char                 m_animName[24];
    char                 m_animGroupName[16];
    float                m_blendDelta;
    CAnimBlendHierarchy* m_pAnimHierarchy;
    uint32               m_endTime;
    CTaskTimer           m_timer;
    CVector              m_vecOffsetAtEnd;
    uint32               m_animFlags;
    int16                m_nAnimId;

public:
    static constexpr auto Type = TASK_SIMPLE_NAMED_ANIM;

    friend void InjectHooksMain();
    static void InjectHooks();

    CTaskSimpleRunNamedAnim();
    CTaskSimpleRunNamedAnim(const char* pAnimName, const char* pAnimGroupName, uint32 animFlags, float blendDelta,
        uint32 endTime, bool bDontInterrupt, bool bRunInSequence, bool bOffsetPed, bool bHoldLastFrame);

    bool ProcessPed(CPed* ped) override;
    eTaskType GetTaskType() override { return Type; }

private: // Wrappers for hooks
    CTaskSimpleRunNamedAnim* Constructor(char const* pAnimName, char const* pAnimGroupName, int32 animFlags, float blendDelta, int32 endTime, bool bDontInterrupt, bool bRunInSequence, bool bOffsetPed, bool bHoldLastFrame) { this->CTaskSimpleRunNamedAnim::CTaskSimpleRunNamedAnim(pAnimName, pAnimGroupName, animFlags, blendDelta, endTime, bDontInterrupt, bRunInSequence, bOffsetPed, bHoldLastFrame); return this; }
    CTaskSimpleRunNamedAnim* Constructor() { this->CTaskSimpleRunNamedAnim::CTaskSimpleRunNamedAnim(); return this; }
    CTaskSimpleRunNamedAnim* Destructor() { this->CTaskSimpleRunNamedAnim::~CTaskSimpleRunNamedAnim(); return this; }
    CTask* Clone_Reversed() { return CTaskSimpleRunNamedAnim::Clone(); }
    bool ProcessPed_Reversed(CPed* ped) { return CTaskSimpleRunNamedAnim::ProcessPed(ped); }
};
VALIDATE_SIZE(CTaskSimpleRunNamedAnim, 0x64);
