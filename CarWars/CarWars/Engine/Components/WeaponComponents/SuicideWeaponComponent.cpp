#include "SuicideWeaponComponent.h"

SuicideWeaponComponent::SuicideWeaponComponent() : WeaponComponent(10000000000.f) {}
void SuicideWeaponComponent::Shoot(glm::vec3 position) {}
void SuicideWeaponComponent::Charge() {}

ComponentType SuicideWeaponComponent::GetType() {
	return ComponentType_SuicideWeapon;
}
void SuicideWeaponComponent::HandleEvent(Event *event) {}

void SuicideWeaponComponent::RenderDebugGui() {
	WeaponComponent::RenderDebugGui();
}