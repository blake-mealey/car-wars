#include "VehicleComponent.h"
#include "../Systems/Content/ContentManager.h"
#include "../Entities/EntityManager.h"

#include "../Systems/Physics/VehicleCreate.h"
#include "imgui/imgui.h"
#include "../Systems/Physics.h"

using namespace physx;

VehicleComponent::VehicleComponent() : VehicleComponent(4, false) { }

VehicleComponent::VehicleComponent(nlohmann::json data) {
	inputTypeDigital = ContentManager::GetFromJson<bool>(data["DigitalInput"], false);

	if (!data["WheelMesh"].is_null()) {
		wheelMeshPrefab = static_cast<MeshComponent*>(ContentManager::LoadComponent<MeshComponent>(data["WheelMesh"]));
	} else {
		wheelMeshPrefab = new MeshComponent("Boulder.obj", "Basic.json", "Boulder.jpg");
	}

    chassisMass = ContentManager::GetFromJson<float>(data["ChassisMass"], 1500.f);
    chassisSize = ContentManager::JsonToVec3(data["ChassisSize"], glm::vec3(2.5f, 2.f, 5.f));

    wheelMass = ContentManager::GetFromJson<float>(data["WheelMass"], 20.f);
    wheelRadius = ContentManager::GetFromJson<float>(data["WheelRadius"], 0.5f);
    wheelWidth = ContentManager::GetFromJson<float>(data["WheelWidth"], 0.4f);
    wheelCount = ContentManager::GetFromJson<size_t>(data["WheelCount"], 4);

    // Load any axle data present in data file
    for (nlohmann::json axle : data["Axles"]) {
        axleData.push_back(AxleData(
            ContentManager::GetFromJson<float>(axle["CenterOffset"], 0.f),
            ContentManager::GetFromJson<float>(axle["WheelInset"], 0.f)
        ));
    }

	Initialize();
}

VehicleComponent::VehicleComponent(size_t _wheelCount, bool _inputTypeDigital) :
        inputTypeDigital(_inputTypeDigital), chassisMass(1500.f), chassisSize(glm::vec3(2.5f, 2.f, 5.f)),
		wheelMass(20.f), wheelRadius(0.5f), wheelWidth(0.4f), wheelCount(_wheelCount) {
	
	wheelMeshPrefab = new MeshComponent("Boulder.obj", "Basic.json", "Boulder.jpg");

    Initialize();
}

void VehicleComponent::Initialize() {
    Physics &physics = Physics::Instance();

    //Create a vehicle that will drive on the plane.
    pxVehicle = createVehicle4W(*this, ContentManager::GetPxMaterial("Default.json"), &physics.GetApi(), &physics.GetCooking());

    //Set the vehicle to rest in first gear.
    //Set the vehicle to use auto-gears.
    pxVehicle->setToRestState();
    pxVehicle->mDriveDynData.forceGearChange(PxVehicleGearsData::eFIRST);
    pxVehicle->mDriveDynData.setUseAutoGears(true);

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
}

void VehicleComponent::UpdateWheelTransforms() {
	PxShape** shapes = new PxShape*[wheelCount];
	pxVehicle->getRigidDynamicActor()->getShapes(shapes, wheelCount, 0);
	for (size_t i = 0; i < wheelCount; ++i) {
		MeshComponent* wheel = wheelMeshes[i];
		PxTransform pose = shapes[i]->getLocalPose();
		wheel->transform.SetPosition(Transform::FromPx(pose.p));
		wheel->transform.SetRotationAxisAngles(Transform::UP, glm::radians(i % 2 == 0 ? 180.f : 0.f));
		wheel->transform.Rotate(Transform::FromPx(pose.q));
	}
	delete[] shapes;
}

float VehicleComponent::GetChassisMass() const {
    return chassisMass;
}

glm::vec3 VehicleComponent::GetChassisSize() const {
    return chassisSize;
}

glm::vec3 VehicleComponent::GetChassisMomentOfInertia() const {
    return glm::vec3((chassisSize.y*chassisSize.y + chassisSize.z*chassisSize.z)*chassisMass / 12.0f,
        (chassisSize.x*chassisSize.x + chassisSize.z*chassisSize.z)*0.8f*chassisMass / 12.0f,
        (chassisSize.x*chassisSize.x + chassisSize.y*chassisSize.y)*chassisMass / 12.0f);
}

glm::vec3 VehicleComponent::GetChassisCenterOfMassOffset() const {
    return glm::vec3(0.0f, -chassisSize.y*0.5f + 0.65f, 0.25f);
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
    Component::RenderDebugGui();
    if (ImGui::TreeNode("Transform")) {
        Transform t = pxVehicle->getRigidDynamicActor()->getGlobalPose();
        if (t.RenderDebugGui()) pxVehicle->getRigidDynamicActor()->setGlobalPose(Transform::ToPx(t), true);
        ImGui::TreePop();
    }
}

ComponentType VehicleComponent::GetType() {
    return ComponentType_Vehicle;
}

void VehicleComponent::HandleEvent(Event *event) {}

void VehicleComponent::SetEntity(Entity* _entity) {
	Component::SetEntity(_entity);
	for (MeshComponent *component : wheelMeshes) {
		/*if (component->GetEntity() != nullptr) {
			EntityManager::RemoveComponent(component->GetEntity(), component);
		}*/
		EntityManager::AddComponent(GetEntity(), component);
	}

    pxVehicle->getRigidDynamicActor()->setGlobalPose(Transform::ToPx(GetEntity()->transform));
    Physics::Instance().GetScene().addActor(*pxVehicle->getRigidDynamicActor());
}

void VehicleComponent::UpdateFromPhysics(physx::PxTransform t) {
    Component::UpdateFromPhysics(t);
    UpdateWheelTransforms();
}
