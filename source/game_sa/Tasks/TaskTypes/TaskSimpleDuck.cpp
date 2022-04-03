#include "StdInc.h"
#include "TaskSimpleDuck.h"
#include "AnimBlendAssociation.h"

void CTaskSimpleDuck::InjectHooks() {
    RH_ScopedClass(CTaskSimpleDuck);
    RH_ScopedCategory(); // TODO: Change this to the appropriate category!

    RH_ScopedInstall(Constructor, 0x691FC0);
    RH_ScopedInstall(Destructor, 0x692030);

    RH_ScopedGlobalInstall(DeleteDuckAnimCB, 0x692550);
    RH_ScopedGlobalInstall(CanPedDuck, 0x692610);

    // RH_ScopedInstall(IsTaskInUseByOtherTasks, 0x61C3D0);
    // RH_ScopedInstall(AbortBecauseOfOtherDuck, 0x692340);
    // RH_ScopedInstall(RestartTask, 0x692390);
    // RH_ScopedInstall(ControlDuckMove, 0x6923F0);
    // RH_ScopedInstall(SetMoveAnim, 0x6939F0);

    // RH_ScopedInstall(Clone_Reversed, 0x692CF0);
    // RH_ScopedInstall(GetTaskType_Reversed, 0x692020);
    // RH_ScopedInstall(MakeAbortable_Reversed, 0x692100);
    // RH_ScopedInstall(ProcessPed_Reversed, 0x694390);
}

// 0x691FC0
CTaskSimpleDuck::CTaskSimpleDuck(eDuckControlType duckControlType, uint16 lengthOfDuck, int16 shotWhizzingCounter) :
    m_nDuckControlType{duckControlType},
    m_nLengthOfDuck{lengthOfDuck},
    m_nShotWhizzingCounter{shotWhizzingCounter},
    m_nStartTime{CTimer::GetTimeInMS()}
{
    // Rest set in the header!
}

// 0x692030
CTaskSimpleDuck::~CTaskSimpleDuck() {
    if (m_pDuckAnim) {
        m_pDuckAnim->SetFinishCallback(CDefaultAnimCallback::DefaultAnimCB);
        if (m_pDuckAnim->m_fBlendAmount > 0.f && m_pDuckAnim->m_fBlendDelta >= 0.f && (m_pDuckAnim->m_nFlags & ANIM_FLAG_PARTIAL)) {
            m_pDuckAnim->m_fBlendDelta = -8.f;
        }
    }

    if (m_pMoveAnim) {
        m_pMoveAnim->SetFinishCallback(CDefaultAnimCallback::DefaultAnimCB);
        if (m_pMoveAnim->m_fBlendAmount > 0.f && m_pMoveAnim->m_fBlendDelta >= 0.f) {
            m_pMoveAnim->m_fBlendDelta = -8.f;
        }
    }
}

// 0x692550
void CTaskSimpleDuck::DeleteDuckAnimCB(CAnimBlendAssociation* assoc, void* data) {
    const auto self = static_cast<CTaskSimpleDuck*>(data); // aka `this`
    if (!self) {
        return;
    }

    if (!assoc) {
        return;
    }

    switch (assoc->m_nAnimId) {
    case ANIM_ID_WEAPON_CROUCH:
    case ANIM_ID_DUCK_COWER: {
        if (!self->m_pMoveAnim || !self->m_bIsAborting) {
            self->m_bIsFinished = true;
        }
        self->m_pMoveAnim = nullptr; // Moved below `if`
        break;
    }
    case ANIM_ID_CROUCH_ROLL_L:
    case ANIM_ID_CROUCH_ROLL_R: {
        self->m_vecMoveCommand.x = 0.0;
        [[fallthrough]];
    }
    case ANIM_ID_GUNCROUCHFWD:
    case ANIM_ID_GUNCROUCHBWD: {
        if (self->m_bIsAborting) {
            self->m_bIsFinished = 1;
        }
        self->m_pMoveAnim = nullptr; // Moved below `if`
        break;
    }
    }
}

// 0x692610
bool CTaskSimpleDuck::CanPedDuck(CPed* ped) {
    if (ped->IsPlayer()) {
        switch (ped->m_nMoveState) {
        case PEDMOVE_RUN:
        case PEDMOVE_SPRINT:
            return false;
        }
    }

    const auto& activeWep = ped->GetActiveWeapon();
    const auto& pedActiveWepInf = activeWep.GetWeaponInfo(ped);

    switch (pedActiveWepInf.m_nWeaponFire) {
    case eWeaponFire::WEAPON_FIRE_MELEE:
    case eWeaponFire::WEAPON_FIRE_USE:
        return true;
    }

    if (activeWep.m_nType == WEAPON_SPRAYCAN) {
        return true;
    }

    if (pedActiveWepInf.flags.bCrouchFire) {
        return true;
    }

    return false;
}

// 0x61C3D0
BOOL CTaskSimpleDuck::IsTaskInUseByOtherTasks() {
    return plugin::CallMethodAndReturn<BOOL, 0x61C3D0, CTaskSimpleDuck*>(this);
}

// 0x692340
int8_t CTaskSimpleDuck::AbortBecauseOfOtherDuck(CPed* ped) {
    return plugin::CallMethodAndReturn<int8_t, 0x692340, CTaskSimpleDuck*, CPed*>(this, ped);
}

// 0x692390
int32 CTaskSimpleDuck::RestartTask(CPed* ped) {
    return plugin::CallMethodAndReturn<int32, 0x692390, CTaskSimpleDuck*, CPed*>(this, ped);
}

// 0x6923F0
void CTaskSimpleDuck::ControlDuckMove(CVector2D moveSpeed) {
    plugin::CallMethod<0x6923F0, CTaskSimpleDuck*, CVector2D>(this, moveSpeed);
}

// 0x6939F0
void CTaskSimpleDuck::SetMoveAnim(CPed* ped) {
    plugin::CallMethod<0x6939F0, CTaskSimpleDuck*, CPed*>(this, ped);
}

// 0x692CF0
CTask* CTaskSimpleDuck::Clone() {
    return plugin::CallMethodAndReturn<CTask*, 0x692CF0, CTaskSimpleDuck*>(this);
}

// 0x692100
bool CTaskSimpleDuck::MakeAbortable(CPed* ped, eAbortPriority priority, CEvent const* event) {
    return plugin::CallMethodAndReturn<bool, 0x692100, CTaskSimpleDuck*, CPed*, eAbortPriority, CEvent const*>(this, ped, priority, event);
}

// 0x694390
bool CTaskSimpleDuck::ProcessPed(CPed* ped) {
    return plugin::CallMethodAndReturn<bool, 0x694390, CTaskSimpleDuck*, CPed*>(this, ped);
}
