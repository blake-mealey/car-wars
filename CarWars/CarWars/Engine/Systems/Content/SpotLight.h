#pragma once

#include <glm/glm.hpp>

struct SpotLight {
	SpotLight(glm::vec3 _color, float _power, glm::vec3 _position, float _angle, glm::vec3 _direction)
		: color(_color), power(_power), position(_position), angle(_angle), direction(_direction) {}

	glm::vec3 color;
	float power;
	glm::vec3 position;
	float angle;
	glm::vec3 direction;		float __padding0[1];
};