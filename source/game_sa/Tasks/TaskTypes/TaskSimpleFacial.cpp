#include "StdInc.h"

#include "TaskSimpleFacial.h"

// 0x690C70
CTaskSimpleFacial::CTaskSimpleFacial(eFacialExpression nFacialExpress, int32 nDuration) : CTaskSimple() {
    m_Duration = nDuration;
    m_Type = nFacialExpress;
}

// See Android, looks like **map**
// 0x690CC0
AnimationId CTaskSimpleFacial::GetAnimId(eFacialExpression expression) {
    switch (expression) {
    case eFacialExpression::SURPRISEPANIC: return ANIM_ID_FACSURP;
    case eFacialExpression::SURPRISEMILD: return ANIM_ID_FACSURPM;

    case eFacialExpression::CURIOSITY:
    case eFacialExpression::HAPPINESS: return ANIM_ID_FACURIOS;

    case eFacialExpression::ANGER:
    case eFacialExpression::SADNESS: return ANIM_ID_FACANGER_0;

    case eFacialExpression::TALKING: return ANIM_ID_FACTALK;
    case eFacialExpression::CHEWING: return ANIM_ID_FACGUM;

    default: return ANIM_ID_UNDEFINED;
    }
}

// 0x692E50
bool CTaskSimpleFacial::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
    if (const auto anim = RpAnimBlendClumpGetAssociation(ped->m_pRwClump, GetAnimId(m_Type))) {
        anim->m_BlendDelta = -4.0f;
    }
    return true;
}

// 0x692E80
bool CTaskSimpleFacial::ProcessPed(CPed* ped) {
    const auto animId = CTaskSimpleFacial::GetAnimId(m_Type);
    const auto anim  = RpAnimBlendClumpGetAssociation(ped->m_pRwClump, animId);

    if (RpAnimBlendClumpGetAssociation(ped->m_pRwClump, ANIM_ID_FACTALK)) {
        if (animId == ANIM_ID_FACTALK) {
            if (CGeneral::GetRandomNumberInRange(0, 100) < 40) {
                anim->m_Speed = CGeneral::GetRandomNumberInRange(0.5f, 3.0f);
            }
            return false;
        }
        if (anim) {
            anim->SetFlag(ANIMATION_FREEZE_LAST_FRAME, true);
            anim->m_BlendDelta = -4.0f;
        }
        return true;
    }

    if (!m_Timer.m_bStarted) {
        if (!anim) {
            CAnimManager::BlendAnimation(ped->m_pRwClump, ANIM_GROUP_DEFAULT, animId, 4.0f);
            m_Timer.Start(m_Duration);
            return false;
        }
        return true;
    }

    if (!anim) {
        return true;
    }

    if (!m_Timer.IsOutOfTime()) {
        return false;
    }

    anim->m_BlendDelta = -4.0f;

    return true;
}
