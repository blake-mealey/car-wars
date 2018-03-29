#pragma once

#include "Component.h"
#include <GL/glew.h>
#include "../Systems/Content/Texture.h"
#include "../Systems/Time.h"
#include "glm/glm.hpp"
#include "../Systems/Content/ShaderProgram.h"

struct Particle {
    float type;
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetime;
};

class ParticleSystemComponent : public Component {
public:
    ParticleSystemComponent();

    ComponentType GetType() override;
    void HandleEvent(Event* event) override;

    void Render(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPos);
private:
    enum Shaders {Update=0, Billboard, Count};

    void UpdateParticles();
    void RenderParticles(const glm::mat4& viewProjectionMatrix, const glm::vec3& cameraPos);

    bool isFirst;
    size_t currentVbo;
    size_t currentTfb;
    GLuint particleBuffer[2];
    GLuint transformFeedback[2];
    Texture* particleTexture;
    Texture* randomTexture;
    Time time;

    ShaderProgram* shaders[Count];
};
