/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "AnimAssociationData.h"
#include "eAnimBlendCallbackType.h"
#include "RenderWare.h"

#include <extensions/WEnum.hpp>

class CAnimBlendNode;
class CAnimBlendHierarchy;
class CAnimBlendStaticAssociation;

enum eAnimationFlags {
    ANIMATION_DEFAULT            = 0,       //0x0,
    ANIMATION_STARTED            = 1 << 0,  //0x1,
    ANIMATION_LOOPED             = 1 << 1,  //0x2,
    ANIMATION_FREEZE_LAST_FRAME  = 1 << 2,  //0x4,
    ANIMATION_UNLOCK_LAST_FRAME  = 1 << 3,  //0x8,  // Animation will be stuck on last frame, if not set
    ANIMATION_PARTIAL            = 1 << 4,  //0x10, // TODO: Flag name is possibly incorrect? Following the usual logic (like `ANIMATION_MOVEMENT`), it should be `ANIMATION_GET_IN_CAR` (See  `RemoveGetInAnims`)
    ANIMATION_MOVEMENT           = 1 << 5,  //0x20,
    ANIMATION_TRANSLATE_Y        = 1 << 6,  //0x40,
    ANIMATION_TRANSLATE_X        = 1 << 7,  //0x80,
    ANIMATION_WALK               = 1 << 8,  //0x100,
    ANIMATION_200                = 1 << 9,  //0x200,
    ANIMATION_ADD_TO_BLEND       = 1 << 10, //0x400, // Possibly should be renamed to ANIMATION_IDLE, see `CPed::PlayFootSteps()`
    ANIMATION_800                = 1 << 11, //0x800,
    ANIMATION_SECONDARY_TASK_ANIM= 1 << 12, //0x1000,
    ANIMATION_FREEZE_TRANSLATION = 1 << 13, //0x2000,
    ANIMATION_BLOCK_REFERENCED   = 1 << 14, //0x4000,
    ANIMATION_INDESTRUCTIBLE     = 1 << 15, //0x8000 // The animation is never destroyed if this flag is set, NO MATTER WHAT
};

class CDefaultAnimCallback {
public:
    static void DefaultAnimCB(class CAnimBlendAssociation* animAssoc, void* something) {
        // nothing here
    }
};

class CAnimBlendLink {
public:
    CAnimBlendLink* next{};
    CAnimBlendLink* prev{};

    CAnimBlendLink() = default;

    void Init() {
        next = nullptr;
        prev = nullptr;
    }

    void Prepend(CAnimBlendLink* link) {
        if (next) {
            next->prev = link;
        }
        link->next = next;
        link->prev = this;
        next = link;
    }

    void Remove() {
        if (prev) {
            prev->next = next;
        }
        if (next) {
            next->prev = prev;
        }
        Init();
    }
};

/*!
* @brief Represents a running animtion for a clump (Usually peds)
* 
* The sequence/frames data is copied from `CAnimBlendHierarchy` to `CAnimBlendAssociation` when a clump requests an animation.
* The instance of `CAnimBlendAssociation` gets destroyed when the ped/clump stops playing the animation.
* But `CAnimBlendHierarchy` is never destroyed and stays in memory unless `CStreaming` forces the IFP to unload (to create space in memory)
* 
* A clump can have one, or more, instances of this class. Usually there's only 1 primary animation,
* but there are also partial animations, which can be played alongside primary animations, like hand gestures or smoking.
* So if an animation moves up to 15 bones in one animation, there'll be 15 instances of `CAnimBlendSequence`,
* and there'll be always one instance of `CAnimBlendHierarchy` for that animation (containing the `CAnimBlendSequence`'s).
*/
class NOTSA_EXPORT_VTABLE CAnimBlendAssociation {
public:
    CAnimBlendLink                m_Link;
    uint16                        m_NumBlendNodes;
    notsa::WEnumS16<AssocGroupId> m_AnimGroupId;
    CAnimBlendNode*               m_BlendNodes; // NOTE: Order of these depends on order of nodes in Clump this was built from
    CAnimBlendHierarchy*          m_BlendHier;
    float                         m_BlendAmount;
    float                         m_BlendDelta; // How much `BlendAmount` changes over time
    float                         m_CurrentTime;
    float                         m_Speed;
    float                         m_TimeStep;
    notsa::WEnumS16<AnimationId>  m_AnimId;
    uint16                        m_Flags; // TODO: use bitfield

    // Callback shit
    eAnimBlendCallbackType m_nCallbackType;
    void (*m_pCallbackFunc)(CAnimBlendAssociation*, void*);
    void* m_pCallbackData;

public:
    CAnimBlendAssociation();
    CAnimBlendAssociation(RpClump* clump, CAnimBlendHierarchy* animHierarchy);
    CAnimBlendAssociation(CAnimBlendAssociation& assoc);
    explicit CAnimBlendAssociation(CAnimBlendStaticAssociation& assoc);

    virtual ~CAnimBlendAssociation();

    #undef GetCurrentTime
    float GetCurrentTime() const { return m_CurrentTime; }

    float GetTimeProgress()                  const;
    float GetBlendAmount(float weight = 1.f) const { return IsPartial() ? m_BlendAmount : m_BlendAmount * weight; }
    float GetBlendDelta()                    const { return m_BlendDelta; }

    AnimationId GetAnimId() const { return m_AnimId; }

    [[nodiscard]] bool IsRunning()        const { return (m_Flags & ANIMATION_STARTED) != 0; }
    [[nodiscard]] bool IsRepeating()      const { return (m_Flags & ANIMATION_LOOPED) != 0; }
    [[nodiscard]] bool IsPartial()        const { return (m_Flags & ANIMATION_PARTIAL) != 0; }
    [[nodiscard]] bool IsMoving()         const { return (m_Flags & ANIMATION_MOVEMENT) != 0; }
    [[nodiscard]] bool HasYTranslation()  const { return (m_Flags & ANIMATION_TRANSLATE_X) != 0; }
    [[nodiscard]] bool HasXTranslation()  const { return (m_Flags & ANIMATION_TRANSLATE_Y) != 0; }
    [[nodiscard]] bool IsIndestructible() const { return (m_Flags & ANIMATION_INDESTRUCTIBLE) != 0; }

    void AllocateAnimBlendNodeArray(int32 count);
    void FreeAnimBlendNodeArray();
    CAnimBlendNode* GetNode(int32 nodeIndex);

    void Init(RpClump* clump, CAnimBlendHierarchy* hierarchy);
    void Init(CAnimBlendAssociation& source);
    void Init(CAnimBlendStaticAssociation& source);

    void ReferenceAnimBlock();
    void SetBlendDelta(float value) { m_BlendDelta = value; }
    void SetBlend(float blendAmount, float blendDelta);
    void SetBlendTo(float blendAmount, float blendDelta);
    void SetCurrentTime(float currentTime);
    void SetDeleteCallback(void(*callback)(CAnimBlendAssociation*, void*), void* data = nullptr);
    void SetFinishCallback(void(*callback)(CAnimBlendAssociation*, void*), void* data = nullptr);
    void Start(float currentTime = 0.f);

    /*!
     * @addr 0x4CEB40
     * @brief Sync the play time of this animation with another
    */
    void SyncAnimation(CAnimBlendAssociation* syncWith);
    bool UpdateBlend(float mult);
    bool UpdateTime(float a1, float a2);
    void UpdateTimeStep(float speedMult, float timeMult);
    bool HasFinished() const;
    [[nodiscard]] uint32 GetHashKey() const noexcept;

    // NOTSA
    void SetFlag(eAnimationFlags flag, bool value = true) {
        if (value)
            m_Flags |= (int)flag;
        else
            m_Flags &= ~(int)flag;
    }

    static CAnimBlendAssociation* FromLink(CAnimBlendLink* link) {
        return (CAnimBlendAssociation*)((byte*)link - offsetof(CAnimBlendAssociation, m_Link));
    }

    void SetSpeed(float speed) {
        m_Speed = speed;
    }

    auto GetNodes() { return std::span{ &m_BlendNodes, m_NumBlendNodes }; }
    void SetDefaultFinishCallback() { SetFinishCallback(CDefaultAnimCallback::DefaultAnimCB, nullptr); }
    auto GetHier() const { return m_BlendHier; }
private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CAnimBlendAssociation* Constructor0()  { this->CAnimBlendAssociation::CAnimBlendAssociation(); return this; }
    CAnimBlendAssociation* Constructor1(RpClump* clump, CAnimBlendHierarchy* animHierarchy) { this->CAnimBlendAssociation::CAnimBlendAssociation(clump, animHierarchy); return this; }
    CAnimBlendAssociation* Constructor2(CAnimBlendAssociation& assoc) { this->CAnimBlendAssociation::CAnimBlendAssociation(assoc); return this; }
    CAnimBlendAssociation* Constructor3(CAnimBlendStaticAssociation& assoc) { this->CAnimBlendAssociation::CAnimBlendAssociation(assoc); return this; }
};
VALIDATE_SIZE(CAnimBlendAssociation, 0x3C);
