#pragma once
#include "AEBankLoader.h"

struct tVirtualChannelSettings {
    int16 BankSlotIDs[300]{};
    int16 SoundIDs[300]{};

    tVirtualChannelSettings() {
        rng::fill(BankSlotIDs, -1);
        rng::fill(SoundIDs, -1);
    }
};
VALIDATE_SIZE(tVirtualChannelSettings, 0x4B0);

class CAEMP3BankLoader : public CAEBankLoader {
public:
    static void InjectHooks();

    CAEMP3BankLoader();

    bool Initialise();
    uint8* GetBankSlot(uint16 bankSlot, uint32& outLength);
    float GetSoundHeadroom(uint16 soundId, int16 bankSlot);
    bool IsSoundBankLoaded(uint16 bankId, int16 bankSlot);
    bool GetSoundBankLoadingStatus(uint16 bankId, int16 bankSlot);
    uint8* GetSoundBuffer(uint16 soundId, int16 bankSlot, uint32& outIndex, uint16& outSampleRate);
    int32 GetLoopOffset(uint16 soundId, int16 bankSlot);
    bool IsSoundLoaded(uint16 bankId, uint16 soundId, int16 bankSlot) const;
    bool GetSoundLoadingStatus(uint16 bankId, uint16 soundId, int16 bankSlot);
    void UpdateVirtualChannels(tVirtualChannelSettings* settings, int16* lengths, int16* loopStartTimes);
    void LoadSoundBank(uint16 bankId, int16 bankSlot);
    void LoadSound(uint16 bankId, uint16 soundId, int16 bankSlot);
    void Service();

    // NOTSA
    bool DoesRequestExist(uint16 bankId, int16 bankSlot, std::optional<int16> numSounds = {}) {
        for (auto& request : m_Requests) {
            if (request.Bank == bankId && request.Slot == bankSlot) {
                if (!numSounds.has_value() || request.SoundID == *numSounds)
                    return true;
            }
        }
        return false;
    }

    CAEBankSlot& GetBankSlot(eSoundBankSlot slot) const;
private:
    void AddRequest(eSoundBank bank, eSoundBankSlot slot, std::optional<eSoundID> sound);

private:
    // NOTSA
    CAEMP3BankLoader* Constructor() {
        this->CAEMP3BankLoader::CAEMP3BankLoader();
        return this;
    }
};
VALIDATE_SIZE(CAEMP3BankLoader, 0x6E4);
