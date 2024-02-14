/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "AnimSequenceFrames.h"

//! The sequence of key-frames of one animated node (bone)
class CAnimBlendSequence {
public:
    //! The bone/frame this sequence is associated with
    //! Thanks to jte for some info
    union {
        eBoneTag m_BoneID{BONE_UNKNOWN};  // IF m_IsUsingBoneTag == TRUE
        uint32   m_FrameHashKey;          // IF m_IsUsingBoneTag == FALSE
    };
    uint16 m_bHasRotation : 1{};
    uint16 m_bHasTranslation : 1{};      // Root key frames have translation values (quaternion).
    uint16 m_bIsCompressed : 1{};        // Compressed key frames.
    uint16 m_bUsingExternalMemory : 1{}; // When this flag is NOT set, you have to loop through all key frames in m_pFrames and free them separately.
    uint16 m_IsUsingBoneTag : 1{};
    uint16 m_FramesNum{};
    void*  m_Frames{};

public:
    static void InjectHooks();

    CAnimBlendSequence() = default; // 0x4D0C10
    ~CAnimBlendSequence();

    void CompressKeyframes(uint8* frameData);
    [[nodiscard]] size_t GetDataSize(bool compressed) const;
    bool MoveMemory();
    void Print();
    void RemoveQuaternionFlips() const;
    void RemoveUncompressedData(uint8* frameData);
    void SetBoneTag(eBoneTag32 boneId);
    void SetName(const char* string);
    void SetNumFrames(uint32 count, bool bHasTranslation, bool compressed, void* frameData);
    void Uncompress(uint8* frameData);
    bool IsCompressed() const { return m_bIsCompressed; }
    bool HasTranslation() const { return m_bHasTranslation; }

    /*!
     * @brief Used to implement copy-paste functions
     * @tparam AsCompressed Whenever `IsCompressed()` is returning true/false at the runtime.
     * @param n The index of the key-frame
     * @return The `n`th key-frame
    */
    template<bool AsCompressed>
    auto GetKeyFrame(size_t n) const {
        assert(IsCompressed() == AsCompressed);
        if constexpr (AsCompressed) {
            return GetCKeyFrame(n);
        } else {
            return GetUKeyFrame(n);
        }
    }

    /*!
     * @brief Get the `n`th key-frame (Use this only for un-compressed anims (if `!IsCompressed()`))
     * @brief The true return type is `KeyFrameTrans*` (if `HasTranslation()`) or `KeyFrame*`, 
     * @brier The former is just returned for convenience.
     * @param n The index of the key-frame
     * @return The `n`th key-frame
    */
    KeyFrameTrans* GetUKeyFrame(size_t n) const { // `U` = Uncompressed
        assert(n < m_FramesNum);
        assert(!m_bIsCompressed);
        return static_cast<KeyFrameTrans*>(m_bHasTranslation ? // Lie a little by always up-casting to `KeyFrameTrans*` to make our lifes easier
            &((KeyFrameTrans*)m_Frames)[n] :
            &((KeyFrame*)m_Frames)[n]
        );
    }

    /*!
    * @brief Get the `n`th key-frame (Use this only for compressed anims (if `IsCompressed()`))
    * @brief The true return type is `KeyFrameTransCompressed*` (if `HasTranslation()`) or `KeyFrameCompressed*`, 
    * @brier The former is just returned for convenience.
    * @param n The index of the key-frame
    * @return The `n`th key-frame
    */
    KeyFrameTransCompressed* GetCKeyFrame(size_t n) const { // `C` = Compressed
        assert(n < m_FramesNum);
        assert(m_bIsCompressed);
        return static_cast<KeyFrameTransCompressed*>(m_bHasTranslation ? // Lie a little by always up-casting to `KeyFrameTransCompressed*` to make our lifes easier
            &((KeyFrameTransCompressed*)m_Frames)[n] :
            &((KeyFrameCompressed*)m_Frames)[n]
        );
    }

    CAnimBlendSequence* Constructor() { this->CAnimBlendSequence::CAnimBlendSequence(); return this; }
    CAnimBlendSequence* Destructor() { this->CAnimBlendSequence::~CAnimBlendSequence(); return this; }
};

VALIDATE_SIZE(CAnimBlendSequence, 0xC);
