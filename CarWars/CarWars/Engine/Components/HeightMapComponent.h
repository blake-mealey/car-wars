#pragma once
#include "MeshComponent.h"

class HeightMapComponent : public MeshComponent {
	Mesh* CreateMesh(nlohmann::json data);
public:
	HeightMapComponent(nlohmann::json data);
};