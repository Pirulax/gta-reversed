#include "StdInc.h"

#include "CMirrors.h"

int32_t& CMirrors::MirrorFlags = *(int32_t*)0xC7C618;
float& CMirrors::MirrorV = *(float*)0xC7C61C;
RwRaster*& CMirrors::pBuffer = *(RwRaster**)0xC7C71C;
RwRaster*& CMirrors::pZBuffer = *(RwRaster**)0xC7C720;
int32_t& CMirrors::TypeOfMirror = *(int32_t*)0xC7C724;
int8_t& CMirrors::bRenderingReflection = *(int8_t*)0xC7C728;
int8_t& CMirrors::d3dRestored = *(int8_t*)0xC7C729;
CVector& CMirrors::MirrorNormal = *(CVector*)0xC803D8;
bool& bFudgeNow = *(bool*)0xC7C72A;
CVector(&Screens8Track)[4][2] = *(CVector(*)[4][2])0x8D5DD8;

/*
constexpr float MX1 = -1333.453f;
constexpr float MX2 = -1477.845f;

constexpr float MY1 = -221.89799f;
constexpr float MY2 = -189.96899f;

constexpr float MZ = 1079.141f;
constexpr float MZ1 = 1067.141f;

CVector Screens8Track_[4][2] = {
    CVector{MX1, MY1, MZ}, CVector{MX1, -189.89799f, MZ}, CVector{MX1, -189.89799f, MZ1}, CVector{MX1, MY1, MZ1},
    CVector{MX2, MY2, MZ}, CVector{MX2, -221.96899f, MZ}, CVector{MX2, -221.96899f, MZ1}, CVector{MX2, MY2, MZ1}
};
*/

void CMirrors::InjectHooks() {
    ReversibleHooks::Install("CMirrors", "Init", 0x723000, &CMirrors::Init);
    ReversibleHooks::Install("CMirrors", "ShutDown", 0x723050, &CMirrors::ShutDown);
    ReversibleHooks::Install("CMirrors", "CreateBuffer", 0x7230A0, &CMirrors::CreateBuffer);
    ReversibleHooks::Install("CMirrors", "BuildCamMatrix", 0x723150, &CMirrors::BuildCamMatrix);
    ReversibleHooks::Install("CMirrors", "RenderMirrorBuffer", 0x726090, &CMirrors::RenderMirrorBuffer);
    ReversibleHooks::Install("CMirrors", "BuildCameraMatrixForScreens", 0x7266B0, &CMirrors::BuildCameraMatrixForScreens);
    ReversibleHooks::Install("CMirrors", "BeforeConstructRenderList", 0x726DF0, &CMirrors::BeforeConstructRenderList);
    ReversibleHooks::Install("CMirrors", "BeforeMainRender", 0x727140, &CMirrors::BeforeMainRender);
}

// 0x723000
void CMirrors::Init() {
    ShutDown();
}

// 0x723050
void CMirrors::ShutDown() {
    if (pBuffer)
        RwRasterDestroy(pBuffer);
    if (pZBuffer)
        RwRasterDestroy(pZBuffer);
    pBuffer = nullptr;
    pZBuffer = nullptr;
    TypeOfMirror = 0;
    MirrorFlags = 0;
}

// 0x7230A0
void CMirrors::CreateBuffer() {
    if (pBuffer)
        return;
    
    const auto depth = RwRasterGetDepth(RwCameraGetRaster(Scene.m_pRwCamera));
    if (g_fx.GetFxQuality() >= FxQuality_e::FXQUALITY_MEDIUM) {
        pBuffer = RwRasterCreate(1024, 512, depth, rwRASTERTYPECAMERATEXTURE);
        if (pBuffer) {
            pZBuffer = RwRasterCreate(1024, 512, depth, rwRASTERTYPEZBUFFER);
            if (pZBuffer)
                return;

            RwRasterDestroy(pBuffer);
            pBuffer = nullptr;
        }
    }

    // Low fx quality / fallback 
    pBuffer = RwRasterCreate(512, 256, depth, rwRASTERTYPECAMERATEXTURE);
    pZBuffer = RwRasterCreate(512, 256, depth, rwRASTERTYPEZBUFFER);
}

// 0x723150
void CMirrors::BuildCamMatrix(CMatrix& mat, CVector pointA, CVector pointB) {
    mat.SetTranslateOnly(pointA);
    mat.GetForward() = Normalized(pointB - pointA);
    mat.GetRight() = CrossProduct({ 0.0f, 0.0f, 1.0f }, mat.GetForward());
    mat.GetUp() = CrossProduct(mat.GetForward(), mat.GetRight());
}

// 0x726090
void CMirrors::RenderMirrorBuffer() {
    if (!TypeOfMirror)
        return;

    RwRaster* raster = RwCameraGetRaster(Scene.m_pRwCamera);
    const CVector2D rastersz{ (float)RwRasterGetWidth(raster), (float)RwRasterGetHeight(raster) };

    DefinedState();

    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,     (void*)rwFILTERLINEAR);
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,         (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     (void*)pBuffer);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void*)rwBLENDONE);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void*)rwBLENDZERO);

    RwImVertexIndex indices[] = { 0, 1, 2, 0, 2, 3 };

    if (MirrorFlags & CAM_STAIRS_FOR_PLAYER || bFudgeNow) {
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void*)TRUE);
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
        RwRenderStateSet(rwRENDERSTATEFOGENABLE,         (void*)TRUE);

        for (int x = 0; x < 2; x++) {
            constexpr CVector2D uvs[4]{
                {0.0f, 0.0f},
                {1.0f, 0.0f},
                {1.0f, 1.0f},
                {0.0f, 1.0f},
            };

            RxObjSpace3DVertex vertices[4];
            for (int i = 0; i < 4; i++) {
                RwIm3DVertexSetRGBA(&vertices[i], 0xFF, 0xFF, 0xFF, 0xFF);
                RwV3dAssign(RwIm3DVertexGetPos(&vertices[i]), &Screens8Track[x][i]);
                RwIm3DVertexSetU(&vertices[i], uvs[i].x);
                RwIm3DVertexSetV(&vertices[i], uvs[i].y);
            }

            if (RwIm3DTransform(vertices, std::size(vertices), nullptr, rwIM3D_VERTEXUV)) {
                RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indices, std::size(indices));
                RwIm3DEnd();
            }
        }
    } else {
        const CVector2D pos[] = {
           { 0.0f,       0.0f,      },
           { 0.0f,       rastersz.y },
           { rastersz.x, rastersz.y },
           { rastersz.x, 0.0f,      }
        };

        constexpr CVector2D uvs[] = {
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f },
            { 0.0f, 0.0f }
        };

        RwIm2DVertex vertices[4];
        for (int i = 0; i < 4; i++) {
            RwIm2DVertexSetRecipCameraZ(&vertices[i], 1.0f / RwCameraGetNearClipPlane(Scene.m_pRwCamera));
            RwIm2DVertexSetIntRGBA(&vertices[i], 0xFF, 0xFF, 0xFF, 0xFF);

            RwIm2DVertexSetScreenX(&vertices[i], pos[i].x);
            RwIm2DVertexSetScreenY(&vertices[i], pos[i].y);
            RwIm2DVertexSetScreenZ(&vertices[i], RwIm2DGetNearScreenZ());

            RwIm3DVertexSetU(&vertices[i], uvs[i].x);
            RwIm3DVertexSetV(&vertices[i], uvs[i].y);
        }
        RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, vertices, std::size(vertices), indices, std::size(indices));
    }

    if (DistanceBetweenPoints(TheCamera.GetPosition(), { 1003.0f, -42.0f, 216.0f }) < 50.0f) {
        const CVector pos[]{
            { 216.0f, -45.0f, 1000.0f },
            { 216.0f, -45.0f, 1006.0f },
            { 216.0f, -39.0f, 1006.0f },
            { 216.0f, -39.0f, 1000.0f }
        };

        constexpr CVector2D uvs[]{
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 1.0f, 1.0f },
            { 0.0f, 1.0f },
        };

        // TODO: This is another very commonly used thing (constructing vertices from a set of pos, uvs)
        // Make a function out of it.

        RxObjSpace3DVertex vertices[4];
        for (int i = 0; i < 4; i++) {
            RwIm3DVertexSetRGBA(&vertices[i], 0xFF, 0xFF, 0xFF, 0xFF);
            RwV3dAssign(RwIm3DVertexGetPos(&vertices[i]), &pos[i]);
            RwIm3DVertexSetU(&vertices[i], uvs[i].x);
            RwIm3DVertexSetV(&vertices[i], uvs[i].y);
        }

        if (RwIm3DTransform(vertices, std::size(vertices), nullptr, rwIM3D_VERTEXUV)) {
            RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, indices, std::size(indices));
            RwIm3DEnd();
        }
    }

    RwRenderStateSet(rwRENDERSTATEFOGENABLE,         (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      (void*)TRUE);
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     (void*)nullptr);
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
    RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void*)rwBLENDSRCALPHA);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void*)rwBLENDINVSRCALPHA);
}

// 0x7266B0
void CMirrors::BuildCameraMatrixForScreens(CMatrix & mat) {
    const uint32_t timeSeconds = (CTimer::m_snTimeInMilliseconds / 1000u) % 32; // Wrapped to (0, 32]
    const uint32_t timeMsLeftInSecond = CTimer::m_snTimeInMilliseconds % 1000u;
    switch (timeSeconds) {
    case 0:
    case 1:
    case 2:
    case 3: {
        BuildCamMatrix(
            mat,
            { ((float)timeMsLeftInSecond / 1000.0f + (float)timeSeconds) * 6.0f - 1249.3f, -224.5f, 1064.2f },
            { -1265.4f, -207.5f, 1053.2f }
        );
        break;
    }
    case 10:
    case 11:
    case 12:
    case 13: {
        BuildCamMatrix(mat,
            { -1406.4f, -135.3f, 1045.65f },
            { -1402.5f, -146.8f, 1043.10f }
        );
        break;
    }
    case 22:
    case 23:
    case 24:
    case 25: {
        BuildCamMatrix(mat,
            { -1479.0f, -290.5f, 1099.54f },
            { -1428.0f, -256.7f, ((float)timeMsLeftInSecond / 1000.0f + (float)(timeSeconds - 22)) * 3.0f + 1057.3f }
        );
        break;
    }
    default: {
        mat.SetRotateZOnly((CTimer::m_snTimeInMilliseconds % 16384) * 0.00038349521f);
        mat.SetTranslateOnly({ -1397.0, -219.0, 1054.0 });
        break;
    }
    }
}

// NOTSA, inlined
bool CMirrors::IsEitherScreenVisibleToCam() {
    for (int i = 0; i < 2; i++) {
        TheCamera.m_bMirrorActive = false;
        if (TheCamera.IsSphereVisible(CVector::AverageN(std::begin(Screens8Track[i]), 4), 8.0f)) {
            return false;
        }
    }
    return false;
}

// 0x726DF0
void CMirrors::BeforeConstructRenderList() {
    if (d3dRestored) {
        d3dRestored = false;
        Init();
    }

    const auto TryUpdate = [] {
        // Check player is in heli/plane
        if (CVehicle* veh = FindPlayerVehicle()) {
            if (veh->IsHeli() || veh->IsPlane())
                return false;
        }

        CCullZoneReflection* pMirrorAttrs = CCullZones::FindMirrorAttributesForCoors_(TheCamera.GetPosition());
        if (!pMirrorAttrs)
            return false;

        if (pMirrorAttrs->flags & CAM_STAIRS_FOR_PLAYER) {
            if (!IsEitherScreenVisibleToCam())
                return false;
        }

        // Actually update cam

        MirrorV = pMirrorAttrs->cm;
        MirrorNormal = CVector{
            (float)pMirrorAttrs->vx,
            (float)pMirrorAttrs->vy,
            (float)pMirrorAttrs->vz,
        } / 100.0f;
        MirrorFlags = pMirrorAttrs->flags;

        TypeOfMirror = (fabs(MirrorNormal.z) <= 0.7f) ? 1 : 2;
        CreateBuffer();

        return true;
    };

    if (!TryUpdate()) {
        ShutDown();
    }

    if (MirrorFlags & CAM_STAIRS_FOR_PLAYER || bFudgeNow) {
        CMatrix mat{};
        BuildCameraMatrixForScreens(mat);
        TheCamera.DealWithMirrorBeforeConstructRenderList(true, MirrorNormal, MirrorV, &mat);
    } else {
        TheCamera.DealWithMirrorBeforeConstructRenderList(true, MirrorNormal, MirrorV, nullptr);
    }
}

void RenderScene() {
    plugin::Call<0x53DF40>();
}

// 0x727140
void CMirrors::BeforeMainRender() {
    if (!TypeOfMirror)
        return;

    RwRaster* prevCamRaster = RwCameraGetRaster(Scene.m_pRwCamera);
    RwRaster* prevCamZRaster = RwCameraGetZRaster(Scene.m_pRwCamera);

    RwCameraSetRaster(Scene.m_pRwCamera, pBuffer);
    RwCameraSetZRaster(Scene.m_pRwCamera, pZBuffer);

    TheCamera.SetCameraUpForMirror();

    RwRGBA color{ 0, 0, 0, 0xFF };
    RwCameraClear(Scene.m_pRwCamera, &color, rwCAMERACLEARZ | rwCAMERACLEARIMAGE | (GraphicsLowQuality() ? rwCAMERACLEARSTENCIL : 0));
    if (RsCameraBeginUpdate(Scene.m_pRwCamera)) {
        bRenderingReflection = true;
        DefinedState();
        RenderScene();
        CVisibilityPlugins::RenderWeaponPedsForPC();
        CVisibilityPlugins::ms_weaponPedsForPC.Clear();
        bRenderingReflection = false;

        RwCameraEndUpdate(Scene.m_pRwCamera);

        RwCameraSetRaster(Scene.m_pRwCamera, prevCamRaster);
        RwCameraSetZRaster(Scene.m_pRwCamera, prevCamZRaster);

        TheCamera.RestoreCameraAfterMirror();
    }
}
