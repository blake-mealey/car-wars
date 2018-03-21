#pragma once

#include "glm/glm.hpp"

class Material {
public:
    Material(glm::vec4 _diffuseColor, glm::vec4 _specularColor, float _specularity, float _emissiveness) :
      diffuseColor(_diffuseColor), specularColor(_specularColor), specularity(_specularity), emissiveness(_emissiveness) {}

	glm::vec4 diffuseColor;
	glm::vec4 specularColor;

	float specularity;

    float emissiveness;

    void RenderDebugGui();
};
