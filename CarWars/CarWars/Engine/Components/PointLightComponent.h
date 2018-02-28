#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <glm/glm.hpp>
#include "../Systems/Content/PointLight.h"

class PointLightComponent : public Component<PointLightComponent> {
	friend class Component<PointLightComponent>;
public:
	PointLightComponent(nlohmann::json data);
	PointLightComponent(glm::vec3 _color, float _power);

	glm::vec3 GetColor() const;
	float GetPower() const;
	PointLight GetData() const;

	static constexpr ComponentType InternalGetType() { return ComponentType_PointLight; }

    void InternalRenderDebugGui();
private:
	glm::vec3 color;
	float power;
};

