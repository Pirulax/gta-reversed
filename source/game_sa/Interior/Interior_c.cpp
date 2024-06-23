#include "StdInc.h"
#include "Interior_c.h"
#include "./InteriorManager_c.h"
#include "./FurnitureManager_c.h"

void Interior_c::InjectHooks() {
    RH_ScopedClass(Interior_c);
    RH_ScopedCategory("Interior");

    //RH_ScopedInstall(Constructor, 0x5921D0, { .reversed = false });
    //RH_ScopedInstall(Destructor, 0x591360, { .reversed = false });
    RH_ScopedInstall(Init, 0x593BF0);
    RH_ScopedInstall(Exit, 0x592230);
    RH_ScopedInstall(Furnish, 0x591590);
    RH_ScopedInstall(UnFurnish, 0x5915D0);
    RH_ScopedInstall(GetBoundingBox, 0x593DB0);
    RH_ScopedInstall(FindBoundingBox, 0x5922C0);
    RH_ScopedInstall(IsPtInside, 0x5913E0);
    RH_ScopedInstall(CalcMatrix, 0x5914D0);
    RH_ScopedInstall(AddGotoPt, 0x591D20);
    RH_ScopedInstall(AddInteriorInfo, 0x591E40);
    RH_ScopedInstall(AddPickups, 0x591F90);
    RH_ScopedInstall(CalcExitPts, 0x5924A0, { .reversed = false });
    RH_ScopedInstall(IsVisible, 0x5929F0);
    
    //
    // Bedroom
    //
    RH_ScopedInstall(FurnishBedroom, 0x593FC0, { .reversed = false });
    RH_ScopedInstall(Bedroom_AddTableItem, 0x593F10);

    //
    // Kitchen
    //
    RH_ScopedInstall(FurnishKitchen, 0x5970B0, { .reversed = false });
    RH_ScopedInstall(Kitchen_FurnishEdges, 0x596930, { .reversed = false });

    //
    // Lounge
    //
    RH_ScopedInstall(FurnishLounge, 0x597740, { .reversed = false });
    RH_ScopedInstall(Lounge_AddTV, 0x597240, { .reversed = false });
    RH_ScopedInstall(Lounge_AddHifi, 0x597430, { .reversed = false });
    RH_ScopedInstall(Lounge_AddChairInfo, 0x5974E0, { .reversed = false });
    RH_ScopedInstall(Lounge_AddSofaInfo, 0x5975C0, { .reversed = false });

    //
    // Office
    //
    RH_ScopedInstall(FurnishOffice, 0x599AF0, { .reversed = false });
    RH_ScopedInstall(Office_PlaceEdgeFillers, 0x599210, { .reversed = false });
    RH_ScopedInstall(Office_PlaceDesk, 0x5993E0, { .reversed = false });
    RH_ScopedInstall(Office_PlaceEdgeDesks, 0x5995B0, { .reversed = false });
    RH_ScopedInstall(Office_FurnishEdges, 0x599770, { .reversed = false });
    RH_ScopedInstall(Office_PlaceDeskQuad, 0x599960, { .reversed = false });
    RH_ScopedInstall(Office_FurnishCenter, 0x599A30, { .reversed = false });

    //
    // Shop
    //
    RH_ScopedInstall(FurnishShop, 0x59A790, { .reversed = false });
    RH_ScopedInstall(Shop_Place3PieceUnit, 0x599BB0, { .reversed = false });
    RH_ScopedInstall(Shop_PlaceEdgeUnits, 0x599DC0, { .reversed = false });
    RH_ScopedInstall(Shop_PlaceCounter, 0x599EF0, { .reversed = false });
    RH_ScopedInstall(Shop_PlaceFixedUnits, 0x59A030, { .reversed = false });
    RH_ScopedInstall(Shop_FurnishCeiling, 0x59A130, { .reversed = false });
    RH_ScopedInstall(Shop_AddShelfInfo, 0x59A140, { .reversed = false });
    RH_ScopedInstall(Shop_FurnishEdges, 0x59A1B0, { .reversed = false });
    RH_ScopedInstall(Shop_FurnishAisles, 0x59A590, { .reversed = false });

    //
    // Furniture
    //
    RH_ScopedInstall(PlaceFurniture, 0x592AA0, { .reversed = false });
    RH_ScopedInstall(PlaceFurnitureOnWall, 0x593120, { .reversed = false });
    RH_ScopedInstall(PlaceFurnitureInCorner, 0x593340, { .reversed = false });
    RH_ScopedInstall(GetFurnitureEntity, 0x5913B0, { .reversed = false });
    RH_ScopedInstall(PlaceObject, 0x5934E0, { .reversed = false });

    //
    // Tiles
    //
    RH_ScopedInstall(ResetTiles, 0x593910, { .reversed = false });
    RH_ScopedInstall(CheckTilesEmpty, 0x591680, { .reversed = false });
    RH_ScopedInstall(SetTilesStatus, 0x591700, { .reversed = false });
    RH_ScopedInstall(SetCornerTiles, 0x5917C0, { .reversed = false });
    RH_ScopedInstall(GetTileStatus, 0x5918E0);
    RH_ScopedInstall(GetNumEmptyTiles, 0x591920, { .reversed = false });
    RH_ScopedInstall(FindEmptyTiles, 0x591C50, { .reversed = false });
    RH_ScopedInstall(GetRandomTile, 0x591B20, { .reversed = false });
    RH_ScopedInstall(GetTileCentre_OG, 0x591BD0);
}

// 0x593BF0
bool Interior_c::Init(const CVector& pos) {
    CalcMatrix(pos);
    ResetTiles();
    if (m_Props->m_IntType != eInteriorType::TESTROOM) {
        const auto& pos = m_Group->GetEntity()->GetPosition();
        if (const auto e = g_interiorMan.GetEnEx()) {
            CGeneral::SetRandomSeed(
                  m_Props->m_seed
                + (uint64)pos.x * (uint64)pos.y * (uint64)pos.z
                + (uint64)e->GetEntranceRect().left * (uint64)e->GetEntranceRect().top * (uint64)e->m_fEntranceZ
            );
        } else {
            CGeneral::SetRandomSeed(
                (uint64)(pos.x * pos.y * pos.z + (float)m_Props->m_seed)
            );
        }
    }
    m_NumGoToPts = 0;
    m_NumIntInfo = 0;
    Furnish();
    CalcExitPts();
    g_interiorMan.SetupInteriorStealData(this);
    switch (m_Props->m_IntType) {
    case eInteriorType::BEDROOM:
    case eInteriorType::LOUNGE: {
        AddPickups();
    }
    }
    return true;
}

// 0x592230
void Interior_c::Exit() {
    const auto& p = m_Mat.pos;
    CPickups::RemovePickUpsInArea(
        p.x - 50.f, p.x + 50.f,
        p.y - 50.f, p.y + 50.f,
        p.z - 50.f, p.z + 50.f
    );
    UnFurnish();
}

// 0x591590
void Interior_c::Furnish() {
    switch (m_Props->m_IntType) {
    case eInteriorType::OFFICE:  FurnishOffice(); break;
    case eInteriorType::LOUNGE:  FurnishLounge(); break;
    case eInteriorType::BEDROOM: FurnishBedroom(); break;
    case eInteriorType::KITCHEN: FurnishKitchen(); break;
    case eInteriorType::SHOP:    FurnishShop(m_Props->m_IntType); break;
    }
}

// 0x5915D0
void Interior_c::UnFurnish() {
    const auto player = FindPlayerPed();
    const auto entityHeldByPlayer = player
        ? player->GetEntityThatThisPedIsHolding()
        : nullptr;
    const auto objHeldByPlayer = entityHeldByPlayer && entityHeldByPlayer->IsObject()
        ? entityHeldByPlayer->AsObject()
        : nullptr;
    for (auto it = m_FurnitureEntityList.GetHead(); it;) {
        const auto f = it;
        it = it->m_pNext;

        if (objHeldByPlayer == f->Entity && objHeldByPlayer->objectFlags.bIsLiftable) {
            objHeldByPlayer->m_nObjectType = OBJECT_TEMPORARY;
            objHeldByPlayer->m_nRemovalTime = CTimer::GetTimeInMS() + 99'999'999;
        } else {
            CWorld::Remove(f->Entity);
            delete f->Entity;
        }

        f->Entity = nullptr;
        m_FurnitureEntityList.RemoveItem(f);
        g_furnitureMan.GetFurnitureList().RemoveItem(f);
    }
}

// 0x593DB0
bool Interior_c::GetBoundingBox(FurnitureEntity_c* fe, CVector(&corners)[4]) {
    switch (m_Props->m_IntType) {
    case eInteriorType::LOUNGE:
    case eInteriorType::BEDROOM:
    case eInteriorType::KITCHEN:
    case eInteriorType::BATHROOM:
    case eInteriorType::HOTELROOM:
    case eInteriorType::MISC:
    case eInteriorType::TESTROOM:
        return false;
    }

    // Calculate bounding box
    int32 tileMinX = fe->TileX, tileMaxX = fe->TileX,
          tileMinY = fe->TileY, tileMaxY = fe->TileY;
    Tiles<int32> tiles{};
    tiles[fe->TileX][fe->TileY] = 1; // TODO: Is this correct, or it's the other way around (eg y, x)?
    FindBoundingBox(
        (int32)fe->TileX, (int32)fe->TileY,
        tileMinX, tileMaxX,
        tileMinY, tileMaxY,
        tiles
    );

    // Calculate corners as world positions
    const auto nr = CPedGeometryAnalyser::ms_fPedNominalRadius;
    const auto minX = (float)tileMinX - 0.5f - nr, maxX = (float)tileMaxX + 0.5f + nr,
               minY = (float)tileMinY - 0.5f - nr, maxY = (float)tileMaxY + 0.5f + nr;
    corners[0] = GetTileCentre(minX, maxY); // top left
    corners[1] = GetTileCentre(minX, minY); // bottom left
    corners[2] = GetTileCentre(maxX, minY); // bottom right
    corners[3] = GetTileCentre(maxX, maxY); // top right

    return true;
}

// 0x5922C0
void Interior_c::FindBoundingBox(
    int32  posX, int32 posY,
    int32& minX, int32& maxX,
    int32& minY, int32& maxY,
    Tiles<int32>& tilesVisited
) {
    // Depth-first search for all bounding box of connected tiles
    for (int32 iterY = posY;;) {
        const auto CheckTile = [&](int32 x, int32 y) {
            if (GetTileStatus(x, y) != eTileStatus::STATE_5) {
                return false;
            }
            if (std::exchange(tilesVisited[x][y], true)) { // Already checked?
                return false;
            }
            FindBoundingBox(x, y, minX, maxX, minY, maxY, tilesVisited);
            return true;
        };

        //> 0x5922DB - Check left side
        if (const auto x = posX - 1; CheckTile(x, iterY)) {
            minX = std::min(minX, x);
        }

        //> 0x59234C -  Check top (since we're going downwards)
        if (const auto y = iterY + 1; CheckTile(posX, y)) {
            maxY = std::max(maxY, y);
        }

        //> 0x5923BA -  Check right
        if (const auto x = posX + 1; CheckTile(x, iterY)) {
            maxX = std::max(maxX, x);
        }

        //
        // Exit conditions
        //

        if (posX < 0 || posX >= m_Props->m_width) {
            break;
        }

        iterY--;

        if (iterY < 0 || iterY >= m_Props->m_depth) {
            break;
        }

        if (GetTileStatus(posX, iterY) != eTileStatus::STATE_5) {
            break;
        }

        if (std::exchange(tilesVisited[posX][iterY], true)) {
            break;
        }

        minY = std::min(minY, iterY);
    }
}

// 0x5913E0
bool Interior_c::IsPtInside(const CVector& pt, CVector bias) {
    const auto x = pt.x - m_Mat.pos.x;
    const auto y = pt.y - m_Mat.pos.y;
    const auto z = pt.z - m_Mat.pos.z;
    return m_Props->m_width * 0.5f + bias.x >= std::fabs(m_Mat.right.x * x + m_Mat.right.y * y + m_Mat.right.z * z)
        && m_Props->m_depth * 0.5f + bias.y >= std::fabs(m_Mat.up.x * x + m_Mat.up.y * y + m_Mat.up.z * z)
        && m_Props->m_height * 0.5f + bias.z >= std::fabs(m_Mat.at.x * x + m_Mat.at.y * y + m_Mat.at.z * z);
}

// 0x5914D0
void Interior_c::CalcMatrix(const CVector& pos) {
    RwMatrixSetIdentity(&m_Mat);
    RwV3d axis{0.f, 0.f, 1.f};
    RwMatrixRotate(&m_Mat, &axis, m_Props->m_rot, rwCOMBINEREPLACE); // Apply rotation
    RwMatrixTranslate(&m_Mat, &pos, rwCOMBINEPOSTCONCAT); // Apply translation
    RwMatrixMultiply(&m_Mat, &m_Mat, RwFrameGetMatrix(RpAtomicGetFrame(m_Group->GetEntity()->m_pRwAtomic))); // Multiply by entity's matrix
}

// 0x591D20
void Interior_c::AddGotoPt(int32 tileX, int32 tileY, float offsetX, float offsetY) {
    if (m_NumGoToPts >= std::size(m_GoToPts)) {
        return;
    }
    switch (GetTileStatus(tileX, tileY)) {
    case eTileStatus::STATE_3:
    case eTileStatus::STATE_7:
        break;
    default:
        return;
    }
    m_GoToPts[m_NumGoToPts++] = {
        .TileX = (int8)tileX,
        .TileY = (int8)tileY,
        .Pos   = GetTileCentre((float)tileX + offsetX, (float)tileY + offsetY)
    };
    switch (GetTileStatus(tileX, tileY)) {
    case eTileStatus::STATE_3:
    case eTileStatus::STATE_0: {
        SetTileStatus(tileX, tileY, eTileStatus::STATE_4);
    }
    }
}

// 0x591E40
bool Interior_c::AddInteriorInfo(eInteriorInfoTypeS32 infoType, float offsetX, float offsetY, int32 direction, CEntity* entityIgnoredCollision) {
    if (m_NumIntInfo >= std::size(m_IntInfos)) {
        return false;
    }

    CVector pos = GetTileCentre(offsetX, offsetY);
    pos.z += 0.8f;

    CVector dir{};
    if (direction != -1) {
        switch (direction) {
        case 3: dir.x = -1.0; break;
        case 1: dir.x = 1.0; break;
        case 2: dir.y = 1.0; break;
        case 0: dir.y = -1.0; break;
        }
        RwV3dTransformVector(&dir, &dir, &m_Mat);
    }

    m_IntInfos[m_NumIntInfo++] = {
        .Type                   = infoType,
        .IsInUse                = false,
        .Pos                    = pos,
        .Dir                    = dir,
        .EntityIgnoredCollision = entityIgnoredCollision
    };

    return true;
}

// 0x591F90
void Interior_c::AddPickups() {
    if (CTimer::GetTimeInMS() - g_interiorMan.GetLastTimePickupsGenerated() < 180'000) {
        return;
    }

    for (auto i = 0u; i < 100u; i++) {
        const auto tileX = CGeneral::GetRandomNumberInRange(m_Props->m_width - 1u);
        const auto tileY = CGeneral::GetRandomNumberInRange(m_Props->m_depth - 1u);

        switch (GetTileStatus(tileX, tileY)) {
        case eTileStatus::STATE_0:
        case eTileStatus::STATE_3:
        case eTileStatus::STATE_4:
            break;
        default:
            continue;
        }

        const auto pickupPos = GetTileCentre(static_cast<float>(tileY), static_cast<float>(tileX));

        if (CGeneral::RandomBool(25.0f)) {
            CPickups::GenerateNewOne_WeaponType(
                pickupPos + CVector{0.f, 0.f, 0.5f},
                [&] {
                    const auto chance = CGeneral::GetRandomNumberInRange(0.0f, 100.0f);
                    if (chance < 40.f) {
                        return WEAPON_BASEBALLBAT; // 40%
                    } else if (chance < 80.f) {
                        return WEAPON_PISTOL; // 80%
                    } else if (chance < 90.f) {
                        return WEAPON_KNIFE; // 90%
                    } else {
                        return WEAPON_SHOTGUN; // 10%
                    }
                }(),
                PICKUP_ONCE,
                CGeneral::GetRandomNumberInRange<uint32>(3, 18),
                false,
                nullptr
            );
        } else {
            CPickups::GenerateNewOne(
                pickupPos,
                ModelIndices::MI_MONEY,
                PICKUP_MONEY,
                CGeneral::GetRandomNumberInRange<uint32>(10, 50),
                0,
                false
            );
        }
    }
}

// 0x5924A0
void Interior_c::CalcExitPts() {
    plugin::CallMethod<0x5924A0, Interior_c*>(this);
}

// 0x5929F0
bool Interior_c::IsVisible() {
    const auto& cp = TheCamera.GetPosition();
    return IsPtInside(cp, {5.f, 5.f, 0.f})
        || m_Props->m_door > 0 && CVector2D::DistSqr(cp, m_DoorPos) <= sq(10.f);
}

//
// Bedroom
//

// 0x593FC0
void Interior_c::FurnishBedroom() {
    return plugin::CallMethod<0x593FC0, Interior_c*>(this);

    /** 
     * So far so good, but incomplete. Not tested.
     **
    m_StyleA = g_furnitureMan.GetRandomId(eInteriorGroupId::BEDROOM, eInteriorSubGroupId::BEDROOM_TABLES, m_Props->m_status);

    SetTilesStatus(0, 2, 2, 2, eTileStatus::STATE_7);

    int32 bedPos;
    eWallS32 bedWall;
    const auto bedObj = PlaceFurnitureOnWall(
        eInteriorGroupId::BEDROOM,
        eInteriorSubGroupId::BEDROOM_BEDS,
        -1,
        0.f,
        1,
        eWall::NA,
        -1,
        0,
        &bedWall,
        &bedPos
    );
    if (bedObj && PlaceFurnitureOnWall(
        eInteriorGroupId::BEDROOM,
        eInteriorSubGroupId::BEDROOM_TABLES,
        m_StyleA,
        0.f,
        0,
        bedWall,
        bedPos - 1
    )) {
        const auto MarkPlacedFurniture = [&](eInteriorInfoType iit, int32 x, int32 y, int32 offX, int32 offY, int32 dir) {
            AddInteriorInfo(iit, (float)offX, (float)offY, dir, bedObj);
            SetTilesStatus(x, y, 1, 1, eTileStatus::STATE_2);
        };
        switch (bedWall) {
        case eWall::Y_A: MarkPlacedFurniture(eInteriorInfoType::UNK_4, 2,                    bedPos + 2,           1,                    bedPos + 2,           0); break; // 0x594186
        case eWall::Y_B: MarkPlacedFurniture(eInteriorInfoType::UNK_3, m_Props->m_width - 3, bedPos + 2,           m_Props->m_width - 2, bedPos + 2,           0); break; // 0x5941AE
        case eWall::X_B: MarkPlacedFurniture(eInteriorInfoType::UNK_3, bedPos + 2,           2,                    bedPos + 2,           1,                    3); break; // 0x594200
        case eWall::X_A: MarkPlacedFurniture(eInteriorInfoType::UNK_4, bedPos + 2,           m_Props->m_depth - 3, bedPos + 2,           m_Props->m_depth - 2, 3); break; // 0x5941D2
        }
    }

    PlaceFurnitureOnWall(eInteriorGroupId::BEDROOM, eInteriorSubGroupId::BEDROOM_WARDROBES, m_StyleA);
    PlaceFurnitureOnWall(eInteriorGroupId::BEDROOM, eInteriorSubGroupId::BEDROOM_DRAWERS, m_StyleA);
    */
}

// 0x593F10
void Interior_c::Bedroom_AddTableItem(eInteriorGroupIdS32 groupId, eInteriorSubGroupIdS32 subGroupId, eWall wall, int32 x, int32 y, int32 d) {
    float oX = (float)x, oY = (float)y;
    if (wall == eWall::X_A || wall == eWall::X_B) {
        oX += 0.5f;
    } else if (wall == eWall::Y_A || wall == eWall::Y_B) {
        oY += 0.5f;
    }
    PlaceObject(
        true,
        g_furnitureMan.GetFurniture(groupId, subGroupId, -1, m_Props->m_status),
        oX + 0.5f, oY + 0.5f, 0.5f,
        (float)d * 90.f
    );
}

//
// Kitchen
//

// 0x5970B0
void Interior_c::FurnishKitchen() {
    plugin::CallMethod<0x5970B0, Interior_c*>(this);
}

// 0x596930
CObject* Interior_c::Kitchen_FurnishEdges() {
    return plugin::CallMethodAndReturn<CObject*, 0x596930, Interior_c*>(this);
}

//
// Lounge
//

// 0x597740
void Interior_c::FurnishLounge() {
    plugin::CallMethod<0x597740, Interior_c*>(this);
}

// 0x597240
CObject* Interior_c::Lounge_AddTV(int32 a2, int32 a3, int32 a4, int32 a5) {
    return plugin::CallMethodAndReturn<CObject*, 0x597240, Interior_c*, int32, int32, int32, int32>(this, a2, a3, a4, a5);
}

// 0x597430
CObject* Interior_c::Lounge_AddHifi(int32 a2, int32 a3, int32 a4, int32 a5) {
    return plugin::CallMethodAndReturn<CObject*, 0x597430, Interior_c*, int32, int32, int32, int32>(this, a2, a3, a4, a5);
}

// 0x5974E0
void Interior_c::Lounge_AddChairInfo(int32 a2, int32 a3, CEntity* entityIgnoredCollision) {
    plugin::CallMethod<0x5974E0, Interior_c*, int32, int32, CEntity*>(this, a2, a3, entityIgnoredCollision);
}

// 0x5975C0
void Interior_c::Lounge_AddSofaInfo(int32 sitType, int32 offsetX, CEntity* entityIgnoredCollision) {
    plugin::CallMethod<0x5975C0, Interior_c*, int32, int32, CEntity*>(this, sitType, offsetX, entityIgnoredCollision);
}

//
// Office
//

// 0x599AF0
void Interior_c::FurnishOffice() {
    plugin::CallMethod<0x599AF0, Interior_c*>(this);
}

// 0x599210
bool Interior_c::Office_PlaceEdgeFillers(int32 arg0, int32 a2, int32 a3, int32 a6, int32 a7) {
    return plugin::CallMethodAndReturn<bool, 0x599210, Interior_c*, int32, int32, int32, int32, int32>(this, arg0, a2, a3, a6, a7);
}

// 0x5993E0
int32 Interior_c::Office_PlaceDesk(int32 a3, int32 arg4, int32 offsetY, int32 a5, uint8 a6, int32 b) {
    return plugin::CallMethodAndReturn<int32, 0x5993E0, Interior_c*, int32, int32, int32, int32, uint8, int32>(this, a3, arg4, offsetY, a5, a6, b);
}

// 0x5995B0
int32 Interior_c::Office_PlaceEdgeDesks(int32 a2, int32 a3, int32 a4, int32 a5, int32 a6) {
    return plugin::CallMethodAndReturn<int32, 0x5995B0, Interior_c*, int32, int32, int32, int32, int32>(this, a2, a3, a4, a5, a6);
}

// 0x599770
void Interior_c::Office_FurnishEdges() {
    plugin::CallMethod<0x599770, Interior_c*>(this);
}

// 0x599960
int32 Interior_c::Office_PlaceDeskQuad(int32 a2, int32 a3, int32 a4, int32 a5) {
    return plugin::CallMethodAndReturn<int32, 0x599960, Interior_c*, int32, int32, int32, int32>(this, a2, a3, a4, a5);
}

// 0x599A30
int32 Interior_c::Office_FurnishCenter() {
    return plugin::CallMethodAndReturn<int32, 0x599A30, Interior_c*>(this);
}

//
// Shop
//

// 0x59A790
void Interior_c::FurnishShop(eInteriorTypeS32 intType) {
    plugin::CallMethod<0x59A790, Interior_c*, eInteriorTypeS32>(this, intType);
}

// 0x59A1B0
void Interior_c::Shop_FurnishEdges() {
    plugin::CallMethod<0x59A1B0, Interior_c*>(this);
}

// 0x59A590
void Interior_c::Shop_FurnishAisles() {
    plugin::CallMethod<0x59A590, Interior_c*>(this);
}

// 0x599BB0
int8 Interior_c::Shop_Place3PieceUnit(int32 a2, int32 a3, int32 a4, int32 a5, int32 a6) {
    return plugin::CallMethodAndReturn<int8, 0x599BB0, Interior_c*, int32, int32, int32, int32, int32>(this, a2, a3, a4, a5, a6);
}

// 0x599DC0
int32 Interior_c::Shop_PlaceEdgeUnits(int32 a2, int32 a3, int32 a4, int32 a5) {
    return plugin::CallMethodAndReturn<int32, 0x599DC0, Interior_c*, int32, int32, int32, int32>(this, a2, a3, a4, a5);
}

// 0x599EF0
int32 Interior_c::Shop_PlaceCounter(uint8 a2) {
    return plugin::CallMethodAndReturn<int32, 0x599EF0, Interior_c*, uint8>(this, a2);
}

// 0x59A030
void Interior_c::Shop_PlaceFixedUnits() {
    return plugin::CallMethod<0x59A030, Interior_c*>(this);
}

// 0x59A130
void Interior_c::Shop_FurnishCeiling() {
    plugin::CallMethod<0x59A130, Interior_c*>(this);
}

// 0x59A140
void Interior_c::Shop_AddShelfInfo(int32 a2, int32 a3, int32 a5) {
    plugin::CallMethod<0x59A140, Interior_c*, int32, int32, int32>(this, a2, a3, a5);
}


//
// Furniture
//

// 0x592AA0
void Interior_c::PlaceFurniture(Furniture_c* a1, int32 a2, int32 a3, float a4, int32 a5, int32 a6, int32* a7, int32* a8, uint8 a9) {
    plugin::CallMethod<0x592AA0, Interior_c*, Furniture_c*, int32, int32, float, int32, int32, int32*, int32*, uint8>(this, a1, a2, a3, a4, a5, a6, a7, a8, a9);
}

// 0x593120
CObject* Interior_c::PlaceFurnitureOnWall(
    eInteriorGroupIdS32 interior,
    eInteriorSubGroupId subGroup,
    int32               type,
    float               z,
    int32               heightInfo,
    eWallS32            wallId,
    int32               pos,
    int32               offset,
    eWallS32*           outWall,
    int32*              outPos,
    int32*              x,
    int32*              y,
    int32*              w,
    int32*              d
) {
    return plugin::CallMethodAndReturn<CObject*, 0x593120>(
        interior,
        subGroup,
        type,
        z,
        heightInfo,
        wallId,
        pos,
        offset,
        outWall,
        outPos,
        x,
        y,
        w,
        d
    );
}

// 0x593340
void Interior_c::PlaceFurnitureInCorner(int32 furnitureGroupId, int32 furnitureSubgroupId, int32 id, float a4, int32 a5, int32 a6, int32 a2, int32* a9, int32* a10, int32* a11,
    int32* a12, int32* a13) {
    plugin::CallMethod<0x593340, Interior_c*, int32, int32, int32, float, int32, int32, int32, int32*, int32*, int32*, int32*, int32*>(this, furnitureGroupId, furnitureSubgroupId,
        id, a4, a5, a6, a2, a9, a10, a11, a12, a13);
}

// 0x5934E0
CObject* Interior_c::PlaceObject(uint8 isStealable, Furniture_c* furniture, float offsetX, float offsetY, float offsetZ, float rotationZ) {
    return plugin::CallMethodAndReturn<CObject*, 0x5934E0, Interior_c*, uint8, Furniture_c*, float, float, float, float>(this, isStealable, furniture, offsetX, offsetY, offsetZ,
                                                                                                                         rotationZ);
}

// 0x5913B0
FurnitureEntity_c* Interior_c::GetFurnitureEntity(CEntity* entity) {
    return plugin::CallMethodAndReturn<FurnitureEntity_c*, 0x5913B0, Interior_c*, CEntity*>(this, entity);
}

//
// Tiles
//

// 0x591680
int8 Interior_c::CheckTilesEmpty(int32 a1, int32 a2, int32 a3, int32 a4, uint8 a5) {
    return plugin::CallMethodAndReturn<int8, 0x591680, Interior_c*, int32, int32, int32, int32, uint8>(this, a1, a2, a3, a4, a5);
}

// 0x591700
void Interior_c::SetTilesStatus(int32 x, int32 y, int32 w, int32 d, eTileStatusS32 status, bool force) {
    plugin::CallMethod<0x591700>(this, x, y, w, d, status, force);
}

// notsa
void Interior_c::SetTileStatus(int32 x, int32 y, eTileStatus status) {
    assert(x > 0 && x < m_Props->m_width);
    assert(y > 0 && y < m_Props->m_depth);
    m_Tiles[x][y] = status;
}

// 0x5917C0
void Interior_c::SetCornerTiles(int32 a4, int32 a3, int32 a5, uint8 a6) {
    plugin::CallMethod<0x5917C0, Interior_c*, int32, int32, int32, uint8>(this, a4, a3, a5, a6);
}

// 0x5918E0
Interior_c::eTileStatusS32 Interior_c::GetTileStatus(int32 x, int32 y) const {
    assert(m_Props->m_width < NUM_TILES_PER_AXIS);
    if (x < 0 || x >= m_Props->m_width) {
        return eTileStatus::STATE_1;
    }

    assert(m_Props->m_depth < NUM_TILES_PER_AXIS);
    if (y < 0 || y >= m_Props->m_depth) {
        return eTileStatus::STATE_1;
    }

    return m_Tiles[x][y];
}

// 0x591920
int32 Interior_c::GetNumEmptyTiles(int32 a2, int32 a3, int32 a4, int32 a5) {
    return plugin::CallMethodAndReturn<int32, 0x591920, Interior_c*, int32, int32, int32, int32>(this, a2, a3, a4, a5);
}

// 0x591B20
int32 Interior_c::GetRandomTile(int32 a2, int32* a3, int32* a4) {
    return plugin::CallMethodAndReturn<int32, 0x591B20, Interior_c*, int32, int32*, int32*>(this, a2, a3, a4);
}

// 0x591BD0
void Interior_c::GetTileCentre_OG(float offsetX, float offsetY, CVector* outPos) const {
    *outPos = GetTileCentre(offsetX, offsetY);
}

// Based on 0x591BD0
CVector Interior_c::GetTileCentre(float offsetX, float offsetY) const {
    CVector pos{
        offsetX + 0.5f - (float)m_Props->m_width * 0.5f,
        offsetY + 0.5f - (float)m_Props->m_depth * 0.5f,
        0.f            - (float)m_Props->m_height * 0.5f
    };
    RwV3dTransformPoint(&pos, &pos, &m_Mat);
    return pos;
}

// 0x591C50
bool Interior_c::FindEmptyTiles(int32 a3, int32 a4, int32* arg8, int32* a5) {
    return plugin::CallMethodAndReturn<bool, 0x591C50, Interior_c*, int32, int32, int32*, int32*>(this, a3, a4, arg8, a5);
}

// 0x593910
void Interior_c::ResetTiles() {
    plugin::CallMethod<0x593910, Interior_c*>(this);
}
