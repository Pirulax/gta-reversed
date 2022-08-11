/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

enum eVehicleHandlingFlags : uint32 {
    VEHICLE_HANDLING_1G_BOOST           = 0x1,
    VEHICLE_HANDLING_2G_BOOST           = 0x2,
    VEHICLE_HANDLING_NPC_ANTI_ROLL      = 0x4,
    VEHICLE_HANDLING_NPC_NEUTRAL_HANDL  = 0x8,
    VEHICLE_HANDLING_NO_HANDBRAKE       = 0x10,
    VEHICLE_HANDLING_STEER_REARWHEELS   = 0x20,
    VEHICLE_HANDLING_HB_REARWHEEL_STEER = 0x40,
    VEHICLE_HANDLING_ALT_STEER_OPT      = 0x80,

    VEHICLE_HANDLING_WHEEL_F_NARROW2    = 0x100,
    VEHICLE_HANDLING_WHEEL_F_NARROW     = 0x200,
    VEHICLE_HANDLING_WHEEL_F_WIDE       = 0x400,
    VEHICLE_HANDLING_WHEEL_F_WIDE2      = 0x800,
    VEHICLE_HANDLING_FRONT_WHEELS       = VEHICLE_HANDLING_WHEEL_F_NARROW2 | VEHICLE_HANDLING_WHEEL_F_NARROW | VEHICLE_HANDLING_WHEEL_F_WIDE | VEHICLE_HANDLING_WHEEL_F_WIDE2,

    VEHICLE_HANDLING_WHEEL_R_NARROW2    = 0x1000,
    VEHICLE_HANDLING_WHEEL_R_NARROW     = 0x2000,
    VEHICLE_HANDLING_WHEEL_R_WIDE       = 0x4000,
    VEHICLE_HANDLING_WHEEL_R_WIDE2      = 0x8000,
    VEHICLE_HANDLING_REAR_WHEELS        = VEHICLE_HANDLING_WHEEL_R_NARROW2 | VEHICLE_HANDLING_WHEEL_R_NARROW | VEHICLE_HANDLING_WHEEL_R_WIDE | VEHICLE_HANDLING_WHEEL_R_WIDE2,

    VEHICLE_HANDLING_HYDRAULIC_GEOM     = 0x10000,
    VEHICLE_HANDLING_HYDRAULIC_INST     = 0x20000,
    VEHICLE_HANDLING_HYDRAULIC_NONE     = 0x40000,
    VEHICLE_HANDLING_NOS_INST           = 0x80000,
    VEHICLE_HANDLING_OFFROAD_ABILITY    = 0x100000,
    VEHICLE_HANDLING_OFFROAD_ABILITY2   = 0x200000,
    VEHICLE_HANDLING_HALOGEN_LIGHTS     = 0x400000,
    VEHICLE_HANDLING_PROC_REARWHEEL_1ST = 0x800000,
    VEHICLE_HANDLING_USE_MAXSP_LIMIT    = 0x1000000,
    VEHICLE_HANDLING_LOW_RIDER          = 0x2000000,
    VEHICLE_HANDLING_STREET_RACER       = 0x4000000,
    VEHICLE_HANDLING_SWINGING_CHASSIS   = 0x10000000
};
