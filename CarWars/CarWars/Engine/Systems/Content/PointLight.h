#pragma once

#include <glm/glm.hpp>

struct PointLight {
	PointLight(glm::vec3 _color, float _power, glm::vec3 _position)
		: color(_color), power(_power), position(_position) {}

	glm::vec3 color;
	float power;
	glm::vec3 position;			float __padding0[1];
};