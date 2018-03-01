#pragma once

#include "json/json.hpp"
#include "Mesh.h"

struct NavigationVertex {
	glm::vec3 position;
	float score;
};

class NavigationMesh {
public:
	NavigationMesh(nlohmann::json data);

private:

	void Initialize();

	Mesh *renderMesh;

	size_t columnCount;
	size_t rowCount;

	NavigationVertex **vertices;
};