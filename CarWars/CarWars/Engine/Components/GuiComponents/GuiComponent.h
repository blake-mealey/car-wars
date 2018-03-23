#pragma once

#include "../Component.h"
#include "../CameraComponent.h"
#include "../../Entities/Transform.h"
#include "../../Systems/Content/Texture.h"
#include "json/json.hpp"

#include "FTGL/ftgl.h"
#include "../GuiEffects/GuiEffect.h"
#include <unordered_set>

struct TextXAlignment {
	enum { Left=0, Centre, Right };
};

struct TextYAlignment {
	enum { Top=0, Centre, Bottom };
};

class GuiComponent : public Component {
public:
    ~GuiComponent() override;
	GuiComponent(nlohmann::json data);

    Transform transform;		// TODO: GuiTransform

	ComponentType GetType() override;
	void HandleEvent(Event *event) override;

	void RenderDebugGui() override;

	void SetText(std::string _text);
	std::string GetText() const;
    
    bool HasText(std::string _text) const;
    bool ContainsText(std::string _text) const;

	void SetTexture(Texture *_texture);
	Texture* GetTexture() const;

	void SetFont(std::string fontName);
	void SetFontSize(int fontSize);
	void SetFontColor(glm::vec4 _fontColor);

    glm::vec2 GetFontDimensions();

	FTFont* GetFont() const;
	glm::vec4 GetFontColor() const;

	Entity* GetGuiRoot() const;

	void SetTextXAlignment(size_t alignment);
	void SetTextYAlignment(size_t alignment);

	size_t GetTextXAlignment() const;
	size_t GetTextYAlignment() const;

	void SetEntity(Entity *_entity) override;

    void SetUvScale(glm::vec2 _uvScale);
    glm::vec2 GetUvScale() const;

    glm::vec2 GetScaledScale() const;
    void SetScaledScale(glm::vec2 scale);

    glm::vec2 GetScaledPosition() const;
    void SetScaledPosition(glm::vec2 position);

    glm::vec2 GetAnchorPoint() const;
    void SetAnchorPoint(glm::vec2 _anchorPoint);

    bool IsSelected() const;
    void SetSelected(bool _selected);

    glm::vec4 GetSelectedFontColor() const;
    void SetSelectedFontColor(glm::vec4 color);
    
    glm::vec4 GetTextureColor() const;
    void SetTextureColor(glm::vec4 color);

    void SetOpacity(float opacity);
    void SetTextureOpacity(float opacity);
    void SetFontOpacity(float opacity);
    void AddOpacity(float opacity);
    void MultiplyOpacity(float opacity);
    float GetTextureOpacity() const;
    float GetFontOpacity() const;

    std::unordered_set<GuiEffect*> GetEffects() const;
    void AddEffect(GuiEffect* effect);
    void RemoveEffect(GuiEffect* effect);

    template<class T>
    T* GetEffect() {
        for (GuiEffect* effect : effects) {
            T* typedEffect = dynamic_cast<T*>(effect);
            if (typedEffect) {
                return typedEffect;
            }
        }
        return nullptr;
    }

    bool IsMaskEnabled() const;
    bool IsMaskInverted() const;
    bool IsClipEnabled() const;
    Transform& GetMask();
    Texture* GetMaskTexture();
    void SetMaskTexture(Texture* texture);

    void SetEmissiveness(float _emissiveness);
    float GetEmissiveness();

private:
    bool selected;

    glm::vec2 anchorPoint;

    glm::vec2 scaledPosition;
    glm::vec2 scaledScale;

	Entity *guiRoot;

    int textAlignment[2];

	FTFont *font;		// TODO: Decide which kind of font to use
	glm::vec4 fontColor;
    glm::vec4 selectedFontColor;
	std::string text;

    glm::vec4 textureColor;
    glm::vec4 selectedTextureColor;
	Texture *texture;
    glm::vec2 uvScale;

    bool maskEnabled;
    bool maskInverted;
    Transform mask;
    bool clipEnabled;
    Texture* maskTexture;

    std::unordered_set<GuiEffect*> effects;

    float emissiveness;
};
