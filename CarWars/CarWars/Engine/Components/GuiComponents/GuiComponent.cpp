#include "GuiComponent.h"
#include "../../Entities/EntityManager.h"
#include "../../Systems/Content/ContentManager.h"
#include "imgui/imgui.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

GuiComponent::~GuiComponent() {
    delete font;
}

GuiComponent::GuiComponent(nlohmann::json data) : guiRoot(nullptr), font(nullptr), texture(nullptr) {
	transform = Transform(data);
	text = ContentManager::GetFromJson<std::string>(data["Text"], "");
	SetFont(ContentManager::GetFromJson<std::string>(data["Font"], "arial.ttf"));
	SetFontSize(ContentManager::GetFromJson<int>(data["FontSize"], 36));
	fontColor = ContentManager::JsonToVec4(data["FontColor"], glm::vec4(0.f, 0.f, 0.f, 1.f));
	selectedFontColor = ContentManager::JsonToVec4(data["SelectedFontColor"], glm::vec4(1.f, 1.f, 1.f, 1.f));
	if (data["Texture"].is_string()) texture = ContentManager::GetTexture(data["Texture"].get<std::string>());
    uvScale = ContentManager::JsonToVec2(data["UvScale"], glm::vec2(1.f));

    scaledPosition = ContentManager::JsonToVec2(data["ScaledPosition"], glm::vec2(0.f, 0.f));
    scaledScale = ContentManager::JsonToVec2(data["ScaledScale"], glm::vec2(0.f, 0.f));

    anchorPoint = ContentManager::JsonToVec2(data["AnchorPoint"], glm::vec2(0.f, 0.f));
    
    selected = ContentManager::GetFromJson<bool>(data["Selected"], false);

    textureColor = ContentManager::JsonToVec4(data["TextureColor"], glm::vec4(1.f));
    selectedTextureColor = ContentManager::JsonToVec4(data["SelectedTextureColor"], glm::vec4(1.f));

    std::string textXAlignment = ContentManager::GetFromJson<std::string>(data["TextXAlignment"], "Centre");
    if (textXAlignment == "Left") {
        textAlignment[0] = TextXAlignment::Left;
    } else if (textXAlignment == "Centre") {
        textAlignment[0] = TextXAlignment::Centre;
    } else if (textXAlignment == "Right") {
        textAlignment[0] = TextXAlignment::Right;
    }
    
    std::string textYAlignment = ContentManager::GetFromJson<std::string>(data["TextYAlignment"], "Centre");
    if (textYAlignment == "Top") {
        textAlignment[1] = TextYAlignment::Top;
    } else if (textYAlignment == "Centre") {
        textAlignment[1] = TextYAlignment::Centre;
    } else if (textYAlignment == "Bottom") {
        textAlignment[1] = TextYAlignment::Bottom;
    }
}

ComponentType GuiComponent::GetType() {
	return ComponentType_GUI;
}

void GuiComponent::HandleEvent(Event *event) { }

void GuiComponent::RenderDebugGui() {
	Component::RenderDebugGui();
	if (ImGui::TreeNode("Transform")) {
		transform.RenderDebugGui(1.f, 1.f);
		ImGui::TreePop();
	}

    ImGui::DragFloat2("Scaled Position", glm::value_ptr(scaledPosition), 0.1f);
    ImGui::DragFloat2("Scaled Scale", glm::value_ptr(scaledScale), 0.1f);
    ImGui::DragFloat2("Anchor Point", glm::value_ptr(anchorPoint), 0.1f);

    ImGui::DragInt2("Text Alignment", textAlignment, 1, TextXAlignment::Left, TextXAlignment::Right);
    //ImGui::InputText("Font", );
    ImGui::ColorEdit4("Font Color", glm::value_ptr(fontColor));
    ImGui::ColorEdit4("Selected Font Color", glm::value_ptr(selectedFontColor));
    //ImGui::InputText("Text", );
    ImGui::ColorEdit4("Texture Color", glm::value_ptr(textureColor));
    ImGui::ColorEdit4("Selected Texture Color", glm::value_ptr(selectedTextureColor));
    ImGui::DragFloat2("UV Scale", glm::value_ptr(uvScale), 0.01f);
}

void GuiComponent::SetText(std::string _text) {
	text = _text;
}

std::string GuiComponent::GetText() const {
	return text;
}

bool GuiComponent::HasText(std::string _text) const {
    return text.compare(_text) == 0;
}

bool GuiComponent::ContainsText(std::string _text) const {
    return text.find(_text) != std::string::npos;
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
	return selected ? selectedFontColor : fontColor;
}

Entity* GuiComponent::GetGuiRoot() const {
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

glm::vec2 GuiComponent::GetScaledScale() const {
    return scaledScale;
}

void GuiComponent::SetScaledScale(glm::vec2 scale) {
    scaledScale = scale;
}

glm::vec2 GuiComponent::GetScaledPosition() const {
    return scaledPosition;
}

void GuiComponent::SetScaledPosition(glm::vec2 position) {
    scaledPosition = position;
}

glm::vec2 GuiComponent::GetAnchorPoint() const {
    return anchorPoint;
}

void GuiComponent::SetAnchorPoint(glm::vec2 _anchorPoint) {
    anchorPoint = _anchorPoint;
}

glm::vec4 GuiComponent::GetSelectedFontColor() const {
    return selectedFontColor;
}

void GuiComponent::SetSelectedFontColor(glm::vec4 color) {
    selectedFontColor = color;
}

glm::vec4 GuiComponent::GetTextureColor() const {
    return selected ? selectedTextureColor : textureColor;
}

void GuiComponent::SetTextureColor(glm::vec4 color) {
    textureColor = color;
}

void GuiComponent::SetOpacity(float opacity) {
    SetTextureOpacity(opacity);
    SetFontOpacity(opacity);
}

void GuiComponent::SetTextureOpacity(float opacity) {
    textureColor.a = opacity;
    selectedTextureColor.a = opacity;
}

void GuiComponent::SetFontOpacity(float opacity) {
    fontColor.a = opacity;
    selectedFontColor.a = opacity;
}

void GuiComponent::AddOpacity(float opacity) {
    SetTextureOpacity(GetTextureOpacity() + opacity);
    SetFontOpacity(GetFontOpacity() + opacity);
}

void GuiComponent::MultiplyOpacity(float opacity) {
    SetTextureOpacity(GetTextureOpacity() * opacity);
    SetFontOpacity(GetFontOpacity() * opacity);
}

float GuiComponent::GetTextureOpacity() const {
    return textureColor.a;
}

float GuiComponent::GetFontOpacity() const {
    return fontColor.a;
}

std::unordered_set<GuiEffect*> GuiComponent::GetEffects() const {
    return effects;
}

bool GuiComponent::IsSelected() const {
    return selected;
}

void GuiComponent::SetSelected(bool _selected) {
    selected = _selected;
}

void GuiComponent::AddEffect(GuiEffect* effect) {
    effects.insert(effect);
    effect->Apply(this);
}

void GuiComponent::RemoveEffect(GuiEffect* effect) {
    effects.erase(effect);
    effect->Remove(this);
}
