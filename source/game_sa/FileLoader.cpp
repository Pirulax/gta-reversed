/*
Plugin-SDK (Grand Theft Auto San Andreas) source file
Authors: GTA Community. See more here
https://github.com/DK22Pac/plugin-sdk
Do not delete this comment block. Respect others' work!
*/
#include "StdInc.h"

#include "CarGenerator.h"
#include "TheCarGenerators.h"
#include "Occlusion.h"
#include "PedType.h"

char (&CFileLoader::ms_line)[512] = *reinterpret_cast<char (*)[512]>(0xB71848);
uint32& gAtomicModelId = *reinterpret_cast<uint32*>(0xB71840);

char (&colFileReadBuffer)[32768] = *(char (*)[32768])0xBC40D8;


void CFileLoader::InjectHooks() {
    using namespace ReversibleHooks;
    Install("CFileLoader", "AddTexDictionaries", 0x5B3910, &CFileLoader::AddTexDictionaries);
    Install("CFileLoader", "LoadTexDictionary", 0x5B3860, &CFileLoader::LoadTexDictionary);
    Install("CFileLoader", "LoadAnimatedClumpObject", 0x5B40C0, &CFileLoader::LoadAnimatedClumpObject);
    Install("CFileLoader", "LoadAtomicFile_stream", 0x5371F0, static_cast<bool(*)(RwStream*, unsigned)>(&CFileLoader::LoadAtomicFile));
    Install("CFileLoader", "LoadAtomicFile", 0x5B39D0, static_cast<void(*)(const char*)>(&CFileLoader::LoadAtomicFile));
    Install("CFileLoader", "LoadLine_File", 0x536F80, static_cast<char*(*)(FILESTREAM)>(&CFileLoader::LoadLine));
    Install("CFileLoader", "LoadLine_Bufer", 0x536FE0, static_cast<char*(*)(char*&, int32&)>(&CFileLoader::LoadLine));
    Install("CFileLoader", "LoadAudioZone", 0x5B4D70, &CFileLoader::LoadAudioZone);
    Install("CFileLoader", "LoadCarGenerator_0", 0x537990, static_cast<void(*)(CFileCarGenerator*, int32)>(&CFileLoader::LoadCarGenerator));
    Install("CFileLoader", "LoadCarGenerator_1", 0x5B4740, static_cast<void(*)(const char*, int32)>(&CFileLoader::LoadCarGenerator));
    Install("CFileLoader", "LoadCarPathNode", 0x5B4380, &CFileLoader::LoadCarPathNode);
    Install("CFileLoader", "StartLoadClumpFile", 0x5373F0, &CFileLoader::StartLoadClumpFile);
    Install("CFileLoader", "FinishLoadClumpFile", 0x537450, &CFileLoader::FinishLoadClumpFile);
    Install("CFileLoader", "LoadClumpFile", 0x5B3A30, static_cast<void(*)(const char*)>(&CFileLoader::LoadClumpFile));
    Install("CFileLoader", "LoadClumpObject", 0x5B4040, &CFileLoader::LoadClumpObject);
    // Install("CFileLoader", "LoadCollisionFile_0", 0x538440, static_cast<bool(*)(uint8*, uint32, uint8)>(&CFileLoader::LoadCollisionFile));
    // Install("CFileLoader", "LoadCollisionFile_1", 0x5B4E60, static_cast<bool(*)(const char*, uint8)>(&CFileLoader::LoadCollisionFile));
    // Install("CFileLoader", "LoadCollisionFileFirstTime", 0x5B5000, &CFileLoader::LoadCollisionFileFirstTime);
    // Install("CFileLoader", "LoadCollisionModel", 0x537580, &CFileLoader::LoadCollisionModel);
    // Install("CFileLoader", "LoadCollisionModelVer2", 0x537EE0, &CFileLoader::LoadCollisionModelVer2);
    // Install("CFileLoader", "LoadCollisionModelVer3", 0x537CE0, &CFileLoader::LoadCollisionModelVer3);
    // Install("CFileLoader", "LoadCollisionModelVer4", 0x537AE0, &CFileLoader::LoadCollisionModelVer4);
    Install("CFileLoader", "LoadCullZone", 0x5B4B40, &CFileLoader::LoadCullZone);
    Install("CFileLoader", "LoadEntryExit", 0x5B8030, &CFileLoader::LoadEntryExit);
    Install("CFileLoader", "LoadGarage", 0x5B4530, &CFileLoader::LoadGarage);
    Install("CFileLoader", "LoadLevel", 0x5B9030, &CFileLoader::LoadLevel);
    Install("CFileLoader", "LoadObject", 0x5B3C60, &CFileLoader::LoadObject);
    Install("CFileLoader", "LoadObjectInstance_inst", 0x538090, static_cast<CEntity* (*)(CFileObjectInstance*, const char*)>(&CFileLoader::LoadObjectInstance));
    Install("CFileLoader", "LoadObjectInstance_file", 0x538690, static_cast<CEntity* (*)(const char*)>(&CFileLoader::LoadObjectInstance));
    Install("CFileLoader", "LoadOcclusionVolume", 0x5B4C80, &CFileLoader::LoadOcclusionVolume);
    Install("CFileLoader", "LoadPathHeader", 0x5B41C0, &CFileLoader::LoadPathHeader);
    Install("CFileLoader", "LoadPedObject", 0x5B7420, &CFileLoader::LoadPedObject);
    Install("CFileLoader", "LoadPedPathNode", 0x5B41F0, &CFileLoader::LoadPedPathNode);
    Install("CFileLoader", "LoadPickup", 0x5B47B0, &CFileLoader::LoadPickup);
    Install("CFileLoader", "LoadStuntJump", 0x5B45D0, &CFileLoader::LoadStuntJump);
    Install("CFileLoader", "LoadTXDParent", 0x5B75E0, &CFileLoader::LoadTXDParent);
    Install("CFileLoader", "LoadTimeCyclesModifier", 0x5B81D0, &CFileLoader::LoadTimeCyclesModifier);
    Install("CFileLoader", "LoadTimeObject", 0x5B3DE0, &CFileLoader::LoadTimeObject);
    Install("CFileLoader", "LoadVehicleObject", 0x5B6F30, &CFileLoader::LoadVehicleObject);
    Install("CFileLoader", "LoadWeaponObject", 0x5B3FB0, &CFileLoader::LoadWeaponObject);
    Install("CFileLoader", "LoadZone", 0x5B4AB0, &CFileLoader::LoadZone);
    Install("CFileLoader", "LoadScene", 0x5B8700, &CFileLoader::LoadScene);
    Install("CFileLoader", "LoadObjectTypes", 0x5B8400, &CFileLoader::LoadObjectTypes);
    Install("CFileLoader", "FindRelatedModelInfoCB", 0x5B3930, &CFileLoader::FindRelatedModelInfoCB);
    Install("CFileLoader", "SetRelatedModelInfoCB", 0x537150, &CFileLoader::SetRelatedModelInfoCB);
}

// copy textures from dictionary to baseDictionary
// 0x5B3910
void CFileLoader::AddTexDictionaries(RwTexDictionary* dictionary, RwTexDictionary* baseDictionary) {
    RwTexDictionaryForAllTextures(baseDictionary, AddTextureCB, dictionary);
}

// save txd to file
// unused
// 0x5B38C0
void CFileLoader::SaveTexDictionary(RwTexDictionary* dictionary, const char* filename) {
    RwStream* stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, filename);
    if (stream) {
        RwTexDictionaryStreamWrite(dictionary, stream);
        RwStreamClose(stream, nullptr);
    }
}

// load txd from file
// 0x5B3860
RwTexDictionary* CFileLoader::LoadTexDictionary(const char* filename) {
    RwStream* stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
    if (stream) {
        bool chunk = RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nullptr, nullptr);
        if (chunk) {
            RwTexDictionary* txd = RwTexDictionaryGtaStreamRead(stream);
            RwStreamClose(stream, nullptr);
            if (txd) {
                return txd;
            }
        }
        RwStreamClose(stream, nullptr);
    }
    return RwTexDictionaryCreate();
}

// 0x5B40C0
int32 CFileLoader::LoadAnimatedClumpObject(const char* line) {
    int32  objID{-1};
    char modelName[24]{};
    char txdName[24]{};
    char animName[16]{ "null" };
    float drawDist{ 2000.f };
    uint32 flags{};

    if (sscanf(line, "%d %s %s %s %f %d", &objID, modelName, txdName, animName, &drawDist, &flags) != 6)
        return -1;

    auto mi = CModelInfo::AddClumpModel(objID);

    mi->m_nKey = CKeyGen::GetUppercaseKey(modelName);
    mi->SetTexDictionary(txdName);
    mi->SetAnimFile(animName);
    mi->SetClumpModelInfoFlags(flags);

    if (std::string_view{ animName } == "null")
        mi->bHasBeenPreRendered = true;
}

// 0x5371F0
bool CFileLoader::LoadAtomicFile(RwStream* stream, uint32 modelId) {
    auto pAtomicModelInfo = CModelInfo::ms_modelInfoPtrs[modelId]->AsAtomicModelInfoPtr();
    bool bUseCommonVehicleTexDictionary = false;
    if (pAtomicModelInfo && pAtomicModelInfo->bUseCommonVehicleDictionary) {
        bUseCommonVehicleTexDictionary = true;
        CVehicleModelInfo::UseCommonVehicleTexDicationary();
    }

    if (RwStreamFindChunk(stream, rwID_CLUMP, nullptr, nullptr)) {
        RpClump* pReadClump = RpClumpStreamRead(stream);
        if (!pReadClump) {
            if (bUseCommonVehicleTexDictionary) {
                CVehicleModelInfo::StopUsingCommonVehicleTexDicationary();
            }
            return false;
        }
        gAtomicModelId = modelId;
        RpClumpForAllAtomics(pReadClump, (RpAtomicCallBack)SetRelatedModelInfoCB, pReadClump);
        RpClumpDestroy(pReadClump);
    }

    if (!pAtomicModelInfo->m_pRwObject)
        return false;

    if (bUseCommonVehicleTexDictionary)
        CVehicleModelInfo::StopUsingCommonVehicleTexDicationary();

    return true;
}

// 0x5B39D0
void CFileLoader::LoadAtomicFile(const char* filename) {
    RwStream* stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
    if (RwStreamFindChunk(stream, rwID_CLUMP, nullptr, nullptr)) {
        RpClump* clump = RpClumpStreamRead(stream);
        if (clump) {
            RpClumpForAllAtomics(clump, FindRelatedModelInfoCB, clump);
            RpClumpDestroy(clump);
        }
    }
    RwStreamClose(stream, nullptr);
}

// unused
// 0x537060
RpClump* CFileLoader::LoadAtomicFile2Return(const char* filename) {
    RpClump* clump = nullptr;
    RwStream* stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
    if (RwStreamFindChunk(stream, rwID_CLUMP, nullptr, nullptr))
        clump = RpClumpStreamRead(stream);

    RwStreamClose(stream, nullptr);
    return clump;
}

// Find first non-null, non-whitespace character
char* FindFirstNonNullOrWS(char* it) {
    // Have to cast to uint8, because signed ASCII is retarded
    for (; *it && (uint8)*it <= (uint8)' '; it++);
    return it;
}

// 0x536F80
// Load line into static buffer (`ms_line`)
char* CFileLoader::LoadLine(FILESTREAM file) {
    if (!CFileMgr::ReadLine(file, ms_line, sizeof(ms_line)))
        return nullptr;

    // Sanitize it (otherwise random crashes appear)
    for (char* it = ms_line; *it; it++) {
        // Have to cast to uint8, because signed ASCII is retarded
        if ((uint8)*it < (uint8)' ' || *it == ',')
            *it = ' ';
    }
 
    return FindFirstNonNullOrWS(ms_line);
}

// 0x536FE0
// Load line from a text buffer
// bufferIt - Iterator into buffer. It is modified by this function to point after the last character of this line
// buffSize - Size of buffer. It is modified to repesent the size of the buffer remaining after the end of this line
char* CFileLoader::LoadLine(char*& bufferIt, int32& buffSize) {
    if (buffSize <= 0 || !*bufferIt)
        return nullptr;

    // Copy with sanitization (Otherwise random crashes appear)
    char* copyIt = s_MemoryHeapBuffer;
    for (; *bufferIt && *bufferIt != '\n' && buffSize != 0; bufferIt++, buffSize--) {
        // Have to cast to uint8, because signed ASCII is retarded
        *copyIt++ = ((uint8)*bufferIt < (uint8)' ' || *bufferIt == ',') ? ' ' : *bufferIt; // Replace chars before space and ',' (comma) by space, otherwise copy
    }

    bufferIt++;    // Make it point after end of line
    *copyIt = 0;   // Null terminate the copy

    return FindFirstNonNullOrWS(s_MemoryHeapBuffer);
}

// IPL -> AUZO
// 0x5B4D70
void CFileLoader::LoadAudioZone(const char* line) {
    char  name[16];
    int32   id;
    int32   enabled;
    float x1, y1, z1;
    float x2, y2, z2;
    float radius;

    int32 iNumRead = sscanf(line, "%s %d %d %f %f %f %f %f %f", name, &id, &enabled, &x1, &y1, &z1, &x2, &y2, &z2);
    if (iNumRead == 9) {
        CAudioZones::RegisterAudioBox(name, id, enabled != 0, x1, y1, z1, x2, y2, z2);
        return;
    }

    sscanf(line, "%s %d %d %f %f %f %f", name, &id, &enabled, &x1, &y1, &z1, &radius);
    CAudioZones::RegisterAudioSphere(name, id, enabled != 0, x1, y1, z1, radius);
}

// unused?
// 0x0
void CFileLoader::LoadBoundingBox(uint8* data, CBoundingBox& outBoundBox) {

}

// 0x537990
void CFileLoader::LoadCarGenerator(CFileCarGenerator* carGen, int32 iplId) {
    auto index = CTheCarGenerators::CreateCarGenerator(
        carGen->m_vecPosn,
        RWRAD2DEG(carGen->m_fAngle),
        carGen->m_nModelId,
        carGen->m_nPrimaryColor,
        carGen->m_nSecondaryColor,
        carGen->m_bForceSpawn,
        carGen->m_nAlarmChance,
        carGen->m_nDoorLockChance,
        carGen->m_nMinDelay,
        carGen->m_nMaxDelay,
        iplId,
        carGen->m_bIgnorePopulationLimit
    );
    if (index >= 0)
        CTheCarGenerators::Get(index)->SwitchOn();
}

// IPL -> CARS
// 0x5B4740
void CFileLoader::LoadCarGenerator(const char* line, int32 iplId) {
    CFileCarGenerator carGen;
    auto iNumRead = sscanf(
        line,
        "%f %f %f %f %d %d %d %d %d %d %d %d",
        &carGen.m_vecPosn.x,
        &carGen.m_vecPosn.y,
        &carGen.m_vecPosn.z,
        &carGen.m_fAngle,
        &carGen.m_nModelId,
        &carGen.m_nPrimaryColor,
        &carGen.m_nSecondaryColor,
        &carGen.m_nFlags,
        &carGen.m_nAlarmChance,
        &carGen.m_nDoorLockChance,
        &carGen.m_nMinDelay,
        &carGen.m_nMaxDelay
    );
    if (iNumRead == 12)
        LoadCarGenerator(&carGen, iplId);
}

// 0x5B4380
void CFileLoader::LoadCarPathNode(const char* line, int32 objModelIndex, int32 pathEntryIndex, bool a4) {
    // Loads some data from the line, and calls a function which does nothing, so the whole function.. does nothing.
    // Leftover from VC
}

// 0x5373F0
bool CFileLoader::StartLoadClumpFile(RwStream* stream, uint32 modelIndex) {
    auto chunk = RwStreamFindChunk(stream, rwID_CLUMP, nullptr, nullptr);
    if (!chunk) {
        return false;
    }

    CBaseModelInfo* modelInfo = CModelInfo::GetModelInfo(modelIndex);
    bool isVehicle = modelInfo->GetModelType() == MODEL_INFO_VEHICLE;

    if (isVehicle)
        CVehicleModelInfo::UseCommonVehicleTexDicationary();

    auto clumpReaded = RpClumpGtaStreamRead1(stream);

    if (isVehicle)
        CVehicleModelInfo::StopUsingCommonVehicleTexDicationary();

    return clumpReaded;
}

// 0x537450
bool CFileLoader::FinishLoadClumpFile(RwStream* stream, uint32 modelIndex) {
    auto modelInfo = static_cast<CClumpModelInfo*>(CModelInfo::ms_modelInfoPtrs[modelIndex]);
    bool isVehicle = modelInfo->GetModelType() == MODEL_INFO_VEHICLE;

    if (isVehicle)
        CVehicleModelInfo::UseCommonVehicleTexDicationary();

    RpClump* clump = RpClumpGtaStreamRead2(stream);

    if (isVehicle)
        CVehicleModelInfo::StopUsingCommonVehicleTexDicationary();

    if (!clump)
        return false;

    modelInfo->SetClump(clump);
    return true;
}

// 0x5372D0
bool CFileLoader::LoadClumpFile(RwStream* stream, uint32 modelIndex) {
    return plugin::CallAndReturn<bool, 0x5372D0, RwStream*, uint32>(stream, modelIndex);
}

// 0x5B3A30
void CFileLoader::LoadClumpFile(const char* filename) {
    RwStream* stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename);
    while (RwStreamFindChunk(stream, rwID_CLUMP, nullptr, nullptr)) {
        RpClump* clump = RpClumpStreamRead(stream);
        if (clump) {
            const char* nodeName = GetFrameNodeName(RpClumpGetFrame(clump));
            auto modelInfo = static_cast<CClumpModelInfo*>(CModelInfo::GetModelInfo(nodeName, nullptr));
            if (modelInfo)
                modelInfo->SetClump(clump);
            else
                RpClumpDestroy(clump);
        }
    }
    RwStreamClose(stream, nullptr);
}

// 0x5B4040
int32 CFileLoader::LoadClumpObject(const char* line) {
    char modelName[24];
    char texName[24];
    int32  objId = MODEL_INVALID;

    auto iNumRead = sscanf(line, "%d %s %s", &objId, modelName, texName);
    if (iNumRead != 3)
        return MODEL_INVALID;

    auto modelInfo = static_cast<CVehicleModelInfo*>(CModelInfo::AddClumpModel(objId));
    modelInfo->SetModelName(modelName);
    modelInfo->SetTexDictionary(texName);
    modelInfo->SetColModel(&CTempColModels::ms_colModelBBox, false);
    return objId;
}

// 0x538440
bool CFileLoader::LoadCollisionFile(uint8* data, uint32 dataSize, uint8 colId) {
    return plugin::CallAndReturn<bool, 0x538440, uint8*, uint32, uint8>(data, dataSize, colId);
}

// 0x5B4E60
bool CFileLoader::LoadCollisionFile(const char* filename, uint8 colId) {
    return plugin::CallAndReturn<bool, 0x5B4E60, const char*, uint8>(filename, colId);
}

// 0x5B5000
bool CFileLoader::LoadCollisionFileFirstTime(uint8* data, uint32 dataSize, uint8 colId) {
    return plugin::CallAndReturn<bool, 0x5B5000, uint8*, uint32, uint8>(data, dataSize, colId);
}

// 0x537580
void CFileLoader::LoadCollisionModel(uint8* data, CColModel& outColModel) {
    plugin::Call<0x537580, uint8*, CColModel&>(data, outColModel);
}

// 0x537EE0
void CFileLoader::LoadCollisionModelVer2(uint8* data, uint32 dataSize, CColModel& outColModel, const char* modelName) {
    plugin::Call<0x537EE0, uint8*, uint32, CColModel&, const char*>(data, dataSize, outColModel, modelName);
}

// 0x537CE0
void CFileLoader::LoadCollisionModelVer3(uint8* data, uint32 dataSize, CColModel& outColModel, const char* modelName) {
    plugin::Call<0x537CE0, uint8*, uint32, CColModel&, const char*>(data, dataSize, outColModel, modelName);
}

// 0x537AE0
void CFileLoader::LoadCollisionModelVer4(uint8* data, uint32 dataSize, CColModel& outColModel, const char* modelName) {
    plugin::Call<0x537AE0, uint8*, uint32, CColModel&, const char*>(data, dataSize, outColModel, modelName);
}

// 0x5B3C60
int32 CFileLoader::LoadObject(const char* line) {
    int32      modelId;
    char     modelName[24];
    char     texName[24];
    float    fDrawDist;
    uint32 nFlags;

    auto iNumRead = sscanf(line, "%d %s %s %f %d", &modelId, modelName, texName, &fDrawDist, &nFlags);
    if (iNumRead != 5 || fDrawDist < 4.0f)
    {
        int32 objType;
        float fDrawDist2_unused, fDrawDist3_unused;
        iNumRead = sscanf((char*)line, "%d %s %s %d", &modelId, modelName, texName, &objType);
        if (iNumRead != 4)
            return -1;

        switch (objType)
        {
        case 1:
            sscanf(line, "%d %s %s %d %f %d", &modelId, modelName, texName, &objType, &fDrawDist, &nFlags);
            break;
        case 2:
            sscanf(line, "%d %s %s %d %f %f %d", &modelId, modelName, texName, &objType, &fDrawDist, &fDrawDist2_unused, &nFlags);
            break;
        case 3:
            sscanf(line, "%d %s %s %d %f %f %f %d", &modelId, modelName, texName, &objType, &fDrawDist, &fDrawDist2_unused, &fDrawDist3_unused, &nFlags);
            break;
        }
    }

    sItemDefinitionFlags flags(nFlags);
    CAtomicModelInfo* pModelInfo;
    if (flags.bIsDamageable)
        pModelInfo = CModelInfo::AddDamageAtomicModel(modelId);
    else
        pModelInfo = CModelInfo::AddAtomicModel(modelId);

    pModelInfo->m_fDrawDistance = fDrawDist;
    pModelInfo->SetModelName(modelName);
    pModelInfo->SetTexDictionary(texName);
    SetAtomicModelInfoFlags(pModelInfo, nFlags);
    return modelId;
}

// 0x5B7670
void CFileLoader::Load2dEffect(const char* line) {
    plugin::Call<0x5B7670, const char*>(line);
}

// 0x538090
CEntity* CFileLoader::LoadObjectInstance(CFileObjectInstance* objInstance, const char* modelName) {
    auto* pInfo = CModelInfo::GetModelInfo(objInstance->m_nModelId);
    if (!pInfo)
        return nullptr;

    CEntity* pNewEntity = nullptr;
    if (pInfo->m_nObjectInfoIndex == -1)
    {
        if (pInfo->GetModelType() == ModelInfoType::MODEL_INFO_CLUMP && pInfo->bHasAnimBlend)
            pNewEntity = new CAnimatedBuilding();
        else
            pNewEntity = new CBuilding();

        pNewEntity->SetModelIndexNoCreate(objInstance->m_nModelId);
        if (pInfo->bDontCastShadowsOn)
            pNewEntity->m_bDontCastShadowsOn = true;

        if (pInfo->m_fDrawDistance < 2.0F)
            pNewEntity->m_bIsVisible = false;
    }
    else
    {
        pNewEntity = new CDummyObject();
        pNewEntity->SetModelIndexNoCreate(objInstance->m_nModelId);
        if (IsGlassModel(pNewEntity) && !CModelInfo::GetModelInfo(pNewEntity->m_nModelIndex)->IsGlassType2())
            pNewEntity->m_bIsVisible = false;
    }

    if (fabs(objInstance->m_qRotation.imag.x) > 0.05F
        || fabs(objInstance->m_qRotation.imag.y) > 0.05F
        || (objInstance->m_bDontStream && objInstance->m_qRotation.imag.x != 0.0f && objInstance->m_qRotation.imag.y != 0.0f))
    {
        objInstance->m_qRotation.imag = -objInstance->m_qRotation.imag;
        pNewEntity->AllocateStaticMatrix();

        auto tempQuat = objInstance->m_qRotation;
        pNewEntity->GetMatrix().SetRotate(tempQuat);
    }
    else
    {
        const auto fMult = objInstance->m_qRotation.imag.z < 0.0f ? 2.0f : -2.0f;
        const auto fHeading = acos(objInstance->m_qRotation.real) * fMult;
        pNewEntity->SetHeading(fHeading);
    }

    pNewEntity->SetPosn(objInstance->m_vecPosition);

    if (objInstance->m_bUnderwater)
        pNewEntity->m_bUnderwater = true;
    if (objInstance->m_bTunnel)
        pNewEntity->m_bTunnel = true;
    if (objInstance->m_bTunnelTransition)
        pNewEntity->m_bTunnelTransition = true;
    if (objInstance->m_bRedundantStream)
        pNewEntity->m_bUnimportantStream = true;
    pNewEntity->m_nAreaCode = objInstance->m_nAreaCode;
    pNewEntity->m_nLodIndex = objInstance->m_nLodInstanceIndex;

    if (objInstance->m_nModelId == ModelIndices::MI_TRAINCROSSING)
    {
        pNewEntity->GetMatrix();
        pNewEntity->AllocateStaticMatrix();
        CObject::SetMatrixForTrainCrossing(&pNewEntity->GetMatrix(), PI * 0.43f);
    }

    auto* pColModel = pInfo->GetColModel();
    if (pColModel)
    {
        if (pColModel->m_boundSphere.m_bFlag0x01)
        {
            if (pColModel->m_boundSphere.m_nColSlot)
            {
                CRect rect;
                pNewEntity->GetBoundRect(&rect);
                auto* pColDef = CColStore::ms_pColPool->GetAt(pColModel->m_boundSphere.m_nColSlot);
                pColDef->m_Area.Restrict(rect);
            }
        }
        else
        {
            pNewEntity->m_bUsesCollision = false;
        }

        if (pColModel->GetBoundingBox().m_vecMin.z + pNewEntity->GetPosition().z < 0.0f)
            pNewEntity->m_bUnderwater = true;
    }

    return pNewEntity;
}

// 0x538090
CEntity* CFileLoader::LoadObjectInstance(const char* line) {
    char modelName[24];
    CFileObjectInstance instance;
    sscanf(
        line,
        "%d %s %d %f %f %f %f %f %f %f %d",
        &instance.m_nModelId,
        modelName,
        &instance.m_nInstanceType,
        &instance.m_vecPosition.x,
        &instance.m_vecPosition.y,
        &instance.m_vecPosition.z,
        &instance.m_qRotation.imag.x,
        &instance.m_qRotation.imag.y,
        &instance.m_qRotation.imag.z,
        &instance.m_qRotation.real,
        &instance.m_nLodInstanceIndex
    );
    return LoadObjectInstance(&instance, modelName);
}

// 0x5B4B40
void CFileLoader::LoadCullZone(const char* line) {
    CVector center;
    float unknown;
    float length;
    float bottom;
    float width;
    float unknown2;
    float zTop;
    CVector mirrorDirection;
    float cm;
    int32 flags, flags2 = 0;

    int32 iNumRead = sscanf(
        line,
        "%f %f %f %f %f %f %f %f %f %d %f %f %f %f",
        &center.x,
        &center.y,
        &center.z,
        &unknown,
        &length,
        &bottom,
        &width,
        &unknown2,
        &zTop,
        &flags,
        &mirrorDirection.x,
        &mirrorDirection.y,
        &mirrorDirection.z,
        &cm
    );
    if (iNumRead == 14) {
        CCullZones::AddMirrorAttributeZone(
            center,
            unknown,
            length,
            bottom,
            width,
            unknown2,
            zTop,
            static_cast<eZoneAttributes>(flags),
            cm,
            mirrorDirection.x, mirrorDirection.y, mirrorDirection.z
        );
        return;
    }

    sscanf(
        line,
        "%f %f %f %f %f %f %f %f %f %d %d",
        &center.x,
        &center.y,
        &center.z,
        &unknown,
        &length,
        &bottom,
        &width,
        &unknown2,
        &zTop,
        &flags,
        &flags2
    );
    CCullZones::AddCullZone(center, unknown, length, bottom, width, unknown2, zTop, flags);
}

// IPL -> ENEX
// 0x5B8030
void CFileLoader::LoadEntryExit(const char* line) {
    uint32 numOfPeds = 2;
    uint32 timeOn = 0, timeOff = 24;
    float enterX, enterY, enterZ;
    float rangeX, rangeY;
    float enteranceAngle;
    float unused;
    float exitX, exitY, exitZ;
    float exitAngle;
    int32 area;
    char interiorName[64]{};
    uint32 skyColor;
    uint32 flags;

    (void)sscanf(
        line,
        "%f %f %f %f %f %f %f %f %f %f %f %d %d %s %d %d %d %d",
        &enterX,
        &enterY,
        &enterZ,
        &enteranceAngle,
        &rangeX,
        &rangeY,
        &unused,
        &exitX,
        &exitY,
        &exitZ,
        &exitAngle,
        &area,
        &flags,
        interiorName,
        &skyColor,
        &numOfPeds,
        &timeOn,
        &timeOff
    );

    auto name = strrchr(interiorName, '"');
    if (name) {
        *name = 0;
        name = &interiorName[1];
    }

    const auto enexPoolIdx = CEntryExitManager::AddOne(
        enterX,
        enterY,
        enterZ,
        enteranceAngle,
        rangeX,
        rangeY,
        unused,
        exitX,
        exitY,
        exitZ,
        exitAngle,
        area,
        flags,
        skyColor,
        timeOn,
        timeOff,
        numOfPeds,
        name
    );
    const auto enex = CEntryExitManager::mp_poolEntryExits->GetAt(enexPoolIdx);
    assert(enex);

    enum Flags {
        UNKNOWN_INTERIOR,
        UNKNOWN_PAIRING,
        CREATE_LINKED_PAIR,
        REWARD_INTERIOR,
        USED_REWARD_ENTRANCE,
        CARS_AND_AIRCRAFT,
        BIKES_AND_MOTORCYCLES,
        DISABLE_ONFOOT
    };

    if (flags & UNKNOWN_INTERIOR)
        enex->m_nFlags.bUnknownInterior = true;

    if (flags & UNKNOWN_PAIRING)
        enex->m_nFlags.bUnknownPairing = true;

    if (flags & CREATE_LINKED_PAIR)
        enex->m_nFlags.bCreateLinkedPair = true;

    if (flags & REWARD_INTERIOR)
        enex->m_nFlags.bRewardInterior = true;

    if (flags & USED_REWARD_ENTRANCE)
        enex->m_nFlags.bUsedRewardEntrance = true;

    if (flags & CARS_AND_AIRCRAFT)
        enex->m_nFlags.bCarsAndAircraft = true;

    if (flags & BIKES_AND_MOTORCYCLES)
        enex->m_nFlags.bBikesAndMotorcycles = true;

    if (flags & DISABLE_ONFOOT)
        enex->m_nFlags.bDisableOnFoot = true;
}

// IPL -> GRGE
// 0x5B4530
void CFileLoader::LoadGarage(const char* line) {
    uint32 flags;
    uint32 type;
    float x1, y1, z1;
    float x2, y2, z2;
    float frontX, frontY;
    char name[128];

    if (sscanf(
        line,
        "%f %f %f %f %f %f %f %f %d %d %s",
        &x1,
        &y1,
        &z1,
        &frontX,
        &frontY,
        &x2,
        &y2,
        &z2,
        &flags,
        &type,
        &name) == 11
    ) {
        CGarages::AddOne(x1, y1, z1, frontX, frontY, x2, y2, z2, (eGarageType)type, 0, name, flags);
    }
}

// 0x5B9030
void CFileLoader::LoadLevel(const char* levelFileName) {
    char pathBuffer[128]{};

    auto pRwCurrTexDict = RwTexDictionaryGetCurrent();
    if (!pRwCurrTexDict) {
        pRwCurrTexDict = RwTexDictionaryCreate();
        RwTexDictionarySetCurrent(pRwCurrTexDict);
    }

    bool hasLoadedAnyIPLs{};

    const auto f = CFileMgr::OpenFile(levelFileName, "r");
    for (auto l = LoadLine(f); l; l = LoadLine(f)) {
        const auto LineBeginsWith = [l](auto what) {
            return strncmp(what, l, strlen(what));
        };

        // Extract path after identifier like: <ID> <PATH>
        char pathBuffer[MAX_PATH]{};
        const auto ExtractPathFor = [&](auto id) {
            // Okay, so..
            // Originally they didn't copy the path into a separate buffer for each "id"
            // But we are going to, for two reasons:
            // - Copy is cheap
            // - Lifetime issue
            // The latter is the reason why they had to make a copy in the first place.
            // If we were to just return `l + strlen(id) + 1` we'd depend on the line buffer's content not to change,
            // but the line is just a static buffer (ms_line) which is modified each time `LoadLine` is called.
            // So if any of the invoked functions call `LoadLine` the path will no longer be valid
            // So in order to prevent this nasty bug we're just going to copy it each time.

            strncpy_s(pathBuffer, l + strlen(id) + 1, std::size(pathBuffer) - 1);
            return pathBuffer;
        };

        if (LineBeginsWith("#"))
            continue; // Skip comment

        if (LineBeginsWith("EXIT"))
            break; // Done

        if (LineBeginsWith("TEXDICTION")) {
            // Originally here they've copied the path into a buffer
            // We ain't gonna do that, there's no point to it
            const auto path = ExtractPathFor("TEXDICTION");

            LoadingScreenLoadingFile(path);

            const auto txd = CFileLoader::LoadTexDictionary(path);
            RwTexDictionaryForAllTextures(txd, AddTextureCB, pRwCurrTexDict);
            RwTexDictionaryDestroy(txd);

        } else if (LineBeginsWith("IPL")) {
            if (!hasLoadedAnyIPLs) {
                MatchAllModelStrings();

                LoadingScreenLoadingFile("Object Data");
                CObjectData::Initialise("DATA\\OBJECT.DAT");

                LoadingScreenLoadingFile("Setup vehicle info data");
                CVehicleModelInfo::SetupCommonData();

                LoadingScreenLoadingFile("Streaming Init");
                CStreaming::Init2();

                CLoadingScreen::NewChunkLoaded();

                LoadingScreenLoadingFile("Collision");
                CColStore::LoadAllBoundingBoxes();

                // TODO: Probably inlined.
                for (auto mi : CModelInfo::ms_modelInfoPtrs) {
                    if (mi) {
                        mi->ConvertAnimFileIndex();
                    }
                }

                hasLoadedAnyIPLs = true;
            }

            // Originally here they've copied the path into a buffer
            // We ain't gonna do that, there's no point to it
            const auto path = ExtractPathFor("IPL");
            LoadingScreenLoadingFile(path);
            CFileLoader::LoadScene(path);
        } else {
            // Deal with the rest (Originally more `else-if`s were used)
            // Sadly we can't put all of the above in here as well
            // because they'd need a capturing lambda, and that would require
            // us to use `std::function` which is just overkill in this case.

            using FnType = void(*)(const char*);
            const struct { const char* id;  FnType fn; } functions[]{
                {"IMG", [](const char* path) {
                    if (path == std::string_view{ "MODELS\\GTA_INT.IMG" }) { // Only allowed to load GTA_INT.IMG
                        CStreaming::AddImageToList(path, true);
                    }
                }},
                {"COLFILE", [](const char* path) { LoadCollisionFile(path, 0); }},
                {"MODELFILE", LoadAtomicFile},
                {"HIERFILE", LoadClumpFile},
                {"IDE", LoadObjectTypes},
                //{"SPLASH", [](const char*) {}} - Unused
            };
            for (const auto& v : functions) {
                if (LineBeginsWith(v.id)) {
                    v.fn(ExtractPathFor(v.id));
                    break;
                }
            }
        }
    }
    CFileMgr::CloseFile(f);

    RwTexDictionarySetCurrent(pRwCurrTexDict);
    if (hasLoadedAnyIPLs)
    {
        CIplStore::LoadAllRemainingIpls();
        CColStore::BoundingBoxesPostProcess();
        CTrain::InitTrains();
        CColStore::RemoveAllCollision();
    }
}

// IPL -> OCCL
// 0x5B4C80
void CFileLoader::LoadOcclusionVolume(const char* line, const char* filename) {
    float fRotX = 0.0F, fRotY = 0.0F;
    uint32 nFlags = 0;
    float fCenterX, fCenterY, fBottomZ, fWidth, fLength, fHeight, fRotZ;

    sscanf(line, "%f %f %f %f %f %f %f %f %f %d ", &fCenterX, &fCenterY, &fBottomZ, &fWidth, &fLength, &fHeight, &fRotX, &fRotY, &fRotZ, &nFlags);
    auto fCenterZ = fHeight * 0.5F + fBottomZ;
    auto strLen = strlen(filename);

    bool bIsInterior = false;
    if (filename[strLen - 7] == 'i' && filename[strLen - 6] == 'n' && filename[strLen - 5] == 't')
        bIsInterior = true;

    COcclusion::AddOne(fCenterX, fCenterY, fCenterZ, fWidth, fLength, fHeight, fRotX, fRotY, fRotZ, nFlags, bIsInterior);
}

// 0x5B41C0
int32 CFileLoader::LoadPathHeader(const char* line, int32& outPathType) {
    int32 id;
    char modelName[32];

    sscanf(line, "%d %d %s", &outPathType, &id, modelName);
    return id;
}

// PEDS
// 0x5B7420
int32 CFileLoader::LoadPedObject(const char* line) {
    int16 audioPedType;
    int16 voice2_1;
    int16 voice_id;
    int modelId{-1};
    int radio2;
    int radio1;
    int flags;
    int carsCanDriveMask;

    // TODO: Should probably increase the size of these to avoid possible buffer overflow
    char animFile[12];
    char modelName[20];
    char pedVoiceType[16];
    char statName[20];
    char animGroup[20];
    char pedType[24];
    char texName[20];
    char voiceMin[56];
    char voiceMax[60];

    (void)sscanf(
        line,
        "%d %s %s %s %s %s %x %x %s %d %d %s %s %s",
        &modelId,
        modelName,
        texName,
        pedType,
        statName,
        animGroup,
        &carsCanDriveMask,
        &flags,
        animFile,
        &radio1,
        &radio2,
        pedVoiceType,
        voiceMin,
        voiceMax
    );

    const auto FindAnimGroup = [animGroup, nAssocGroups = CAnimManager::ms_numAnimAssocDefinitions] {
        for (auto i = 0; i < nAssocGroups; i++) {
            if (CAnimManager::GetAnimGroupName((AssocGroupId)i) == std::string_view{animGroup}) {
                return i;
            }
        }
        return nAssocGroups;
    };

    const auto mi = CModelInfo::AddPedModel(modelId);

    mi->m_nKey = CKeyGen::GetUppercaseKey(modelName);
    mi->SetTexDictionary(texName);
    mi->SetAnimFile(animFile);
    mi->SetColModel(&colModelPeds, false);
    mi->m_nPedType = CPedType::FindPedType(pedType);
    mi->m_nStatType = CPedStats::GetPedStatType(statName);
    mi->m_nAnimType = FindAnimGroup();
    mi->m_nCarsCanDriveMask = carsCanDriveMask;
    mi->m_nPedFlags = flags;
    mi->m_nRadio2 = radio2 + 1;
    mi->m_nRadio1 = radio1 + 1;
    mi->m_nRace = CPopulation::FindPedRaceFromName(modelName);
    mi->m_nPedAudioType = CAEPedSpeechAudioEntity::GetAudioPedType(pedVoiceType);
    mi->m_nVoiceMin = CAEPedSpeechAudioEntity::GetVoice(voiceMin, mi->m_nPedAudioType);
    mi->m_nVoiceMax = CAEPedSpeechAudioEntity::GetVoice(voiceMax, mi->m_nPedAudioType);
    mi->m_nVoiceId = mi->m_nVoiceMin;

    return modelId;
}

// useless
// 0x5B41F0
void CFileLoader::LoadPedPathNode(const char* line, int32 objModelIndex, int32 pathEntryIndex) {
    // This function loads the file, reads some data
    // and calls `CPathFind::StoreDetachedNodeInfoPed` or `CPathFind::StoreNodeInfoPed`
    // but both functions are NOPs, so this function basically doesn't do anything useful.
}

// 0x5B47B0
// https://gta.fandom.com/wiki/PICK
void CFileLoader::LoadPickup(const char* line) {
    CVector pos{};
    int32 weaponType{};
    if (sscanf(line, "%d %f %f %f", &weaponType, &pos.x, &pos.y, &pos.z) == 4) {
        // TODO: Maybe, some day, use enums here (eModelID for the model, and eWeaponType for the wepID)
        const auto GetModel = [weaponType] {
            switch (weaponType) {
            case 4:
                return 331;
            case 5:
                return 334;
            case 6:
                return 335;
            case 9:
                return 333;
            case 10:
                return 336;
            case 11:
                return 337;
            case 12:
                return 338;
            case 13:
                return 339;
            case 14:
                return 341;
            case 15:
                return 344;
            case 16:
                return 342;
            case 17:
                return 363;
            case 18:
                return 346;
            case 19:
                return 347;
            case 20:
                return 348;
            case 21:
                return 349;
            case 22:
            case 45:
                return 351;
            case 23:
                return 372;
            case 24:
                return 352;
            case 25:
                return 353;
            case 26:
                return 355;
            case 27:
                return 356;
            case 28:
                return 357;
            case 29:
                return 358;
            case 31:
                return 361;
            case 32:
            case 44:
                return 362;
            case 33:
                return 321;
            case 34:
                return 322;
            case 35:
                return 323;
            case 36:
                return 324;
            case 37:
                return 325;
            case 38:
                return 326;
            case 39:
                return 327;
            case 40:
                return 328;
            case 41:
                return 330;
            case 43:
                return 343;
            case 46:
                return 359;
            case 47:
                return 360;
            case 48:
                return 364;
            case 49:
                return 365;
            case 50:
                return 366;
            case 51:
                return 367;
            case 52:
                return 368;
            case 53:
                return 369;
            case 54:
                return 370;
            case 55:
                return 371;
            default:
                return -1;
            }
        };
        if (const auto model = GetModel(); model != -1) {
            CPickups::GenerateNewOne(pos, model, 2, 0, 0, false, nullptr);
        }
    }
}

// 0x5B45D0
void CFileLoader::LoadStuntJump(const char* line) {
    CVector b1Min;
    CVector b1Max;
    CVector b2Min;
    CVector b2Max;
    CVector cameraPosn;
    int32     reward;

    int32 iNumRead = sscanf(
        line,
        "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %d",
        &b1Min.x,
        &b1Min.y,
        &b1Min.z,
        &b1Max.x,
        &b1Max.y,
        &b1Max.z,

        &b2Min.x,
        &b2Min.y,
        &b2Min.z,
        &b2Max.x,
        &b2Max.y,
        &b2Max.z,

        &cameraPosn.x,
        &cameraPosn.y,
        &cameraPosn.z,
        &reward
    );
    if (iNumRead == 16) {
         CBoundingBox start{b1Min, b1Max};
         CBoundingBox target{b2Min, b2Max};
         CStuntJumpManager::AddOne(start, target, cameraPosn, reward);
    }
}

// 0x5B75E0
int32 CFileLoader::LoadTXDParent(const char* line) {
    char name[32];
    char parentName[32];

    sscanf(line, "%s %s", name, parentName);
    int32 txdSlot = CTxdStore::FindTxdSlot(name);
    if (txdSlot == INVALID_POOL_SLOT)
        txdSlot = CTxdStore::AddTxdSlot(name);

    int32 parentSlot = CTxdStore::FindTxdSlot(parentName);
    if (parentSlot == INVALID_POOL_SLOT)
        parentSlot = CTxdStore::AddTxdSlot(parentName);

    CTxdStore::ms_pTxdPool->GetAt(txdSlot)->m_wParentIndex = parentSlot;

    return parentSlot;
}

// IPL -> TCYC
// 0x5B81D0
void CFileLoader::LoadTimeCyclesModifier(const char* line) {
    CVector vec1, vec2;
    int32 farClip;
    int32 extraColor;
    float extraColorIntensity;
    float falloffDist = 100.0f;
    float unused = 1.0f;
    float lodDistMult = 1.0f;

    auto iNumRead = sscanf(
        line,
        "%f %f %f %f %f %f %d %d %f %f %f %f",
        &vec1.x,
        &vec1.y,
        &vec1.z,
        &vec2.x,
        &vec2.y,
        &vec2.z,
        &farClip,
        &extraColor,
        &extraColorIntensity,
        &falloffDist,
        &unused,
        &lodDistMult
    );
    if (iNumRead < 12)
        lodDistMult = unused;

    CBox box;
    box.Set(vec1, vec2);
    return CTimeCycle::AddOne(box, farClip, extraColor, extraColorIntensity, falloffDist, lodDistMult);
}

// 0x5B3DE0
int32 CFileLoader::LoadTimeObject(const char* line) {
    int32 modelId;
    char    modelName[24];
    char    texName[24];
    float   drawDistance[3];
    int32 flags;
    int32 timeOn;
    int32 timeOff;

    int32 numValuesRead = sscanf(line, "%d %s %s %f %d %d %d", &modelId, modelName, texName, &drawDistance[0], &flags, &timeOn, &timeOff);

    if (numValuesRead != 7 || drawDistance[0] < 4.0) {
        int32 numObjs;

        if (sscanf(line, "%d %s %s %d", &modelId, modelName, texName, &numObjs) != 4)
            return -1;

        switch (numObjs) {
        case 1:
            sscanf(line, "%d %s %s %d %f %d %d %d", &modelId, modelName, texName, &numObjs, &drawDistance[0], &flags, &timeOn, &timeOff);
            break;
        case 2:
            sscanf(line, "%d %s %s %d %f %f %d %d %d", &modelId, modelName, texName, &numObjs, &drawDistance[0], &drawDistance[1], &flags, &timeOn, &timeOff);
            break;
        case 3:
            sscanf(line, "%d %s %s %d %f %f %f %d %d %d", &modelId, modelName, texName, &numObjs, &drawDistance[0], &drawDistance[1], &drawDistance[2], &flags, &timeOn, &timeOff);
            break;
        }
    }

    CTimeModelInfo* timeModel = CModelInfo::AddTimeModel(modelId);
    timeModel->m_fDrawDistance = drawDistance[0];
    timeModel->SetModelName(modelName);
    timeModel->SetTexDictionary(texName);

    CTimeInfo* timeInfo = timeModel->GetTimeInfo();
    timeInfo->SetTimes(timeOn, timeOff);

    SetAtomicModelInfoFlags(timeModel, flags);

    CTimeInfo* otherTimeInfo = timeInfo->FindOtherTimeModel(modelName);
    if (otherTimeInfo)
        otherTimeInfo->SetOtherTimeModel(modelId);

    return modelId;
}

// 0x5B6F30
int32 CFileLoader::LoadVehicleObject(const char* line) {
    int32_t modelId{ -1 };
    char modelName[24]{};
    char texName[24]{};
    char type[8]{};
    char handlingName[8]{};
    char gameName[32]{};
    char anims[16]{};
    char vehCls[16]{};
    uint32_t frq{}, flags{};
    tVehicleCompsUnion vehComps{};
    uint32_t misc{}; // `m_fBikeSteerAngle` if model type is BMX/Bike, otherwise `m_nWheelModelIndex`
    float wheelSizeFront{}, wheelSizeRear{};
    int32_t wheelUpgradeCls{ -1 };

    sscanf(line, "%d %s %s %s %s %s %s %s %d %d %x %d %f %f %d",
        &modelId,
        modelName,
        texName,
        type,
        handlingName,
        gameName,
        anims,
        vehCls,
        &frq,
        &flags,
        &vehComps.m_nComps,
        &misc,
        &wheelSizeFront,
        &wheelSizeRear,
        &wheelUpgradeCls
    );

    uint16_t nTxdSlot = CTxdStore::FindTxdSlot("vehicle");
    if (nTxdSlot == -1)
        nTxdSlot = CTxdStore::AddTxdSlot("vehicle");

    auto pVehModelInfo = CModelInfo::AddVehicleModel(modelId);
    pVehModelInfo->SetModelName(modelName);
    pVehModelInfo->SetTexDictionary(texName);
    CTxdStore::ms_pTxdPool->GetAt(pVehModelInfo->m_nTxdIndex)->m_wParentIndex = nTxdSlot;
    pVehModelInfo->SetAnimFile(anims);

    // Replace `_` with ` ` (space)
    std::replace(gameName, gameName + strlen(gameName), '_', ' ');
    pVehModelInfo->SetGameName(gameName);

    pVehModelInfo->m_nFlags = flags;
    pVehModelInfo->m_extraComps = vehComps;


    // This isn't exactly R* did it, but that code is hot garbage anyways
    // They've used strcmp all the way, and.. It's bad.

    pVehModelInfo->SetVehicleType(type);
    switch (pVehModelInfo->m_nVehicleType) {
    case eVehicleType::VEHICLE_AUTOMOBILE:
    case eVehicleType::VEHICLE_MTRUCK:
    case eVehicleType::VEHICLE_QUAD:
    case eVehicleType::VEHICLE_HELI:
    case eVehicleType::VEHICLE_PLANE:
    case eVehicleType::VEHICLE_TRAILER: {
        pVehModelInfo->SetWheelSizes(wheelSizeFront, wheelSizeRear);
        pVehModelInfo->m_nWheelModelIndex = misc;
        break;
    }
    case eVehicleType::VEHICLE_FPLANE: {
        pVehModelInfo->SetWheelSizes(1.0f, 1.0f);
        pVehModelInfo->m_nWheelModelIndex = misc;
        break;
    }
    case eVehicleType::VEHICLE_BIKE:
    case eVehicleType::VEHICLE_BMX: {
        pVehModelInfo->SetWheelSizes(wheelSizeFront, wheelSizeRear);
        pVehModelInfo->m_fBikeSteerAngle = (float)misc;
        break;
    }
    }

    pVehModelInfo->SetHandlingId(handlingName);
    pVehModelInfo->m_nWheelUpgradeClass = wheelUpgradeCls;

    pVehModelInfo->SetVehicleClass(vehCls);
    if (pVehModelInfo->m_nVehicleClass != eVehicleClass::VEHICLE_CLASS_IGNORE) {
        pVehModelInfo->m_nFrq = frq;
    }

    return modelId;
}

// 0x5B3FB0
int32 CFileLoader::LoadWeaponObject(const char* line) {
    int32 objId;
    char modelName[24];
    char texName[24];
    char animName[16];
    int32 weaponType;
    float drawDist;

    sscanf(line, "%d %s %s %s %d %f", &objId, modelName, texName, animName, &weaponType, &drawDist);
    CWeaponModelInfo* weaponModel = CModelInfo::AddWeaponModel(objId);
    weaponModel->SetModelName(modelName);
    weaponModel->m_fDrawDistance = drawDist;
    weaponModel->SetTexDictionary(texName);
    weaponModel->SetAnimFile(animName);
    weaponModel->SetColModel(&CTempColModels::ms_colModelWeapon, false);
    return objId;
}

// 0x5B4AB0
void CFileLoader::LoadZone(const char* line) {
    char name[24];
    signed int type;
    CVector min, max;
    int32 island;
    char zoneName[12];

    auto iNumRead = sscanf(line, "%s %d %f %f %f %f %f %f %d %s", name, &type, &min.x, &min.y, &min.z, &max.x, &max.y, &max.z, &island, zoneName);
    if (iNumRead == 10)
        CTheZones::CreateZone(name, static_cast<eZoneType>(type), min.x, min.y, min.z, max.x, max.y, max.z, static_cast<eLevelName>(island), zoneName);
}

// 0x5B51E0
void LinkLods(int32 a1) {
    plugin::Call<0x5B51E0, int32>(a1);
}

// 0x5B8700
void CFileLoader::LoadScene(const char* filename) {
    gCurrIplInstancesCount = 0;

    enum class SectionID {
        NONE = 0, // NOTSA - Placeholder value

        PATH = 1,
        INST = 2,
        MULT = 3,
        ZONE = 4,
        CULL = 5,
        OCCL = 6,
        GRGE = 8,
        ENEX = 9,
        PICK = 10,
        CARS = 11,
        JUMP = 12,
        TCYC = 13,
        AUZO = 14,
    };

    auto sectionId{SectionID::NONE};

    int32 nPathEntryIndex{ -1 }, pathHeaderId{};
    int32 pathType{};

    auto file = CFileMgr::OpenFile(filename, "rb");
    for (char* line = LoadLine(file); line; line = LoadLine(file)) {
        if (!line[0]) // Emtpy line
            continue;

        const auto LineBeginsWith = [linesv = std::string_view{line}](auto what) {
            return linesv.starts_with(what);
        };

        if (LineBeginsWith("#"))
            continue; // Skip comment

        if (sectionId != SectionID::NONE) {
            if (LineBeginsWith("end")) {
                sectionId = SectionID::NONE;
                continue;
            }

            switch (sectionId) {
            case SectionID::INST: {
                CEntity* pObjInstance = LoadObjectInstance(line);
                gCurrIplInstances[gCurrIplInstancesCount++] = pObjInstance;
                break;
            }
            case SectionID::ZONE:
                LoadZone(line);
                break;
            case SectionID::CULL:
                LoadCullZone(line);
                break;
            case SectionID::OCCL:
                LoadOcclusionVolume(line, filename);
                break;
            case SectionID::PATH: {
                // This section doesn't do anything useful.
                // `LoadPedPathNode` is a NOP basically.
                // This is a leftover from VC. (Source: https://gta.fandom.com/wiki/Item_Placement#PATH )

                if (nPathEntryIndex == -1) {
                    pathHeaderId = LoadPathHeader(line, pathType);
                }
                else {
                    switch (pathType) {
                    case 0:
                        LoadPedPathNode(line, pathHeaderId, nPathEntryIndex);
                        break;
                    case 1:
                        LoadCarPathNode(line, pathHeaderId, nPathEntryIndex, false);
                        break;
                    case 2:
                        LoadCarPathNode(line, pathHeaderId, nPathEntryIndex, true);
                        break;
                    }
                    if (++nPathEntryIndex == 12)
                        nPathEntryIndex = -1;
                }
                break;
            }
            case SectionID::GRGE:
                LoadGarage(line);
                break;
            case SectionID::ENEX:
                LoadEntryExit(line);
                break;
            case SectionID::PICK:
                LoadPickup(line);
                break;
            case SectionID::CARS:
                LoadCarGenerator(line, 0);
                break;
            case SectionID::JUMP:
                LoadStuntJump(line);
                break;
            case SectionID::TCYC:
                LoadTimeCyclesModifier(line);
                break;
            case SectionID::AUZO:
                LoadAudioZone(line);
                break;
            }

            if (sectionId == SectionID::PATH)
                break; // TODO: Unsure why it stops after a path section.

        } else {
            const auto FindSectionID = [&] {
                const struct { std::string_view name; SectionID id; } mapping[]{
                    {"path", SectionID::PATH},
                    {"inst", SectionID::INST},
                    {"mult", SectionID::MULT},
                    {"zone", SectionID::ZONE},
                    {"cull", SectionID::CULL},
                    {"occl", SectionID::OCCL},
                    {"grge", SectionID::GRGE},
                    {"enex", SectionID::ENEX},
                    {"pick", SectionID::PICK},
                    {"cars", SectionID::CARS},
                    {"jump", SectionID::JUMP},
                    {"tcyc", SectionID::TCYC},
                    {"auzo", SectionID::AUZO},
                };

                for (const auto& [itname, id] : mapping) {
                    if (LineBeginsWith(itname)) {
                        return id;
                    }
                }

                return SectionID::NONE; // Possible if the line was empty, let's move on to the next line.
            };
            sectionId = FindSectionID();
        }
    }
    CFileMgr::CloseFile(file);

    // This really seems like should be in CIplStore...
    auto newIPLIndex{ -1 };
    if (gCurrIplInstancesCount > 0) {
        newIPLIndex = CIplStore::GetNewIplEntityIndexArray(gCurrIplInstancesCount);
        std::ranges::copy(gCurrIplInstances, gCurrIplInstances + gCurrIplInstancesCount, CIplStore::GetIplEntityIndexArray(newIPLIndex));
    }
    LinkLods(CIplStore::SetupRelatedIpls(filename, newIPLIndex, &gCurrIplInstances[gCurrIplInstancesCount]));
    CIplStore::RemoveRelatedIpls(newIPLIndex); // I mean this totally makes sense, doesn't it?
}

// 0x5B8400
void CFileLoader::LoadObjectTypes(const char* filename) {
    /* Unused
    char filenameCopy[MAX_PATH]{};
    strcpy(filenameCopy, filename);
    */

    enum class SectionID {
        NONE = 0, // NOTSA - Placeholder value

        OBJS = 1,
        TOBJ = 3,
        WEAP = 4,
        HIER = 5,
        ANIM = 6,
        CARS = 7,
        PEDS = 8,
        PATH = 9,
        TDFX = 10, // 2DFX (but enum names can't start with a number, so..)
        TXDP = 11
    };

    auto sectionId{ SectionID::NONE };

    int32 nPathEntryIndex{ -1 }, pathHeaderId{};
    int32 pathType{};

    auto file = CFileMgr::OpenFile(filename, "rb");
    for (const char* line = LoadLine(file); line; line = LoadLine(file)) {
        if (!line[0])
            continue;

        const auto LineBeginsWith = [linesv = std::string_view{ line }](auto what) {
            return linesv.starts_with(what);
        };

        if (LineBeginsWith("#"))
            continue;

        if (sectionId != SectionID::NONE) {
            // Process section

            if (LineBeginsWith("end")) {
                sectionId = SectionID::NONE;
                continue;
            }

            switch (sectionId) {
            case SectionID::OBJS:
                LoadObject(line);
                break;
            case SectionID::TOBJ:
                LoadTimeObject(line);
                break;
            case SectionID::WEAP:
                LoadWeaponObject(line);
                break;
            case SectionID::HIER:
                LoadClumpObject(line);
                break;
            case SectionID::ANIM:
                LoadAnimatedClumpObject(line);
                break;
            case SectionID::CARS:
                LoadVehicleObject(line);
                break;
            case SectionID::PEDS:
                LoadPedObject(line);
                break;
                // R* does something weird with the object IDs frm the above cases,
                // but it isn't used, so I wont put it in here, cause it would require jumps..
            case SectionID::PATH: {
                // Leftover from VC, as path's are loaded differently in SA.
                // That is, all this does nothing in the end.

                if (nPathEntryIndex == -1) {
                    pathHeaderId = LoadPathHeader(line, pathType);
                } else {
                    switch (pathType) {
                    case 0:
                        LoadPedPathNode(line, pathHeaderId, nPathEntryIndex);
                        break;
                    case 1:
                        LoadCarPathNode(line, pathHeaderId, nPathEntryIndex, false);
                        break;
                    case 2:
                        LoadCarPathNode(line, pathHeaderId, nPathEntryIndex, true);
                        break;
                    }
                    if (++nPathEntryIndex == 12)
                        nPathEntryIndex = -1;
                }
                break;
            }
            case SectionID::TDFX:
                Load2dEffect(line);
                break;
            case SectionID::TXDP:
                LoadTXDParent(line);
                break;
            }
        } else {
            // Find out next section

            const auto FindSectionID = [&] {
                const struct { std::string_view name; SectionID id; } mapping[]{
                    {"objs", SectionID::OBJS},
                    {"tobj", SectionID::TOBJ},
                    {"weap", SectionID::WEAP},
                    {"hier", SectionID::HIER},
                    {"anim", SectionID::ANIM},
                    {"cars", SectionID::CARS},
                    {"peds", SectionID::PEDS},
                    {"path", SectionID::PATH},
                    {"2dfx", SectionID::TDFX},
                    {"txdp", SectionID::TXDP},
                };

                for (const auto& [name, id] : mapping) {
                    if (LineBeginsWith(name)) {
                        return id;
                    }
                }

                return SectionID::NONE; // May happen if line was empty. It's fine.
            };
            sectionId = FindSectionID();
        }
    }
    CFileMgr::CloseFile(file);
}

// 0x5B3AC0
void CFileLoader::ReloadObjectTypes(const char* arg1) {
    // NOP
}

// Izzotop: Untested and may be wrong, see at your own risk
// unused
// 0x5B6E10
void CFileLoader::ReloadPaths(const char* filename) {
    int32  objModelIndex;
    int32  id;
    char unused[4];

    bool pathAllocated = false;
    int32 pathEntryIndex = -1;
    FILESTREAM file = CFileMgr::OpenFile(filename, "r");
    for (char* line = LoadLine(file); line; line = LoadLine(file)) {
        if (*line == '#' || !*line)
            continue;

        if (pathAllocated) {
            if (make_fourcc3(line, "end")) {
                pathAllocated = false;
            } else if (pathEntryIndex == -1) {
                sscanf(line, "%d %d %s", &id, &objModelIndex, unused);
                pathEntryIndex = 0;
            } else {
                if (id) {
                    if (id == 1) {
                        LoadCarPathNode(line, objModelIndex, pathEntryIndex, false);
                    } else if (id == 2) {
                        LoadCarPathNode(line, objModelIndex, pathEntryIndex, true);
                    }
                } else {
                    LoadPedPathNode(line, objModelIndex, pathEntryIndex);
                }

                if (++pathEntryIndex == 12)
                    pathEntryIndex = -1;
            }
        } else if (make_fourcc4(line, "path")) {
            pathAllocated = true;
            CPathFind::AllocatePathFindInfoMem();
        }
    }

    CFileMgr::CloseFile(file);
}

/**
 * @param atomic callback atomic
 * @param data clump object (RpClump*)
 * @return callback atomic
 * @addr 0x5B3930
 */
RpAtomic* CFileLoader::FindRelatedModelInfoCB(RpAtomic* atomic, void* data) {
    char name[24] = {0};
    bool bDamage = false;

    const char* nodeName = GetFrameNodeName(RpAtomicGetFrame(atomic));
    GetNameAndDamage(nodeName, name, bDamage);

    int32 modelId = MODEL_INVALID;
    CBaseModelInfo* modelInfo = CModelInfo::GetModelInfo(name, &modelId);
    if (modelInfo) {
        CAtomicModelInfo* atomicModelInfo = modelInfo->AsAtomicModelInfoPtr();
        CVisibilityPlugins::SetAtomicRenderCallback(atomic, nullptr);
        if (bDamage) {
            atomicModelInfo->AsDamageAtomicModelInfoPtr()->SetDamagedAtomic(atomic);
        } else {
            atomicModelInfo->SetAtomic(atomic);
        }
        RpClumpRemoveAtomic(static_cast<RpClump*>(data), atomic);
        RwFrame* pRwFrame = RwFrameCreate();
        RpAtomicSetFrame(atomic, pRwFrame);
        CVisibilityPlugins::SetAtomicId(atomic, modelId);
    }
    return atomic;
}

/**
 * @param atomic callback atomic
 * @param data clump object (RpClump*)
 * @return callback atomic
 * @addr 0x537150
 */
RpAtomic* CFileLoader::SetRelatedModelInfoCB(RpAtomic* atomic, void* data) {
    char name[24] = {0};
    bool bDamage = false;

    auto pAtomicModelInfo = CModelInfo::GetModelInfo(gAtomicModelId)->AsAtomicModelInfoPtr();
    const char* frameNodeName = GetFrameNodeName(RpAtomicGetFrame(atomic));

    GetNameAndDamage(frameNodeName, name, bDamage);
    CVisibilityPlugins::SetAtomicRenderCallback(atomic, nullptr);
    if (bDamage) {
        pAtomicModelInfo->AsDamageAtomicModelInfoPtr()->SetDamagedAtomic(atomic);
    } else {
        pAtomicModelInfo->SetAtomic(atomic);
    }
    RpClumpRemoveAtomic(static_cast<RpClump*>(data), atomic);
    RwFrame* newFrame = RwFrameCreate();
    RpAtomicSetFrame(atomic, newFrame);
    CVisibilityPlugins::SetAtomicId(atomic, gAtomicModelId);
    return atomic;
}

/**
 * Adds texture to the dictionary
 * @param texture callback texture
 * @param dict texture dictionary (RwTexDictionary*)
 * @return callback texture
 * @addr 0x5B38F0
 */
RwTexture* AddTextureCB(RwTexture* texture, void* dict) {
    RwTexDictionaryAddTexture((RwTexDictionary*)dict, texture);
    return texture;
}

/**
 * Makes a copy of atomic and adds it to clump
 * @param atomic callback atomic
 * @param data clump object (RpClump*)
 * @return callback atomic
 * @addr 0x537290
 */
RpAtomic* CloneAtomicToClumpCB(RpAtomic* atomic, void* data) {
    RpAtomic* clone = RpAtomicClone(atomic);
    auto frame = static_cast<RwFrame*>(atomic->object.object.parent);
    RpAtomicSetFrame(clone, frame->root);
    RpClumpAddAtomic(static_cast<RpClump*>(data), clone);
    return atomic;
}

// Gets file name from a path
// 0x5B3660
const char* GetFilename(const char* filepath) {
    const char* pch = strrchr(filepath, '\\');
    return pch ? pch + 1 : filepath;
}

// 0x5B3680
void LoadingScreenLoadingFile(const char* str) {
    const char* screenName = GetFilename(str);
    sprintf(gString, "Loading %s", screenName);
    LoadingScreen("Loading the Game", gString);
}
