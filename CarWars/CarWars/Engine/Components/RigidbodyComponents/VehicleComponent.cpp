#include "VehicleComponent.h"
#include "../../Systems/Content/ContentManager.h"
#include "../../Entities/EntityManager.h"

#include "imgui/imgui.h"
#include "../../Systems/Physics.h"
#include "../Colliders/ConvexMeshCollider.h"
#include "../Colliders/BoxCollider.h"
#include "../WeaponComponents/WeaponComponent.h"
#include "../CameraComponent.h"
#include "../../Systems/Physics/VehicleTireFriction.h"

#include "../../Systems/Physics/RaycastGroups.h"
#include "../../Systems/Game.h"
#include "../GuiComponents/GuiComponent.h"
#include "../GuiComponents/GuiHelper.h"
#include "../Tweens/Tween.h"
#include "PennerEasing/Quint.h"
#include "../../Systems/Effects.h"
#include "../Engine/Systems/Audio.h"
#include <glm/gtx/string_cast.hpp>
#include "PennerEasing/Linear.h"
#include "../../Systems/Physics/VehicleCreate.h"

using namespace physx;

VehicleComponent::VehicleComponent() : VehicleComponent(4, false) { }

VehicleComponent::~VehicleComponent() {
    pxVehicle->release();
    delete wheelMeshPrefab;
}

VehicleComponent::VehicleComponent(nlohmann::json data) : RigidDynamicComponent(data) {
    inputTypeDigital = ContentManager::GetFromJson<bool>(data["DigitalInput"], false);

    if (!data["WheelMesh"].is_null()) {
        wheelMeshPrefab = static_cast<MeshComponent*>(ContentManager::LoadComponent<MeshComponent>(data["WheelMesh"]));
    }
    else {
        wheelMeshPrefab = new MeshComponent("Boulder.obj", "Basic.json", "Boulder.jpg");
    }

    chassisSize = ContentManager::JsonToVec3(data["ChassisSize"], glm::vec3(2.5f, 2.f, 5.f));

    wheelMass = ContentManager::GetFromJson<float>(data["WheelMass"], 20.f);
    wheelRadius = ContentManager::GetFromJson<float>(data["WheelRadius"], 0.5f);
    wheelWidth = ContentManager::GetFromJson<float>(data["WheelWidth"], 0.4f);
    wheelCount = ContentManager::GetFromJson<size_t>(data["WheelCount"], 4);

	boostPower = ContentManager::GetFromJson<float>(data["BoostPower"], 10.f);

	boostCooldown = Time(ContentManager::GetFromJson<float>(data["BoostCooldown"], 5.f));
	lastBoost = Time(-boostCooldown.GetSeconds());
    
	// Load any axle data present in data file
    for (nlohmann::json axle : data["Axles"]) {
        axleData.push_back(AxleData(
            ContentManager::GetFromJson<float>(axle["CenterOffset"], 0.f),
            ContentManager::GetFromJson<float>(axle["WheelInset"], 0.f)
        ));
    }

    Initialize();
}

VehicleComponent::VehicleComponent(size_t _wheelCount, bool _inputTypeDigital) : RigidDynamicComponent(),
    inputTypeDigital(_inputTypeDigital), chassisSize(glm::vec3(2.5f, 2.f, 5.f)),
    wheelMass(20.f), wheelRadius(0.5f), wheelWidth(0.4f), wheelCount(_wheelCount), boostPower(10.f) {

	boostCooldown = Time(5.f);
	lastBoost = Time(-boostCooldown.GetSeconds());
    wheelMeshPrefab = new MeshComponent("Boulder.obj", "Basic.json", "Boulder.jpg");

    Initialize();
}

void VehicleComponent::InitializeWheelsSimulationData(const PxVec3* wheelCenterActorOffsets) {
    //Set up the wheels.
    PxVehicleWheelData wheels[PX_MAX_NB_WHEELS]; {
        //Set up the wheel data structures with mass, moi, radius, width.
        for (PxU32 i = 0; i < wheelCount; i++) {
            wheels[i].mMass = wheelMass;
            wheels[i].mMOI = GetWheelMomentOfIntertia();
            wheels[i].mRadius = wheelRadius;
            wheels[i].mWidth = wheelWidth;
        }

        //Enable the handbrake for the rear wheels only.
        wheels[PxVehicleDrive4WWheelOrder::eREAR_LEFT].mMaxHandBrakeTorque = 4000.0f;
        wheels[PxVehicleDrive4WWheelOrder::eREAR_RIGHT].mMaxHandBrakeTorque = 4000.0f;
        //Enable steering for the front wheels only.
        wheels[PxVehicleDrive4WWheelOrder::eFRONT_LEFT].mMaxSteer = PxPi*0.3333f;
        wheels[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT].mMaxSteer = PxPi*0.3333f;
    }

    //Set up the tires.
    PxVehicleTireData tires[PX_MAX_NB_WHEELS];
    {
        //Set up the tires.
        for (PxU32 i = 0; i < wheelCount; i++)
        {
            tires[i].mType = TIRE_TYPE_NORMAL;
        }
    }

    //Set up the suspensions
    PxVehicleSuspensionData suspensions[PX_MAX_NB_WHEELS];
    {
        //Compute the mass supported by each suspension spring.
        PxF32 suspSprungMasses[PX_MAX_NB_WHEELS];
        PxVehicleComputeSprungMasses(wheelCount, wheelCenterActorOffsets, Transform::ToPx(GetChassisCenterOfMassOffset()), mass, 1, suspSprungMasses);

        //Set the suspension data.
        for (PxU32 i = 0; i < wheelCount; i++)
        {
            suspensions[i].mMaxCompression = 0.3f;
            suspensions[i].mMaxDroop = 0.1f;
            suspensions[i].mSpringStrength = 35000.0f;
            suspensions[i].mSpringDamperRate = 4500.0f;
            suspensions[i].mSprungMass = suspSprungMasses[i];
        }

        //Set the camber angles.
        const PxF32 camberAngleAtRest = 0.0;
        const PxF32 camberAngleAtMaxDroop = 0.01f;
        const PxF32 camberAngleAtMaxCompression = -0.01f;
        for (PxU32 i = 0; i < wheelCount; i += 2)
        {
            suspensions[i + 0].mCamberAtRest = camberAngleAtRest;
            suspensions[i + 1].mCamberAtRest = -camberAngleAtRest;
            suspensions[i + 0].mCamberAtMaxDroop = camberAngleAtMaxDroop;
            suspensions[i + 1].mCamberAtMaxDroop = -camberAngleAtMaxDroop;
            suspensions[i + 0].mCamberAtMaxCompression = camberAngleAtMaxCompression;
            suspensions[i + 1].mCamberAtMaxCompression = -camberAngleAtMaxCompression;
        }
    }

    //Set up the wheel geometry.
    PxVec3 suspTravelDirections[PX_MAX_NB_WHEELS];
    PxVec3 wheelCentreCMOffsets[PX_MAX_NB_WHEELS];
    PxVec3 suspForceAppCMOffsets[PX_MAX_NB_WHEELS];
    PxVec3 tireForceAppCMOffsets[PX_MAX_NB_WHEELS];
    {
        //Set the geometry data.
        for (PxU32 i = 0; i < wheelCount; i++)
        {
            //Vertical suspension travel.
            suspTravelDirections[i] = PxVec3(0, -1, 0);

            //Wheel center offset is offset from rigid body center of mass.
            wheelCentreCMOffsets[i] = wheelCenterActorOffsets[i] - Transform::ToPx(GetChassisCenterOfMassOffset());

            //Suspension force application point 0.3 metres below 
            //rigid body center of mass.
            suspForceAppCMOffsets[i] =
                PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);

            //Tire force application point 0.3 metres below 
            //rigid body center of mass.
            tireForceAppCMOffsets[i] =
                PxVec3(wheelCentreCMOffsets[i].x, -0.3f, wheelCentreCMOffsets[i].z);
        }
    }

    //Set up the filter data of the raycast that will be issued by each suspension.
    PxFilterData qryFilterData;
    setupNonDrivableSurface(qryFilterData);

    //Set the wheel, tire and suspension data.
    //Set the geometry data.
    //Set the query filter data
    for (PxU32 i = 0; i < wheelCount; i++) {
        wheelsSimData->setWheelData(i, wheels[i]);
        wheelsSimData->setTireData(i, tires[i]);
        wheelsSimData->setSuspensionData(i, suspensions[i]);
        wheelsSimData->setSuspTravelDirection(i, suspTravelDirections[i]);
        wheelsSimData->setWheelCentreOffset(i, wheelCentreCMOffsets[i]);
        wheelsSimData->setSuspForceAppPointOffset(i, suspForceAppCMOffsets[i]);
        wheelsSimData->setTireForceAppPointOffset(i, tireForceAppCMOffsets[i]);
        wheelsSimData->setSceneQueryFilterData(i, qryFilterData);
        wheelsSimData->setWheelShapeMapping(i, PxI32(i + colliders.size() - (wheelCount + 1)));
    }

    //Add a front and rear anti-roll bar
    PxVehicleAntiRollBarData barFront;
    barFront.mWheel0 = PxVehicleDrive4WWheelOrder::eFRONT_LEFT;
    barFront.mWheel1 = PxVehicleDrive4WWheelOrder::eFRONT_RIGHT;
    barFront.mStiffness = 20000.0f;
    wheelsSimData->addAntiRollBarData(barFront);
    PxVehicleAntiRollBarData barRear;
    barRear.mWheel0 = PxVehicleDrive4WWheelOrder::eREAR_LEFT;
    barRear.mWheel1 = PxVehicleDrive4WWheelOrder::eREAR_RIGHT;
    barRear.mStiffness = 20000.0f;
    wheelsSimData->addAntiRollBarData(barRear);
}

void VehicleComponent::CreateVehicle() {
    Physics &physics = Physics::Instance();

    PxMaterial *material = ContentManager::GetPxMaterial("Default.json");

    {
        //Wheel and chassis query filter data.
        //Optional: cars don't drive on other cars.
        PxFilterData wheelQryFilterData;
        setupNonDrivableSurface(wheelQryFilterData);
		wheelQryFilterData.word0 = GetRaycastGroup();
        PxFilterData chassisQryFilterData;
        setupNonDrivableSurface(chassisQryFilterData);
		chassisQryFilterData.word0 = GetRaycastGroup();

        //Construct a convex mesh for a cylindrical wheel.
        PxConvexMesh* wheelMesh = createWheelMesh(wheelWidth, wheelRadius, physics.GetApi(), physics.GetCooking());
        for (PxU32 i = 0; i < wheelCount; ++i) {
            ConvexMeshCollider *collider = new ConvexMeshCollider("Wheels", material, wheelQryFilterData, false, wheelMesh);
            AddCollider(collider);
            wheelColliders.push_back(collider);
        }

        AddCollider(new BoxCollider("Chassis", material, chassisQryFilterData, false, chassisSize));
    }

    //Set up the sim data for the wheels.
    wheelsSimData = PxVehicleWheelsSimData::allocate(wheelCount);
    {
        //Compute the wheel center offsets from the origin.
        PxVec3 wheelCenterActorOffsets[PX_MAX_NB_WHEELS];
        const PxVec3 chassisDims = Transform::ToPx(chassisSize);
        for (PxU32 i = PxVehicleDrive4WWheelOrder::eFRONT_LEFT; i < wheelCount; i += 2) {
            const AxleData axle = axleData[i / 2];
            wheelCenterActorOffsets[i + 0] = PxVec3((-chassisDims.x + wheelWidth) * 0.5f + axle.wheelInset, -(chassisDims.y*0.5f + wheelRadius), axle.centerOffset);
            wheelCenterActorOffsets[i + 1] = PxVec3((chassisDims.x - wheelWidth) * 0.5f - axle.wheelInset, -(chassisDims.y*0.5f + wheelRadius), axle.centerOffset);
        }

        //Set up the simulation data for all wheels.
        InitializeWheelsSimulationData(wheelCenterActorOffsets);
    }

    //Set up the sim data for the vehicle drive model.
    {
        //Diff
        PxVehicleDifferential4WData diff;
        diff.mType = PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD;
        driveSimData.setDiffData(diff);

        //Engine
        PxVehicleEngineData engine;
        engine.mPeakTorque = 1000.0f;
        engine.mMaxOmega = 1200.0f;//approx x10 rpm
        driveSimData.setEngineData(engine);

        //Gears
        PxVehicleGearsData gears;
        gears.mSwitchTime = 0.1f;
        driveSimData.setGearsData(gears);

        //Clutch
        PxVehicleClutchData clutch;
        clutch.mStrength = 100.0f;
        driveSimData.setClutchData(clutch);

        //Ackermann steer accuracy
        PxVehicleAckermannGeometryData ackermann;
        ackermann.mAccuracy = 1.0f;
        ackermann.mAxleSeparation =
            wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).z -
            wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).z;
        ackermann.mFrontWidth =
            wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).x -
            wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).x;
        ackermann.mRearWidth =
            wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_RIGHT).x -
            wheelsSimData->getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT).x;
        driveSimData.setAckermannGeometryData(ackermann);
    }

    //Create a vehicle from the wheels and drive sim data.
    pxVehicle = PxVehicleDrive4W::allocate(wheelCount);
    pxVehicle->setup(&physics.GetApi(), actor, *wheelsSimData, driveSimData, wheelCount - 4);

    //Free the sim data because we don't need that any more.
    wheelsSimData->free();
}

void VehicleComponent::Initialize() {
    Physics &physics = Physics::Instance();

	raycastGroup = RaycastGroups::AddVehicleGroup();

    SetMomentOfInertia(GetChassisMomentOfInertia());
    SetCenterOfMassOffset(GetChassisCenterOfMassOffset());

    //Create a vehicle that will drive on the plane.
    CreateVehicle();

    // Fill any remaining any remaining axle data
    const float axleCount = ceil(static_cast<float>(wheelCount) * 0.5f);
    for (size_t i = axleData.size(); i < axleCount; ++i) {
        axleData.push_back(AxleData(glm::mix(0.5f*chassisSize.z, -0.5f*chassisSize.z, static_cast<float>(i) / axleCount)));
    }

    // Create the meshes for each of the wheels
    for (size_t i = 0; i < wheelCount; ++i) {
        MeshComponent* wheel = new MeshComponent(wheelMeshPrefab);
        wheelMeshes.push_back(wheel);
    }

    UpdateWheelTransforms();
}

void VehicleComponent::UpdateWheelTransforms() {
    for (size_t i = 0; i < wheelCount; ++i) {
        MeshComponent* wheel = wheelMeshes[i];
        Transform pose = wheelColliders[i]->GetLocalTransform();
        wheel->transform.SetPosition(pose.GetLocalPosition());
        wheel->transform.SetRotationAxisAngles(Transform::UP, glm::radians(i % 2 == 0 ? 180.f : 0.f));
        wheel->transform.Rotate(pose.GetLocalRotation());
    }
}

float VehicleComponent::GetChassisMass() const {
    return mass;
}

glm::vec3 VehicleComponent::GetChassisSize() const {
    return chassisSize;
}

glm::vec3 VehicleComponent::GetChassisMomentOfInertia() const {
    return glm::vec3((chassisSize.y*chassisSize.y + chassisSize.z*chassisSize.z)*mass / 12.0f,
        (chassisSize.x*chassisSize.x + chassisSize.z*chassisSize.z)*0.8f*mass / 12.0f,
        (chassisSize.x*chassisSize.x + chassisSize.y*chassisSize.y)*mass / 12.0f);
}

glm::vec3 VehicleComponent::GetChassisCenterOfMassOffset() const {
    return glm::vec3(0.0f, -1.0f, 0.25f);
}

float VehicleComponent::GetWheelMass() const {
    return wheelMass;
}

float VehicleComponent::GetWheelRadius() const {
    return wheelRadius;
}

float VehicleComponent::GetWheelWidth() const {
    return wheelWidth;
}

float VehicleComponent::GetWheelMomentOfIntertia() const {
    return 0.5f*wheelMass*wheelRadius*wheelRadius;
}

size_t VehicleComponent::GetWheelCount() const {
    return wheelCount;
}

std::vector<AxleData> VehicleComponent::GetAxleData() const {
    return axleData;
}

void VehicleComponent::RenderDebugGui() {
    RigidDynamicComponent::RenderDebugGui();
    
    if (ImGui::TreeNode("Drive")) {
        if (ImGui::TreeNode("Differential")) {
            PxVehicleDifferential4WData diffData = driveSimData.getDiffData();
            bool changed = false;
            changed = changed || ImGui::DragFloat("Centre Bias", &diffData.mCentreBias, 0.1f);
            changed = changed || ImGui::DragFloat("Front Bias", &diffData.mFrontBias, 0.1f);
            changed = changed || ImGui::DragFloat("Rear Bias", &diffData.mRearBias, 0.1f);
            changed = changed || ImGui::DragFloat("Front Left-Right Split", &diffData.mFrontLeftRightSplit, 0.1f);
            changed = changed || ImGui::DragFloat("Rear Left-Right Split", &diffData.mRearLeftRightSplit, 0.1f);
            changed = changed || ImGui::DragFloat("Front-Rear Split", &diffData.mFrontRearSplit, 0.1f);
            if (changed)
                driveSimData.setDiffData(diffData);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Engine")) {
            PxVehicleEngineData engineData = driveSimData.getEngineData();
            bool changed = false;
            changed = changed || ImGui::DragFloat("Peak Torque", &engineData.mPeakTorque, 1.f);
            changed = changed || ImGui::DragFloat("Max Omega", &engineData.mMaxOmega, 1.f);
            if (changed)
                driveSimData.setEngineData(engineData);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Gears")) {
            PxVehicleGearsData gearsData = driveSimData.getGearsData();
            bool changed = false;
            changed = changed || ImGui::DragFloat("Switch Time", &gearsData.mSwitchTime, 0.01f);
            if (changed)
                driveSimData.setGearsData(gearsData);
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Clutch")) {
            PxVehicleClutchData clutchData = driveSimData.getClutchData();
            bool changed = false;
            changed = changed || ImGui::DragFloat("Strength", &clutchData.mStrength, 1.f);
            if (changed)
                driveSimData.setClutchData(clutchData);
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }

    // TODO: Implement all of these
    if (ImGui::TreeNode("Wheels")) {
        if (ImGui::TreeNode("Wheels")) {
            /*PxVehicleWheelData wheelData = wheelsSimData->getWheelData(0);
            bool changed = false;
            if (changed)
                wheelsSimData->setWheelData(0, wheelData);*/
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Tires")) {
            ImGui::TreePop();
        }

        if (ImGui::TreeNode("Suspensions")) {
            ImGui::TreePop();
        }

        ImGui::TreePop();
    }
}

ComponentType VehicleComponent::GetType() {
    return ComponentType_Vehicle;
}

void VehicleComponent::HandleEvent(Event *event) {}

void VehicleComponent::SetEntity(Entity* _entity) {
    RigidbodyComponent::SetEntity(_entity);
    for (MeshComponent *component : wheelMeshes) {
        /*if (component->GetEntity() != nullptr) {
        EntityManager::RemoveComponent(component->GetEntity(), component);
        }*/
        EntityManager::AddComponent(GetEntity(), component);
    }

    //Set the vehicle to rest in first gear.
    //Set the vehicle to use auto-gears.
    pxVehicle->setToRestState();
    pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
    pxVehicle->mDriveDynData.setUseAutoGears(true);
}

void VehicleComponent::UpdateFromPhysics(physx::PxTransform t) {
    Component::UpdateFromPhysics(t);
    UpdateWheelTransforms();
}


void VehicleComponent::TakeDamage(WeaponComponent* damager, float _damage) {
	if (!damager) return;
    PlayerData* attacker = Game::GetPlayerFromEntity(damager->GetEntity());
    PlayerData* me = Game::GetPlayerFromEntity(GetEntity());

    if (attacker->teamIndex == me->teamIndex) return;
    health -= _damage * (1.f-resistance);

    HumanData* attackerPlayer = Game::GetHumanFromEntity(damager->GetEntity());
    if (attackerPlayer) {
        Entity* entity = EntityManager::FindFirstChild(attackerPlayer->camera->GetGuiRoot(), "HitIndicator");
        GuiComponent* gui = entity->GetComponent<GuiComponent>();
        GuiHelper::OpacityEffect(gui, 0.5, 0.8f, 0.1, 0.1);
    }

    HumanData *myPlayer = Game::GetHumanFromEntity(GetEntity());
    if (myPlayer) {
		{
            Entity* entity = EntityManager::FindFirstChild(myPlayer->camera->GetGuiRoot(), "DamageIndicator");
            GuiComponent* gui = entity->GetComponent<GuiComponent>();

            GuiHelper::OpacityEffect(gui, 1.0, 0.8f, 0.25, 0.25);

            // NOTE: This isn't really a tween... but it's a nice hacky use for the tween system
            // We should probably make a special version of the tween for exactly this case
            const std::string tweenTag = "DamageIndicator" + std::to_string(myPlayer->id);
            Tween* oldTween = Effects::Instance().FindTween(tweenTag);
            if (oldTween) Effects::Instance().DestroyTween(oldTween);
            auto tween = Effects::Instance().CreateTween<float, easing::Linear::easeIn>(0.f, 1.f, 1.0, StateManager::gameTime);
            tween->SetTag(tweenTag);
            tween->SetUpdateCallback([gui, myPlayer, attacker](float& value) mutable {
                if (!myPlayer->alive || !attacker->alive) return;
                const glm::vec3 cameraPos = myPlayer->camera->GetPosition();
                const glm::vec3 cameraForward = normalize(Transform::ProjectVectorOnPlane(myPlayer->camera->GetForward(), Transform::UP));
                const glm::vec3 cameraRight = normalize(Transform::ProjectVectorOnPlane(myPlayer->camera->GetRight(), Transform::UP));
                const glm::vec3 direction = normalize(Transform::ProjectVectorOnPlane(cameraPos - attacker->vehicleEntity->transform.GetGlobalPosition(), Transform::UP));
                const float sign = dot(cameraRight, direction) < 0.f ? 1.f : -1.f;
                const float theta = sign * acos(dot(cameraForward, direction));
                gui->transform.SetRotationAxisAngles(-Transform::FORWARD, theta);
            });
            tween->Start();
        }
        
        {
            const float healthPercent = glm::max(0.f, health) / 1000.f;
            Entity* entity = EntityManager::FindFirstChild(myPlayer->camera->GetGuiRoot(), "HealthBar");
            GuiComponent* gui = GuiHelper::GetSecondGui(entity);

            const std::string tweenTag = "HealthBar" + std::to_string(myPlayer->id);
            Effects::Instance().DestroyTween(tweenTag);
            
            Transform& mask = gui->GetMask();
            const glm::vec3 start = mask.GetLocalScale();
            const glm::vec3 end = gui->transform.GetLocalScale() * glm::vec3(healthPercent, 1.f, 1.f);
            auto tween = Effects::Instance().CreateTween<glm::vec3, easing::Quint::easeOut>(start, end, 0.1, StateManager::gameTime);
            tween->SetTag(tweenTag);
            tween->SetUpdateCallback([&mask](glm::vec3& value) mutable {
                mask.SetScale(value);
            });
            tween->Start();
        }
    }

    if (health <= 0) {
        attacker->killCount++;
        Game::gameData.teams[attacker->teamIndex].killCount++;

        for (size_t i = 0; i < Game::gameData.humanCount; ++i) {
            HumanData& player = Game::humanPlayers[i];
            Entity* killFeed = EntityManager::FindFirstChild(player.camera->GetGuiRoot(), "KillFeed");

            Entity* row = ContentManager::LoadEntity("Menu/KillFeedRow.json", killFeed);
            std::vector<GuiComponent*> guis = row->GetComponents<GuiComponent>();
            GuiComponent* player0Gui = guis[0];
            GuiComponent* player1Gui = guis[1];
            GuiComponent* weaponGui = guis[2];
            
            std::vector<Entity*> rows = EntityManager::GetChildren(killFeed);

            player1Gui->SetText(me->name);
            const glm::vec2 fontDims = player1Gui->GetFontDimensions();
            
            Texture* weaponTexture = nullptr;
            switch (damager->GetType()) {
            case ComponentType_MachineGun:
                weaponTexture = ContentManager::GetTexture("HUD/bullets.png");
                break;
            case ComponentType_RocketLauncher:
                weaponTexture = ContentManager::GetTexture("HUD/explosion.png");
                break;
            case ComponentType_RailGun:
                weaponTexture = ContentManager::GetTexture("HUD/target.png");
                break;
            default:;
            }
            
            weaponGui->SetTexture(weaponTexture);
            weaponGui->transform.Translate(-glm::vec3(fontDims.x + 10.f, 0.f, 0.f));
            
            player0Gui->SetText(attacker->name);
            player0Gui->transform.Translate(-glm::vec3(fontDims.x + 50.f, 0.f, 0.f));

            constexpr size_t maxCount = 5;

            const std::string tweenTag = "KillFeed";
            Tween* oldTween = Effects::Instance().FindTween(tweenTag);
            if (oldTween) Effects::Instance().DestroyTween(oldTween);

            auto tween = Effects::Instance().CreateTween<float, easing::Quint::easeOut>(0.f, 1.f, 0.5, StateManager::gameTime);
            tween->SetTag(tweenTag);
            tween->SetUpdateCallback([rows, maxCount](float& value) mutable {
                for (int j = 0; j < rows.size(); ++j) {
                    Entity* row = rows[j];

                    // Tween in position
                    float start = 30.f * (static_cast<int>(rows.size()) - 2 - j);
                    float end = 30.f * (static_cast<int>(rows.size()) - 1 - j);
                    GuiHelper::SetGuiYPositions(row, 20.f + glm::mix(start, end, value));

                    // Tween in/out opacity
                    for (GuiComponent* gui : row->GetComponents<GuiComponent>()) {
                        if (rows.size() >= maxCount && j < rows.size() - maxCount) {
                            gui->SetOpacity(1.f - value);
                        } else if (gui->GetTextureOpacity() < 1.f || gui->GetFontOpacity() < 1.f) {
                            gui->SetOpacity(value);
                        }
                    }
                }
            });

            if (rows.size() >= maxCount) {
                tween->SetFinishedCallback([rows, maxCount](float& value) mutable {
                    for (size_t i = 0; i < rows.size() - maxCount; ++i) {
                        EntityManager::DestroyEntity(rows[i]);
                    }
                });
            }

            tween->Start();
        }

        me->deathCount++;
        me->alive = false;
		me->diedTime = StateManager::gameTime;

        Physics::Instance().AddToDelete(GetEntity());
    }
}

float VehicleComponent::GetHealth() {
	return health;
}

size_t VehicleComponent::GetRaycastGroup() const {
	return raycastGroup;
}


void VehicleComponent::Boost(glm::vec3 boostDir) {
	if (GetTimeSinceBoost() > boostCooldown && boostDir != glm::vec3(0)) {
		pxVehicle->getRigidDynamicActor()->addForce(-Transform::ToPx(glm::normalize(boostDir) * boostPower * GetChassisMass()), PxForceMode::eIMPULSE, true);
		lastBoost = StateManager::gameTime;

		HumanData* player = Game::GetHumanFromEntity(GetEntity());
		if (player) {
			Entity* bar = EntityManager::FindFirstChild(player->camera->GetGuiRoot(), "BoostBar");

			GuiComponent* boostBar = GuiHelper::GetSecondGui(bar);
            Transform& mask = boostBar->GetMask();

			const Time emptyTime = 1.0;

            const glm::vec3 emptyStart = mask.GetLocalScale();
            const glm::vec3 emptyEnd = boostBar->transform.GetLocalScale() * glm::vec3(0.f, 1.f, 1.f);
			auto tweenEmpty = Effects::Instance().CreateTween<glm::vec3, easing::Quint::easeOut>(emptyStart, emptyEnd, emptyTime, StateManager::gameTime);
			tweenEmpty->SetUpdateCallback([&mask](glm::vec3 &value) mutable {
                mask.SetScale(value);
			});

            const glm::vec3 fillStart = emptyEnd;
            const glm::vec3 fillEnd = boostBar->transform.GetLocalScale();
            auto tweenFill = Effects::Instance().CreateTween<glm::vec3, easing::Linear::easeNone>(fillStart, fillEnd , boostCooldown - emptyTime, StateManager::gameTime);
			tweenFill->SetUpdateCallback([&mask](glm::vec3 &value) mutable {
                mask.SetScale(value);
			});
			tweenEmpty->SetNext(tweenFill);
			tweenEmpty->Start();

			//play boost sound
		}
	}
	else {
	//uable to boost sound??	
	}
}

void VehicleComponent::HandleAcceleration(float forwardPower, float backwardPower) {
	const float amountPressed = abs(forwardPower - backwardPower);
	bool brake = false;
	float speed = pxVehicle->computeForwardSpeed();
	if (backwardPower) {
		if (amountPressed < 0.01) { // if both are pressed 
			brake = true;
			pxVehicleInputData.setAnalogBrake(1.f);
		}
		else { // just backward power
			if (speed > 5.f) { // going fast brake 
				brake = true;
				pxVehicleInputData.setAnalogBrake(backwardPower);
			}
			else { // go to reverse
				pxVehicleInputData.setAnalogBrake(0.0f);
				pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eREVERSE);
			}
		}
	}
	else {// if not trying to reverse release brake
		pxVehicleInputData.setAnalogBrake(0.0f);
	}

	if (amountPressed > 0.1 && forwardPower > backwardPower) {
		if (pxVehicle->mDriveDynData.getCurrentGear() < PxVehicleGearsData::eNEUTRAL || speed <= 5.f) {
			pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
		}
	}

	if (!brake && amountPressed > 0.01) pxVehicleInputData.setAnalogAccel(amountPressed);
	else{
		pxVehicleInputData.setAnalogAccel(0);
	}
}

void VehicleComponent::Steer( float amount) {
	pxVehicleInputData.setAnalogSteer(amount);
}

void VehicleComponent::Handbrake( float amount) {
	pxVehicleInputData.setAnalogHandbrake(amount);
}

Time VehicleComponent::GetTimeSinceBoost() {
	return StateManager::gameTime - lastBoost;
}

void VehicleComponent::OnContact(RigidbodyComponent* body) {
    VehicleComponent* otherVehicle = body->GetEntity()->GetComponent<VehicleComponent>();
    if (otherVehicle) {
        Audio::Instance().PlayAudio("Content/Sounds/car-on-car2.mp3");
    } else {
        Audio::Instance().PlayAudio("Content/Sounds/car-on-car.mp3");
    }
}

void VehicleComponent::OnTrigger(RigidbodyComponent* body) {
    
}
