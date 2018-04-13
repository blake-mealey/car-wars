#pragma once

#include <vector>

class RaycastGroups {
public:
	static size_t GetGroupsMask();
	static size_t GetGroupsMask(size_t excludeMask);

	static size_t GetDefaultGroup();
	static size_t GetPowerUpGroup();
	static size_t AddVehicleGroup();
private:
	static size_t allGroups;
};
