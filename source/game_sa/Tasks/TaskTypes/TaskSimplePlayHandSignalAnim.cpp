#include "StdInc.h"

#include "TaskSimplePlayHandSignalAnim.h"

#include "IKChainManager_c.h"

void CTaskSimplePlayHandSignalAnim::InjectHooks() {
    RH_ScopedClass(CTaskSimplePlayHandSignalAnim);
    RH_ScopedCategory("Tasks/TaskTypes");

    RH_ScopedVirtualInstall(Clone, 0x61B980);
    RH_ScopedVirtualInstall(GetId, 0x61AEA0);
    RH_ScopedVirtualInstall(MakeAbortable, 0x61AF50);
    RH_ScopedVirtualInstall(ProcessPed, 0x61BDA0);
    RH_ScopedInstall(StartAnim, 0x61AF60);
}

CTaskSimplePlayHandSignalAnim::CTaskSimplePlayHandSignalAnim(AnimationId animationId, float fBlendFactor, bool bFatHands, bool bHoldLastFrame) : CTaskSimpleAnim(bHoldLastFrame) {
    m_nAnimationBlockIndex = animationId;
    m_pLeftHandObject = nullptr;
    m_pRightHandObject = nullptr;
    m_fBlendFactor = fBlendFactor;
    m_bUseFatHands = bFatHands;
}

CTaskSimplePlayHandSignalAnim::~CTaskSimplePlayHandSignalAnim() {
    if (m_pLeftHandObject) {
        CWorld::Remove(m_pLeftHandObject);
        delete m_pLeftHandObject;
        --CObject::nNoTempObjects;
    }

    if (m_pRightHandObject) {
        CWorld::Remove(m_pRightHandObject);
        delete m_pRightHandObject;
        --CObject::nNoTempObjects;
    }
}

CTask* CTaskSimplePlayHandSignalAnim::Clone() {
    return CTaskSimplePlayHandSignalAnim::Clone_Reversed();
}
CTask* CTaskSimplePlayHandSignalAnim::Clone_Reversed() {
    return new CTaskSimplePlayHandSignalAnim(m_nAnimationBlockIndex, m_fBlendFactor, m_bUseFatHands, m_bHoldLastFrame);
}

eTaskType CTaskSimplePlayHandSignalAnim::GetTaskType() {
    return CTaskSimplePlayHandSignalAnim::GetId_Reversed();
}
eTaskType CTaskSimplePlayHandSignalAnim::GetId_Reversed() {
    return TASK_SIMPLE_HANDSIGNAL_ANIM;
}

bool CTaskSimplePlayHandSignalAnim::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
    return CTaskSimplePlayHandSignalAnim::MakeAbortable_Reversed(ped, priority, event);
}
bool CTaskSimplePlayHandSignalAnim::MakeAbortable_Reversed(CPed* ped, eAbortPriority priority, const CEvent* event) {
    return CTaskSimpleAnim::MakeAbortable(ped, priority, event);
}

bool CTaskSimplePlayHandSignalAnim::ProcessPed(CPed* ped) {
    return CTaskSimplePlayHandSignalAnim::ProcessPed_Reversed(ped);
}
bool CTaskSimplePlayHandSignalAnim::ProcessPed_Reversed(CPed* ped) {
    if (g_ikChainMan.IsArmPointing(1, ped) || m_bIsFinished)
        return true;

    if (!m_pAnim) {
        if (CObject::nNoTempObjects + 1 >= 150)
            return true;

        CTaskSimplePlayHandSignalAnim::StartAnim(ped);
    }

    return m_bIsFinished;
}

void CTaskSimplePlayHandSignalAnim::StartAnim(CPed* ped) {
    const AnimationId animId = static_cast<const AnimationId>(CGeneral::GetRandomNumberInRange((int32)ANIM_ID_GSIGN1, (int32)ANIM_ID_GSIGN5 + 1));

    // Pointing / weapon logic
    if (ped->GetEntityThatThisPedIsHolding() || g_ikChainMan.IsArmPointing(0, ped) || ped->GetActiveWeapon().m_nType != eWeaponType::WEAPON_UNARMED) {
        m_pAnim = CAnimManager::BlendAnimation(ped->m_pRwClump, ANIM_GROUP_HANDSIGNALL, animId, m_fBlendFactor);
        m_pAnim->SetFinishCallback(CTaskSimpleAnim::FinishRunAnimCB, this);

        if (m_nAnimationBlockIndex == -1)
            return;

        const auto handIndex = m_bUseFatHands ? MODEL_FHANDL : MODEL_SHANDL;
        m_pLeftHandObject = new CHandObject(handIndex, ped, true);
        if (!m_pLeftHandObject)
            return;

        CWorld::Add(m_pLeftHandObject);
        auto* animHierarchy = CAnimManager::GetAnimAssociation(ANIM_GROUP_LHAND, m_nAnimationBlockIndex)->m_pHierarchy;
        CAnimManager::AddAnimation(m_pLeftHandObject->m_pRwClump, animHierarchy, 0);
        ++CObject::nNoTempObjects;
        return;
    }

    // Both arms logic
    m_pAnim = CAnimManager::BlendAnimation(ped->m_pRwClump, ANIM_GROUP_HANDSIGNAL, animId, m_fBlendFactor);
    m_pAnim->SetFinishCallback(CTaskSimpleAnim::FinishRunAnimCB, this);
    if (m_nAnimationBlockIndex == -1)
        return;

    const auto leftHandIndex = m_bUseFatHands ? MODEL_FHANDL : MODEL_SHANDL;
    m_pLeftHandObject = new CHandObject(leftHandIndex, ped, true);
    if (m_pLeftHandObject) {
        CWorld::Add(m_pLeftHandObject);
        auto* animHierarchy = CAnimManager::GetAnimAssociation(ANIM_GROUP_LHAND, m_nAnimationBlockIndex)->m_pHierarchy;
        CAnimManager::AddAnimation(m_pLeftHandObject->m_pRwClump, animHierarchy, 0);
        ++CObject::nNoTempObjects;
    }

    if (m_nAnimationBlockIndex == -1)
        return;

    const auto rightHandIndex = m_bUseFatHands ? MODEL_FHANDR : MODEL_SHANDR;
    m_pRightHandObject = new CHandObject(rightHandIndex, ped, false);
    if (m_pRightHandObject) {
        CWorld::Add(m_pRightHandObject);
        auto* animHierarchy = CAnimManager::GetAnimAssociation(ANIM_GROUP_RHAND, m_nAnimationBlockIndex)->m_pHierarchy;
        CAnimManager::AddAnimation(m_pRightHandObject->m_pRwClump, animHierarchy, 0);
        ++CObject::nNoTempObjects;
    }
}
