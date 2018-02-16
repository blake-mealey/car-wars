#pragma once

#include <glm/glm.hpp>

struct DirectionLight {
	DirectionLight(glm::vec3 _color, glm::vec3 _direction) : color(_color), direction(_direction) {}

	glm::vec3 color;			float __padding0[1];
	glm::vec3 direction;		float __padding1[1];
};