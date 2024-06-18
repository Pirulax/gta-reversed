#include "StdInc.h"

#include "Checkpoint.h"
#include "3dMarkers.h"

void CCheckpoint::InjectHooks() {
    RH_ScopedClass(CCheckpoint);
    RH_ScopedCategoryGlobal();

    RH_ScopedInstall(Render, 0x725C00);
}

// 0x725C00
void CCheckpoint::Render() {
    switch (m_Type) {
    case eCheckpointType::TUBE:
    case eCheckpointType::ENDTUBE:
    case eCheckpointType::EMPTYTUBE: {
        //> 0x725C1C - Make sure it's above the water
        float waterZ;
        const auto wasUnderWater = CWaterLevel::GetWaterLevelNoWaves(m_Pos, &waterZ) && waterZ >= m_Pos.z;
        if (wasUnderWater) {
            m_Pos.z = waterZ;
        }

        //> 0x725CB4 - Render flag for finish checkpoint
        if (m_Type == eCheckpointType::ENDTUBE) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     RWRSTATE(RwTextureGetRaster(gpFinishFlagTex)));
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      RWRSTATE(TRUE));
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       RWRSTATE(TRUE));
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
            RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS,    RWRSTATE(rwTEXTUREADDRESSWRAP));

            CVector flagPos;
            CVector2D flagSz;
            if (CSprite::CalcScreenCoors(m_Pos + CVector{ 0.f, 0.f, 3.f }, &flagPos, &flagSz.x, &flagSz.y, false, true)) {
                CSprite::RenderOneXLUSprite(
                    flagPos,
                    flagSz * (m_Size / 2.f),
                    m_Colour.r,
                    m_Colour.g,
                    m_Colour.b,
                    255,
                    1.f / flagPos.z,
                    m_Colour.a,
                    0,
                    0
                );
            }

            RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     RWRSTATE(0));
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      RWRSTATE(TRUE));
            RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       RWRSTATE(TRUE));
            RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
        }

        //> 0x725DEE - Render the marker (Checkpoints are basically just markers with some extra stuff)
        C3dMarkers::PlaceMarker(
            m_ID,
            e3dMarkerType::MARKER3D_TUBE,
            m_Pos,
            m_Size * 1.5f,
            m_Colour.r,
            m_Colour.g,
            m_Colour.b,
            128,
            m_PulsePeriod,
            m_PulseFraction,
            1,
            0.f, 0.f, 0.f,
            wasUnderWater
        );
        
        //> 0x725DF6 - Render direction arrow too
        if (m_Type == eCheckpointType::TUBE) { // There was a variable that was only set to true if m_Type != eCheckpointType::ENDTUBE, so this condition boils down to this....
            C3dMarkers::DirectionArrowSet(
                m_Pos,
                m_Size * 0.625f,
                255,
                64,
                64,
                255,
                -m_Fwd.x,
                -m_Fwd.y,
                -m_Fwd.z
            );
        }

        break;
    }
    case eCheckpointType::TORUS: { // 0x725E6D
        if (sq(m_Size * 2.f) >= (m_Pos - FindPlayerCoors()).SquaredMagnitude()) {
            break;
        }
        [[fallthrough]];
    }
    case eCheckpointType::TORUS_NOFADE:
    case eCheckpointType::TORUSROT:
    case eCheckpointType::TORUS_UPDOWN:
    case eCheckpointType::TORUS_DOWN: { // 0x725FE1
        C3dMarkers::PlaceMarker(
            m_ID,
            MARKER3D_TORUS,
            m_Pos,
            m_Size,
            m_Colour.r,
            m_Colour.g,
            m_Colour.b,
            m_Colour.a,
            m_PulsePeriod,
            m_PulseFraction,
            m_RotateRate,
            m_Fwd.x,
            m_Fwd.y,
            m_Fwd.z,
            false
        );
        break;
    }
    case eCheckpointType::TORUSTHROUGH: { // 0x725F05
        const auto PlaceTorusMarker = [this](size_t i) {
            // NOTE: Originally this would underflow... Not a desired behaviour.
            const auto a = (int32)m_Colour.a - (int32)(16 * i);
            if constexpr (notsa::IsFixBugs()) {
                if (a < 0) {
                    return;
                }
            }
            CVector p = m_Pos + m_Fwd * ((float)i * m_MultiSize);
            C3dMarkers::PlaceMarker(
                m_ID,
                MARKER3D_TORUS,
                p,
                m_Size,
                m_Colour.r,
                m_Colour.g,
                m_Colour.b,
                (uint8)a,
                m_PulsePeriod,
                m_PulseFraction,
                m_RotateRate,
                m_Fwd.x,
                m_Fwd.y,
                m_Fwd.z,
                false
            );
        };
        for (size_t i = 1; i <= 4; i++) {
            PlaceTorusMarker(i);
        }
        PlaceTorusMarker(0);
    }
    default:
        NOTSA_UNREACHABLE("Unknown checkpoint type ({})", m_Type.get_underlaying());
    }
}

// Based on 0x722900
void CCheckpoint::SetPosition(const CVector& pos) {
    m_Pos.x = pos.x;
    m_Pos.y = pos.y;
    switch (m_Type) {
    case eCheckpointType::TORUS_UPDOWN: m_Pos.z = m_MultiSize + pos.z; break;
    case eCheckpointType::TORUS_DOWN:   break; // Ignore Z axis
    default:                            m_Pos.z = pos.z; break;
    }
}

// Based on 0x722FC0
void CCheckpoint::MarkAsDeleted() {
    m_IsUsed = false;
    m_Type   = eCheckpointType::NA;
    m_ID     = 0;
}

// Based on 0x722970
void CCheckpoint::SetHeading(float heading) {
    m_Fwd.x = std::cos(RWDEG2RAD(heading));
    m_Fwd.y = std::sin(RWDEG2RAD(heading));
    m_Fwd.Normalise();
}
