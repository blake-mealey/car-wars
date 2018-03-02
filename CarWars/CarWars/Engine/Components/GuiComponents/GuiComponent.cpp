#include "GuiComponent.h"
#include "../../Entities/EntityManager.h"
#include "../../Systems/Content/ContentManager.h"
#include <iostream>

GuiComponent::GuiComponent(nlohmann::json data) : guiRoot(nullptr), font(nullptr) {
	transform = Transform(data);
	text = ContentManager::GetFromJson<std::string>(data["Text"], "");
	SetFont(ContentManager::GetFromJson<std::string>(data["Font"], "arial.ttf"));
	SetFontSize(ContentManager::GetFromJson<int>(data["FontSize"], 36));
	fontColor = ContentManager::JsonToVec4(data["FontColor"], glm::vec4(0.f));
}

ComponentType GuiComponent::GetType() {
	return ComponentType_GUI;
}

void GuiComponent::HandleEvent(Event *event) { }

void GuiComponent::RenderDebugGui() {
	Component::RenderDebugGui();
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

void GuiComponent::SetEntity(Entity *_entity) {
	Component::SetEntity(_entity);
	do {
		if (_entity->HasTag("GuiRoot")) guiRoot = _entity;
		_entity = EntityManager::GetParent(_entity);
	} while (!guiRoot && _entity);
}
