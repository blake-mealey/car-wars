#include "GuiComponent.h"
#include "../../Entities/EntityManager.h"
#include "../../Systems/Content/ContentManager.h"
#include "imgui/imgui.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

GuiComponent::GuiComponent(nlohmann::json data) : guiRoot(nullptr), font(nullptr), texture(nullptr) {
	transform = Transform(data);
	text = ContentManager::GetFromJson<std::string>(data["Text"], "");
	SetFont(ContentManager::GetFromJson<std::string>(data["Font"], "arial.ttf"));
	SetFontSize(ContentManager::GetFromJson<int>(data["FontSize"], 36));
	fontColor = ContentManager::JsonToVec4(data["FontColor"], glm::vec4(0.f));
	if (data["Texture"].is_string()) texture = ContentManager::GetTexture(data["Texture"].get<std::string>());
    uvScale = ContentManager::JsonToVec2(data["UvScale"], glm::vec2(1.f));

    textAlignment[0] = TextXAlignment::Centre;
    textAlignment[1] = TextYAlignment::Centre;
}

ComponentType GuiComponent::GetType() {
	return ComponentType_GUI;
}

void GuiComponent::HandleEvent(Event *event) { }

/*size_t textXAlignment;
	size_t textYAlignment;

	FTFont *font;		// TODO: Decide which kind of font to use
	glm::vec4 fontColor;
	std::string text;

	Texture *texture;
    glm::vec2 uvScale;*/

void GuiComponent::RenderDebugGui() {
	Component::RenderDebugGui();
	if (ImGui::TreeNode("Transform")) {
		transform.RenderDebugGui(1.f, 1.f);
		ImGui::TreePop();
	}

    ImGui::DragInt2("Text Alignment", textAlignment, 1, TextXAlignment::Left, TextXAlignment::Right);
    //ImGui::InputText("Font", );
    ImGui::ColorEdit4("Font Color", glm::value_ptr(fontColor));
    //ImGui::InputText("Text", );
    ImGui::DragFloat2("UV Scale", glm::value_ptr(uvScale), 0.1f);
}

void GuiComponent::SetText(std::string _text) {
	text = _text;
}

std::string GuiComponent::GetText() const {
	return text;
}

void GuiComponent::SetTexture(Texture *_texture) {
	texture = _texture;
}

Texture* GuiComponent::GetTexture() const {
	return texture;
}

void GuiComponent::SetFont(std::string fontName) {
	font = new FTGLPixmapFont(("./Content/Fonts/" + fontName).c_str());
	if (font->Error())
		std::cout << "WARNING: Font " << fontName << " failed to loat with FT_Error: " << font->Error() << std::endl;
}

void GuiComponent::SetFontSize(int fontSize) {
	font->FaceSize(fontSize);
}

void GuiComponent::SetFontColor(glm::vec4 _fontColor) {
	fontColor = _fontColor;
}

FTFont *GuiComponent::GetFont() const {
	return font;
}

glm::vec4 GuiComponent::GetFontColor() const {
	return fontColor;
}

Entity* GuiComponent::GetGuiRoot() {
	return guiRoot;
}

void GuiComponent::SetTextXAlignment(size_t alignment) {
	textAlignment[0] = alignment;
}

void GuiComponent::SetTextYAlignment(size_t alignment) {
	textAlignment[1] = alignment;
}

size_t GuiComponent::GetTextXAlignment() const {
	return textAlignment[0];
}

size_t GuiComponent::GetTextYAlignment() const {
	return textAlignment[1];
}

void GuiComponent::SetEntity(Entity *_entity) {
	Component::SetEntity(_entity);
    transform.parent = &_entity->transform;
	do {
		if (_entity->HasTag("GuiRoot")) guiRoot = _entity;
		_entity = EntityManager::GetParent(_entity);
	} while (!guiRoot && _entity);
}

void GuiComponent::SetUvScale(glm::vec2 _uvScale) {
    uvScale = _uvScale;
}

glm::vec2 GuiComponent::GetUvScale() const {
    return uvScale;
}
