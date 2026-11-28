#pragma once

#include "TaskComplex.h"
#include "Vector.h"

class CPedGroup;

class NOTSA_EXPORT_VTABLE CTaskComplexGangFollower : public CTaskComplex {
public:
    static constexpr bool ms_bUseClimbing = true; // 0x8D2EDC
    static constexpr auto Type            = eTaskType::TASK_COMPLEX_GANG_FOLLOWER;


    CTaskComplexGangFollower(CPedGroup* pedGroup, CPed* ped, uint8 a4, CVector pos, float a6);
    CTaskComplexGangFollower(const CTaskComplexGangFollower&);
    ~CTaskComplexGangFollower() override;

    CTask*    Clone() const override { return new CTaskComplexGangFollower{ *this }; }
    eTaskType GetTaskType() const override { return Type; }
    bool      MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) override;
    CTask*    CreateNextSubTask(CPed* ped) override;
    CTask*    CreateFirstSubTask(CPed* ped) override;
    CTask*    ControlSubTask(CPed* ped) override;

    CVector CalculateOffsetPosition();
    void    SetFollowLeader(bool v) { m_FollowLeader = v; }

    CPed* GetLeader() const { return m_Leader; }

public:
    static void InjectHooks();

private:
    CPedGroup* m_PedGroup{};
    CPed*      m_Leader{};
    CVector    m_LeaderInitialPos{};
    CVector    m_OffsetPos{};
    CVector    m_InitialOffsetPos{};
    float      m_TargetRadius{};
    uint8      m_GrpMemIdx{};
    bool       m_AnimsRef : 1 { false };
    bool       m_LeaveGroup : 1 { false };
    bool       m_FollowLeader : 1 { true };
    bool       m_IsInPlayersGroup : 1 { m_Leader == FindPlayerPed() };
    bool       m_IsUsingStandingStillOffsets : 1 { true };
    CTaskTimer m_ExhaleTimer{};
};
VALIDATE_SIZE(CTaskComplexGangFollower, 0x4C);
