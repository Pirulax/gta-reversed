#pragma once

#include "TaskComplex.h"
#include "TaskTimer.h"
#include "Vector.h"

class CColSphere;

class NOTSA_EXPORT_VTABLE CTaskComplexAvoidOtherPedWhileWandering : public CTaskComplex {
    using PedsToAvoidArray = std::array<CPed*, 16>;
public:
    CPed*      m_PedToAvoid{};
    CVector    m_StartPt{};
    CVector    m_TargetPt{};
    CVector    m_DetourTargetPt{};
    CVector    m_NewTargetPt{};
    CTaskTimer m_Timer{};
    CTaskTimer m_DontQuitYetTimer{};
    eMoveState m_MoveState{};
    bool       m_bDoingIK : 1{};
    bool       m_bWantsToQuit : 1{};
    bool       m_bMovingTarget : 1{};

public:
    static constexpr auto Type = TASK_COMPLEX_AVOID_OTHER_PED_WHILE_WANDERING;

    CTaskComplexAvoidOtherPedWhileWandering(CPed* ped, const CVector& targetPoint, eMoveState moveState, /*NOTSA=>*/ bool bMovingTarget = false);
    CTaskComplexAvoidOtherPedWhileWandering(const CTaskComplexAvoidOtherPedWhileWandering&);
    ~CTaskComplexAvoidOtherPedWhileWandering() override;

    CTask*    Clone() override { return new CTaskComplexAvoidOtherPedWhileWandering{ *this }; }
    eTaskType GetTaskType() override { return Type; }
    bool      MakeAbortable(CPed* ped, eAbortPriority priority, CEvent const* event) override;
    CTask*    CreateNextSubTask(CPed* ped) override;
    CTask*    CreateFirstSubTask(CPed* ped) override;
    CTask*    ControlSubTask(CPed* ped) override;

    void QuitIK(CPed* ped);
    void SetUpIK(CPed* ped);

    bool       NearbyPedsInSphere(CPed* ped, const CColSphere& colSphere, PedsToAvoidArray& pedsToCheck, PedsToAvoidArray& pedsInSphere);
    CColSphere ComputeSphere(PedsToAvoidArray& accountedPeds);
    void       ComputeAvoidSphere(CPed* ped, CColSphere& colSphere);
    bool       ComputeRouteRoundSphere(CPed* ped, CColSphere& spToAvoid);
    bool       ComputeDetourTarget(CPed* ped);

private:
    CTaskComplexAvoidOtherPedWhileWandering* Constructor(CPed* ped, const CVector& targetPoint, eMoveState moveState) {
        this->CTaskComplexAvoidOtherPedWhileWandering::CTaskComplexAvoidOtherPedWhileWandering(ped, targetPoint, moveState);
        return this;
    }

private:
    friend void InjectHooksMain();
    static void InjectHooks();

    // 0x66A100
    auto Constructor(CPed* ped, const CVector& targetPoint, eMoveState moveState) {
        this->CTaskComplexAvoidOtherPedWhileWandering::CTaskComplexAvoidOtherPedWhileWandering(ped, targetPoint, moveState);
        return this;
    }

    // 0x66A1D0
    auto Destructor() {
        this->CTaskComplexAvoidOtherPedWhileWandering::~CTaskComplexAvoidOtherPedWhileWandering();
        return this;
    }

};

VALIDATE_SIZE(CTaskComplexAvoidOtherPedWhileWandering, 0x60);
