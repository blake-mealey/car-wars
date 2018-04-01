#pragma once

#include "Component.h"
#include <json/json.hpp>
#include "../Systems/Content/Texture.h"
#include "../Entities/Transform.h"

class BillboardComponent : public Component {
public:
    BillboardComponent(nlohmann::json data);

    ComponentType GetType() override;
    void HandleEvent(Event* event) override;

    void SetEntity(Entity* _entity) override;

    void RenderDebugGui() override;

    Texture *GetTexture() const;
    glm::vec2 GetUvScale() const;

    Transform transform;
private:
    Texture *texture;
    glm::vec2 uvScale;
};
