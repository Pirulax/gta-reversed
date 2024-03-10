#include "StdInc.h"

#include "AEVehicleAudioEntity.h"

#include "AEAudioHardware.h"
#include "AEAudioUtility.h"
#include "AESoundManager.h"
#include "AEAudioUtility.h"

CPed*&                  CAEVehicleAudioEntity::s_pPlayerAttachedForRadio                     = *(CPed**)0xB6B98C;
CPed*&                  CAEVehicleAudioEntity::s_pPlayerDriver                               = *(CPed**)0xB6B990;
bool&                   CAEVehicleAudioEntity::s_HelicoptorsDisabled                         = *(bool*)0xB6B994;
int16&                  CAEVehicleAudioEntity::s_NextDummyEngineSlot                         = *(int16*)0xB6B998;
tVehicleAudioSettings*& CAEVehicleAudioEntity::s_pVehicleAudioSettingsForRadio               = *reinterpret_cast<tVehicleAudioSettings**>(0xB6B99C);
tEngineDummySlot        (&CAEVehicleAudioEntity::s_DummyEngineSlots)[NUM_DUMMY_ENGINE_SLOTS] = *reinterpret_cast<tEngineDummySlot (*)[NUM_DUMMY_ENGINE_SLOTS]>(0xB6B9A0);

const tVehicleAudioSettings (&gVehicleAudioSettings)[NUM_VEH_AUDIO_SETTINGS] = *reinterpret_cast<const tVehicleAudioSettings (*)[232]>(0x860AF0);

bool IsSurfaceAudioGrass(eSurfaceType surface) {
    return g_surfaceInfos.IsAudioGrass(surface) || g_surfaceInfos.IsAudioLongGrass(surface);
}

bool IsSurfaceAudioEitherGravelWaterSand(eSurfaceType surface) {
    return g_surfaceInfos.IsAudioGravel(surface) || g_surfaceInfos.IsAudioSand(surface) || g_surfaceInfos.IsAudioWater(surface);
}

void CAEVehicleAudioEntity::InjectHooks() {
    RH_ScopedVirtualClass(CAEVehicleAudioEntity, 0x862CEC, 1);
    RH_ScopedCategory("Audio/Entities");

    RH_ScopedInstall(Constructor, 0x4F63E0);
    RH_ScopedInstall(Destructor, 0x6D0A10);
    RH_ScopedVMTInstall(UpdateParameters, 0x4FB6C0);
    RH_ScopedInstall(Initialise, 0x4F7670);
    RH_ScopedInstall(StaticInitialise, 0x5B99F0);
    RH_ScopedInstall(Terminate, 0x4FB8C0);
    RH_ScopedInstall(GetAircraftNearPosition, 0x4F96A0);
    RH_ScopedInstall(GetFlyingMetalVolume, 0x4F6150);
    RH_ScopedInstall(GetVehicleAudioSettings, 0x4F5C10);
    RH_ScopedInstall(RequestBankSlot, 0x4F4D10);
    RH_ScopedInstall(DemandBankSlot, 0x4F4E60);
    RH_ScopedInstall(DoesBankSlotContainThisBank, 0x4F4E30);
    RH_ScopedInstall(StoppedUsingBankSlot, 0x4F4DF0);
    RH_ScopedInstall(EnableHelicoptors, 0x4F4EF0);
    RH_ScopedInstall(DisableHelicoptors, 0x4F4EE0);
    RH_ScopedInstall(EnableHelicoptor, 0x4F5C00);
    RH_ScopedInstall(DisableHelicoptor, 0x4F5BF0);
    RH_ScopedInstall(StaticGetPlayerVehicleAudioSettingsForRadio, 0x4F4ED0);
    RH_ScopedInstall(StaticService, 0x4F4EC0);
    RH_ScopedInstall(GetVehicleTypeForAudio, 0x4F4F00);
    RH_ScopedInstall(IsAccInhibited, 0x4F4F70);
    RH_ScopedInstall(IsAccInhibitedBackwards, 0x4F4FC0);
    RH_ScopedInstall(IsAccInhibitedForLowSpeed, 0x4F4FF0, { .reversed = false });
    RH_ScopedInstall(IsAccInhibitedForTime, 0x4F5020);
    RH_ScopedInstall(InhibitAccForTime, 0x4F5030);
    RH_ScopedInstall(InhibitCrzForTime, 0x4F5060);
    RH_ScopedInstall(IsCrzInhibitedForTime, 0x4F5050);
    RH_ScopedInstall(GetAccelAndBrake, 0x4F5080, { .reversed = false });
    RH_ScopedInstall(GetVolumeForDummyIdle, 0x4F51F0, { .reversed = false });
    RH_ScopedInstall(GetFrequencyForDummyIdle, 0x4F5310, { .reversed = false });
    RH_ScopedInstall(GetVolumeForDummyRev, 0x4F53D0, { .reversed = false });
    RH_ScopedInstall(GetFrequencyForDummyRev, 0x4F54F0, { .reversed = false });
    RH_ScopedInstall(CancelVehicleEngineSound, 0x4F55C0, { .reversed = false });
    RH_ScopedInstall(UpdateVehicleEngineSound, 0x4F56D0);
    RH_ScopedInstall(JustGotInVehicleAsDriver, 0x4F5700, { .reversed = false });
    RH_ScopedInstall(TurnOnRadioForVehicle, 0x4F5B20);  // -
    RH_ScopedInstall(TurnOffRadioForVehicle, 0x4F5B60); // -
    RH_ScopedInstall(PlayerAboutToExitVehicleAsDriver, 0x4F5BA0);
    RH_ScopedInstall(CopHeli, 0x4F5C40);
    RH_ScopedInstall(GetFreqForIdle, 0x4F5C60, { .reversed = false });
    RH_ScopedInstall(GetBaseVolumeForBicycleTyre, 0x4F60B0);
    RH_ScopedInstall(GetVolForPlayerEngineSound, 0x4F5D00, { .reversed = false });
    RH_ScopedInstall(JustFinishedAccelerationLoop, 0x4F5E50);
    RH_ScopedInstall(UpdateGasPedalAudio, 0x4F5EB0, { .reversed = false });
    RH_ScopedInstall(GetVehicleDriveWheelSkidValue, 0x4F5F30, { .reversed = false });
    RH_ScopedInstall(GetVehicleNonDriveWheelSkidValue, 0x4F6000, { .reversed = false });
    RH_ScopedInstall(GetHornState, 0x4F61E0, { .reversed = false });
    RH_ScopedInstall(GetSirenState, 0x4F62A0, { .reversed = false });
    RH_ScopedInstall(StopGenericEngineSound, 0x4F6320);
    RH_ScopedInstall(RequestNewPlayerCarEngineSound, 0x4F7A50, { .reversed = false });
    RH_ScopedInstall(StartVehicleEngineSound, 0x4F7F20, { .reversed = false });
    RH_ScopedInstall(GetFreqForPlayerEngineSound, 0x4F8070, { .reversed = false });
    RH_ScopedInstall(PlaySkidSound, 0x4F8360, { .reversed = false });
    RH_ScopedInstall(PlayRoadNoiseSound, 0x4F84D0, { .reversed = false });
    RH_ScopedInstall(PlayFlatTyreSound, 0x4F8650, { .reversed = false });
    RH_ScopedInstall(PlayReverseSound, 0x4F87D0, { .reversed = false });
    RH_ScopedInstall(ProcessVehicleFlatTyre, 0x4F8940, { .reversed = false });
    RH_ScopedInstall(ProcessVehicleRoadNoise, 0x4F8B00, { .reversed = false });
    RH_ScopedInstall(ProcessReverseGear, 0x4F8DF0, { .reversed = false });
    RH_ScopedInstall(ProcessVehicleSkidding, 0x4F8F10, { .reversed = false });
    RH_ScopedInstall(ProcessRainOnVehicle, 0x4F92C0);
    RH_ScopedInstall(PlayAircraftSound, 0x4F93C0, { .reversed = false });
    RH_ScopedInstall(PlayBicycleSound, 0x4F9710, { .reversed = false });
    RH_ScopedInstall(PlayHornOrSiren, 0x4F99D0, { .reversed = false });
    RH_ScopedInstall(UpdateBoatSound, 0x4F9E90, { .reversed = false });
    RH_ScopedInstall(ProcessBoatMovingOverWater, 0x4FA0C0, { .reversed = false });
    RH_ScopedInstall(UpdateTrainSound, 0x4FA1C0, { .reversed = false });
    RH_ScopedInstall(ProcessTrainTrackSound, 0x4FA3F0, { .reversed = false });
    RH_ScopedInstall(PlayTrainBrakeSound, 0x4FA630, { .reversed = false });
    RH_ScopedInstall(ProcessDummyRCPlane, 0x4FA7C0, { .reversed = false });
    RH_ScopedInstall(ProcessDummyRCHeli, 0x4FAA80, { .reversed = false });
    RH_ScopedInstall(UpdateGenericVehicleSound, 0x4FAD40, { .reversed = false }); // PlaySound?
    RH_ScopedInstall(ProcessEngineDamage, 0x4FAE20, { .reversed = false });
    RH_ScopedInstall(ProcessNitro, 0x4FB070, { .reversed = false });
    RH_ScopedInstall(ProcessMovingParts, 0x4FB260, { .reversed = false });
    RH_ScopedInstall(ProcessPlayerVehicleEngine, 0x4FBB10, { .reversed = false });
    RH_ScopedInstall(ProcessDummyStateTransition, 0x4FCA10, { .reversed = false });
    RH_ScopedInstall(JustGotOutOfVehicleAsDriver, 0x4FCF40, { .reversed = false });
    RH_ScopedInstall(JustWreckedVehicle, 0x4FD0B0, { .reversed = false });
    RH_ScopedInstall(ProcessPlayerProp, 0x4FD290, { .reversed = false });
    RH_ScopedInstall(ProcessDummyProp, 0x4FD8F0, { .reversed = false });
    RH_ScopedInstall(ProcessAIProp, 0x4FDFD0, { .reversed = false });
    RH_ScopedInstall(ProcessPlayerHeli, 0x4FE420, { .reversed = false });
    RH_ScopedInstall(ProcessDummyHeli, 0x4FE940, { .reversed = false });
    RH_ScopedInstall(ProcessAIHeli, 0x4FEE20, { .reversed = false });
    RH_ScopedInstall(ProcessPlayerSeaPlane, 0x4FF320, { .reversed = false });
    RH_ScopedInstall(ProcessDummySeaPlane, 0x4FF7C0, { .reversed = false });
    RH_ScopedInstall(ProcessGenericJet, 0x4FF900, { .reversed = false });
    RH_ScopedInstall(ProcessDummyBicycle, 0x4FFDC0, { .reversed = false });
    RH_ScopedInstall(ProcessPlayerBicycle, 0x500040, { .reversed = false });
    RH_ScopedInstall(ProcessVehicleSirenAlarmHorn, 0x5002C0, { .reversed = false });
    RH_ScopedInstall(ProcessBoatEngine, 0x5003A0, { .reversed = false });
    RH_ScopedInstall(ProcessDummyTrainEngine, 0x500710, { .reversed = false });
    RH_ScopedInstall(ProcessPlayerTrainBrakes, 0x500AB0, { .reversed = false });
    RH_ScopedInstall(ProcessPlayerCombine, 0x500CE0, { .reversed = false });
    RH_ScopedInstall(ProcessDummyRCCar, 0x500DC0, { .reversed = false });
    RH_ScopedInstall(ProcessDummyHovercraft, 0x500F50, { .reversed = false });
    RH_ScopedInstall(ProcessDummyGolfCart, 0x501270, { .reversed = false });
    RH_ScopedInstall(ProcessDummyVehicleEngine, 0x501480, { .reversed = false });
    RH_ScopedInstall(ProcessPlayerJet, 0x501650);
    RH_ScopedInstall(ProcessDummyJet, 0x501960);
    RH_ScopedInstall(ProcessAircraft, 0x501C50);
    RH_ScopedInstall(ProcessVehicle, 0x501E10);
    RH_ScopedInstall(ProcessSpecialVehicle, 0x501AB0);
    RH_ScopedInstall(Service, 0x502280, { .reversed = false });

    RH_ScopedOverloadedInstall(AddAudioEvent, "0", 0x4F6420, void(CAEVehicleAudioEntity::*)(eAudioEvents, float), { .reversed = false });
    RH_ScopedOverloadedInstall(AddAudioEvent, "1", 0x4F7580, void(CAEVehicleAudioEntity::*)(eAudioEvents, CVehicle*), { .reversed = false });
}

// 0x4F63E0
CAEVehicleAudioEntity::CAEVehicleAudioEntity() :
    CAEAudioEntity(),
    m_SkidSound() {
    m_IsInitialized = false;
}

// 0x6D0A10
CAEVehicleAudioEntity::~CAEVehicleAudioEntity() {
    if (m_IsInitialized) {
        Terminate();
    }
}

// 0x4F7670
void CAEVehicleAudioEntity::Initialise(CEntity* entity) {
    assert(entity && entity->IsVehicle());

    m_TimeLastServiced                   = 0;
    m_pEntity                    = entity;
    m_IsPlayerDriver              = false;
    m_IsPlayerPassenger           = false;
    m_IsPlayerDriverAboutToExit        = false;
    m_IsWreckedVehicle             = false;
    m_State               = 0;
    m_AuGear          = 0;
    m_CrzCount                     = 0;
    m_DummySlot          = -1;
    m_RainDropCounter           = 0;
    m_DoCountStalls                     = 0;
    m_StalledCount                     = 0;
    m_SwapStalledTime                     = 0;
    m_IsSilentStalled                     = false;
    m_TimeSplashLastTriggered = 0;
    m_TimeBeforeAllowAccelerate          = 0;
    m_TimeBeforeAllowCruise          = 0;
    m_bNitroOnLastFrame         = false;
    m_IsHelicopterDisabled   = false;
    m_ACPlayPositionThisFrame        = -1;
    m_ACPlayPositionLastFrame  = -1;
    m_ACPlayPositionWhenStopped                    = 0;
    m_ACPlayPercentWhenStopped                    = 0;
    m_FramesAgoACLooped           = 0;

    for (auto i = 0; auto& sound : m_EngineSounds) {
        sound.Init(i++);
    }

    m_HornVolume                = -100.0f;
    m_CurrentDummyEngineVolume = -100.0f;
    m_SurfaceSoundType             = -1;
    m_RoadNoiseSoundType        = -1;
    m_FlatTireSoundType         = -1;
    m_ReverseGearSoundType      = -1;
    m_CurrentDummyEngineFrequency                    = -1.0f;
    m_CurrentRotorFrequency           = -1.0f;
    m_CurrentNitroRatio                    = -1.0f;

    m_SurfaceSound   = nullptr;
    m_RoadNoiseSound   = nullptr;
    m_FlatTireSound    = nullptr;
    m_ReverseGearSound = nullptr;
    m_HornSound     = nullptr;
    m_SirenSound       = nullptr;
    m_FastSirenSound = nullptr;

    m_MovingPartSmoothedSpeed = 0.0f;
    m_FadeIn = 1.0f;
    m_FadeOut = 0.0f;

    m_AuSettings        = GetVehicleAudioSettings(entity->m_nModelIndex);
    m_HasSiren = entity->AsVehicle()->UsesSiren();
    if (m_AuSettings.RadioType == eRadioType::RADIO_UNKNOWN) {
        m_AuSettings.RadioStation = eRadioID::RADIO_OFF;
    }

    m_EventVolume = GetDefaultVolume(AE_GENERAL_VEHICLE_SOUND);

    switch (entity->m_nModelIndex) {
    case MODEL_PIZZABOY:
    case MODEL_CADDY:
    case MODEL_FAGGIO:
    case MODEL_BAGGAGE:
    case MODEL_FORKLIFT:
    case MODEL_VORTEX:
    case MODEL_KART:
    case MODEL_MOWER:
    case MODEL_SWEEPER:
    case MODEL_TUG:
        m_IsSingleGear = true;
        break;
    default:
        m_IsSingleGear = false;
        break;
    }

    switch (m_AuSettings.VehicleAudioType) {
    case VEHICLE_SOUND_CAR:
        m_EventVolume -= 1.5F;
        m_PlayerEngineBank = m_AuSettings.PlayerBank;
        m_DummyEngineBank = m_AuSettings.DummyBank;
        if (m_IsInitialized) {
            return;
        }

        if (m_AuSettings.DummyBank != -1 && m_AuSettings.DummyBank != 129) {
            m_DummySlot = RequestBankSlot(m_AuSettings.DummyBank);
        }

        m_IsInitialized = true;
        return;

    case VEHICLE_SOUND_MOTORCYCLE:
    case VEHICLE_SOUND_BICYCLE:
        m_PlayerEngineBank = m_AuSettings.PlayerBank;

        if (m_AuSettings.IsMotorcycle()) {
            m_EventVolume = m_EventVolume - 1.5F;
        }

        if (m_IsInitialized) {
            return;
        }

        m_DummyEngineBank = m_AuSettings.DummyBank;
        if (m_DummyEngineBank != -1) {
            m_DummySlot = RequestBankSlot(m_AuSettings.DummyBank);
        }

        m_IsInitialized = true;
        return;

    case VEHICLE_SOUND_BOAT:
    case VEHICLE_SOUND_TRAIN:
        m_PlayerEngineBank = m_AuSettings.PlayerBank;
        m_DummyEngineBank = m_AuSettings.DummyBank;
        if (m_IsInitialized) {
            return;
        }

        if (m_AuSettings.DummyBank != -1 && m_AuSettings.DummyBank != 129) {
            m_DummySlot = RequestBankSlot(m_AuSettings.DummyBank);
        }

        m_IsInitialized = true;
        return;

    case VEHICLE_SOUND_HELI:
    case VEHICLE_SOUND_NON_VEH:
        m_DummyEngineBank = m_AuSettings.DummyBank;
        m_PlayerEngineBank = m_AuSettings.PlayerBank;

        m_IsInitialized = true;
        return;

    case VEHICLE_SOUND_PLANE:
        m_DummyEngineBank = m_AuSettings.DummyBank;
        m_PlayerEngineBank = m_AuSettings.PlayerBank;
        if (m_IsInitialized) {
            return;
        }

        if (m_AuSettings.DummyBank != -1) {
            m_DummySlot = RequestBankSlot(m_AuSettings.DummyBank);
        }

        m_IsInitialized = true;
        return;

    case VEHICLE_SOUND_TRAILER:
        m_PlayerEngineBank = m_AuSettings.PlayerBank;
        m_EventVolume   = m_EventVolume - 1.5F;
        if (m_IsInitialized) {
            return;
        }

        m_DummyEngineBank = m_AuSettings.DummyBank;
        if (m_DummyEngineBank != -1) {
            m_DummySlot = RequestBankSlot(m_AuSettings.DummyBank);
        }

        m_IsInitialized = true;
        return;

    default:
        return;
    }
}

// 0x5B99F0
void CAEVehicleAudioEntity::StaticInitialise() {
    for (auto& slot : s_DummyEngineSlots) {
        slot.Reset();
    }

    AEAudioHardware.LoadSoundBank(138, 19);
    AEAudioHardware.LoadSoundBank(13, 18);
    AEAudioHardware.LoadSoundBank(74, 17);

    s_pVehicleAudioSettingsForRadio = nullptr;
    s_pPlayerAttachedForRadio       = nullptr;
    s_pPlayerDriver                 = nullptr;
    s_NextDummyEngineSlot           = 0;
    s_HelicoptorsDisabled           = false;
}

// 0x4FB8C0
void CAEVehicleAudioEntity::Terminate() {
    if (!m_IsInitialized) {
        return;
    }

    for (auto& sound : m_EngineSounds) {
        if (sound.m_pSound) {
            sound.m_pSound->SetIndividualEnvironment(SOUND_REQUEST_UPDATES, false);
            sound.m_pSound->StopSound();
            sound.m_pSound = nullptr;
        }
    }

    PlaySkidSound(-1);
    PlayTrainBrakeSound(-1);

    if (m_SurfaceSound) {
        m_SurfaceSound->SetIndividualEnvironment(SOUND_REQUEST_UPDATES, false);
        m_SurfaceSound->StopSound();
        m_SurfaceSound = nullptr;
        m_SurfaceSoundType = -1;
    }

    if (m_RoadNoiseSound) {
        m_RoadNoiseSound->SetIndividualEnvironment(SOUND_REQUEST_UPDATES, false);
        m_RoadNoiseSound->StopSound();
        m_RoadNoiseSound      = nullptr;
        m_RoadNoiseSoundType = -1;
    }

    if (m_FlatTireSound) {
        m_FlatTireSound->SetIndividualEnvironment(SOUND_REQUEST_UPDATES, false);
        m_FlatTireSound->StopSound();
        m_FlatTireSound      = nullptr;
        m_FlatTireSoundType = -1;
    }

    if (m_ReverseGearSound) {
        m_ReverseGearSound->SetIndividualEnvironment(SOUND_REQUEST_UPDATES, false);
        m_ReverseGearSound->StopSound();
        m_ReverseGearSound      = nullptr;
        m_ReverseGearSoundType = -1;
    }

    if (m_HornSound) {
        m_HornSound->StopSoundAndForget();
        m_HornSound = nullptr;
    }

    if (m_SirenSound) {
        m_SirenSound->StopSoundAndForget();
        m_SirenSound = nullptr;
    }

    if (m_FastSirenSound) {
        m_FastSirenSound->StopSoundAndForget();
        m_FastSirenSound = nullptr;
    }

    if (m_IsPlayerDriver) {
        TurnOffRadioForVehicle();
    }

    // 0x4FBA43
    if (m_DummySlot != -1) {
        const auto usedSlot = m_DummySlot - 7;
        if (usedSlot >= 0 && usedSlot < NUM_DUMMY_ENGINE_SLOTS && s_DummyEngineSlots[usedSlot].m_nBankId == m_DummyEngineBank) {
            s_DummyEngineSlots[usedSlot].m_nUsageCount = std::max<int16>(0, s_DummyEngineSlots[usedSlot].m_nUsageCount - 1);
        }
        m_DummySlot = -1;
    }

    m_DummyEngineBank = -1;
    m_PlayerEngineBank = -1;
    m_pEntity                      = nullptr;
    m_State                 = 0;

    if (m_IsPlayerDriver) {
        m_IsPlayerDriver = false;
        s_pPlayerDriver = nullptr;
    } else if (m_IsPlayerPassenger) {
        m_IsPlayerPassenger = false;
    } else {
        m_IsPlayerDriverAboutToExit = false;
        m_IsInitialized            = false;
        return;
    }

    // OG | Replaced by TurnOffRadioForVehicle
    // s_pPlayerAttachedForRadio = nullptr;
    // s_pVehicleAudioSettingsForRadio = nullptr;

    m_IsPlayerDriverAboutToExit = false;
    m_IsInitialized            = false;
}

// 0x4F4ED0
tVehicleAudioSettings* CAEVehicleAudioEntity::StaticGetPlayerVehicleAudioSettingsForRadio() {
    return s_pVehicleAudioSettingsForRadio;
}

// 0x4F4EE0
void CAEVehicleAudioEntity::EnableHelicoptors() {
    s_HelicoptorsDisabled = true;
}

// 0x4F4EF0
void CAEVehicleAudioEntity::DisableHelicoptors() {
    s_HelicoptorsDisabled = false;
}

// 0x4F5C00
void CAEVehicleAudioEntity::EnableHelicoptor() {
    m_IsHelicopterDisabled = false;
}

// 0x4F5BF0
void CAEVehicleAudioEntity::DisableHelicoptor() {
    m_IsHelicopterDisabled = true;
}

// 0x4F4E30
bool CAEVehicleAudioEntity::DoesBankSlotContainThisBank(int16 bankSlot, int16 bankId) {
    const auto usedSlot = bankSlot - 7; // magic number
    if (usedSlot < 0 || usedSlot > NUM_DUMMY_ENGINE_SLOTS) {
        return false;
    }

    return s_DummyEngineSlots[usedSlot].m_nBankId == bankId;
}

// 0x4F4E60
int16 CAEVehicleAudioEntity::DemandBankSlot(int16 bankId) {
    const auto requestResult = RequestBankSlot(bankId);
    if (requestResult != -1) {
        return requestResult;
    }

    auto slotToFree = 0;
    for (auto i = 1; i < NUM_DUMMY_ENGINE_SLOTS; ++i) {
        if (s_DummyEngineSlots[i].m_nUsageCount < s_DummyEngineSlots[0].m_nUsageCount) {
            slotToFree = i;
            break;
        }
    }

    s_DummyEngineSlots[slotToFree].m_nUsageCount = 0;
    return RequestBankSlot(bankId);
}

// 0x4F4D10
int16 CAEVehicleAudioEntity::RequestBankSlot(int16 bankId) {
    int16 freeSlot = -1;
    for (int16 i = 0; i < NUM_DUMMY_ENGINE_SLOTS; ++i) {
        auto& dummyEng = s_DummyEngineSlots[i];
        if (dummyEng.m_nBankId == bankId) {
            dummyEng.m_nUsageCount++;
            return i + 7; // todo: magic number
        }

        if (dummyEng.m_nUsageCount <= 0) {
            freeSlot = i;
        }
    }

    if (freeSlot == -1) {
        return -1;
    }

    for (auto i = 0; i < NUM_DUMMY_ENGINE_SLOTS; ++i) {
        const auto slot     = (s_NextDummyEngineSlot + i) % NUM_DUMMY_ENGINE_SLOTS;
        auto&      dummyEng = s_DummyEngineSlots[slot];
        if (dummyEng.m_nUsageCount > 0) {
            continue;
        }

        freeSlot = slot;
        ++dummyEng.m_nUsageCount;
        s_NextDummyEngineSlot = (freeSlot + 1) % NUM_DUMMY_ENGINE_SLOTS;
        break;
    }

    // todo: magic number
    AESoundManager.CancelSoundsInBankSlot(freeSlot + 7, true);
    AEAudioHardware.LoadSoundBank(bankId, freeSlot + 7);
    s_DummyEngineSlots[freeSlot].m_nBankId     = bankId;
    s_DummyEngineSlots[freeSlot].m_nUsageCount = 1;
    return freeSlot + 7;
}

void CAEVehicleAudioEntity::StoppedUsingBankSlot(int16 bankSlot) {
    const auto usedSlot = bankSlot - 7; // todo: magic number
    if (usedSlot < 0 || usedSlot > NUM_DUMMY_ENGINE_SLOTS) {
        return;
    }

    auto& dummyEng         = s_DummyEngineSlots[usedSlot];
    dummyEng.m_nUsageCount = std::max<int16>(0, dummyEng.m_nUsageCount - 1);
}

// 0x4F5C10
tVehicleAudioSettings CAEVehicleAudioEntity::GetVehicleAudioSettings(int16 vehId) {
    return gVehicleAudioSettings[vehId - 400];
}

// 0x4FB6C0
void CAEVehicleAudioEntity::UpdateParameters(CAESound* sound, int16 curPlayPos) {
    if (!sound) {
        return;
    }

    if (sound->m_nEvent == AE_BONNET_FLUBBER_FLUBBER) {
        if (!sound->m_pPhysicalEntity) {
            return;
        }

        const auto fBaseVolume = GetDefaultVolume(AE_BONNET_FLUBBER_FLUBBER);
        sound->m_fVolume       = fBaseVolume + GetFlyingMetalVolume(sound->m_pPhysicalEntity->AsPhysical());
        return;
    }

    if (sound->m_nEvent == AE_BOAT_HIT_WAVE) {
        if (curPlayPos == -1) {
            return;
        }

        sound->SetPosition(m_pEntity->GetPosition());
        if (CWeather::UnderWaterness >= 0.5F) {
            sound->StopSound();
        }
    }

    if (curPlayPos == -1) {
        if (sound == m_SurfaceSound) {
            m_SurfaceSound = nullptr;
        } else if (sound == m_RoadNoiseSound) {
            m_RoadNoiseSound = nullptr;
        } else if (sound == m_FlatTireSound) {
            m_FlatTireSound = nullptr;
        } else if (sound == m_ReverseGearSound) {
            m_ReverseGearSound = nullptr;
        } else if (sound == m_HornSound) {
            m_HornSound = nullptr;
        } else if (sound == m_SirenSound) {
            m_SirenSound = nullptr;
        } else if (sound == m_FastSirenSound) {
            m_FastSirenSound = nullptr;
        } else {
            for (auto& engSound : m_EngineSounds) {
                if (engSound.m_pSound == sound) {
                    engSound.m_pSound = nullptr;
                    break;
                }
            }
        }
    } else {
        if ((m_AuSettings.IsFlyingVehicle() || m_AuSettings.IsNonVeh()) && m_EngineSounds[3].m_pSound) {
            sound->SetPosition(GetAircraftNearPosition());
        } else {
            sound->SetPosition(m_pEntity->GetPosition());
        }
    }

    if ((m_AuSettings.IsCar() || m_AuSettings.IsMotorcycle()) && sound == m_EngineSounds[4].m_pSound) {
        m_ACPlayPositionLastFrame = m_ACPlayPositionThisFrame;
        m_ACPlayPositionThisFrame       = curPlayPos;
    }
}

// 0x4F6420
void CAEVehicleAudioEntity::AddAudioEvent(eAudioEvents event, float fVolume) {
    plugin::CallMethod<0x4F6420, CAEVehicleAudioEntity*, eAudioEvents, float>(this, event, fVolume);
}

// 0x4F7580
void CAEVehicleAudioEntity::AddAudioEvent(eAudioEvents event, CVehicle* vehicle) {
    plugin::CallMethod<0x4F7580, CAEVehicleAudioEntity*, eAudioEvents, CVehicle*>(this, event, vehicle);
}

// 0x502280
void CAEVehicleAudioEntity::Service() {
    plugin::CallMethod<0x502280, CAEVehicleAudioEntity*>(this);
}

void CAEVehicleAudioEntity::StaticService() {
    // NOP
}

// 0x4F96A0
CVector CAEVehicleAudioEntity::GetAircraftNearPosition() {
    static const int16 snHeliAudioComponent = tComponent::COMPONENT_WING_RR;

    if (m_AuSettings.IsHeli()) {
        CVector pos;
        m_pEntity->AsVehicle()->GetComponentWorldPosition(snHeliAudioComponent, pos);
        return pos;
    } else {
        return m_pEntity->GetPosition();
    }
}

// 0x4F6150
float CAEVehicleAudioEntity::GetFlyingMetalVolume(CPhysical* physical) {
    const auto speed = physical->m_vecTurnSpeed.SquaredMagnitude();
    if (speed >= 0.5625F) {
        return CAEAudioUtility::AudioLog10(1.0F) * 10.0F;
    }

    const auto fVol = speed / 0.5625F;
    if (fVol < 1.0e-10F) {
        return -100.0F;
    }

    return CAEAudioUtility::AudioLog10(fVol) * 10.0F;
}

// 0x4FA630
void CAEVehicleAudioEntity::PlayTrainBrakeSound(int16 soundType, float speed, float volume) {
    plugin::CallMethod<0x4FA630, CAEAudioEntity*, int16, float, float>(this, soundType, speed, volume);
}

// 0x4F4F00
int16 CAEVehicleAudioEntity::GetVehicleTypeForAudio() {
    if (!m_IsInitialized || !m_pEntity) {
        return 2;
    }

    switch (m_AuSettings.VehicleAudioTypeForName) {
    case 0:
    case 1:
    case 2:
    case 4:
    case 5:
    case 9:
    case 10:
    case 14:
    case 22:
    case 23:
    case 31:
    case 38:
        return 0;
    case 25:
    case 29:
    case 30:
        return 1;
    default:
        return 2;
    }
}

// 0x4F4F70
bool CAEVehicleAudioEntity::IsAccInhibited(cVehicleParams& params) const {
    return !AEAudioHardware.IsSoundBankLoaded(m_PlayerEngineBank, 40)
        || params.ThisBrake > 0
        || params.IsHandbrakeOn || IsAccInhibitedBackwards(params);
}

// inlined?
// 0x4F4FC0
bool CAEVehicleAudioEntity::IsAccInhibitedBackwards(cVehicleParams& params) const {
    return params.WheelSpin > 5.0f || !params.NumDriveWheelsOnGround;
}

// 0x4F4FF0
bool CAEVehicleAudioEntity::IsAccInhibitedForLowSpeed(cVehicleParams& params) const {
    return !m_IsSingleGear && params.Speed < 0.1f; // 0.1f is static
}

// inlined?
// 0x4F5020
bool CAEVehicleAudioEntity::IsAccInhibitedForTime() const {
    return m_TimeBeforeAllowAccelerate < CTimer::GetTimeInMS();
}

// [ANDROID IDB] used in CAEVehicleAudioEntity::ProcessPlayerVehicleEngine
// 0x4F5030
void CAEVehicleAudioEntity::InhibitAccForTime(uint32 time) {
    m_TimeBeforeAllowAccelerate = time + CTimer::GetTimeInMS();
}

// 0x4F5060
void CAEVehicleAudioEntity::InhibitCrzForTime(uint32 time) {
    m_TimeBeforeAllowCruise = time + CTimer::GetTimeInMS();
}

// inlined?
// 0x4F5050
bool CAEVehicleAudioEntity::IsCrzInhibitedForTime() const {
    return m_TimeBeforeAllowCruise < CTimer::GetTimeInMS();
}

// 0x4F5080
void CAEVehicleAudioEntity::GetAccelAndBrake(cVehicleParams& params) {
    return plugin::CallMethod<0x4F5080, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);

    if (CReplay::Mode == MODE_PLAYBACK) {
        params.ThisAccel = (int16)(std::clamp(params.Vehicle->m_fGasPedal, 0.0f, 1.0f) * 255.0f);
        params.ThisBrake = (int16)(std::clamp(params.Vehicle->m_fBreakPedal, 0.0f, 1.0f) * 255.0f);
        return;
    }

    if (s_pPlayerDriver
        && !m_IsPlayerDriverAboutToExit
        && s_pPlayerDriver->m_nPedState != PEDSTATE_ARRESTED
        && s_pPlayerDriver->IsAlive()) {
        CPad* pad        = s_pPlayerDriver->AsPlayer()->GetPadFromPlayer();
        params.ThisAccel = pad->GetAccelerate();
        params.ThisBrake = pad->GetBrake();
        return;
    }

    params.ThisAccel = 0;
    params.ThisBrake = 0;
}

// 0x4F51F0
float CAEVehicleAudioEntity::GetVolumeForDummyIdle(float fGearRevProgress, float fRevProgressBaseline) {
    return plugin::CallMethodAndReturn<float, 0x4F51F0, CAEVehicleAudioEntity*, float, float>(this, fGearRevProgress, fRevProgressBaseline);

    if (m_pEntity->m_nModelIndex == MODEL_CADDY) {
        return -3.0f - 30.0f;
    }

    auto*      vehicle               = m_pEntity->AsVehicle();
    const auto fGearRevToMaxProgress = std::clamp<float>(fGearRevProgress / 0.2f, 0.0f, 1.0f);

    float volume = 3.0f * fGearRevToMaxProgress - 3.0f;
    if (vehicle->vehicleFlags.bIsDrowning) {
        volume -= 6.0f;
    }

    if (vehicle->m_pTrailer) {
        volume += 6.0f;
    }

    volume += m_AuSettings.EngineVolumeOffset;

    if (m_State == 2) {
        static float afVolumePoints[5][2] = {
  // 0x8CC0C4
            {0.0000f,  1.000f},
            { 0.3000f, 1.000f},
            { 0.5000f, 1.000f},
            { 0.7000f, 0.707f},
            { 1.0001f, 0.000f}
        };
        const auto p = CAEAudioUtility::GetPiecewiseLinear(fRevProgressBaseline, (int16)std::size(afVolumePoints), afVolumePoints);
        volume += CAEAudioUtility::AudioLog10(p) * 20.0f;
    } else if (fRevProgressBaseline <= 0.99f) {
        volume += CAEAudioUtility::AudioLog10(fRevProgressBaseline) * 10.0f;
    }
    return volume;
}

// 0x4F5310
float CAEVehicleAudioEntity::GetFrequencyForDummyIdle(float fGearRevProgress, float fRevProgressBaseline) {
    return plugin::CallMethodAndReturn<float, 0x4F5310, CAEVehicleAudioEntity*, float, float>(this, fGearRevProgress, fRevProgressBaseline);

    if (m_pEntity->m_nModelIndex == MODEL_CADDY) {
        return 1.0f;
    }

    auto*      vehicle               = m_pEntity->AsVehicle();
    const auto fGearRevToMaxProgress = std::clamp<float>(fGearRevProgress / 0.2f, 0.0f, 1.0f);

    float freq = 0.35f * fGearRevToMaxProgress + 0.85f;
    if (vehicle->vehicleFlags.bIsDrowning) {
        freq *= 0.7f;
    }

    if (m_State == 2) {
        static float afFreqPoints[5][2] = {
  // 0x8CC0EC
            {0.0000f,  1.00f},
            { 0.3000f, 1.00f},
            { 0.5000f, 0.85f},
            { 0.7000f, 0.85f},
            { 1.0001f, 0.85f},
        };
        freq *= CAEAudioUtility::GetPiecewiseLinear(fRevProgressBaseline, (int16)std::size(afFreqPoints), afFreqPoints);
    }
    return freq;
}

// 0x4F53D0
float CAEVehicleAudioEntity::GetVolumeForDummyRev(float fRatio, float fFadeRatio) {
    return plugin::CallMethodAndReturn<float, 0x4F53D0, CAEVehicleAudioEntity*, float, float>(this, fRatio, fFadeRatio);

    static float points[][2] = {
  // 0x8CC114
        {0.0000f,  0.000f},
        { 0.3000f, 0.000f},
        { 0.5000f, 0.707f},
        { 0.7000f, 1.000f},
        { 1.0001f, 1.000f},
    };

    auto v4     = (fRatio - 0.15f) / (1.0f - 0.15f);
    auto volume = 4.5f * std::clamp(v4, 0.0f, 1.0f) + -4.5f;

    if (m_State == 2) {
        if (fFadeRatio <= 0.99f) {
            auto p = CAEAudioUtility::GetPiecewiseLinear(fFadeRatio, (int16)std::size(points), points);
            volume += CAEAudioUtility::AudioLog10(p) * 20.0f;
        } else {
            volume += 0.0f;
        }
    } else {
        volume += CAEAudioUtility::AudioLog10(1.0f - fFadeRatio) * 10.0f;
    }

    assert(m_pEntity->IsVehicle());
    if (m_pEntity->AsAutomobile()->vehicleFlags.bIsDrowning) {
        volume -= 6.0f;
    }

    if (m_pEntity->AsAutomobile()->m_pTrailer) {
        volume += 6.0f;
    }

    return volume + m_AuSettings.EngineVolumeOffset;
}

// 0x4F54F0
float CAEVehicleAudioEntity::GetFrequencyForDummyRev(float fRatio, float fFadeRatio) {
    return plugin::CallMethodAndReturn<float, 0x4F54F0, CAEVehicleAudioEntity*, float, float>(this, fRatio, fFadeRatio);

    static float points[][2] = {
  // 0x8CC13C
        {0.0000f,  1.0f},
        { 0.5000f, 1.0f},
        { 0.7000f, 1.2f},
        { 0.8500f, 1.0f},
        { 1.0001f, 1.0f},
    };

    auto v4       = (fRatio - 0.15f) / (1.0f - 0.15f);
    auto baseFreq = (1.5f - 0.9f) * std::clamp(v4, 0.0f, 1.0f) + 0.9f;

    float frequency;
    if (m_State == 2 && fFadeRatio <= 0.99f) {
        frequency = CAEAudioUtility::GetPiecewiseLinear(fFadeRatio, (int16)std::size(points), points);
    } else {
        frequency = 1.0f;
    }

    assert(m_pEntity->IsVehicle());
    if (m_pEntity->AsAutomobile()->vehicleFlags.bIsDrowning) {
        baseFreq *= 0.7f;
    }

    return frequency * baseFreq;
}

// 0x4F55C0
void CAEVehicleAudioEntity::CancelVehicleEngineSound(int16 soundId) {
    return plugin::CallMethod<0x4F55C0, CAEVehicleAudioEntity*, int16>(this, soundId);

    auto&      sound       = m_EngineSounds[soundId].m_pSound;
    const auto soundLength = sound ? sound->m_nSoundLength : -1;

    if (size_t(soundId) > std::size(m_EngineSounds)) {
        return;
    }

    if (sound) {
        sound->SetIndividualEnvironment(SOUND_REQUEST_UPDATES, false);
        sound->StopSound();
        sound = nullptr;
    }

    if (soundId != 4) {
        return;
    }

    m_ACPlayPositionWhenStopped = m_ACPlayPositionThisFrame;
    m_TimeACStopped = CTimer::GetTimeInMS();

    m_ACPlayPositionThisFrame       = -1;
    m_ACPlayPositionLastFrame = -1;

    if (soundLength <= 0) {
        m_ACPlayPercentWhenStopped = 0;
    } else {
        auto v7   = std::clamp(float(m_ACPlayPositionWhenStopped) / (float)soundLength, 0.0f, 1.0f);
        m_ACPlayPercentWhenStopped = int16(v7 * 100.0f);
    }
}

// 0x4F56D0
void CAEVehicleAudioEntity::UpdateVehicleEngineSound(int16 engineState, float speed, float volume) {
    if (CAESound* sound = m_EngineSounds[engineState].m_pSound) {
        sound->m_fVolume = m_EventVolume + volume;
        sound->m_fSpeed  = speed;
    }
}

// 0x4F5700
void CAEVehicleAudioEntity::JustGotInVehicleAsDriver() {
    return plugin::CallMethod<0x4F5700, CAEVehicleAudioEntity*>(this);

    s_pPlayerDriver = m_pEntity->AsVehicle()->m_pDriver;

    m_EventVolume = GetDefaultVolume(AE_GENERAL_VEHICLE_SOUND);

    switch (m_AuSettings.VehicleAudioType) {
    case VEHICLE_SOUND_CAR:
    case VEHICLE_SOUND_MOTORCYCLE: {
        m_AuGear         = 0;
        m_CrzCount                    = 0;
        m_ACPlayPositionThisFrame       = -1;
        m_ACPlayPositionLastFrame = -1;

        if (AESoundManager.AreSoundsPlayingInBankSlot(40)) {
            AESoundManager.CancelSoundsInBankSlot(40, false);
        }

        if (m_DummyEngineBank == -1 || m_PlayerEngineBank == -1) {
            return;
        }

        if (m_DummySlot == -1) {
            m_DummySlot = DemandBankSlot(m_DummyEngineBank);
        }

        if (AEAudioHardware.IsSoundBankLoaded(m_PlayerEngineBank, 40)) {
            AEAudioHardware.LoadSoundBank(m_PlayerEngineBank, 40);
        }

        if (m_State != 1 || m_DummySlot == -1) {
            for (auto i = 0u; i < std::size(m_EngineSounds); i++) {
                CancelVehicleEngineSound(i);
            }
            m_State = 0;
        } else {
            for (auto i = 0u; i < std::size(m_EngineSounds); i++) {
                if (i != 2) {
                    CancelVehicleEngineSound(i);
                }
            }
            m_State = 0;
        }
        break;
    }
    case VEHICLE_SOUND_PLANE:
    case VEHICLE_SOUND_HELI:
    case VEHICLE_SOUND_NON_VEH: {
        for (auto i = 0u; i < std::size(m_EngineSounds); i++) {
            CancelVehicleEngineSound(i);
        }

        if (m_PlayerEngineBank != -1 && !AEAudioHardware.IsSoundBankLoaded(m_PlayerEngineBank, 40)) {
            if (AESoundManager.AreSoundsPlayingInBankSlot(40)) {
                AESoundManager.CancelSoundsInBankSlot(40, false);
            }
            AEAudioHardware.LoadSoundBank(m_PlayerEngineBank, 40);
        }

        if (m_DummyEngineBank != -1 && m_DummySlot == -1) {
            m_DummySlot = DemandBankSlot(m_DummyEngineBank);
        }

        m_CurrentRotorFrequency           = -1.0f;
        m_CurrentDummyEngineVolume = -100.0f;
        break;
    }
    case VEHICLE_SOUND_BICYCLE: {
        if (AESoundManager.AreSoundsPlayingInBankSlot(40)) {
            AESoundManager.CancelSoundsInBankSlot(40, false);
        }

        for (auto i = 0u; i < std::size(m_EngineSounds); i++) {
            CancelVehicleEngineSound(i);
        }

        if (m_DummySlot == -1) {
            m_DummySlot = DemandBankSlot(m_DummyEngineBank);
        }

        if (AEAudioHardware.IsSoundBankLoaded(m_DummyEngineBank, 40)) {
            AEAudioHardware.LoadSoundBank(m_DummyEngineBank, 40);
        }
        break;
    }
    case VEHICLE_SOUND_BOAT: {
        if (m_DummySlot == -1) {
            m_DummySlot = DemandBankSlot(m_DummyEngineBank);
        }
        break;
    }
    case VEHICLE_SOUND_TRAIN: {
        if (AESoundManager.AreSoundsPlayingInBankSlot(40)) {
            AESoundManager.CancelSoundsInBankSlot(40, false);
        }

        if (m_DummySlot == -1) {
            m_DummySlot = DemandBankSlot(m_DummyEngineBank);
        }

        if (AEAudioHardware.IsSoundBankLoaded(m_DummyEngineBank, 40)) {
            AEAudioHardware.LoadSoundBank(m_DummyEngineBank, 40);
        }

        break;
    }
    case VEHICLE_SOUND_TRAILER: {
        if (AESoundManager.AreSoundsPlayingInBankSlot(40)) {
            AESoundManager.CancelSoundsInBankSlot(40, false);
        }

        for (auto i = 0u; i < std::size(m_EngineSounds); i++) {
            CancelVehicleEngineSound(i);
        }
        if (m_DummyEngineBank != -1 && m_DummySlot == -1) {
            m_DummySlot = DemandBankSlot(m_DummyEngineBank);
            if (AEAudioHardware.IsSoundBankLoaded(m_DummyEngineBank, 40)) {
                AEAudioHardware.LoadSoundBank(m_DummyEngineBank, 40);
            }
        }
        break;
    }
    }
}

// 0x4F5B20
void CAEVehicleAudioEntity::TurnOnRadioForVehicle() {
    s_pPlayerAttachedForRadio       = m_pEntity->AsVehicle()->m_pDriver;
    s_pVehicleAudioSettingsForRadio = &m_AuSettings;
    switch (m_AuSettings.RadioType) {
    case RADIO_CIVILIAN:
    case RADIO_SPECIAL:
    case RADIO_UNKNOWN:
    case RADIO_EMERGENCY:
        AudioEngine.StartRadio(&m_AuSettings);
        break;
    }
}

// 0x4F5B60
void CAEVehicleAudioEntity::TurnOffRadioForVehicle() {
    switch (m_AuSettings.RadioType) {
    case RADIO_CIVILIAN:
    case RADIO_SPECIAL:
    case RADIO_UNKNOWN:
    case RADIO_EMERGENCY:
        AudioEngine.StopRadio(&m_AuSettings, false);
        break;
    }
    s_pVehicleAudioSettingsForRadio = nullptr;
    s_pPlayerAttachedForRadio       = nullptr;
}

// 0x4F5BA0
void CAEVehicleAudioEntity::PlayerAboutToExitVehicleAsDriver() {
    m_IsPlayerDriverAboutToExit = true;
    if (m_IsPlayerDriver) {
        TurnOffRadioForVehicle();
    }
}

// 0x4F5C40
bool CAEVehicleAudioEntity::CopHeli() {
    const auto modelIndex = m_pEntity->m_nModelIndex;
    return modelIndex == MODEL_MAVERICK || modelIndex == MODEL_VCNMAV;
}

// 0x4F5C60
float CAEVehicleAudioEntity::GetFreqForIdle(float fRatio) const {
    static float points[][2] = {
        {0.0000f,  0.00f},
        { 0.0750f, 0.70f},
        { 0.1500f, 1.10f},
        { 0.2500f, 1.25f},
        { 1.0001f, 1.70f},
    };
    return CAEAudioUtility::GetPiecewiseLinear(fRatio, (int16)std::size(points), points);
}

// 0x4F60B0
float CAEVehicleAudioEntity::GetBaseVolumeForBicycleTyre(float fGearVelocityProgress) const {
    static float points[][2] = {
        {0.0000f,  0.00f},
        { 0.1000f, 0.30f},
        { 0.2000f, 0.45f},
        { 0.5000f, 0.85f},
        { 1.0001f, 1.00f},
    };
    return CAEAudioUtility::GetPiecewiseLinear(fGearVelocityProgress, (int16)std::size(points), points);
}

// 0x4F5D00
float CAEVehicleAudioEntity::GetVolForPlayerEngineSound(cVehicleParams& params, int16 gear) {
    return plugin::CallMethodAndReturn<float, 0x4F5D00, CAEVehicleAudioEntity*, cVehicleParams&, int16>(this, params, gear);

    constexpr float fLowSpeedVol = -2.0f;

    float fVolume = 0.0f;
    switch (gear) {
    case 1:
        fVolume = 4.0f * params.RealRevsRatio - 4.0f;
        break;
    case 2: {
        if (params.Vehicle->m_nModelIndex == MODEL_CADDY) {
            fVolume = -30.0f - 3.0f;
        } else {
            fVolume = 1.5f * params.RealRevsRatio - 3.0f;
        }
        break;
    }
    case 3: {
        if (m_IsSingleGear) {
            fVolume = fLowSpeedVol;
        } else {
            fVolume = -0.013333f * m_CrzCount;
        }
        break;
    }
    case 4: {
        if (m_IsSingleGear) {
            fVolume = fLowSpeedVol;
        } else {
            fVolume = 0.4f * float(m_AuGear) + fLowSpeedVol;
        }
        break;
    }
    case 5:
        return GetVolForPlayerEngineSound(params, 2);
    default:
        break;
    }

    CVehicle* vehicle = m_pEntity->AsVehicle();

    fVolume += m_AuSettings.EngineVolumeOffset;
    if (vehicle->vehicleFlags.bIsDrowning) {
        fVolume -= 6.0f;
    }

    if (vehicle->m_pTrailer) {
        fVolume += 6.0f;
    }

    if (m_bNitroOnLastFrame && m_CurrentNitroRatio <= 1.0f && m_CurrentNitroRatio >= 0.0f) {
        fVolume += 3.0f * m_CurrentNitroRatio;
    }

    return fVolume;
}

// 0x4F5E50
bool CAEVehicleAudioEntity::JustFinishedAccelerationLoop() {
    if (m_FramesAgoACLooped < 10) {
        m_FramesAgoACLooped++;
        return false;
    }

    if (m_ACPlayPositionThisFrame < 0) {
        return false;
    }

    if (m_ACPlayPositionLastFrame < 0) {
        return false;
    }

    CAESound* sound = m_EngineSounds[4].m_pSound;
    if (!sound || m_ACPlayPositionThisFrame <= sound->m_nSoundLength - 120) {
        return false;
    }

    m_FramesAgoACLooped = 0;
    return true;
}

// 0x4F5EB0
void CAEVehicleAudioEntity::UpdateGasPedalAudio(CVehicle* vehicle, int32 vehicleType) {
    const float power  = std::fabs(vehicle->m_fGasPedal);
    float&      gasPtr = (vehicleType == VEHICLE_TYPE_BIKE ? vehicle->AsBike()->m_fGasPedalAudioRevs : vehicle->AsAutomobile()->m_fGasPedalAudio);

    if (power <= gasPtr) {
        gasPtr = std::max(power, gasPtr - 0.07f);
    } else {
        gasPtr = std::min(power, gasPtr + 0.09f);
    }
}

// 0x4F5F30
float CAEVehicleAudioEntity::GetVehicleDriveWheelSkidValue(CVehicle* vehicle, int32 wheelState, float fSomeGasPedalAudioStuff, cTransmission& transmission, float fVelocity) {
    return plugin::CallMethodAndReturn<float, 0x4F5F30, CAEVehicleAudioEntity*, CVehicle*, int32, float, cTransmission&, float>(this, vehicle, wheelState, fSomeGasPedalAudioStuff, transmission, fVelocity);

    if (!s_bVehicleDriveWheelSkidEnabled) {
        return 0.0f;
    }

    switch (wheelState) {
    case WHEEL_STATE_SPINNING: {
        if (fSomeGasPedalAudioStuff > 0.4f) {
            return (fSomeGasPedalAudioStuff - 0.4f) * 1.66f * 0.75f;
        }
        break;
    }
    case WHEEL_STATE_SKIDDING: {
        return std::min(1.0f, std::fabs(fVelocity)) * 0.75f;
    }
    case WHEEL_STATE_FIXED: {
        const float velocity = std::fabs(fVelocity);
        if (velocity > 0.04f) {
            return std::min(velocity * 5.0f, 1.0f) * 1.2f;
        }
        return 0.0f;
    }
    default:
        return 0.0f;
    }
}

// 0x4F6000
float CAEVehicleAudioEntity::GetVehicleNonDriveWheelSkidValue(CVehicle* vehicle, int32 wheelState, cTransmission& transmission, float velocity) {
    return plugin::CallMethodAndReturn<float, 0x4F6000, CAEVehicleAudioEntity*, CVehicle*, int32, cTransmission&, float>(this, vehicle, wheelState, transmission, velocity);

    if (!s_bVehicleNonDriveWheelSkidValueEnabled) {
        return 0.0f;
    }

    switch (wheelState) {
    case WHEEL_STATE_SKIDDING: {
        const float vel = std::min(1.0f, std::fabs(velocity)); // at most 1.0f
        return m_AuSettings.IsBicycle() ? vel * 0.75f * 0.2f : vel * 0.75f;
    }
    case WHEEL_STATE_FIXED: {
        const float vel = std::fabs(velocity);
        return vel > 0.04f ? std::min(vel * 5.0f, 1.0f) * 1.2f : 0.0f;
    }
    default:
        return 0.0f;
    }
}

// 0x4F61E0
void CAEVehicleAudioEntity::GetHornState(bool* out, cVehicleParams& params) {
    plugin::CallMethod<0x4F61E0, CAEVehicleAudioEntity*, bool*, cVehicleParams&>(this, out, params);
}

// 0x4F62A0
void CAEVehicleAudioEntity::GetSirenState(bool& bSirenOrAlarm, bool& bHorn, cVehicleParams& params) {
    return plugin::CallMethod<0x4F62A0, CAEVehicleAudioEntity*, bool&, bool&, cVehicleParams&>(this, bSirenOrAlarm, bHorn, params);

    if (m_IsWreckedVehicle || !m_HasSiren) {
        bSirenOrAlarm = false;
        return;
    }

    CVehicle* vehicle = params.Vehicle;
    if (vehicle->vehicleFlags.bSirenOrAlarm == 0u) {
        bSirenOrAlarm = false;
        return;
    }

    bSirenOrAlarm = true;
    if (vehicle->m_nStatus == STATUS_ABANDONED) {
        bSirenOrAlarm = false;
        return;
    }

    if (vehicle->m_nHornCounter && vehicle->m_nModelIndex != MODEL_MRWHOOP) {
        bHorn = true;
        return;
    }
    bHorn = false;
}

// 0x4F6320
void CAEVehicleAudioEntity::StopGenericEngineSound(int16 index) {
    if (CAESound*& sound = m_EngineSounds[index].m_pSound) {
        sound->SetIndividualEnvironment(SOUND_REQUEST_UPDATES, false);
        sound->StopSound();
        sound = nullptr;
    }
}

// 0x4F7A50
void CAEVehicleAudioEntity::RequestNewPlayerCarEngineSound(int16 vehicleSoundId, float speed, float changeSound) {
    plugin::CallMethod<0x4F7A50, CAEVehicleAudioEntity*, int16, float, float>(this, vehicleSoundId, speed, changeSound);
}

// 0x4F7F20
void CAEVehicleAudioEntity::StartVehicleEngineSound(int16 engineState, float engineSoundSpeed, float volume) {
    plugin::CallMethod<0x4F7F20, CAEVehicleAudioEntity*, int16, float, float>(this, engineState, engineSoundSpeed, volume);
}

// 0x4F8070
float CAEVehicleAudioEntity::GetFreqForPlayerEngineSound(cVehicleParams& params, int16 engineStateMAYBE) {
    return plugin::CallMethodAndReturn<float, 0x4F8070, CAEVehicleAudioEntity*, cVehicleParams&, int32>(this, params, engineStateMAYBE);
}

// 0x4F8360
void CAEVehicleAudioEntity::PlaySkidSound(int16 newSkidSoundType, float speed, float volume) {
    const float vol = m_EventVolume + volume;
    if (m_SurfaceSoundType == newSkidSoundType) {
        if (m_SurfaceSoundType != -1) {
            m_SkidSound.UpdateTwinLoopSound(m_pEntity->GetPosition(), vol, speed);
        }
    } else {
        if (m_SkidSound.m_bIsInitialised) {
            m_SkidSound.StopSoundAndForget();
        }
        m_SurfaceSoundType = newSkidSoundType;
        if (newSkidSoundType != -1) {
            m_SkidSound.Initialise(19, newSkidSoundType, newSkidSoundType + 1, this, 200, 1'000, -1, -1);
            const auto& posn = m_pEntity->GetPosition();
            m_SkidSound.PlayTwinLoopSound(posn, vol, speed, 2.5f, 1.0f, SOUND_DEFAULT);
        }
    }
}

// 0x4F84D0
void CAEVehicleAudioEntity::PlayRoadNoiseSound(int16 newRoadNoiseSoundType, float speed, float volumeDelta) {
    return plugin::CallMethod<0x4F84D0, CAEVehicleAudioEntity*, int16, float, float>(this, newRoadNoiseSoundType, speed, volumeDelta);

    const float volume = m_EventVolume + volumeDelta;
    if (m_RoadNoiseSoundType != newRoadNoiseSoundType) {
        if (m_RoadNoiseSound) {
            m_RoadNoiseSound->SetIndividualEnvironment(SOUND_REQUEST_UPDATES, false);
            m_RoadNoiseSound->StopSound();
            m_RoadNoiseSound = nullptr;
        }

        // Create new sound
        m_RoadNoiseSoundType = newRoadNoiseSoundType;
        if (newRoadNoiseSoundType != -1) {
            const auto& pos = m_pEntity->GetPosition();
            CAESound    sound;
            sound.Initialise(19, newRoadNoiseSoundType, this, pos, volume);
            sound.m_fSpeed         = speed;
            sound.m_fSoundDistance = 3.0f;
            m_RoadNoiseSound       = AESoundManager.RequestNewSound(&sound);
        }
    } else if (m_RoadNoiseSoundType != -1 && m_RoadNoiseSound) {
        // Same sound type already initialised, just set speed and volume
        m_RoadNoiseSound->m_fSpeed  = speed;
        m_RoadNoiseSound->m_fVolume = volume;
    }
}

// 0x4F8650
void CAEVehicleAudioEntity::PlayFlatTyreSound(int16 newFlatTyreSoundType, float speed, float volumeDelta) {
    return plugin::CallMethod<0x4F8650, CAEVehicleAudioEntity*, int16, float, float>(this, newFlatTyreSoundType, speed, volumeDelta);

    const float volume = m_EventVolume + volumeDelta;
    if (m_FlatTireSoundType != newFlatTyreSoundType) {
        if (m_FlatTireSound) {
            m_FlatTireSound->SetIndividualEnvironment(4, false);
            m_FlatTireSound->StopSound();
            m_FlatTireSound = nullptr;
        }

        // Create new sound
        m_FlatTireSoundType = newFlatTyreSoundType;
        if (newFlatTyreSoundType != -1) {
            const auto& pos = m_pEntity->GetPosition();
            CAESound    sound;
            sound.Initialise(19, newFlatTyreSoundType, this, pos, volume);
            sound.m_fSpeed         = speed;
            sound.m_fSoundDistance = 2.0f;
            m_FlatTireSound        = AESoundManager.RequestNewSound(&sound);
        }
    } else if (m_FlatTireSoundType != -1 && m_FlatTireSound) {
        // Same sound type already initialised, just set speed and volume
        m_FlatTireSound->m_fSpeed  = speed;
        m_FlatTireSound->m_fVolume = volume;
    }
}

// 0x4F87D0
void CAEVehicleAudioEntity::PlayReverseSound(int16 newReverseGearSoundType, float speed, float volumeDelta) {
    return plugin::CallMethod<0x4F87D0, CAEVehicleAudioEntity*, int16, float, float>(this, newReverseGearSoundType, speed, volumeDelta);

    const float volume = m_EventVolume + volumeDelta;
    if (m_ReverseGearSoundType != newReverseGearSoundType) {
        if (m_ReverseGearSound) {
            m_ReverseGearSound->SetIndividualEnvironment(4, false);
            m_ReverseGearSound->StopSound();
            m_ReverseGearSound = nullptr;
        }

        // Create new sound
        m_ReverseGearSoundType = newReverseGearSoundType;
        if (newReverseGearSoundType != -1) {
            CAESound sound;
            sound.Initialise(19, newReverseGearSoundType, this, m_pEntity->GetPosition(), volume);
            sound.m_fSpeed     = speed;
            m_ReverseGearSound = AESoundManager.RequestNewSound(&sound);
        }
    } else if (m_ReverseGearSoundType != -1 && m_ReverseGearSound) {
        // Same sound type already initialised, just set speed and volume
        m_ReverseGearSound->m_fSpeed  = speed;
        m_ReverseGearSound->m_fVolume = volume;
    }
}

// 0x4F9E90
void CAEVehicleAudioEntity::UpdateBoatSound(int16 engineState, int16 bankSlotId, int16 sfxId, float speed, float volume) {
    return plugin::CallMethod<0x4F9E90, CAEVehicleAudioEntity*, int16, int16, int16, float, float>(this, engineState, bankSlotId, sfxId, speed, volume);

    if (m_AuSettings.VehicleAudioType != VEHICLE_SOUND_NON_VEH) {
        if (engineState != 6 && m_DummySlot == -1) {
            return;
        }
    }

    const float fVolume = m_EventVolume + volume;
    if (UpdateGenericEngineSound(engineState, fVolume, speed)) {
        return;
    }

    const auto DoPlaySound = [&](float fSoundDistance) {
        PlayGenericEngineSound(engineState, bankSlotId, sfxId, fVolume, speed, fSoundDistance);
    };

    switch (engineState) {
    case 2: {
        if (AEAudioHardware.IsSoundBankLoaded(m_DummyEngineBank, m_DummySlot)) {
            DoPlaySound(3.5f);
        }
        break;
    }
    case 3: {
        if (AEAudioHardware.IsSoundBankLoaded(138u, 19)) {
            DoPlaySound(5.0f);
        }
        break;
    }
    case 6: {
        if (AEAudioHardware.IsSoundBankLoaded(39u, 2)) {
            DoPlaySound(5.0f);
        }
        break;
    }
    default:
        break;
    }
}

// 0x4FA1C0
void CAEVehicleAudioEntity::UpdateTrainSound(int16 engineState, int16 bankSlotId, int16 sfxId, float speed, float volume) {
    return plugin::CallMethod<0x4FA1C0, CAEVehicleAudioEntity*, int16, int16, int16, float, float>(this, engineState, bankSlotId, sfxId, speed, volume);

    if (m_DummySlot == -1) {
        return;
    }

    const float fVolume = m_EventVolume + volume;

    if (UpdateGenericEngineSound(engineState, fVolume, speed)) {
        return;
    }

    const auto DoPlaySound = [&](float fSoundDistance) {
        PlayGenericEngineSound(engineState, bankSlotId, sfxId, fVolume, speed, fSoundDistance);
    };

    switch (engineState) {
    case 1: {
        if (AEAudioHardware.IsSoundBankLoaded(m_DummyEngineBank, m_DummySlot)) {
            DoPlaySound(m_DummyEngineBank == 133 ? 4.0f : 4.5f);
        }
        break;
    }
    case 2: {
        if (AEAudioHardware.IsSoundBankLoaded(m_DummyEngineBank, m_DummySlot)) {
            DoPlaySound(m_DummyEngineBank == 133 ? 5.0f : 4.5f);
        }
        break;
    }
    case 3: {
        if (AEAudioHardware.IsSoundBankLoaded(39u, 2)) {
            DoPlaySound(4.5f);
        }
        break;
    }
    default:
        break;
    }
}

// 0x4F93C0
void CAEVehicleAudioEntity::PlayAircraftSound(int16 engineState, int16 bankSlotId, int16 sfxId, float volume, float speed) {
    return plugin::CallMethod<0x4F93C0, CAEVehicleAudioEntity*, int16, int16, int16, float, float>(this, engineState, bankSlotId, sfxId, volume, speed);

    const float fVolume = m_EventVolume + volume;
    if (CAESound* sound = m_EngineSounds[engineState].m_pSound) {
        sound->m_fVolume = fVolume;
        sound->m_fSpeed  = speed;
        return; // Sound exists, update params, and early out
    }

    // todo (Izzotop): volume 0.0f ?
    const auto DoPlaySound = [&](float fSoundDistance, float fTimeScale = 1.0f) {
        PlayGenericEngineSound(engineState, bankSlotId, sfxId, fVolume, speed, fSoundDistance, fTimeScale);
    };

    switch (engineState) {
    case 1:
        DoPlaySound(6.0f);
        break;
    case 2:
    case 4:
        DoPlaySound(4.0f);
        break;
    case 3:
        DoPlaySound(1.0f, 0.4f);
        break;
    case 5:
        DoPlaySound(4.5f);
        break;
    case 7:
        DoPlaySound(5.0f);
        break;
    default:
        break;
    }
}

// 0x4F9710
void CAEVehicleAudioEntity::PlayBicycleSound(int16 engineState, int16 bankSlotId, int16 sfxId, float volume, float speed) {
    return plugin::CallMethod<0x4F9710, CAEVehicleAudioEntity*, int16, int16, int16, float, float>(this, engineState, bankSlotId, sfxId, volume, speed);

    if (m_DummySlot == -1) {
        return;
    }

    if (engineState == 3) {
        if (m_EngineSounds[3].m_pSound) {
            return;
        }
        if (!AEAudioHardware.IsSoundBankLoaded(39, 2)) {
            return;
        }

        static constexpr struct { // 0x8CC1D8
            int16 sfxId, playPos;
        } RandomArray[] = {
            {5,  38},
            { 7, 25},
            { 8, 25},
            { 9, 50}
        };

        const auto& toPlay = CGeneral::RandomChoice(RandomArray);
        PlayGenericEngineSound(
            engineState,
            2,
            toPlay.sfxId,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            eSoundEnvironment(SOUND_REQUEST_UPDATES | SOUND_START_PERCENTAGE),
            toPlay.playPos
        );

        return;
    }

    if (UpdateGenericEngineSound(engineState, volume, speed)) {
        return;
    }

    switch (engineState) {
    case 1: {
        if (AEAudioHardware.IsSoundBankLoaded(m_DummyEngineBank, m_DummySlot)) {
            PlayGenericEngineSound(engineState, bankSlotId, sfxId, volume, speed, 2.5f);
        }
        break;
    }
    case 2: {
        if (AEAudioHardware.IsSoundBankLoaded(m_PlayerEngineBank, 40)) {
            PlayGenericEngineSound(engineState, bankSlotId, sfxId, volume, speed, 1.5f);
        }
        break;
    }
    }
}

// 0x4F99D0
void CAEVehicleAudioEntity::PlayHornOrSiren(bool bPlayHornTone, bool bPlaySirenOrAlarm, bool bPlayHorn, cVehicleParams& params) {
    return plugin::CallMethod<0x4F99D0, CAEVehicleAudioEntity*, bool, bool, bool, cVehicleParams&>(this, bPlayHornTone, bPlaySirenOrAlarm, bPlayHorn, params);

    const auto nHornToneSoundInBank = m_AuSettings.HornType;
    const auto DoTryStopHornSound   = [&] {
        if (m_HornSound) {
            m_HornSound->StopSoundAndForget();
            m_HornSound = nullptr;
        }
    };

    if (!m_IsHornOn && bPlayHornTone) {
        const bool bBanksLoaded    = AEAudioHardware.IsSoundBankLoaded(74u, SLOT_HORNS_AND_SIRENS) && AEAudioHardware.IsSoundBankLoaded(138u, 19);
        const auto DoPlayHornSound = [&](float fSpeed) {
            const auto& posn = m_pEntity->GetPosition();
            m_tempSound.Initialise(SLOT_HORNS_AND_SIRENS, nHornToneSoundInBank, this, posn, m_HornVolume, 2.5f, fSpeed, 0.17f, false, SOUND_REQUEST_UPDATES);
            m_HornSound = AESoundManager.RequestNewSound(&m_tempSound);
            return true;
        };

        if (!nHornToneSoundInBank) {
            DoTryStopHornSound();
            m_HornVolume = GetDefaultVolume(AE_BIKE_BELL);
            if (!bBanksLoaded) {
                return;
            }
            DoPlayHornSound(1.0f);
        } else {
            if (m_HornSound) {
                return;
            }

            if (nHornToneSoundInBank <= 0 || nHornToneSoundInBank > 9) {
                return;
            }

            m_HornVolume = m_AuSettings.EngineVolumeOffset + GetDefaultVolume(AE_CAR_HORN);
            if (!bBanksLoaded) {
                return;
            }

            DoPlayHornSound(m_AuSettings.HornPitch);
        }
    } else {
        if (nHornToneSoundInBank == 0) {
            m_HornVolume = std::max(m_HornVolume - 1.5f, -100.0f);
            if (m_HornVolume <= -100.0f) {
                DoTryStopHornSound();
            } else if (m_HornSound) {
                m_HornSound->m_fVolume = m_HornVolume; // Just modify speed
            }
        }
    }

    if (bPlaySirenOrAlarm && !bPlayHorn) {
        if (!m_IsSirenOn || m_IsFastSirenOn) {
            // there was no siren last frame OR
            // horn was playing and siren should've been playing, but couldn't have been
            // because the horn was playing (and not it isn't)

            if (m_SirenSound || !AEAudioHardware.IsSoundBankLoaded(74u, SLOT_HORNS_AND_SIRENS)) {
                return;
            }

            const bool bIsWhoopieModel = params.Vehicle->m_nModelIndex == MODEL_MRWHOOP;
            if (bIsWhoopieModel) {
                if (!AEAudioHardware.IsSoundBankLoaded(79u, 40)) {
                    return;
                }
            }

            m_SirenSound = PlaySound(bIsWhoopieModel ? 40 : SLOT_HORNS_AND_SIRENS, bIsWhoopieModel ? 3 : 10, GetDefaultVolume(AE_SIREN), 1.0f, 3.0f, 0.25f);
        }
    } else if (m_IsSirenOn && !m_IsFastSirenOn && m_SirenSound) {
        m_SirenSound->StopSoundAndForget();
        m_SirenSound = nullptr;
    }

    if (bPlayHorn) {
        if (!m_IsFastSirenOn && !m_FastSirenSound && AEAudioHardware.IsSoundBankLoaded(74u, SLOT_HORNS_AND_SIRENS)) {
            m_FastSirenSound = PlaySound(SLOT_HORNS_AND_SIRENS, 11, GetDefaultVolume(AE_SIREN), 1.0f, 3.0f, 0.25f);
        }
    } else if (m_IsFastSirenOn) {
        if (m_FastSirenSound) {
            m_FastSirenSound->StopSoundAndForget();
            m_FastSirenSound = nullptr;
        }
    }
}

// 0x4FCF40
void CAEVehicleAudioEntity::JustGotOutOfVehicleAsDriver() {
    return plugin::CallMethod<0x4FCF40, CAEVehicleAudioEntity*>(this);

    m_IsPlayerDriverAboutToExit = false;
    switch (m_AuSettings.VehicleAudioType) {
    case VEHICLE_SOUND_CAR:
    case VEHICLE_SOUND_MOTORCYCLE: {
        m_EventVolume = GetDefaultVolume(AE_GENERAL_VEHICLE_SOUND) - 1.5f;
        m_FadeIn                    = 1.0f;
        m_FadeOut                    = 1.0f;

        if (m_State == 6) {
            for (auto i = 0u; i < std::size(m_EngineSounds); i++) {
                if (i != 2) {
                    CancelVehicleEngineSound(i);
                }
            }
            m_State = 1;
        } else {
            for (auto i = 0u; i < std::size(m_EngineSounds); i++) {
                CancelVehicleEngineSound(i);
            }
            if (m_State != 0) {
                m_State = 10;
            }
        }
        PlayReverseSound(-1);
        m_AuGear = 0;
        s_pPlayerDriver     = nullptr;
        break;
    }
    case VEHICLE_SOUND_TRAIN: {
        PlayTrainBrakeSound(-1);
        s_pPlayerDriver = nullptr;
        break;
    }
    case VEHICLE_SOUND_TRAILER: {
        m_EventVolume = GetDefaultVolume(AE_GENERAL_VEHICLE_SOUND) - 1.5f;
        s_pPlayerDriver              = nullptr;
        break;
    }
    case VEHICLE_SOUND_PLANE:
    case VEHICLE_SOUND_HELI:
    case VEHICLE_SOUND_NON_VEH: {
        for (auto i = 0u; i < std::size(m_EngineSounds); i++) {
            CancelVehicleEngineSound(i);
        }
        m_CurrentRotorFrequency           = -1.0f;
        m_CurrentDummyEngineVolume = -100.0f;
        break;
    }
    case VEHICLE_SOUND_BICYCLE: {
        s_pPlayerDriver = nullptr;
        break;
    }
    }
}

// 0x4FD0B0
void CAEVehicleAudioEntity::JustWreckedVehicle() {
    return plugin::CallMethod<0x4FD0B0, CAEVehicleAudioEntity*>(this);

    if (m_IsPlayerDriver) {
        // FIX_BUGS? Should be only for RADIO_CIVILIAN, RADIO_UNKNOWN, RADIO_EMERGENCY
        TurnOffRadioForVehicle();
    }

    switch (m_AuSettings.VehicleAudioType) {
    case VEHICLE_SOUND_CAR:
    case VEHICLE_SOUND_MOTORCYCLE:
    case VEHICLE_SOUND_BICYCLE:
    case VEHICLE_SOUND_BOAT:
    case VEHICLE_SOUND_HELI:
    case VEHICLE_SOUND_PLANE:
    case VEHICLE_SOUND_NON_VEH:
    case VEHICLE_SOUND_TRAIN:
    case VEHICLE_SOUND_TRAILER: {
        for (auto i = 0u; i < std::size(m_EngineSounds); i++) {
            CancelVehicleEngineSound(i);
        }
        break;
    }
    }

    PlaySkidSound(-1);

    // todo: do better
    // Originally copied and pasted code, but this is way more readable
    struct {
        CAESound** ppSound;
        int16*     pnSoundType;
    } SoundsToStop[] = {
        {&m_SurfaceSound,    &m_SurfaceSoundType       },
        { &m_RoadNoiseSound,   &m_RoadNoiseSoundType  },
        { &m_FlatTireSound,    &m_FlatTireSoundType   },
        { &m_ReverseGearSound, &m_ReverseGearSoundType},
        { &m_HornSound,     nullptr                 },
        { &m_SirenSound,       nullptr                 },
        { &m_FastSirenSound, nullptr                 }
    };

    // Stop sounds, and reset member variables (that's why we use float pointers)
    for (const auto& sounds : SoundsToStop) {
        if (CAESound*& sound = *sounds.ppSound) {
            sound->SetIndividualEnvironment(SOUND_REQUEST_UPDATES, false);
            sound->StopSound();
            sound = nullptr;
            if (*sounds.pnSoundType) {
                *sounds.pnSoundType = -1;
            }
        }
    }
}

// 0x4FAD40
void CAEVehicleAudioEntity::UpdateGenericVehicleSound(int16 engineStateSoundIndex, int16 bankSlotId, int16 gameBank, int16 sfxId, float speed, float volume, float distance) {
    return plugin::CallMethod<0x4FAD40, CAEVehicleAudioEntity*, int16, int16, int16, int16, float, float, float>(this, engineStateSoundIndex, bankSlotId, gameBank, sfxId, speed, volume, distance);

    const float finalVolume = m_EventVolume + volume;
    if (CAESound* sound = m_EngineSounds[engineStateSoundIndex].m_pSound) {
        sound->m_fSpeed  = speed;
        sound->m_fVolume = finalVolume;
    } else if (AEAudioHardware.IsSoundBankLoaded(gameBank, bankSlotId)) {
        m_EngineSounds[engineStateSoundIndex].m_pSound = PlaySound(bankSlotId, sfxId, finalVolume, speed);
    }
}

// 0x4F8940
void CAEVehicleAudioEntity::ProcessVehicleFlatTyre(cVehicleParams& params) {
    plugin::CallMethod<0x4F8940, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4F8B00
void CAEVehicleAudioEntity::ProcessVehicleRoadNoise(cVehicleParams& params) {
    return plugin::CallMethod<0x4F8B00, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);

    CVehicle* vehicle = params.Vehicle;

    const auto CancelRoadNoise = [=] {
        PlayRoadNoiseSound(-1, 0.0f, 0.0f);
    };

    // Check if any wheels touch the ground. (Perhaps params.m_nWheelsOnGround could be used?)
    const auto GetNumContactWheels = [=]() -> uint8 {
        switch (params.BaseVehicleType) {
        case VEHICLE_TYPE_AUTOMOBILE: return vehicle->AsAutomobile()->m_nNumContactWheels;
        case VEHICLE_TYPE_BIKE: return vehicle->AsBike()->m_nNoOfContactWheels;
        default: return 4;
        }
    };

    switch (vehicle->m_nModelIndex) {
    case MODEL_ARTICT1:
    case MODEL_ARTICT2:
    case MODEL_PETROTR:
    case MODEL_ARTICT3:
        if (!GetNumContactWheels()) {
            CancelRoadNoise();
        }
        break;
    default: {
        if (!params.Transmission) {
            CancelRoadNoise();
            return;
        }
    }
    }

    const float velocity = std::fabs(params.Speed);
    if (velocity <= 0.0f) {
        CancelRoadNoise();
        return;
    }

    float fSpeed = 1.0f;
    {
        const auto& camPos = TheCamera.GetPosition();
        const auto& vehPos = m_pEntity->GetPosition();

        const float someDistance     = 72.0f; // 0x8CBD10
        const float distanceProgress = (vehPos - camPos).Magnitude() / someDistance;

        fSpeed = 0.75f + std::max(1.0f, distanceProgress) / 2.0f;
    }

    int16 nRoadNoiseSound = -1;
    float fVolumeBase     = -12.0f;
    if (IsSurfaceAudioGrass(vehicle->m_nContactSurface)) {
        fSpeed *= 1.3f;
        fVolumeBase += 0.0f; // 0xB6B9E4
        nRoadNoiseSound = 21;
    } else if (IsSurfaceAudioEitherGravelWaterSand(vehicle->m_nContactSurface)) {
        fVolumeBase += 4.5f;
        nRoadNoiseSound = 22;
    }

    const float logarithmicVolume = std::min(1.0f, 2.0f * velocity);
    float       fVolume           = -100.0f;
    if (logarithmicVolume > 0.0008f) {
        fVolume = fVolumeBase + CAEAudioUtility::AudioLog10(logarithmicVolume) * 20.0f;
    }

    // Playing sound with -100.0f volume, in case logarithmicVolume <= 0.0008f, doesn't make much sense..
    PlayRoadNoiseSound(nRoadNoiseSound, fSpeed, fVolume);
}

// 0x4F8DF0
void CAEVehicleAudioEntity::ProcessReverseGear(cVehicleParams& params) {
    return plugin::CallMethod<0x4F8DF0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);

    static constexpr float BASE_SPEED  = +0.75f; // 0x8CBD24
    static constexpr float SPEED_MULT  = +0.20f; // 0x8CBD28
    static constexpr float BASE_VOLUME = -6.00f; // 0x8CBD28

    const auto vehicle = params.Vehicle->AsAutomobile();
    if (vehicle->vehicleFlags.bEngineOn && (vehicle->m_fGasPedal < 0.0f || !vehicle->m_nCurrentGear)) { // Check if we are reversing

        float fReverseGearVelocityProgress = 0.0f;
        if (vehicle->m_nWheelsOnGround) {
            fReverseGearVelocityProgress = params.Speed / params.Transmission->m_maxReverseGearVelocity;
        } else {
            if (vehicle->m_wheelsOnGrounPrev) {
                vehicle->m_fGasPedalAudio *= 0.4f;
            }
            fReverseGearVelocityProgress = vehicle->m_fGasPedalAudio;
        }
        fReverseGearVelocityProgress = std::fabs(fReverseGearVelocityProgress);

        PlayReverseSound(
            vehicle->m_fGasPedal >= 0.0f ? 20 : 19,                 // soundType
            BASE_SPEED + fReverseGearVelocityProgress * SPEED_MULT, // speed
            fReverseGearVelocityProgress > 0.0f                     // volume
                ? BASE_VOLUME + CAEAudioUtility::AudioLog10(fReverseGearVelocityProgress) * 20.0f
                : -100.0f
        );
    } else { // Cancel sound otherwise
        PlayReverseSound(-1, 0.0f, 0.0f);
    }
}

// 0x4F8F10
void CAEVehicleAudioEntity::ProcessVehicleSkidding(cVehicleParams& params) {
    return plugin::CallMethod<0x4F8F10, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);

    tWheelState* wheelStates  = nullptr;
    float*       aWheelTimers = nullptr;

    bool  bAreRearWheelsNotSkidding = false;
    float fUnk                      = 0.0f;
    auto  nWheels                   = 0;

    switch (params.BaseVehicleType) {
    case VEHICLE_TYPE_AUTOMOBILE: {
        nWheels = 4;

        auto vehicle = params.Vehicle->AsAutomobile();
        fUnk         = vehicle->m_fGasPedalAudio;

        bAreRearWheelsNotSkidding = vehicle->m_aWheelState[CAR_WHEEL_REAR_LEFT] != WHEEL_STATE_SKIDDING && vehicle->m_aWheelState[CAR_WHEEL_REAR_RIGHT] != WHEEL_STATE_SKIDDING;
        // todo: aWheelTimers = vehicle->m_anWheelTimer;
        // todo: wheelStates = vehicle->m_anWheelState;
        break;
    }
    case VEHICLE_TYPE_BIKE: {
        nWheels = 2;

        auto* bike                = params.Vehicle->AsBike();
        fUnk                      = bike->m_fGasPedalAudioRevs;
        bAreRearWheelsNotSkidding = bike->m_aWheelState[1] != WHEEL_STATE_SKIDDING;
        wheelStates               = bike->m_aWheelState;
        // todo: aWheelTimers = bike->m_wheelCollisionState;
        break;
    }
    default:
        return;
    }

    // Calculate skid values sum of all wheels
    float fTotalSkidValue = 0.0f;
    for (auto i = 0; i < nWheels; i++) {
        const bool        bIsFrontWheel  = i == CAR_WHEEL_FRONT_LEFT || i == CAR_WHEEL_FRONT_RIGHT;
        const tWheelState thisWheelState = wheelStates[i];

        if (thisWheelState == WHEEL_STATE_NORMAL) {
            continue;
        }
        if (aWheelTimers[i] == 0.0f) {
            continue;
        }
        if (bAreRearWheelsNotSkidding && bIsFrontWheel && thisWheelState == WHEEL_STATE_SKIDDING) {
            continue;
        }

        const auto dt = params.Transmission->m_nDriveType;
        if (dt == '4' || dt == 'F' && bIsFrontWheel || dt == 'R' && !bIsFrontWheel) {
            fTotalSkidValue += GetVehicleDriveWheelSkidValue(params.Vehicle, thisWheelState, fUnk, *params.Transmission, params.Speed);
        } else {
            fTotalSkidValue += GetVehicleNonDriveWheelSkidValue(params.Vehicle, thisWheelState, *params.Transmission, params.Speed);
        }
    }

    const auto StopSkidSound = [&] {
        PlaySkidSound(-1, 0.0f, 0.0f);
    };
    if (fTotalSkidValue <= 0.0f) {
        StopSkidSound();
        return;
    }

    int16 soundId     = -1;
    float fBaseVolume = 0.0f, fSpeed = 1.0f;

    if (m_AuSettings.VehicleAudioType == VEHICLE_SOUND_BICYCLE) {
        soundId     = 0;
        fBaseVolume = -12.0f;
        fSpeed      = 0.9f + fTotalSkidValue * 0.25f;
    } else {
        if (IsSurfaceAudioGrass(params.Vehicle->m_nContactSurface)) {
            soundId     = 6;
            fBaseVolume = -12.0f;
            fSpeed      = 0.2f * fTotalSkidValue + 0.9f;
        } else if (IsSurfaceAudioEitherGravelWaterSand(params.Vehicle->m_nContactSurface)) {
            soundId     = 8;
            fBaseVolume = -9.0f;
            fSpeed      = 0.2f * fTotalSkidValue + 0.9f;
        } else {
            soundId = 24;
            fSpeed  = 0.125f * fTotalSkidValue + 0.8f;

            if (m_AuSettings.IsMotorcycle()) {
                fSpeed *= 1.2f;
            }
        }

        switch (m_AuSettings.VehicleAudioType) {
        case VEHICLE_SOUND_PLANE:
        case VEHICLE_SOUND_HELI: {
            fBaseVolume += 12.0f;
            break;
        }
        }
    }

    const float fAvgSkidValPerWheel = fTotalSkidValue / (float)nWheels;
    if (fAvgSkidValPerWheel > 0.00001f) {
        const float fVolume = 0.0f + fBaseVolume + CAEAudioUtility::AudioLog10(fAvgSkidValPerWheel) * 20.0f;
        if (fVolume >= -100.0f) {
            PlaySkidSound(soundId, fSpeed, fVolume);
        } else {
            StopSkidSound();
        }
    } else {
        StopSkidSound();
    }
}

// 0x4F92C0
void CAEVehicleAudioEntity::ProcessRainOnVehicle(cVehicleParams& params) {
    if (!AEAudioHardware.IsSoundBankLoaded(BANK_RAIN_SOUNDS, SLOT_RAIN_ON_VEHICLE)) {
        return;
    }

    if (CAEWeatherAudioEntity::m_sfRainVolume <= -100.0f) {
        return;
    }

    if (++m_RainDropCounter < 3) {
        return;
    }

    m_RainDropCounter = 0;

    const auto sfxId  = (int16)CAEAudioUtility::GetRandomNumberInRange(12, 15);
    const auto volume = 0.0f + CAEWeatherAudioEntity::m_sfRainVolume;
    m_tempSound.Initialise(SLOT_RAIN_ON_VEHICLE, sfxId, this, m_pEntity->GetPosition(), volume);
    m_tempSound.m_fSpeedVariability = 0.1f;
    m_tempSound.m_nEvent            = AE_RAIN_COLLISION;
    AESoundManager.RequestNewSound(&m_tempSound);
}

// 0x4FA0C0
void CAEVehicleAudioEntity::ProcessBoatMovingOverWater(cVehicleParams& params) {
    return plugin::CallMethod<0x4FA0C0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);

    auto* boat = params.Vehicle->AsBoat();

    // Originally there was a multiply by 1.33, that's the recp. of 0.75f, which makes sense
    // because the abs. velocity is clamped to 0.75f
    const float fVelocityProgress = std::min(0.75f, std::fabs(params.Speed)) / 0.75f;

    float fVolume = -100.0f;
    if (boat->m_nBoatFlags.bOnWater && fVelocityProgress >= 0.00001f) {
        fVolume = CAEAudioUtility::AudioLog10(fVelocityProgress) * 20.0f;
        fVolume += (m_AuSettings.VehicleAudioType == VEHICLE_SOUND_NON_VEH) ? 12.0f : 3.0f;
    }

    float fSpeed = 0.8f + fVelocityProgress * 0.2f;
    if (CWeather::UnderWaterness >= 0.5f) {
        fSpeed *= 0.185f;
        fVolume += 6.0f;
    }

    UpdateBoatSound(6, 2, 3, fSpeed, fVolume);
}

// 0x4FA3F0
void CAEVehicleAudioEntity::ProcessTrainTrackSound(cVehicleParams& params) {
    plugin::CallMethod<0x4FA3F0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// Android
void CAEVehicleAudioEntity::ProcessPlayerTrainEngine(cVehicleParams& params) {
    // NOP
}

// 0x4FA7C0
void CAEVehicleAudioEntity::ProcessDummyRCPlane(cVehicleParams& params) {
    plugin::CallMethod<0x4FA7C0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// Android
void CAEVehicleAudioEntity::ProcessPlayerRCPlane(cVehicleParams& params) {
    // NOP
}

// 0x4FAA80
void CAEVehicleAudioEntity::ProcessDummyRCHeli(cVehicleParams& params) {
    plugin::CallMethod<0x4FAA80, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// Android
void CAEVehicleAudioEntity::ProcessPlayerRCHeli(cVehicleParams& params) {
    // NOP
}

// Android
void CAEVehicleAudioEntity::ProcessPlayerRCCar(cVehicleParams& params) {
    // NOP
}

// Android
void CAEVehicleAudioEntity::ProcessPlayerHovercraft(cVehicleParams& params) {
    // NOP
}

// Android
void CAEVehicleAudioEntity::ProcessPlayerGolfCart(cVehicleParams& params) {
    // NOP
}

// 0x4FAE20
void CAEVehicleAudioEntity::ProcessEngineDamage(cVehicleParams& params) {
    plugin::CallMethod<0x4FAE20, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4FB070
void CAEVehicleAudioEntity::ProcessNitro(cVehicleParams& params) {
    plugin::CallMethod<0x4FB070, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4FB260
void CAEVehicleAudioEntity::ProcessMovingParts(cVehicleParams& params) {
    return plugin::CallMethod<0x4FB260, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);

    switch (params.Vehicle->m_nModelIndex) {
    case MODEL_PACKER:
    case MODEL_DOZER:
    case MODEL_DUMPER:

        // Note: Rockstar originally may have wanted it to have sound,
        // but they've only left it in the first `if` check, it has no `case` in the `switch` below.
        // case MODEL_CEMENT:

    case MODEL_ANDROM:
    case MODEL_FORKLIFT:
        break;
    default:
        return;
    }

    auto* vehicle = params.Vehicle->AsAutomobile();

    float fComponentMoveProgress = (float)(vehicle->m_wMiscComponentAngle - vehicle->m_wMiscComponentAnglePrev) / 30.0f;
    fComponentMoveProgress       = std::clamp<float>(std::fabs(fComponentMoveProgress), 0.0f, 1.0f);
    if (fComponentMoveProgress <= m_MovingPartSmoothedSpeed) {
        fComponentMoveProgress = std::max(m_MovingPartSmoothedSpeed - 0.2f, fComponentMoveProgress);
    } else {
        fComponentMoveProgress = std::min(m_MovingPartSmoothedSpeed + 0.2f, fComponentMoveProgress);
    }
    m_MovingPartSmoothedSpeed = fComponentMoveProgress;

    float fSpeed = 1.0f, fVolume = 0.0f;
    int16 bankSlot = -1, bank = -1, sfxId = -1;

    switch (vehicle->m_nModelIndex) {
    case MODEL_DUMPER: {
        bankSlot = 19;
        bank     = 138;
        sfxId    = 15;
        if (fComponentMoveProgress <= 0.0f) {
            fSpeed  = 0.9f;
            fVolume = 14.0f;
        } else {
            fSpeed  = 1.1f;
            fVolume = 20.f;
        }
        break;
    }
    case MODEL_PACKER: {
        bankSlot = 19;
        bank     = 138;
        sfxId    = 15;
        if (fComponentMoveProgress <= 0.0f) {
            fSpeed  = 0.8f;
            fVolume = 3.0f;
        } else {
            fSpeed  = 1.0f;
            fVolume = 9.0f;
        }
        break;
    }
    case MODEL_DOZER: {
        bankSlot = 19;
        bank     = 138;
        sfxId    = 15;
        if (fComponentMoveProgress <= 0.0f) {
            fSpeed  = 0.9f;
            fVolume = 2.0f;
        } else {
            fSpeed  = 1.1f;
            fVolume = 6.0f;
        }
        break;
    }
    case MODEL_FORKLIFT: {
        bankSlot = m_DummySlot;
        bank     = 57;
        sfxId    = 2;
        if (fComponentMoveProgress <= 0.0f) {
            fSpeed  = 0.8f;
            fVolume = -18.0f;
        } else {
            fSpeed  = 1.0f;
            fVolume = -6.0f;
        }
        break;
    }
    case MODEL_ANDROM: {
        bankSlot = 19;
        bank     = 138;
        sfxId    = 15;
        if (fComponentMoveProgress <= 0.0f) {
            fSpeed  = 0.8f;
            fVolume = 21.0f;
        } else {
            fSpeed  = 1.0f;
            fVolume = 24.0f;
        }
        break;
    }
    }

    fVolume += CAEAudioUtility::AudioLog10(m_MovingPartSmoothedSpeed) * 20.0f;
    if (fVolume <= -100.0f) {
        if (auto& sound = m_EngineSounds[11].m_pSound) {
            sound->SetIndividualEnvironment(4, 0);
            sound->StopSound();
            sound = nullptr;
        }
    } else {
        UpdateGenericVehicleSound(11, bankSlot, bank, sfxId, fSpeed, fVolume, 1.5f);
    }
}

// 0x4FBB10
void CAEVehicleAudioEntity::ProcessPlayerVehicleEngine(cVehicleParams& params) {
    plugin::CallMethod<0x4FBB10, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4FCA10
void CAEVehicleAudioEntity::ProcessDummyStateTransition(int16 newState, float fRatio, cVehicleParams& params) {
    plugin::CallMethod<0x4FCA10, CAEVehicleAudioEntity*, int16, float, cVehicleParams&>(this, newState, fRatio, params);
}


// 0x4FDFD0
void CAEVehicleAudioEntity::ProcessAIProp(cVehicleParams& params) {
    plugin::CallMethod<0x4FDFD0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4FE420
void CAEVehicleAudioEntity::ProcessPlayerHeli(cVehicleParams& params) {
    plugin::CallMethod<0x4FE420, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4FE940
void CAEVehicleAudioEntity::ProcessDummyHeli(cVehicleParams& params) {
    plugin::CallMethod<0x4FE940, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4FEE20
void CAEVehicleAudioEntity::ProcessAIHeli(cVehicleParams& params) {
    plugin::CallMethod<0x4FEE20, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4FF320
void CAEVehicleAudioEntity::ProcessPlayerSeaPlane(cVehicleParams& params) {
    plugin::CallMethod<0x4FF320, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4FF7C0
void CAEVehicleAudioEntity::ProcessDummySeaPlane(cVehicleParams& params) {
    plugin::CallMethod<0x4FF7C0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4FF900
void CAEVehicleAudioEntity::ProcessGenericJet(bool bEngineOn, cVehicleParams& params, float fEngineSpeed, float fAccelRatio, float fBrakeRatio, float fStalledVolume, float fStalledFrequency) {
    plugin::CallMethod<0x4FF900, CAEVehicleAudioEntity*, uint8, cVehicleParams&, float, float, float, float, float>(this, bEngineOn, params, fEngineSpeed, fAccelRatio, fBrakeRatio, fStalledVolume, fStalledFrequency);
}

// 0x4FFDC0
void CAEVehicleAudioEntity::ProcessDummyBicycle(cVehicleParams& params) {
    plugin::CallMethod<0x4FFDC0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x500040
void CAEVehicleAudioEntity::ProcessPlayerBicycle(cVehicleParams& params) {
    plugin::CallMethod<0x500040, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x5002C0
void CAEVehicleAudioEntity::ProcessVehicleSirenAlarmHorn(cVehicleParams& params) {
    return plugin::CallMethod<0x5002C0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);

    bool bHorn = false, bSirenOrAlarm = false;
    GetSirenState(bSirenOrAlarm, bHorn, params);

    if (!bSirenOrAlarm) {
        CVehicle* vehicle = params.Vehicle;
        if (vehicle->m_nAlarmState == 0 || vehicle->m_nAlarmState == -1 || vehicle->m_nStatus == STATUS_WRECKED) {
            GetHornState(&bHorn, params);
        } else {
            const auto time = CTimer::GetTimeInMS();
            if (time > vehicle->m_nHornCounter) {
                vehicle->m_nHornCounter = time + 750;
            }
            m_IsHornOn = vehicle->m_nHornCounter < time + 750 / 2;
        }
    }

    PlayHornOrSiren(m_IsHornOn, bSirenOrAlarm, bHorn, params);
    m_IsSirenOn = bSirenOrAlarm;
    m_IsFastSirenOn         = bHorn;
}

// 0x5003A0
void CAEVehicleAudioEntity::ProcessBoatEngine(cVehicleParams& params) {
    plugin::CallMethod<0x5003A0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x500710
void CAEVehicleAudioEntity::ProcessDummyTrainEngine(cVehicleParams& params) {
    plugin::CallMethod<0x500710, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x500AB0
void CAEVehicleAudioEntity::ProcessPlayerTrainBrakes(cVehicleParams& params) {
    plugin::CallMethod<0x500AB0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x500CE0
void CAEVehicleAudioEntity::ProcessPlayerCombine(cVehicleParams& params) {
    plugin::CallMethod<0x500CE0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x500DC0
void CAEVehicleAudioEntity::ProcessDummyRCCar(cVehicleParams& params) {
    plugin::CallMethod<0x500DC0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x500F50
void CAEVehicleAudioEntity::ProcessDummyHovercraft(cVehicleParams& params) {
    plugin::CallMethod<0x500F50, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x501270
void CAEVehicleAudioEntity::ProcessDummyGolfCart(cVehicleParams& params) {
    plugin::CallMethod<0x501270, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x501480
void CAEVehicleAudioEntity::ProcessDummyVehicleEngine(cVehicleParams& params) {
    plugin::CallMethod<0x501480, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// copy-paste elimination
float CalculatePlaneEngineSpeed(CPlane* plane, float accel, float brake) {
    const auto t = plane->m_autoPilot.m_vehicleRecordingId >= 0
        ? 0.7f + 0.3f * std::clamp(std::max(accel, brake), 0.f, 1.f)
        : plane->m_fPropSpeed / 0.34f; // * 2.941 ???
    return (std::clamp(t, 0.2f, 1.f) - 0.2f) * 1.25f;
}

// notsa - copy-paste elimination (Used in ProcessPlayerProp, ProcessDummyProp, ProcessPlayerJet)
void CAEVehicleAudioEntity::ProcessPlanePropellerStall(CPlane* plane, float& outVolume, float& outFreq) {
    const auto hp = plane->m_fHealth;

    // NOTE: Why not use piecewise?
    std::tie(outVolume, outFreq) = [&]() -> std::pair<float, float> {
        if (hp >= 650.f) {
            return { 0.f, 1.f };
        }
        if (hp >= 460.f) { // 0x50180B 
            return { -6.f, 0.95f };
        }
        if (hp >= 390.f) { // 0x5017EA
            return { -9.f, 0.90f };
        }
        if (hp >= 250.f) { // 0x5017C2
            return { -12.f, 0.85f };
        }
        return { -18.f, 0.80f }; // 0x50177C
    }();

    if (m_DoCountStalls) {
        m_StalledCount = 15;
    }

    if (hp >= 650.f) {
        return;
    }

    if (CTimer::GetTimeInMS() >= m_SwapStalledTime) {
        constexpr static struct {
            float FastStallProbability;
            struct {
                uint32 Min, Max;
            } Intervals[2];
            auto GetRandomStallTime() const {
                const auto& i = Intervals[CAEAudioUtility::ResolveProbability(FastStallProbability)];
                return CAEAudioUtility::GetRandomNumberInRange(i.Min, i.Max);
            }
        } s_StallTimes[2]{
            { 0.95f, {{ 400, 420 }, { 75, 175 }} }, // Silent
            { 0.85f, {{ 175, 250 }, { 25, 75  }} }, // Non-Silent
        };
        m_SwapStalledTime = CTimer::GetTimeInMS() + s_StallTimes[m_IsSilentStalled].GetRandomStallTime();
        m_IsSilentStalled = !m_IsSilentStalled;
    }
    m_StalledCount = std::max(0, m_StalledCount - 1);
}

// 0x4FD290
void CAEVehicleAudioEntity::ProcessPlayerProp(cVehicleParams& params) {
    plugin::CallMethod<0x4FD290, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x4FD8F0
void CAEVehicleAudioEntity::ProcessDummyProp(cVehicleParams& params) {
    plugin::CallMethod<0x4FD8F0, CAEVehicleAudioEntity*, cVehicleParams&>(this, params);
}

// 0x501650
void CAEVehicleAudioEntity::ProcessPlayerJet(cVehicleParams& vp) {
    auto* const plane = vp.Vehicle->AsPlane();
    const auto isRecordingVehicle = plane->m_autoPilot.m_vehicleRecordingId >= 0;

    if (isRecordingVehicle) {
        vp.ThisAccel = 0;
        vp.ThisBrake = 0;
    } else {
        GetAccelAndBrake(vp);
    }
    const auto accel = vp.ThisAccel * 255.f;
    const auto brake = vp.ThisBrake * 255.f;

    // 0x5016A1
    const auto engineSpeed = std::max(CalculatePlaneEngineSpeed(plane, accel, brake), 0.2f);

    // 0x501746
    float stalledVolume, stalledFreq;
    ProcessPlanePropellerStall(plane, stalledVolume, stalledFreq);

    // 0x501946
    ProcessGenericJet(
        plane->vehicleFlags.bEngineOn,
        vp,
        engineSpeed,
        accel,
        brake,
        stalledVolume,
        stalledFreq
    );
}

// 0x501960
void CAEVehicleAudioEntity::ProcessDummyJet(cVehicleParams& vp) {
    auto* const plane = vp.Vehicle->AsPlane();
    ProcessGenericJet(
        plane->vehicleFlags.bEngineOn || plane->m_autoPilot.m_vehicleRecordingId >= 0,
        vp,
        CalculatePlaneEngineSpeed(plane, plane->m_fGasPedal, plane->m_fBreakPedal),
        plane->m_fGasPedal / 255.f,
        plane->m_fBreakPedal / 255.f,
        0.f,
        1.f
    );
}

// 0x501C50
void CAEVehicleAudioEntity::ProcessAircraft(cVehicleParams& params) {
    if (m_IsWreckedVehicle) { // location optimized
        return;
    }

    if (!AEAudioHardware.IsSoundBankLoaded(138, 19)) {
        return;
    }

    auto* vehicle = params.Vehicle;
    switch (m_AuSettings.VehicleAudioType) {
    case VEHICLE_SOUND_HELI: {
        if (s_HelicoptorsDisabled || m_IsHelicopterDisabled) {
            JustWreckedVehicle();
        } else if (m_IsPlayerDriver) {
            ProcessPlayerHeli(params);
        } else if (vehicle->m_nStatus == STATUS_PHYSICS) {
            ProcessAIHeli(params);
        } else {
            ProcessDummyHeli(params);
        }
        break;
    }
    case VEHICLE_SOUND_PLANE: {
        switch (vehicle->m_nModelIndex) {
        case MODEL_SHAMAL:
        case MODEL_HYDRA:
        case MODEL_AT400:
        case MODEL_ANDROM: { // Originally there was an `isPlane` variable. I just rearranged stuff, to look nicer
            if (m_IsPlayerDriver) {
                ProcessPlayerJet(params);
            } else {
                ProcessDummyJet(params);
            }
            break;
        }
        default: {
            if (m_IsPlayerDriver) {
                ProcessPlayerProp(params);
            } else if (vehicle->m_nStatus == STATUS_PHYSICS || vehicle->m_autoPilot.m_vehicleRecordingId >= 0) {
                ProcessAIProp(params);
            } else {
                ProcessDummyProp(params);
            }
        }
        }
        break;
    }
    case VEHICLE_SOUND_NON_VEH: {
        if (m_IsPlayerDriver) {
            ProcessPlayerSeaPlane(params);
        } else {
            ProcessDummySeaPlane(params);
        }
        break;
    }
    }
}

// 0x501E10
void CAEVehicleAudioEntity::ProcessVehicle(CPhysical* physical) {
    auto* const vehicle        = physical->AsVehicle();
    const auto  isStatusSimple = vehicle->m_nStatus == STATUS_SIMPLE;

    cVehicleParams params{};
    params.Vehicle               = vehicle;
    params.ModelIndexMinusOffset = physical->m_nModelIndex - 400;
    params.BaseVehicleType       = vehicle->m_nVehicleType;
    params.SpecificVehicleType   = vehicle->m_nVehicleSubType;
    params.Transmission          = vehicle->m_pHandlingData
        ? &vehicle->m_pHandlingData->m_transmissionData
        : nullptr;
    params.Speed                 = isStatusSimple
        ? DotProduct(physical->m_vecMoveSpeed, physical->m_matrix->GetForward())
        : vehicle->m_autoPilot.m_speed / 50.0f;

    switch (m_AuSettings.VehicleAudioType) {
    case VEHICLE_SOUND_CAR:
        UpdateGasPedalAudio(vehicle, vehicle->m_nVehicleType);

        if (m_IsWreckedVehicle) {
            break;
        }

        ProcessVehicleRoadNoise(params);

        if (m_IsPlayerDriver) {
            ProcessReverseGear(params);
        }

        if (!isStatusSimple) {
            ProcessVehicleSkidding(params);
            ProcessVehicleFlatTyre(params);
        }

        ProcessVehicleSirenAlarmHorn(params);

        if (m_IsPlayerDriver) {
            ProcessPlayerVehicleEngine(params);
        } else {
            ProcessDummyVehicleEngine(params);
        }

        if (!isStatusSimple) {
            ProcessEngineDamage(params);
        }

        ProcessRainOnVehicle(params);

        if (params.Vehicle->IsAutomobile()) {
            ProcessNitro(params);
        }

        ProcessMovingParts(params);

        if (params.Vehicle->m_nModelIndex == MODEL_COMBINE) {
            ProcessPlayerCombine(params);
        }

        vehicle->AsAutomobile()->m_fMoveDirection = params.Speed;
        break;

    case VEHICLE_SOUND_MOTORCYCLE: {
        if (m_IsWreckedVehicle) {
            break;
        }

        UpdateGasPedalAudio(vehicle, vehicle->m_nVehicleType);
        ProcessVehicleRoadNoise(params);

        if (!isStatusSimple) {
            ProcessVehicleSkidding(params);
        }

        ProcessVehicleSirenAlarmHorn(params);

        if (m_IsPlayerDriver) {
            ProcessPlayerVehicleEngine(params);
        } else {
            ProcessDummyVehicleEngine(params);
        }

        if (!isStatusSimple) {
            ProcessEngineDamage(params);
            ProcessVehicleFlatTyre(params);
        }

        ProcessRainOnVehicle(params);

        vehicle->AsBike()->m_fPrevSpeed = params.Speed;
        break;
    }
    case VEHICLE_SOUND_BICYCLE: {
        if (m_IsWreckedVehicle) {
            break;
        }

        UpdateGasPedalAudio(vehicle, vehicle->m_nVehicleType);
        ProcessVehicleSkidding(params);
        ProcessVehicleSirenAlarmHorn(params);

        if (m_IsPlayerDriver) {
            ProcessPlayerBicycle(params);
        } else {
            ProcessDummyBicycle(params);
        }

        vehicle->AsBike()->m_fPrevSpeed = params.Speed;
        break;
    }
    case VEHICLE_SOUND_BOAT: {
        ProcessBoatEngine(params);
        ProcessRainOnVehicle(params);
        ProcessBoatMovingOverWater(params);
        break;
    }
    case VEHICLE_SOUND_HELI: {
        UpdateGasPedalAudio(vehicle, vehicle->m_nVehicleType);
        ProcessAircraft(params);
        ProcessRainOnVehicle(params);
        break;
    }
    case VEHICLE_SOUND_PLANE: {
        UpdateGasPedalAudio(vehicle, vehicle->m_nVehicleType);
        ProcessAircraft(params);
        ProcessRainOnVehicle(params);
        ProcessMovingParts(params);
        break;
    }
    case VEHICLE_SOUND_NON_VEH: {
        ProcessAircraft(params);
        ProcessRainOnVehicle(params);
        ProcessBoatMovingOverWater(params);
        break;
    }
    case VEHICLE_SOUND_USED_BY_NONE_VEH:
        break;
    case VEHICLE_SOUND_TRAIN: {
        ProcessTrainTrackSound(params);
        ProcessDummyTrainEngine(params);
        if (m_IsPlayerDriver) {
            ProcessPlayerTrainBrakes(params);
        }
        break;
    }
    case VEHICLE_SOUND_TRAILER: {
        ProcessSpecialVehicle(params);
        break;
    }
    case VEHICLE_SOUND_SPECIAL:
        break;
    default:
        NOTSA_UNREACHABLE();
    }
}

// 0x501AB0
void CAEVehicleAudioEntity::ProcessSpecialVehicle(cVehicleParams& params) {
    switch (params.Vehicle->GetModelId()) {
    case MODEL_ARTICT1:
    case MODEL_ARTICT2:
    case MODEL_PETROTR:
    case MODEL_ARTICT3: {
        ProcessVehicleRoadNoise(params);
        ProcessRainOnVehicle(params);
        break;
    }
    case MODEL_RCBANDIT:
    case MODEL_RCTIGER: {
        ProcessDummyRCCar(params);
        break;
    }
    case MODEL_CADDY: {
        ProcessVehicleRoadNoise(params);
        ProcessVehicleSkidding(params);
        ProcessVehicleFlatTyre(params);
        ProcessVehicleSirenAlarmHorn(params);
        ProcessDummyGolfCart(params);
        ProcessRainOnVehicle(params);
        if (params.Vehicle->GetStatus() != STATUS_SIMPLE) {
            ProcessEngineDamage(params);
        }
        break;
    }
    case MODEL_RCBARON: {
        ProcessDummyRCPlane(params);
        break;
    }
    case MODEL_RCRAIDER:
    case MODEL_RCGOBLIN: {
        ProcessDummyRCHeli(params);
        break;
    }
    case MODEL_VORTEX: {
        ProcessDummyHovercraft(params);
        ProcessRainOnVehicle(params);
        ProcessVehicleSirenAlarmHorn(params);
        break;
    }
    }
}

// NOTSA, Custom (Most likely originally existed, but got inlined
bool CAEVehicleAudioEntity::UpdateGenericEngineSound(int16 index, float fVolume, float fSpeed) {
    if (CAESound* sound = m_EngineSounds[index].m_pSound) {
        sound->m_fSpeed  = fSpeed;
        sound->m_fVolume = fVolume;
        return true;
    }
    return false;
}

// NOTSA
bool CAEVehicleAudioEntity::PlayGenericEngineSound(int16 index, int16 bankSlotId, int16 sfxId, float fVolume, float fSpeed, float fSoundDistance, float fTimeScale, eSoundEnvironment individualEnvironment, int16 playPos) {
    if (m_EngineSounds[index].m_pSound) {
        // Early out if sound already being played. For safety.
        return false;
    }

    m_EngineSounds[index].m_pSound = PlaySound(bankSlotId, sfxId, fVolume, fSpeed, fSoundDistance, fTimeScale, individualEnvironment, playPos);
    return true;
}

// NOTSA
CAESound* CAEVehicleAudioEntity::PlaySound(int16 bankSlotId, int16 sfxId, float fVolume, float fSpeed, float fSoundDistance, float fTimeScale, eSoundEnvironment individualEnvironment, int16 playPos) {
    m_tempSound.Initialise(bankSlotId, sfxId, this, m_pEntity->GetPosition(), fVolume, fSoundDistance, fSpeed, fTimeScale, false, individualEnvironment, 0.0f, playPos);
    return AESoundManager.RequestNewSound(&m_tempSound);
}
