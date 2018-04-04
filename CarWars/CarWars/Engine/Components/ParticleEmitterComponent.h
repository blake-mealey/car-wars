#pragma once

#include "Component.h"
#include <json/json.hpp>
#include <GL/glew.h>
#include <glm/detail/type_vec3.hpp>
#include "../Systems/Content/Texture.h"
#include "../Entities/Transform.h"
#include "../Systems/Time.h"

#define MAX_PARTICLES 10000

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float lifetimeSeconds;
};

class ParticleEmitterComponent : public Component {
public:
    ~ParticleEmitterComponent();
    explicit ParticleEmitterComponent(nlohmann::json data);

    ComponentType GetType() override;
    void HandleEvent(Event* event) override;

    void SetEntity(Entity* _entity) override;

    void Update();
    void Sort(glm::vec3 cameraPosition);

    void AddParticle(glm::vec3 p, glm::vec3 v);
    void Emit(size_t count = 1);

    GLuint GetVao() const;
    size_t GetParticleCount() const;

    bool IsLockedToEntity() const;

    glm::vec2 GetInitialScale() const;
    glm::vec2 GetFinalScale() const;

    Texture* GetTexture() const;
    glm::vec4 GetInitialColor() const;
    glm::vec4 GetFinalColor() const;
    float GetEmissiveness() const;
    
    bool IsSprite() const;
    int GetSpriteColumns() const;
    int GetSpriteRows() const;
    glm::vec2 GetSpriteSize() const;
    float GetAnimationCycles() const;

    float GetLifetimeSeconds() const;
    void SetSpawnRate(float _spawnRate);

    Transform transform;

private:

    void UpdateBuffers();
    void InitializeBuffers();

    size_t emitOnSpawn;
    float emitConeMinAngle;
    float emitConeMaxAngle;

    bool lockedToEntity;

    glm::vec3 acceleration;
    float initialSpeed;

    glm::vec2 initialScale;
    glm::vec2 finalScale;

    Texture* texture;
    glm::vec4 initialColor;
    glm::vec4 finalColor;
    float emissiveness;

    bool isSprite;
    int spriteColumns;
    int spriteRows;
    glm::vec2 spriteSize;
    float animationCycles;

    Time lifetime;
    Time spawnRate;
    Time nextSpawn;

    std::vector<Particle> particles;

    GLuint vao;
    GLuint vbo;
};
