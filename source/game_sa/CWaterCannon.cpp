#include "StdInc.h"
#include "CWaterCannon.h"

RxVertexIndex (&CWaterCannon::m_auRenderIndices)[18] = *(RxVertexIndex (*)[18])0xC80700;

void CWaterCannon::InjectHooks() {
    ReversibleHooks::Install("CWaterCannon", "Constructor", 0x728B10, &CWaterCannon::Constructor);
    ReversibleHooks::Install("CWaterCannon", "Destructor", 0x728B30, &CWaterCannon::Destructor);
    ReversibleHooks::Install("CWaterCannon", "Init", 0x728B40, &CWaterCannon::Init);
    ReversibleHooks::Install("CWaterCannon", "Update_OncePerFrame", 0x72A280, &CWaterCannon::Update_OncePerFrame);
    ReversibleHooks::Install("CWaterCannon", "Update_NewInput", 0x728C20, &CWaterCannon::Update_NewInput);
    // ReversibleHooks::Install("CWaterCannon", "PushPeds", 0x7295E0, &CWaterCannon::PushPeds);
    // ReversibleHooks::Install("CWaterCannon", "Render", 0x728DA0, &CWaterCannon::Render);
}

// 0x728B10
CWaterCannon::CWaterCannon() {
    // NOP
}

CWaterCannon* CWaterCannon::Constructor() {
    this->CWaterCannon::CWaterCannon();
    return this;
}

// 0x728B30
CWaterCannon::~CWaterCannon() {
    // NOP
}

CWaterCannon* CWaterCannon::Destructor() {
    CWaterCannon::~CWaterCannon();
    return this;
}

// 0x728B40
void CWaterCannon::Init() {
    m_nId = 0;
    m_nSectionsCount = 0;
    m_nCreationTime = CTimer::m_snTimeInMilliseconds;
    m_anSectionState[0] = '\0';

    m_auRenderIndices[0] = 0;
    m_auRenderIndices[1] = 1;
    m_auRenderIndices[2] = 2;

    m_auRenderIndices[3] = 1;
    m_auRenderIndices[4] = 3;
    m_auRenderIndices[5] = 2;

    m_auRenderIndices[6] = 4;
    m_auRenderIndices[7] = 5;
    m_auRenderIndices[8] = 6;

    m_auRenderIndices[9] = 5;
    m_auRenderIndices[10] = 7;
    m_auRenderIndices[11] = 6;

    m_auRenderIndices[12] = 8;
    m_auRenderIndices[13] = 9;
    m_auRenderIndices[14] = 10;

    m_auRenderIndices[15] = 9;
    m_auRenderIndices[16] = 11;
    m_auRenderIndices[17] = 10;

    m_audio.Initialise(this);
}

bool CWaterCannon::HasActiveSection() const {
    const auto end = std::end(m_anSectionState);
    return std::find(std::begin(m_anSectionState), end, true) != end;
}

// 0x72A280
void CWaterCannon::Update_OncePerFrame(short a1) {
    if (CTimer::GetTimeMs() > m_nCreationTime + 150) {
        const auto section = (m_nSectionsCount + 1) % SECTIONS_COUNT;
        m_nSectionsCount = section;
        m_anSectionState[section] = false;
    }

    for (int i = 0; i < SECTIONS_COUNT; i++) {
        if (m_anSectionState[i]) {
            CVector& speed = m_sectionMoveSpeed[i];
            speed.z -= CTimer::ms_fTimeStep / 250.0f;

            CVector& point = m_sectionPoint[i];
            point += speed * CTimer::ms_fTimeStep;

            // Originally done in a seprate loop, but we do it here
            gFireManager.ExtinguishPointWithWater(point, 2.0f, 0.5f);
        }
    }

    if ((uint8_t)(CTimer::m_FrameCounter + a1) % 4 == 0) { // Notice cast to byte
        PushPeds();
    }

    if (!HasActiveSection()) {
        m_nId = 0;
    }
}

// 0x728C20
void CWaterCannon::Update_NewInput(CVector* start, CVector* end) {
    m_sectionPoint[m_nSectionsCount]     = *start;
    m_sectionMoveSpeed[m_nSectionsCount] = *end;
    m_anSectionState[m_nSectionsCount]   = 1;
}

// 0x7295E0
void CWaterCannon::PushPeds() {
    plugin::CallMethod<0x7295E0, CWaterCannon*>(this);
}

// 0x728DA0
void CWaterCannon::Render() {
    plugin::CallMethod<0x728DA0, CWaterCannon*>(this);
}
