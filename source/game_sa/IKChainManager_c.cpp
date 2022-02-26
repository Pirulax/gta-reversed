#include "StdInc.h"

#include "IKChainManager_c.h"
#include "IKChain_c.h"
#include "BoneNodeManager_c.h"
#include "TaskSimpleIKManager.h"
#include "TaskSimpleIKLookAt.h"

IKChainManager_c& g_ikChainMan = *(IKChainManager_c*)0xC15448;

void IKChainManager_c::InjectHooks() {
    RH_ScopedClass(IKChainManager_c);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Init, 0x6180A0);
    RH_ScopedInstall(Exit, 0x6180D0);
    RH_ScopedInstall(Reset, 0x618140);
    RH_ScopedInstall(Update, 0x6186D0);
    RH_ScopedInstall(AddIKChain, 0x618750);
    RH_ScopedInstall(RemoveIKChain, 0x618170);
    RH_ScopedInstall(CanAccept, 0x618800);
    RH_ScopedInstall(IsLooking, 0x6181A0);
    RH_ScopedInstall(GetLookAtEntity, 0x6181D0);
    RH_ScopedInstall(GetLookAtOffset, 0x618210);
    RH_ScopedInstall(AbortLookAt, 0x618280);
    RH_ScopedInstall(CanAcceptLookAt, 0x6188B0);
    RH_ScopedInstall(LookAt, 0x618970);
    RH_ScopedInstall(IsArmPointing, 0x6182B0);
    RH_ScopedInstall(AbortPointArm, 0x6182F0);
    // RH_ScopedInstall(IsFacingTarget, 0x618330);
    // RH_ScopedInstall(PointArm, 0x618B60);
}

// 0x6180A0
bool IKChainManager_c::Init() {
    for (auto&& v : m_chains) {
        m_freeList.AddItem(&v);
    }
    return true;
}

// 0x6180D0
void IKChainManager_c::Exit() {
    for (auto it = m_activeList.GetTail(); it; it = m_activeList.GetPrev(it)) {
        static_cast<IKChain_c*>(it)->Exit();
    }
    m_activeList.RemoveAll();
    m_freeList.RemoveAll();
}

// 0x618140
void IKChainManager_c::Reset() {
    Exit();
    Init();
}

// 0x6186D0
void IKChainManager_c::Update(float timeStep) {
    UNUSED(timeStep);

    for (auto i = 0u; i < 4u; i++) {
        CWorld::IncrementCurrentScanCode();

        for (auto it = m_activeList.GetHead(); it; it = it = m_activeList.GetNext(it)) {
            auto& v = *(IKChain_c*)it;
            if (v.m_indexInList == i) {
                // Update RpHAnim of ped (if any) - TODO: Maybe move this code into `IKChain_c::Update` as well..
                if (v.m_ped) {
                    if (!v.m_ped->IsScanCodeCurrent()) {
                        v.m_ped->UpdateRpHAnim();
                        v.m_ped->SetCurrentScanCode();
                    }
                }

                // Now update the IKChain itself
                v.Update(timeStep);
            }
        }
    }
}

/*!
* @addr 0x618750
* @brief Tries initing a new chain from the free list.
* @returns A new `IKChain_c` object, unless there are no more free chains or it's init failed.
*/
IKChain_c* IKChainManager_c::AddIKChain(const char* name, int32 IndexInList, CPed* ped, int32 animId, RwV3d bonePosn, int32 animId_1, CEntity* entity, int32 offsetBoneTag,
    RwV3d posn, float a11, int32 priority) {
    if (auto chain = m_freeList.RemoveHead()) {
        if (chain->Init(name, IndexInList, ped, animId, bonePosn, animId_1, entity, offsetBoneTag, posn, a11, priority)) {
            m_activeList.AddItem(chain);
            return chain;
        }
        m_freeList.AddItem(chain); // Failed, add back to free list
    }
    return nullptr; // No more free chains
}

// 0x618170
void IKChainManager_c::RemoveIKChain(IKChain_c* chain) {
    chain->Exit();
    m_activeList.RemoveItem(chain);
    m_freeList.AddItem(chain);
}

// 0x618800
bool IKChainManager_c::CanAccept(CPed* ped, float dist) {
    if (!ped->GetModellingMatrix() || !ped->IsAlive() || !ped->GetIsOnScreen()) {
        return false;
    }

    return TheCamera.m_PlayerWeaponMode.m_nMode == MODE_SNIPER
        || dist >= 999.f
        || dist*dist >= (TheCamera.GetPosition() - ped->GetPosition()).SquaredMagnitude();
}

CTaskSimpleIKManager* GetPedIKManagerTask(CPed* ped) {
    return static_cast<CTaskSimpleIKManager*>(ped->GetTaskManager().GetTaskSecondary(TASK_SECONDARY_IK));
}

CTaskSimpleIKLookAt* GetPedIKLookAtTask(CPed* ped) {
    if (const auto mgr = GetPedIKManagerTask(ped)) {
        return static_cast<CTaskSimpleIKLookAt*>(mgr->GetTaskAtSlot(0));
    }
    return nullptr;
}

// 0x6181A0
bool IKChainManager_c::IsLooking(CPed* ped) {
    return !!GetPedIKLookAtTask(ped);
}

// 0x6181D0
CEntity* IKChainManager_c::GetLookAtEntity(CPed* ped) {
    if (const auto task = GetPedIKLookAtTask(ped)) {
        return task->GetLookAtEntity();
    }
    return nullptr;
}

// 0x618210
CVector IKChainManager_c::GetLookAtOffset(CPed* ped) { // TODO: It's possible this is incorrect, originally it took the vector as an arg (although that's probably a compiler optimization)
    if (const auto task = GetPedIKLookAtTask(ped)) {
        return task->GetLookAtOffset();
    }
    return {};
}

// 0x618280
void IKChainManager_c::AbortLookAt(CPed* ped, uint32 blendOutTime) {
    if (const auto task = GetPedIKLookAtTask(ped)) {
        task->BlendOut(blendOutTime);
    }
}

// 0x6188B0
bool IKChainManager_c::CanAcceptLookAt(CPed* ped) {
    if (!CanAccept(ped, 20.f)) {
        return false;
    }

    // If ped doesn't accept look at IK's abort it (if any) and return false
    if (!ped->bDontAcceptIKLookAts) {
        if (IsLooking(ped)) {
            AbortLookAt(ped);
        }
        return false;
    }

    if (ped->m_pedIK.bUnk) {
        return false;
    }

    const auto GetPedClumpAnimAssoc = [ped](AnimationId anim) {
        return RpAnimBlendClumpGetAssociation(ped->m_pRwClump, anim);
    };

    if (rng::any_of(std::to_array({ ANIM_ID_DRNKBR_PRTL, ANIM_ID_SMKCIG_PRTL, ANIM_ID_DRNKBR_PRTL_F }), GetPedClumpAnimAssoc)) {
        return false;
    }

    return !RpAnimBlendClumpGetAssociation(ped->m_pRwClump, ANIM_ID_SMKCIG_PRTL_F);
}

// 0x618970
void IKChainManager_c::LookAt(Const char* purpose, CPed* ped, CEntity* targetEntity, int32 time, ePedBones pedBoneId, CVector* posn, bool useTorso, float fSpeed, int32 blendTime, uint8 priority, bool bForceLooking) {
    if (!bForceLooking) {
        if (!CanAcceptLookAt(ped) || *(bool*)0xC1542C) { // TODO `byte_C1542C` | staticref
            return;
        }
    }

    // Make sure we have an IK manager task
    if (!GetPedIKManagerTask(ped)) {
        ped->GetTaskManager().SetTaskSecondary(new CTaskSimpleIKManager(), TASK_SECONDARY_IK);
    }

    const auto lookAtOffset = posn ? *posn : CVector{};

    // Now, either update existing task or create one
    if (const auto lookAt = GetPedIKLookAtTask(ped)) {
        if (priority < lookAt->m_nPriority) {
            return;
        }
        if (useTorso || !lookAt->m_bUseTorso) {
            lookAt->UpdateLookAtInfo(purpose, ped, targetEntity, time, (int32)pedBoneId, lookAtOffset, useTorso && lookAt->m_bUseTorso, fSpeed, blendTime, priority);
        } else {
            AbortLookAt(ped);
        }
    } else { // Doesn't have task yet, create it
        GetPedIKManagerTask(ped)->AddIKChainTask(
            new CTaskSimpleIKLookAt{purpose, targetEntity, time, (int32)pedBoneId, lookAtOffset, useTorso, fSpeed, (uint32)blendTime, priority}, 0);
    }
}

// 0x6182B0
bool IKChainManager_c::IsArmPointing(int32 nSlot, CPed* ped) { // May be __stdcall
    const auto mgr = GetPedIKManagerTask(ped);
    return mgr && mgr->GetTaskAtSlot(nSlot + 1);
}

// 0x6182F0
void IKChainManager_c::AbortPointArm(int32 slot, CPed* ped, int32 blendOutTime) {
    const auto mgr = GetPedIKManagerTask(ped);
    if (const auto lookAt = static_cast<CTaskSimpleIKChain*>(mgr->GetTaskAtSlot(slot + 1))) {
        lookAt->BlendOut(blendOutTime);
    }
}

// 0x618330
bool IKChainManager_c::IsFacingTarget(CPed* ped, int32 a2) {
    return plugin::CallMethodAndReturn<bool, 0x618330, IKChainManager_c*, CPed*, int32>(this, ped, a2);
}

// 0x618B60
void IKChainManager_c::PointArm(Const char* taskName, int32 a2, CPed* ped, CEntity* target, ePedBones pedBoneId, CVector* posn, float fSpeedMB, int32 blendTimeMB, float a9) {
    plugin::CallMethod<0x618B60, IKChainManager_c*, const char*, int32, CPed*, CEntity*, ePedBones, CVector*, float, int32, float>(this, taskName, a2, ped, target, pedBoneId, posn, fSpeedMB, blendTimeMB, a9);
}
