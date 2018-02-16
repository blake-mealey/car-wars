#pragma once

#include "glm/glm.hpp"

struct Material {
    Material(glm::vec3 _diffuseColor, glm::vec3 _specularColor, float _specularity, float _emissiveness) :
      diffuseColor(_diffuseColor), specularColor(_specularColor), specularity(_specularity), emissiveness(_emissiveness) {}

	glm::vec3 diffuseColor;
	glm::vec3 specularColor;

	float specularity;

    float emissiveness;
};
