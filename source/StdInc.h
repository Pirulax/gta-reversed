#pragma once
#pragma message("Compiling precompiled header.\n")

#include <cstdio>
#include <cmath>
#include <cinttypes>
#include <algorithm>
#include <numeric>
#include <random>
#include <list>
#include <map>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <functional>
#include <iostream>
#include <cassert>
#include <array>
#include <vector>
#include <cstring>
#include <tuple>
#include <initializer_list>
#include <format>
#include "app/platform/win/winincl.h"

#include "Base.h"
#include "config.h"

#include "HookSystem.h"
#include "reversiblehooks/ReversibleHooks.h"

#include <extensions/Casting.hpp>

#include <Tracy.hpp>

// DirectX
#ifdef _WIN32
#include <d3d9.h>
#include <dinput.h>
#endif

// RenderWare
#include "RenderWare/D3DIndexDataBuffer.h"
#include "RenderWare/D3DResourceSystem.h"
#include "RenderWare/D3DTextureBuffer.h"
#include "RenderWare/rw/rpanisot.h"
#include "RenderWare/rw/rpcriter.h"
#include "RenderWare/rw/rperror.h"
#include "RenderWare/rw/rphanim.h"
#include "RenderWare/rw/rpmatfx.h"
#include "RenderWare/rw/rpskin.h"
#include "RenderWare/rw/rpuvanim.h"
#include "RenderWare/rw/rpworld.h"
#include "RenderWare/rw/rtanim.h"
#include "RenderWare/rw/rtbmp.h"
#include "RenderWare/rw/rtdict.h"
#include "RenderWare/rw/rtpng.h"
#include "RenderWare/rw/rtquat.h"
#include "RenderWare/rw/rwcore.h"
#include "RenderWare/rw/rwplcore.h"
#include "RenderWare/rw/rwtexdict.h"
#include "RenderWare/rw/skeleton.h"
#include "RenderWare/RenderWare.h"
#include <PluginBase.h>

// oswrapper
#include "oswrapper/oswrapper.h"

#include "app_debug.h"

#include "app/app.h"
#include "app/app_light.h"
#include "platform.h"

#include "EntryInfoNode.h"
#include "EntryInfoList.h"
#include "KeyGen.h"
#include "Link.h"
#include "LinkList.h"
#include "Matrix.h"
#include "MatrixLink.h"
#include "MatrixLinkList.h"
#include "PtrNode.h"
#include "PtrNodeDoubleLink.h"
#include "PtrNodeSingleLink.h"
#include "PtrList.h"
#include "PtrListDoubleLink.h"
#include "PtrListSingleLink.h"
#include "QuadTreeNode.h"
#include "Quaternion.h"
#include "Rect.h"
#include "Store.h"
#include "Vector.h"
#include "Vector2D.h"
#include "ListItem_c.h"
#include "List_c.h"
#include "SArray.h"

#include "Pool.h"
#include "Pools/Pools.h"
#include <Pools/IplDefPool.h>
#include <Pools/PedPool.h>
#include <Pools/VehiclePool.h>
#include <Pools/BuildingPool.h>
#include <Pools/ObjectPool.h>
#include <Pools/DummyPool.h>
#include <Pools/ColModelPool.h>
#include <Pools/TaskPool.h>
#include <Pools/PedIntelligencePool.h>
#include <Pools/PtrNodeSingleLinkPool.h>
#include <Pools/PtrNodeDoubleLinkPool.h>
#include <Pools/EntryInfoNodePool.h>
#include <Pools/PointRoutePool.h>
#include <Pools/PatrolRoutePool.h>
#include <Pools/EventPool.h>
#include <Pools/NodeRoutePool.h>
#include <Pools/TaskAllocatorPool.h>
#include <Pools/PedAttractorPool.h>

#include "GxtChar.h"
#include "RwHelper.h"

#include "common.h"

#include "Enums/eCheats.h"
#include "Enums/AnimationEnums.h"
#include "Enums/eAnimBlendCallbackType.h"
#include "Enums/eAudioEvents.h"
#include "Enums/eCamMode.h"
#include "Enums/eCarMission.h"
#include "Enums/eClothesModelPart.h"
#include "Enums/eClothesTexturePart.h"
#include "Enums/eCrimeType.h"
#include "Enums/eDecisionMakerEvents.h"
#include "Enums/eEntityStatus.h"
#include "Enums/eEntityType.h"
#include "Enums/eEventType.h"
#include "Enums/eFontAlignment.h"
#include "Enums/eGameState.h"
#include "Enums/eModelID.h"
#include "Enums/eBoneTag.h"
#include "Enums/ePedModel.h"
#include "Enums/ePedState.h"
#include "Enums/eRadioID.h"
#include "Enums/eReplay.h"
#include "Enums/eScriptCommands.h"
#include "Enums/eSoundID.h"
#include "Enums/eSprintType.h"
#include "Enums/eStatModAbilities.h"
#include "Enums/eStats.h"
#include "Enums/eStatsReactions.h"
#include "Enums/eSurfaceType.h"
#include "Enums/eTargetDoor.h"
#include "Enums/eTaskType.h"
#include "Enums/eVehicleClass.h"
#include "Enums/eVehicleHandlingFlags.h"
#include "Enums/eVehicleHandlingModelFlags.h"
#include "Enums/eWeaponFire.h"
#include "Enums/eWeaponFlags.h"
#include "Enums/eWeaponModel.h"
#include "Enums/eWeaponType.h"
#include "Enums/eWinchType.h"
#include "Enums/eItemDefinitionFlags.h"
#include "Enums/eMeleeCombo.h"

#include "constants.h"
#include "ModelIndices.h"
#include "PedGeometryAnalyser.h"
#include "Debug.h"
#include "MemoryMgr.h"
#include "CullZones.h"
#include "VehicleScanner.h"
#include "LoadMonitor.h"
#include "PedStuckChecker.h"
#include "DecisionMakerTypes.h"
#include "InformGroupEventQueue.h"
#include "InformFriendsEventQueue.h"
#include "Events/GroupEventHandler.h"
#include "Events/EventInAir.h"
#include "Events/EventAcquaintancePed.h"
#include "Events/EventSource.h"
#include "Events/EventVehicleDamageCollision.h"
#include "Events/EventVehicleCollision.h"
#include "Events/EventVehicleThreat.h"
#include "Events/EventVehicleToSteal.h"
#include "Events/EventVehicleDamageWeapon.h"
#include "Events/EventVehicleDied.h"
#include "Events/EventVehicleOnFire.h"
#include "Events/EventVehicleHitAndRun.h"
#include "Events/EventDraggedOutCar.h"
#include "Events/EventKnockOffBike.h"
#include "Events/EventGotKnockedOverByCar.h"
#include "Events/EventAttractor.h"
#include "Events/EventGunShot.h"
#include "Events/EventGunShotWhizzedBy.h"
#include "Events/EventPlayerCommandToGroup.h"
#include "Events/EventPlayerCommandToGroupAttack.h"
#include "Events/EventPlayerCommandToGroupGather.h"
#include "Events/EventPotentialWalkIntoPed.h"
#include "Events/EventPotentialWalkIntoFire.h"
#include "Events/EventPotentialWalkIntoObject.h"
#include "Events/EventPotentialWalkIntoVehicle.h"
#include "Events/EventHitByWaterCannon.h"
#include "Events/EventInWater.h"
#include "Events/EventDeath.h"
#include "Events/EventDeadPed.h"
#include "Events/EventVehicleDamage.h"
#include "Events/EventPedToFlee.h"
#include "Events/EventPedToChase.h"
#include "Events/EventEditableResponse.h"
#include "Events/EventSoundQuiet.h"
#include "Events/EventHandlerHistory.h"
#include "Events/EventGlobalGroup.h"
#include "Events/EventGroupEvent.h"
#include "Events/EventGroup.h"
#include "Events/EventGunAimedAt.h"
#include "Events/EventScriptCommand.h"
#include "Events/EventDamage.h"
#include "Events/EventCreatePartnerTask.h"
#include "Events/EventHandler.h"
#include "Events/EventScanner.h"
#include "Events/Event.h"
#include "Crime.h"
#include "SurfaceInfo_c.h"
#include "SurfaceInfos_c.h"
#include "Replay.h"
#include "VehicleAnimGroupData.h"
#include "Collision/ColStore.h"
#include "Collision/ColAccel.h"
#include "PedDamageResponseCalculator.h"
#include "PedDamageResponse.h"
#include "WaterLevel.h"
#include "WaterCreature_c.h"
#include "WaterCreatureManager_c.h"
#include "CdStreamInfo.h"
#include "3dMarker.h"
#include "3dMarkers.h"
#include "Accident.h"
#include "AccidentManager.h"
#include "AttractorScanner.h"
#include "AutoPilot.h"
#include "BouncingPanel.h"
#include "Bridge.h"
#include "BrightLights.h"
#include "BulletTrace.h"
#include "BulletTraces.h"
#include "Cam.h"
#include "Camera.h"
#include "CamPathSplines.h"
#include "CarAI.h"
#include "CarEnterExit.h"
#include "Cheat.h"
#include "Clock.h"
#include "Clothes.h"
#include "ClothesBuilder.h"
#include "Coronas.h"
#include "Cover.h"
#include "Cranes.h"
#include "CrimeBeingQd.h"
#include "CutsceneMgr.h"
#include "Darkel.h"
#include "Date.h"
#include "Decision.h"
#include "DecisionMaker.h"
#include "DecisionSimple.h"
#include "Directory.h"
#include "Draw.h"
#include "EntityScanner.h"
#include "EntryExit.h"
#include "Explosion.h"
#include "FileLoader.h"
#include "FileMgr.h"
#include "FileObjectInstance.h"
#include "Font.h"
#include "Formation.h"
#include "Game.h"
#include "GangInfo.h"
#include "Gangs.h"
#include "GangWars.h"
#include "General.h"
#include "GenericGameStorage.h"
#include "cHandlingDataMgr.h"
#include "HudColours.h"
#include "IniFile.h"
#include "IplStore.h"
#include "LoadedCarGroup.h"
#include "Localisation.h"
#include "MenuManager.h"
#include "Messages.h"
#include "Mirrors.h"
#include "MissionCleanup.h"
#include "NodeAddress.h"
#include "NodeRoute.h"
#include "ObjectData.h"
#include "CustomRoadsignMgr.h"
#include "CompressedVector.h"
#include "CompressedMatrixNotAligned.h"
#include "KeyboardState.h"
#include "MouseControllerState.h"
#include "ControllerState.h"
#include "Pad.h"
#include "PathFind.h"
#include "PedGroup.h"
#include "PedGroupIntelligence.h"
#include "PedGroupMembership.h"
#include "PedGroupPlacer.h"
#include "PedGroups.h"
#include "PedIK.h"
#include "PedIntelligence.h"
#include "PedList.h"
#include "PedTaskPair.h"
#include "PedAttractorManager.h"
#include "Pickup.h"
#include "Pickups.h"
#include "PlayerPedData.h"
#include "PlayerInfo.h"
#include "PointLights.h"
#include "PointList.h"
#include "PolyBunch.h"
#include "PopCycle.h"
#include "Population.h"
#include "ProjectileInfo.h"
#include "QueuedMode.h"
#include "Reference.h"
#include "References.h"
#include "RegisteredCorona.h"
#include "RegisteredMotionBlurStreak.h"
#include "Renderer.h"
#include "RepeatSector.h"
#include "Restart.h"
#include "RGBA.h"
#include "RideAnimData.h"
#include "RoadBlocks.h"
#include "Scene.h"
#include "ScriptResourceManager.h"
#include "ScriptsForBrains.h"
#include "Sector.h"
#include "SetPiece.h"
#include "SetPieces.h"
#include "ShinyTexts.h"
#include "Shopping.h"
#include "ShotInfo.h"
#include "SimpleTransform.h"
#include "SpecialFX.h"
#include "SpecialPlateHandler.h"
#include "Sprite.h"
#include "Sprite2d.h"
#include "Stats.h"
#include "StencilShadowObject.h"
#include "StencilShadows.h"
#include "StoredCollPoly.h"
#include "StreamedScripts.h"
#include "Streaming.h"
#include "StreamingInfo.h"
#include "StuckCarCheck.h"
#include "Text/Text.h"
#include "TheZones.h"
#include "TimeCycle.h"
#include "Timer.h"
#include "TxdStore.h"
#include "VisibilityPlugins.h"
#include "Wanted.h"
#include "Weapon.h"
#include "WeaponEffects.h"
#include "WeaponInfo.h"
#include "Weather.h"
#include "World.h"
#include "Zone.h"
#include "ZoneInfo.h"
#include "IplDef.h"
#include "RpHAnimBlendInterpFrame.h"
#include "RwObjectNameIdAssocation.h"
#include "TxdDef.h"
#include "GameLogic.h"
#include "Maths.h"
#include "Remote.h"
#include "Animation/AnimAssociationData.h"
#include "Animation/AnimBlendFrameData.h"
#include "Animation/AnimSequenceFrames.h"
#include "Animation/AnimationStyleDescriptor.h"
#include "Animation/AnimBlendAssocGroup.h"
#include "Animation/AnimBlendAssociation.h"
#include "Animation/AnimBlendClumpData.h"
#include "Animation/AnimBlendHierarchy.h"
#include "Animation/AnimBlendNode.h"
#include "Animation/AnimBlendSequence.h"
#include "Animation/AnimBlendStaticAssociation.h"
#include "Animation/AnimBlock.h"
#include "Animation/AnimManager.h"
#include "Audio/AudioEngine.h"
#include "Audio/AESound.h"
#include "Audio/AudioZones.h"
#include "Collision/Collision.h"
#include "FireManager.h"

#include "Entity/AnimatedBuilding.h"
#include "Entity/Building.h"
#include "Entity/Entity.h"
#include "Entity/Physical.h"
#include "Entity/Placeable.h"
#include "Entity/Dummy/Dummy.h"
#include "Entity/Dummy/DummyObject.h"
#include "Entity/Dummy/DummyPed.h"
#include "Entity/Object/CutsceneObject.h"
#include "Entity/Object/HandObject.h"
#include "Entity/Object/Object.h"
#include "Entity/Object/Projectile.h"
#include "Entity/Ped/CivilianPed.h"
#include "Entity/Ped/CopPed.h"
#include "Entity/Ped/EmergencyPed.h"
#include "Entity/Ped/Ped.h"
#include "Entity/Ped/PlayerSkin.h"
#include "Entity/Ped/PlayerPed.h"
#include "Entity/Vehicle/Automobile.h"
#include "Entity/Vehicle/Bike.h"
#include "Entity/Vehicle/Bmx.h"
#include "Entity/Vehicle/Boat.h"
#include "Entity/Vehicle/Heli.h"
#include "Entity/Vehicle/MonsterTruck.h"
#include "Entity/Vehicle/Plane.h"
#include "Entity/Vehicle/QuadBike.h"
#include "Entity/Vehicle/Trailer.h"
#include "Entity/Vehicle/Train.h"
#include "Entity/Vehicle/Vehicle.h"

#include "Audio/config/eAudioBank.h"
#include "Audio/config/eAudioSlot.h"
#include "Audio/config/eSFX.h"

#include "Fx/eFxInfoType.h"
#include "Fx/FxManager.h"
#include "Fx/FxPrtMult.h"
#include "Fx/Fx.h"

#include "Models/AtomicModelInfo.h"
#include "Models/LodAtomicModelInfo.h"
#include "Models/DamageAtomicModelInfo.h"
#include "Models/BaseModelInfo.h"
#include "Models/ClumpModelInfo.h"
#include "Models/ModelInfo.h"
#include "Models/PedModelInfo.h"
#include "Models/TimeModelInfo.h"
#include "Models/LodTimeModelInfo.h"
#include "Models/VehicleModelInfo.h"
#include "Models/WeaponModelInfo.h"
#include "Plugins/JPegCompressPlugin/JPegCompress.h"
#include "Plugins/NodeNamePlugin/NodeName.h"
#include "Plugins/PipelinePlugin/PipelinePlugin.h"
#include "Plugins/CollisionPlugin/CollisionPlugin.h"
#include "Plugins/RpAnimBlendPlugin/RpAnimBlend.h"
#include "Plugins/TwoDEffectPlugin/2dEffect.h"

#include "Scripts/RunningScript.h"
#include "Scripts/TheScripts.h"
#include "Tasks/Task.h"
#include "Tasks/TaskComplex.h"
#include "Tasks/TaskManager.h"
#include "Tasks/TaskSimple.h"
#include "Tasks/TaskTimer.h"
#include "Tasks/TaskComplexSequence.h"
#include "Tasks/TaskComplexUseSequence.h"
#include "Tasks/TaskSequences.h"
#include "Tasks/PedScriptedTaskRecord.h"
#include "Tasks/ScriptedBrainTaskStore.h"

#include "RenderBuffer.hpp"

#ifdef EXTRA_DEBUG_FEATURES
#include "toolsmenu/DebugModules/COcclusionDebugModule.h"
#endif
