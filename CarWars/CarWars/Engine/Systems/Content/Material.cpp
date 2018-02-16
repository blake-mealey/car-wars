#include "Material.h"
#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>

void Material::RenderDebugGui() {
    ImGui::ColorEdit3("Diffuse", glm::value_ptr(diffuseColor));
    ImGui::ColorEdit3("Specular", glm::value_ptr(specularColor));
    ImGui::DragFloat("Specularity", &specularity, 0.01f);
    ImGui::DragFloat("Emissiveness", &emissiveness, 0.01f, 0.f, 1.f);
}
