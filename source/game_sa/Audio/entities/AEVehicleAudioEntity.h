/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "AEAudioEntity.h"
#include "cTransmission.h"
#include "AETwinLoopSoundEntity.h"
#include <extensions/FixedFloat.hpp>
#include "eAudioEvents.h"
#include "eRadioID.h"
#include "eVehicleType.h"
#include "eSoundBank.h"
#include "eSoundBankSlot.h"
#include <Audio/Enums/eAEVehicleSoundType.h>
#include <Audio/Enums/eAEVehicleAudioTypeForName.h>
#include <Audio/Enums/eAEVehicleAudioType.h>
#include <Audio/Enums/eAERadioType.h>
#include "./AEVehicleAudioEntity.VehicleAudioSettings.h"
#include <DamageManager.h> // tComponent

enum tWheelState : int32;

class CVehicle;
class CPlane;
class CPed;

namespace notsa {
namespace debugmodules {
    class VehicleAudioEntityDebugModule;
};
};

class NOTSA_EXPORT_VTABLE CAEVehicleAudioEntity : public CAEAudioEntity {
    friend notsa::debugmodules::VehicleAudioEntityDebugModule;

protected: // Config:
    // Config struct - Obviously this is notsa, but it's necessary for the debug module
    static inline struct Config {
        float FreqUnderwaterFactor   = 0.7f; // 0x8CBC48
        tComponent HeliAudioComponent = COMPONENT_WING_RR; //!< 0x8CBD4C - Where audio is placed for helis

        float IdRollOffFactor{ 2.f };               // 0x8CBC2C
        float RevRollOffFactor{ 2.f };              // 0x8CBC30
        float RoadNoiseSoundRollOffFactor{ 3.f };   // 0x8CBD14
        float FlatTireSoundRollOffFactor{ 3.f };    // 0x8CBD20
        float ReverseGearSoundRollOffFactor{ 1.f }; // 0xNONE

        float VolOffsetOnGround{1.5f}; // 0x8CBD50

        struct {
            float StepDown{0.07f}, StepUp{0.09f}; // 0x8CBC28, 0x8CBC24
        } GasPedal;

        struct {
            float VolOffset{-9.f}; // 0x8CBD1C
            float FrqGearVelocityFactor{0.4f}; // 0x8CBD18
        } FlatTyre;

        struct {
            float VolBase{-12.f}; // 0x8CBD00
            float FadeOutDistance{72.f}; // 0x8CBD10 - Distance vehicle <-> cam

            struct {
                float FrqFactor{1.3f}; // 0x8CBD04
                float VolOffset{0.0f}; // 0xB6B9E4
            } GrassSurface;

            struct {
                float FrqFactor{1.0f}; // 0x8CBD0C
                float VolOffset{4.5f}; // 0x8CBD08
            } WetSurface;

            struct {
                float FrqFactor{1.f}; // 0xNONE
                float VolOffset{0.f}; // 0xNONE
            } StdSurface;
        } RoadNoise;

        struct {
            // Common values:
            float VolBase{ 0.f };                  // 0xB6B9E0
            float VolOffsetForPlaneOrHeli{ 12.f }; // 0x8CBCF4

            // For BMX only:
            struct {
                float VolBase{ -12.f };                             // 0x8CBCF8
                float FrqBase{ 0.9f }, FrqWheelSkidFactor{ 0.25f }; // 0x8CBCEC, 0x8CBCF0
            } BMX;

            // These don't apply to BMX:
            struct {
                float VolBase{ -12.f };                            // 0xNONE
                float FrqBase{ 0.9f }, FrqWheelSkidFactor{ 0.2f }; // 0x8CBCDC, 0x8CBCE0
            } GrassSurface;

            struct {
                float VolBase{ -9.f };                             // 0xNONE
                float FrqBase{ 0.9f }, FrqWheelSkidFactor{ 0.2f }; // 0x8CBCD4, 0x8CBCD8
            } WetSurface;

            struct {
                float FrqFactorForBikes{ 1.2f };                     // 0xNONE
                float VolBase{ 0.f };                                // 0xNONE
                float FrqBase{ 0.8f }, FrqWheelSkidFactor{ 0.125f }; // 0x8CBCE4, 0x8CBCE8
            } StdSurface;
        } Skid;

        struct { // TODO: Move into `Skid`
            bool Enabled{true}; // 0x8CBD80
            float SpinningFactor{1.f}, SkiddingFactor{1.f}, StationaryFactor{1.2f}; // 0x8CBD88, 0x8CBD84, 0x8CBD8C
        } DriveWheelSkid;

        struct { // TODO: Move into `Skid`
            bool Enabled{true}; // 0x8CBD81
            float SkiddingFactor{1.f}, StationaryFactor{1.2f}; // 0x8CBD84, 0x8CBD8C
        } NonDriveWheelSkid;

        struct {
            float Doppler{0.17f}; // 0x8CBEC4
            float BikeBellFadeOut{1.5f}; // 0x8CBECC
        } Horn;

        struct {
            float Doppler{0.25f}; // 0x8CBEC8
        } Siren;

        struct {
            float FrqBase{ 0.75f }, FrqRevsFactor{ 0.2f }; // 0x8CBD24, 0x8CBD28
            float VolBase{ -6.f };                         // 0x8CBD2C
        } ReverseGear;

        struct {
            uint16 FramesPerDrop{ 3 }; // 0x8CBD94
            float  VolBase{ 0.f };     // 0xB6B9EC
        } RainOnVehicle;

        struct {
            float StepDown{0.085f}, StepUp{0.075f}; // 0x8CC044, 0x8CC040

            // There are 2 sounds (NITRO1, NITRO2)
            struct {
                float FrqMin, FrqMax;
                float VolMin, VolMax;
            } Sounds[2]{
                {
                    0.75f, 0.84f, // 0x8CC028, 0x8CC02C
                    -6.f,  2.f    // 0x8CC020, 0x8CC024
                }, {
                    0.8f, 1.f,    // 0x8CC038, 0x8CC03C
                    -36.f, -9.f   // 0x8CC030, 0x8CC034
                }
            };
        } Nitro;

        struct {
            struct {
                float FrqMin{ 0.9f }, FrqMax{ 1.1f };   // 0x8CC000, 0x8CC004
                float VolMin{ -24.f }, VolMax{ -21.f }; // 0x8CBFF8, w0x8CBFFC
                float RollOff{ 1.f };                   // 0x8CC008
            } SteamSound;

            struct {
                float FrqMin{ 0.9f }, FrqMax{ 1.2f };   // 0x8CC014, 0x8CC018
                float VolMin{ -24.f }, VolMax{ -18.f }; // 0x8CC00C, 0x8CC010
                float RollOff{ 2.5f };                  // 0x8CC01C
            } FuckedSound;
        } EngineDamage;

        struct {
            float StepDown{0.2f}, StepUp{0.2f}; // 0x8CC04C, 0x8CC048
            float AngleDeltaFactor{30.f}; // 8CC050

            struct Props {
                std::optional<eSoundBankSlot> Slot{}; // `m_DummySlot` is used if nullopt
                eSoundBank     Bank{};
                eSoundID       Sfx{};
                struct { // Either of these 2 is used based on the delta of the part angle, see `ProcessMovingParts`
                    float Frq{};
                    float Vol{};
                } SoundParamsByPartDir[2]{};
            };
            std::unordered_map<eModelID, Props> PropsByModel{
                {MODEL_DUMPER, Props{
                    .Slot = SND_BANK_SLOT_VEHICLE_GEN,
                    .Bank = SND_BANK_GENRL_VEHICLE_GEN,
                    .Sfx = 15,
                    .SoundParamsByPartDir = {
                        {0.9f, 14.f}, // 0x8CC090, 0x8CC088
                        {1.1f, 20.f}, // 0x8CC08C, 0x8CC084
                    }
                }},
                {MODEL_PACKER, Props{
                    .Slot = SND_BANK_SLOT_VEHICLE_GEN,
                    .Bank = SND_BANK_GENRL_VEHICLE_GEN,
                    .Sfx = 15,
                    .SoundParamsByPartDir = {
                        {0.8f, 3.f}, // 0x8CC070, 0x8CC068,
                        {1.f, 9.f},  // 0x8CC06C, 0x8CC064
                    }
                }},
                {MODEL_DOZER, Props{
                    .Slot = SND_BANK_SLOT_VEHICLE_GEN,
                    .Bank = SND_BANK_GENRL_VEHICLE_GEN,
                    .Sfx = 15,
                    .SoundParamsByPartDir = {
                        {0.9f, 2.f}, // 0x8CC080, 0x8CC078
                        {1.1f, 6.f}, // 0x8CC07C, 0x8CC074
                    }
                }},
                {MODEL_FORKLIFT, Props{
                    .Slot = std::nullopt,
                    .Bank = SND_BANK_GENRL_FORKLIFT_D,
                    .Sfx = 2,
                    .SoundParamsByPartDir = {
                        {0.8f, -18.f}, // 0x8CC060, 0x8CC058
                        {1.f, -6.f}    // 0x8CC05C, 0x8CC054,
                    }
                }},
                {MODEL_ANDROM, Props{
                    .Slot = SND_BANK_SLOT_VEHICLE_GEN,
                    .Bank = SND_BANK_GENRL_VEHICLE_GEN,
                    .Sfx = 15,
                    .SoundParamsByPartDir = {
                        {0.8f, 21.f}, // 0x8CC0A0, 0x8CC098
                        {1.f, 24.f},  // 0x8CC09C, 0x8CC094
                    }
                }}
            };
        } MovingParts;

        struct {
            float VolBaseOfSeaplane{ 12.f };     //	0x8CBEDC
            float VolBase{ 3.f };                //	0x8CBED8
            float VolUnderwaterOffset{ 6.f };    //	0x8CBEF0

            float FrqBase{ 0.8f };               // 0xNONE
            float FrqSpeedFactor{ 0.2f };        // 0xNONE
            float FrqUnderwaterFactor{ 0.185f }; //	0x8CBEEC
        } BoatMovingOverWater;

        struct {
            float EngineSoundRollOff{ 3.5f };    // 0x8CBEE0
            float DistantSoundRollOff{ 5.f };    // 0x8CBEE4
            float WaterSkimSoundRollOff{ 2.5f }; // 0x8CBEE8
        } BoatSounds;

        struct {
            float FrqEngineRollFactor{ 0.1f };    // 0x8CBED4
            float FrqEngineBase{ 0.6f };          // 0xNONE
            float FrqEngineOnWaterFactor{ 0.4f }; // 0xNONE
            float FrqEngineInAirFactor{ 0.7f };   // 0xB6BA74

            float VolBase{ -3.f }; // 0xNONE
            float VolEngineSpeedFactor{3.f}; // 0xNONE
        } Boat;

        struct {
             struct TheProps {
                 float EngineSoundRollOff;
                 float TrackSoundRollOff;
                 float DistantSoundRollOff;

                 struct {
                     float VolBase;
                     float FrqMin, FrqMax;
                 } TrackSound;

                 struct {
                     float VolBase;
                     float VolPlayerAcFactor{3.f}; // 0x8CBF10
                     float FrqMin, FrqMax;
                     float FrqSlopeFactor;
                 } EngineSound;

                 struct {
                     float VolBase{9.0f}; // 0x8CBF3C
                     float FrqMin{0.68f}, FrqMax{1.f}; // 0x8CBF44, 0x8CBF48
                 };
             };         
             TheProps Tram{
                .EngineSoundRollOff  = 4.f, // 0x8CBF18
                .TrackSoundRollOff   = 5.f, // 0x8CBF1C
                .DistantSoundRollOff = 4.5f, // 0x8CBEF4

                .TrackSound = {
                    .VolBase = 10.f,                  // 0x8CBF38
                    .FrqMin  = 0.8f, .FrqMax  = 1.2f, // 0x8CBF2C, 0x8CBF28
                },
                .EngineSound = {
                    .VolBase        = 11.f,                 // 0x8CBF30
                    .FrqMin         = 0.9f, .FrqMax = 1.1f, // 0x8CBF24, 0x8CBF20
                    .FrqSlopeFactor = -0.35f,               // 0x8CBF50
                }
            };
            TheProps Generic{
                .EngineSoundRollOff  = 4.5f, // 0x8CBEFC
                .TrackSoundRollOff   = 4.5f, // 8CBF00
                .DistantSoundRollOff = 4.5f, // 0x8CBEF4

                .TrackSound = {
                    .VolBase = 6.f,                   // 0x8CBF14
                    .FrqMin  = 0.8f, .FrqMax  = 1.2f, // 0x8CBF08, 0x8CBF04
                },
                .EngineSound = {
                    .VolBase        = 0.f,                   // 0xB6BA00
                    .FrqMin         = 0.8f, .FrqMax  = 1.2f, // 0x8CBF08, 0x8CBF04
                    .FrqSlopeFactor = -0.6f,                 // 0x8CBF4C
                }
             };
         } Train;

        struct {
            float VolumeUnderwaterOffset = 6.f; // 0x8CBC44
            float VolumeTrailerOffset    = 6.f; // 0x8CBC40

            // ...Idle:
            struct {
                float Ratio      = 0.2f; // 0x8CBBF0

                float VolumeBase = -3.f; // 0x8CBC00
                float VolumeMax  = 0.f;  // 0xB6B9CC

                float FreqBase   = 0.85f; // 0x8CBBF8
                float FreqMax    = 1.2f;  // 0x8CBBFC
            } ID{};

            // ...Rev:
            struct {
                float Ratio      = 0.15f; // 0x8CBBF4

                float VolumeBase = -4.5f; // 0xB6BA2C
                float VolumeMax  = 0.f;   // 0xB6B9D0

                float FreqBase   = 0.9f; // 0x8CBC0C
                float FreqMax    = 1.5f; // 0x8CBC10
            } Rev{};

            // Golf Cart
            struct {
                float FrqMin{ 0.85f }, FrqMax{ 1.2f }; // 0x8CBFD0, 0x8CBFD4
                float VolOffset{ -3.f };                 // 0x8CBFCC
            } GolfCart;
        } DummyEngine{};

        struct {
            float CrzSpeedOffset{ 0.001f };              // 0x8CBD34
            int32 CrzMaxCnt{ 150 };                      // 0x8CBC8C

            float ZMoveSpeedThreshold{ 0.2f };           // 0x8CBD38
            int32 MaxAuGear{ 5 };                        // 0xdeadbeef
            float SingleGearVolume{ -2.f };              // 0xB6BA3C
            float VolNitroFactor{ 3.f }; // 0x8CBC4C

            float FrqNitroFactor{ 0.12f };                                       // 0x8CBC50
            float FrqWheelSpinFactor{ 0.25f };                                   // 0x8CBCB8
            float FrqBikeLeanFactor{ 0.12f };                                    // 0x8CBD6C
            float FrqPlayerBikeBoostOffset{ 0.1f };                              // 0x8CBD70
            float FrqZMoveSpeedLimitMin{ -0.2f }, FrqZMoveSpeedLimitMax{ 0.2f }; // 0x8CBD44, 0x8CBD40
            float FrqZMoveSpeedFactor{ 0.15f };                                  // 0x8CBD3C

            struct {
                float FrqWheelSpinFactor{ 0.2f }; // 0x8CBCAC
                float FrqOffset{ 1.f };
                float FrqMin{ 0.2f }, FrqMax{ 0.4f }; // 0x8CBCB0, 0x8CBCB4
                float VolMin{ -4.f }, VolMax{ 0.f };  // 0x8CBCA8, 0xB6B9DC
            } Rev; // CAR_REV   

            struct {
                float FrqMin{ 0.85f }, FrqMax{ 1.5f };  // 0x8CBC70, 0x8CBC74
                float VolMin{ -3.5f }, VolMax{ -1.5f }; // 0x8CBC78, 0x8CBC7C
            } ID; // CAR_ID    

            struct {
                float FrqSingleGear{ 1.f };              // 0xB6BA58
                float FrqMin{ 0.925f }, FrqMax{ 1.25f }; // 0x8CBC80, 0x8CBC84
                float VolMin{ 0.f }, VolMax{ 2.f };      // 0xB6B9D8, 0x8CBC88
                float RollOffFactor{ 2.f }; // 0x8CBC38 
            } Crz;                          // PLAYER_CRZ

            struct {
                float FrqSingleGear{ 1.f };                                            // 0x8CBC6C
                float FrqMultiGearOffset{ 1.f };                                       // 0x8CBC60
                float FrqPerGearFactor[6]{ 0.4f, 0.22f, 0.13f, 0.075f, 0.f, -0.075f }; // 0x8CC1C0
                float VolMin{ -2.f }, VolMax{ 0.f };                                   // 0x8CBC68, 0xB6B9D4
                float RollOffFactor{ 2.f };                                            // 0x8CBC34
                uint32 SoundLength{ 1000 };                                           // 0x8CBCCC
                float LoopMaxPlayTimePercentage{ 80.f };                               // 0x8CBCC8
                int32 LoopInterval{ 120 };                                             // 0x8CBCBC
                int32 LoopFrameCnt{ 10 };                                              // 0x8CBCC0
                float WheelSpinThreshold{ 150.f / 255.f };                             // 0x8CBC54
                float SpeedOffset{ 0.0015f };                                          // 0x8CBD30
                float InhibitForLowSpeedLimit{ 0.1f };                                 // 0x8CBCD0
            } AC; // PLAYER_AC

            struct {
                float FrqMin{ 0.75f }, FrqMax{ 1.25f }; // 0x8CBC94, 0x8CBC98
                float RollOffFactor{ 2.f };             // 0x8CBC3C
            } Off; // PLAYER_OFF
        } PlayerEngine;
    } s_Config{};
    static inline Config s_DefaultConfig{};

public: // Enums:
    // Indices for `EngineSound[]` (?) depending on the vehicle type:
    enum eAircraftSoundType { // For planes (aircrafts)
        AE_SOUND_AIRCRAFT_DISTANT     = 1,
        AE_SOUND_AIRCRAFT_FRONT       = 2,
        AE_SOUND_AIRCRAFT_NEAR        = 3,
        AE_SOUND_AIRCRAFT_REAR        = 4,
        AE_SOUND_AIRCRAFT_THRUST      = 5,
        AE_SOUND_PLANE_WATER_SKIM     = 6,
        AE_SOUND_AIRCRAFT_JET_DISTANT = 7,
    };

    using eVehicleEngineSoundType = int16;

    enum eCarEngineSoundType : eVehicleEngineSoundType { // For automobiles
        AE_SOUND_ENGINE_OFF     = 0,

        AE_SOUND_CAR_REV        = 1, // For Dummy/Player
        AE_SOUND_CAR_ID         = 2, // For Dummy/Player
        AE_SOUND_PLAYER_CRZ     = 3,
        AE_SOUND_PLAYER_AC      = 4,
        AE_SOUND_PLAYER_OFF     = 5,

        AE_SOUND_PLAYER_REVERSE = 6,

        AE_SOUND_NITRO1         = 7,
        AE_SOUND_NITRO2         = 8,

        AE_SOUND_STEAM          = 9,

        AE_SOUND_FUCKED         = 10,

        AE_SOUND_MOVING_PARTS   = 11,

        AE_SOUND_ENGINE_MAX     = 12,
    };

    enum eTrainEngineSoundType : eVehicleEngineSoundType { // For trains
        AE_SOUND_TRAIN_ENGINE  = 1,
        AE_SOUND_TRAIN_TRACK   = 2,
        AE_SOUND_TRAIN_DISTANT = 3,
    };

    enum ePlayerHeliSoundType : eVehicleEngineSoundType { // For heli
        AE_PLAYER_HELI_FRONT = 0,
        AE_PLAYER_HELI_REAR  = 1,
        AE_PLAYER_HELI_START = 2,
        AE_PLAYER_HELI_TAIL  = 3,
    };

    enum eBicycleSoundType : eVehicleEngineSoundType { // For bicycle
        AE_SOUND_BICYCLE_TYRE        = 1,
        AE_SOUND_BICYCLE_SPROCKET_1  = 2,
        AE_SOUND_BICYCLE_CHAIN_CLANG = 3,
    };

    enum eBoatEngineSoundType : eVehicleEngineSoundType { // For boat
        AE_SOUND_BOAT_IDLE       = 1,
        AE_SOUND_BOAT_ENGINE     = 2,
        AE_SOUND_BOAT_DISTANT    = 3,
        AE_SOUND_BOAT_PADDING1   = 4,
        AE_SOUND_BOAT_PADDING2   = 5,
        AE_SOUND_BOAT_WATER_SKIM = 6,
    };

    enum eDummyEngineSoundType : eVehicleEngineSoundType{ // For all other dummies
        AE_DUMMY_CRZ = 0x0,
        AE_DUMMY_ID = 0x1,
    };

    enum class eAEState : uint8 {
        CAR_OFF              = 0,

        // Used for vehicles the player is *not* inside of
        DUMMY_ID             = 1, // Idle
        DUMMY_CRZ            = 2, // Cruising (?)

        // Used for vehicles the player is in
        PLAYER_AC_FULL       = 3, // Acceleration (?)
        PLAYER_WHEEL_SPIN    = 4,
        PLAYER_CRZ           = 5, // Cruising (?)
        PLAYER_ID            = 6, // Idle (?)
        PLAYER_REVERSE       = 7,
        PLAYER_REVERSE_OFF   = 8,
        PLAYER_FAILING_TO_AC = 9, // AC = accelerate (?)

        // Keep this at the bottom
        NUM_STATES
    };

public: // Structs:
    struct tEngineSound {
        uint32    EngineSoundType{};
        CAESound* Sound{};
    };
    VALIDATE_SIZE(tEngineSound, 0x8);

    struct tDummyEngineSlot {
        eSoundBankS16 BankID{ SND_BANK_UNK };
        int16         RefCnt{ 0 };
    };
    VALIDATE_SIZE(tDummyEngineSlot, 0x4);

    struct tVehicleParams {
        int32                    SpecificVehicleType{ VEHICLE_TYPE_IGNORE };
        int32                    BaseVehicleType{ VEHICLE_TYPE_IGNORE };
        bool                     IsDistCalculated{ false };
        float                    DistSq{ 0.0f };
        CVehicle*                Vehicle{ nullptr };
        cTransmission*           Transmission{ nullptr };
        uint32                   ModelIndexMinusOffset{ 0 }; // Offset is `400`
        float                    Speed{ 0.0f };
        FixedFloat<int16, 255.f> ThisAccel{ 0 };
        FixedFloat<int16, 255.f> ThisBrake{ 0 };
        float                    AbsSpeed{ 0.0f };
        float                    ZOverSpeed{};
        float                    SpeedRatio{ 0.0f };
        float*                   GasPedalAudioRevs{ nullptr };
        float                    PrevSpeed{ 0.0f };
        uint8                    RealGear{ 0 };
        bool                     IsHandbrakeOn{ false };
        float                    RealRevsRatio{ 0.0f };
        float                    WheelSpin{ 0.0f };
        int16                    NumGears{};
        uint8                    NumDriveWheelsOnGround{};
        uint8                    NumDriveWheelsOnGroundLastFrame{};
        tWheelState*             WheelState{};
    };
    VALIDATE_SIZE(tVehicleParams, 0x4C);

public:
    static inline auto& s_pPlayerAttachedForRadio       = StaticRef<CPed*>(0xB6B98C);
    static inline auto& s_pPlayerDriver                 = StaticRef<CPed*>(0xB6B990);
    static inline auto& s_HelicoptorsDisabled           = StaticRef<bool>(0xB6B994);
    static inline auto& s_NextDummyEngineSlot           = StaticRef<int16>(0xB6B998);
    static inline auto& s_pVehicleAudioSettingsForRadio = StaticRef<tVehicleAudioSettings*>(0xB6B98C);
    static inline auto& s_DummyEngineSlots              = StaticRef<std::array<tDummyEngineSlot, SND_BANK_SLOT_DUMMY_END - SND_BANK_SLOT_DUMMY_FIRST>>(0xB6B9A0);

public:
    CAEVehicleAudioEntity();
    ~CAEVehicleAudioEntity();

    void UpdateParameters(CAESound* sound, int16 curPlayPos) override;

    void Initialise(CEntity* entity);
    static void StaticInitialise();

    void Terminate();

    static bool DoesBankSlotContainThisBank(eSoundBankSlot bankSlot, eSoundBank bankId);
    static eSoundBankSlot DemandBankSlot(eSoundBank bankId);
    static eSoundBankSlot RequestBankSlot(eSoundBank bankId);
    static void StoppedUsingBankSlot(eSoundBankSlot bankSlot);
    static tDummyEngineSlot* GetDummyEngineSlot(eSoundBankSlot bankSlot);

    static tVehicleAudioSettings* StaticGetPlayerVehicleAudioSettingsForRadio();
    static tVehicleAudioSettings GetVehicleAudioSettings(int16 vehId);

    void AddAudioEvent(eAudioEvents event, float p1 = 0.f);
    void AddAudioEvent(eAudioEvents event, CVehicle* vehicle);

    void Service();
    static void StaticService();

    eAEVehicleAudioType GetVehicleTypeForAudio() const;

    void InhibitAccForTime(uint32 time);
    bool IsAccInhibited(tVehicleParams& params) const;
    bool IsAccInhibitedBackwards(tVehicleParams& params) const;
    bool IsAccInhibitedForLowSpeed(tVehicleParams& params) const;
    [[nodiscard]] bool IsAccInhibitedForTime() const;

    void InhibitCrzForTime(uint32 time);
    [[nodiscard]] bool IsCrzInhibitedForTime() const;

    void JustGotInVehicleAsDriver();
    void TurnOnRadioForVehicle();
    void TurnOffRadioForVehicle();
    void PlayerAboutToExitVehicleAsDriver();
    bool CopHeli();
    bool JustFinishedAccelerationLoop();
    void PlaySkidSound(int16 soundType, float speed = 1.0f, float volume = -100.0f);
    void JustWreckedVehicle();
    CVector GetAircraftNearPosition();
    float GetFlyingMetalVolume(CPhysical* physical);
    void GetSirenState(bool& bSirenOrAlarm, bool& bHorn, tVehicleParams& params) const;
    void PlayTrainBrakeSound(int16 soundType, float speed = 1.0f, float volume = -100.0f);
    void JustGotOutOfVehicleAsDriver();

    void  StartVehicleEngineSound(eVehicleEngineSoundType st, float speed, float volume);
    void  CancelVehicleEngineSound(size_t engineSoundStateId);
    void  CancelAllVehicleEngineSounds(std::optional<size_t> except = std::nullopt); // notsa
    void  RequestNewPlayerCarEngineSound(eVehicleEngineSoundType st, float speed = 1.f, float volume = -100.f);
    float GetFreqForPlayerEngineSound(tVehicleParams& params, eVehicleEngineSoundType st) const;
    float GetVolForPlayerEngineSound(tVehicleParams& params, eVehicleEngineSoundType st);
    void  UpdateVehicleEngineSound(int16, float, float);
    void  UpdateOrRequestVehicleEngineSound(eVehicleEngineSoundType st, float freq, float volume);
    void  StopGenericEngineSound(int16 index);
    bool  UpdateGenericEngineSound(int16 index, float fVolume = 1.0f, float fSpeed = 1.0f);

    void UpdateGasPedalAudio(CVehicle* vehicle, int32 vehType);
    void UpdateBoatSound(eBoatEngineSoundType st, eSoundBankSlot bslot, eSoundID sfx, float speed, float volume);
    void UpdateTrainSound(eTrainEngineSoundType st, eSoundBankSlot slot, eSoundID sfx, float speed, float volume);
    void UpdateGenericVehicleSound(eVehicleEngineSoundType st, eSoundBankSlot bankSlot, eSoundBank bank, eSoundID sfx, float speed, float volume, float rollOff);


    static void EnableHelicoptors();
    static void DisableHelicoptors();
    void EnableHelicoptor();
    void DisableHelicoptor();

    static constexpr float GetDummyRevRatioProgress(float ratio);
    float GetVolumeForDummyIdle(float fGearRevProgress, float fRevProgressBaseline) const;
    float GetFrequencyForDummyIdle(float fGearRevProgress, float fRevProgressBaseline) const;
    [[nodiscard]] float GetFreqForIdle(float fRatio) const;

    static constexpr float GetDummyIdleRatioProgress(float ratio);
    float GetVolumeForDummyRev(float fRatio, float fFadeRatio) const;
    float GetFrequencyForDummyRev(float fRatio, float fFadeRatio) const;

    float GetVehicleDriveWheelSkidValue(CVehicle* vehicle, tWheelState wheelState, float fUnk, cTransmission& transmission, float fVelocity) const noexcept;
    float GetVehicleNonDriveWheelSkidValue(CVehicle* vehicle, tWheelState wheelState, cTransmission& transmission, float velocity) const noexcept;

    [[nodiscard]] float GetBaseVolumeForBicycleTyre(float fGearVelocityProgress) const;
    void GetHornState(bool* out, tVehicleParams& params) const noexcept;
    bool GetHornState(tVehicleParams& params) const noexcept;
    void GetAccelAndBrake(tVehicleParams& vp) const;

    void PlayAircraftSound(eAircraftSoundType es, eSoundBankSlot slot, eSoundID sfx, float volume = -100.0f, float speed = 1.0f);
    void PlayRoadNoiseSound(eSoundID sfx, float speed = 1.0f, float volume = -100.0f);
    void PlayFlatTyreSound(eSoundID sfx, float speed = 1.0f, float volume = -100.0f);
    void PlayReverseSound(eSoundID sfx, float speed = 1.0f, float volume = -100.0f);
    void PlayHornOrSiren(bool bPlayHornTone, bool bPlaySirenOrAlarm, bool bPlayHorn, tVehicleParams& params);
    void PlayBicycleSound(eBicycleSoundType es, eSoundBankSlot slot, eSoundID sfx, float volume = -100.0f, float speed = 1.0f);

    void ProcessVehicleFlatTyre(tVehicleParams& params);
    void ProcessVehicleRoadNoise(tVehicleParams& params);
    void ProcessReverseGear(tVehicleParams& params);
    void ProcessVehicleSkidding(tVehicleParams& params);
    void ProcessRainOnVehicle(tVehicleParams& params);
    void ProcessGenericJet(bool bEngineOn, tVehicleParams& params, float fEngineSpeed, float fAccelRatio, float fBrakeRatio, float fStalledVolume, float fStalledFrequency);
    void ProcessDummyJet(tVehicleParams& params);
    void ProcessPlayerJet(tVehicleParams& params);
    void ProcessDummySeaPlane(tVehicleParams& params);
    void ProcessPlayerSeaPlane(tVehicleParams& params);
    void ProcessAIHeli(tVehicleParams& params);
    void ProcessDummyHeli(tVehicleParams& params);
    void ProcessPlayerHeli(tVehicleParams& params);
    void ProcessAIProp(tVehicleParams& params);
    void ProcessProp(tVehicleParams& vp, bool isProp); // notsa
    void ProcessDummyProp(tVehicleParams& params);
    void ProcessPlayerProp(tVehicleParams& params);
    void ProcessAircraft(tVehicleParams& params);
    void ProcessPlayerBicycle(tVehicleParams& params);
    void ProcessDummyBicycle(tVehicleParams& params);
    void ProcessDummyStateTransition(eAEState newState, float fRatio, tVehicleParams& params);
    void ProcessDummyVehicleEngine(tVehicleParams& params);
    void ProcessPlayerVehicleEngine(tVehicleParams& params);
    void ProcessVehicleSirenAlarmHorn(tVehicleParams& params);
    void ProcessBoatEngine(tVehicleParams& params);
    void ProcessBoatMovingOverWater(tVehicleParams& params);
    void ProcessDummyTrainEngine(tVehicleParams& params);
    void ProcessTrainTrackSound(tVehicleParams& params);
    void ProcessPlayerTrainEngine(tVehicleParams& params);
    void ProcessPlayerTrainBrakes(tVehicleParams& params);
    void ProcessDummyRCPlane(tVehicleParams& params);
    void ProcessPlayerRCPlane(tVehicleParams& params);
    void ProcessDummyRCHeli(tVehicleParams& params);
    void ProcessPlayerRCHeli(tVehicleParams& params);
    void ProcessPlayerRCCar(tVehicleParams& params);
    void ProcessPlayerHovercraft(tVehicleParams& params);
    void ProcessPlayerGolfCart(tVehicleParams& params);
    void ProcessDummyGolfCart(tVehicleParams& params);
    void ProcessDummyHovercraft(tVehicleParams& params);
    void ProcessDummyRCCar(tVehicleParams& params);
    void ProcessPlayerCombine(tVehicleParams& params);
    void ProcessEngineDamage(tVehicleParams& params);
    void ProcessNitro(tVehicleParams& params);
    void ProcessMovingParts(tVehicleParams& params);
    void ProcessVehicle(CPhysical* vehicle);
    void ProcessSpecialVehicle(tVehicleParams& params);

#undef PlaySound
    CAESound* PlaySound(eSoundBankSlot bankSlot, eSoundID sound,
                        float fVolume = 1.0f, float fSpeed = 1.0f, float fSoundDistance = 1.0f,
                        float fTimeScale = 1.0f, eSoundEnvironment individualEnvironment = SOUND_REQUEST_UPDATES, int16 playPos = 0);

    auto GetVehicle() const { return m_Entity->AsVehicle(); }

private:
    void ProcessPropStall(CPlane* plane, float& outVolume, float& outFreq);
    bool EnsureHasDummySlot() noexcept;
    bool EnsureSoundBankIsLoaded(bool isDummy, bool turnOffIfNotLoaded = true);
    auto GetEngineSound(eVehicleEngineSoundType st) const noexcept { return m_EngineSounds[st].Sound; }
    void StopNonEngineSounds() noexcept;
    void GenericPlaySurfaceSound(eSoundID sfx, float speed, float volume, float rollOff) noexcept;

public:
    int16                  m_DoCountStalls;
    tVehicleAudioSettings  m_AuSettings;
    bool                   m_IsInitialized;
    bool                   m_IsPlayerDriver;
    bool                   m_IsPlayerPassenger;
    bool                   m_IsPlayerDriverAboutToExit;
    bool                   m_IsWreckedVehicle;
    eAEState               m_State;    //!< Self explainatory
    uint8                  m_AuGear;   //!< Still not sure
    float                  m_CrzCount; //!< Max value is defined in the config (`MaxCrzCount`) - Used when in the last gear and the engine is maxed out
    bool                   m_IsSingleGear;
    int16                  m_RainDropCounter;
    int16                  m_StalledCount;
    uint32                 m_SwapStalledTime;
    bool                   m_IsSilentStalled; // ProcessPlayerProp
    bool                   m_IsHelicopterDisabled;

    bool                   m_IsHornOn;
    bool                   m_IsSirenOn;
    bool                   m_IsFastSirenOn;
    float                  m_HornVolume;
    bool                   m_HasSiren;

    uint32                 m_TimeSplashLastTriggered;
    uint32                 m_TimeBeforeAllowAccelerate;
    uint32                 m_TimeBeforeAllowCruise;

    float                  m_EventVolume;

    eSoundBankS16          m_DummyEngineBank;
    eSoundBankS16          m_PlayerEngineBank;
    eSoundBankSlotS16      m_DummySlot;
    std::array<tEngineSound, AE_SOUND_ENGINE_MAX> m_EngineSounds;

    int32                  m_TimeLastServiced;

    int16                  m_ACPlayPositionThisFrame;
    int16                  m_ACPlayPositionLastFrame;
    int16                  m_FramesAgoACLooped;
    int16                  m_ACPlayPercentWhenStopped;  // [0, 100]
    uint32                 m_TimeACStopped;
    int16                  m_ACPlayPositionWhenStopped;

    eSoundID               m_SurfaceSoundType; //!< Used for `m_SkidSound` (Not `m_SurfaceSound`!)
    CAESound*              m_SurfaceSound; //!< Not actually used, instead `m_SkidSound` is used

    eSoundID               m_RoadNoiseSoundType;
    CAESound*              m_RoadNoiseSound;

    eSoundID               m_FlatTireSoundType;
    CAESound*              m_FlatTireSound;

    eSoundID               m_ReverseGearSoundType;
    CAESound*              m_ReverseGearSound;

    eSoundID               m_HornSoundType;
    CAESound*              m_HornSound;
    CAESound*              m_SirenSound;
    CAESound*              m_FastSirenSound;

    CAETwinLoopSoundEntity m_SkidSound;

    float                  m_CurrentRotorFrequency;
    float                  m_CurrentDummyEngineVolume;
    float                  m_CurrentDummyEngineFrequency;
    float                  m_MovingPartSmoothedSpeed;

    float                  m_FadeIn;
    float                  m_FadeOut;

    bool                   m_bNitroOnLastFrame;
    float                  m_CurrentNitroRatio;


private:
    friend void InjectHooksMain();
    static void InjectHooks();

    CAEVehicleAudioEntity* Constructor() { this->CAEVehicleAudioEntity::CAEVehicleAudioEntity(); return this; }
    CAEVehicleAudioEntity* Destructor() { this->CAEVehicleAudioEntity::~CAEVehicleAudioEntity(); return this; }
};
VALIDATE_SIZE(CAEVehicleAudioEntity, 0x24C);

inline std::optional<const char*> EnumToString(CAEVehicleAudioEntity::eAEState s) {
    using enum CAEVehicleAudioEntity::eAEState;
    switch (s) {
    case CAR_OFF:               return "CAR_OFF";
    case DUMMY_ID:              return "DUMMY_ID";
    case DUMMY_CRZ:             return "DUMMY_CRZ";
    case PLAYER_AC_FULL:        return "PLAYER_AC_FULL";
    case PLAYER_WHEEL_SPIN:     return "PLAYER_WHEEL_SPIN";
    case PLAYER_CRZ:            return "PLAYER_CRZ";
    case PLAYER_ID:             return "PLAYER_ID";
    case PLAYER_REVERSE:        return "PLAYER_REVERSE";
    case PLAYER_REVERSE_OFF:    return "PLAYER_REVERSE_OFF";
    case PLAYER_FAILING_TO_AC:  return "PLAYER_FAILING_TO_AC";
    default:                    return std::nullopt;
    }
}

inline std::optional<const char*> EnumToString(CAEVehicleAudioEntity::eCarEngineSoundType st) {
    switch (st) {
    case CAEVehicleAudioEntity::AE_SOUND_ENGINE_OFF:    return "ENGINE_OFF";
    case CAEVehicleAudioEntity::AE_SOUND_CAR_REV:       return "CAR_REV";
    case CAEVehicleAudioEntity::AE_SOUND_CAR_ID:        return "CAR_ID";
    case CAEVehicleAudioEntity::AE_SOUND_PLAYER_CRZ:    return "PLAYER_CRZ";
    case CAEVehicleAudioEntity::AE_SOUND_PLAYER_AC:     return "PLAYER_AC";
    case CAEVehicleAudioEntity::AE_SOUND_PLAYER_OFF:    return "PLAYER_OFF";
    case CAEVehicleAudioEntity::AE_SOUND_PLAYER_REVERSE:return "PLAYER_REVERSE";
    case CAEVehicleAudioEntity::AE_SOUND_NITRO1:        return "NITRO1";
    case CAEVehicleAudioEntity::AE_SOUND_NITRO2:        return "NITRO2";
    case CAEVehicleAudioEntity::AE_SOUND_STEAM:         return "STEAM";
    case CAEVehicleAudioEntity::AE_SOUND_FUCKED:        return "FUCKED";
    case CAEVehicleAudioEntity::AE_SOUND_MOVING_PARTS:  return "MOVING_PARTS";
    case CAEVehicleAudioEntity::AE_SOUND_ENGINE_MAX:    return "ENGINE_MAX";
    default:                                            return std::nullopt;
    }
}

inline std::optional<const char*> EnumToString(eAERadioType rt) {
    switch (rt) {
    case eAERadioType::AE_RT_CIVILIAN:  return "CIVILIAN";
    case eAERadioType::AE_RT_SPECIAL:   return "SPECIAL";
    case eAERadioType::AE_RT_UNKNOWN:   return "UNKNOWN";
    case eAERadioType::AE_RT_EMERGENCY: return "EMERGENCY";
    case eAERadioType::AE_RT_DISABLED:  return "DISABLED";
    default:                            return std::nullopt;
    }
}

inline std::optional<const char*> EnumToString(eAEVehicleSoundType v) {
    switch (v) {
    case AE_CAR:                    return "CAR";
    case AE_BIKE:                   return "BIKE";
    case AE_BMX:                    return "BMX";
    case AE_BOAT:                   return "BOAT";
    case AE_AIRCRAFT_HELICOPTER:    return "AIRCRAFT_HELICOPTER";
    case AE_AIRCRAFT_PLANE:         return "AIRCRAFT_PLANE";
    case AE_AIRCRAFT_SEAPLANE:      return "AIRCRAFT_SEAPLANE";
    case AE_ONE_GEAR:               return "ONE_GEAR";
    case AE_TRAIN:                  return "TRAIN";
    case AE_SPECIAL:                return "SPECIAL";
    case AE_NO_VEHICLE:             return "NO_VEHICLE";
    default:                        return std::nullopt;
    }
}
