#include "RaycastGroups.h"

size_t RaycastGroups::allGroups = 1;

size_t RaycastGroups::GetGroupsMask() {
	return allGroups;
}

size_t RaycastGroups::GetGroupsMask(size_t excludeMask) {
	return allGroups ^ excludeMask;
}

size_t RaycastGroups::GetDefaultGroup() {
	return 1;
}

size_t RaycastGroups::AddVehicleGroup() {
	static size_t lastGroup = 1;
	size_t vehicleGroup = 1 << ++lastGroup;
	allGroups |= vehicleGroup;
	return vehicleGroup;
}