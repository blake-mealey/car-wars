#include "GuiComponent.h"
#include "../../Entities/EntityManager.h"
#include "../../Systems/Content/ContentManager.h"
#include "imgui/imgui.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

/*GuiComponent::~GuiComponent() {
	delete font;
}*/

GuiComponent::GuiComponent(nlohmann::json data) : guiRoot(nullptr), font(nullptr), texture(nullptr) {
	transformID = EntityManager::AddTransform(Transform(data));
	//transform = Transform(data);
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

    std::string textXAlignment = ContentManager::GetFromJson<std::string>(data["TextXAlignment"], "Centre");
    if (textXAlignment == "Left") {
        textAlignment[0] = TextXAlignment::Left;
    } else if (textXAlignment == "Centre") {
        textAlignment[0] = TextXAlignment::Centre;
    } else if (textXAlignment == "Right") {
        textAlignment[0] = TextXAlignment::Right;
    }
    
    std::string textYAlignment = ContentManager::GetFromJson<std::string>(data["TextYAlignment"], "Centre");
    if (textXAlignment == "Top") {
        textAlignment[1] = TextYAlignment::Top;
    } else if (textXAlignment == "Centre") {
        textAlignment[1] = TextYAlignment::Centre;
    } else if (textXAlignment == "Bottom") {
        textAlignment[1] = TextYAlignment::Bottom;
    }
}

void GuiComponent::InternalRenderDebugGui() {
	//Component::RenderDebugGui();
	if (ImGui::TreeNode("Transform")) {
		EntityManager::GetTransform(transformID).RenderDebugGui(1.f, 1.f);
		ImGui::TreePop();
	}

    ImGui::DragFloat2("Scaled Position", glm::value_ptr(scaledPosition), 0.1f);
    ImGui::DragFloat2("Scaled Scale", glm::value_ptr(scaledScale), 0.1f);
    ImGui::DragFloat2("Anchor Point", glm::value_ptr(anchorPoint), 0.1f);

    ImGui::DragInt2("Text Alignment", textAlignment, 1, TextXAlignment::Left, TextXAlignment::Right);
    //ImGui::InputText("Font", );
    ImGui::ColorEdit4("Font Color", glm::value_ptr(fontColor));
    //ImGui::InputText("Text", );
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

void GuiComponent::InternalSetEntity(Entity& _entity) {
	//Component::SetEntity(_entity);
	Entity* e = &_entity;
    EntityManager::GetTransform(transformID).parentID = e->transformID;
	do {
		if (e->HasTag("GuiRoot")) guiRoot = e;
		e = EntityManager::GetParent(&_entity);
	} while (!guiRoot && e);
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

bool GuiComponent::IsSelected() const {
    return selected;
}

void GuiComponent::SetSelected(bool _selected) {
    selected = _selected;
}
