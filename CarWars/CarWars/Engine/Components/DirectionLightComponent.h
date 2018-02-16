#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <glm/glm.hpp>
#include "../Systems/Content/DirectionLight.h"

class DirectionLightComponent : public Component {
public:
	DirectionLightComponent(nlohmann::json data);
	DirectionLightComponent(glm::vec3 _color, glm::vec3 _direction);

	glm::vec3 GetColor() const;
	glm::vec3 GetDirection() const;
	DirectionLight GetData() const;
	bool IsShadowCaster() const;
	void SetShadowCaster(bool _castsShadows);

    void SetDirection(glm::vec3 _direction);

	ComponentType GetType() override;
	void HandleEvent(Event* event) override;
private:
	glm::vec3 color;
	glm::vec3 direction;
	bool castsShadows;
};

