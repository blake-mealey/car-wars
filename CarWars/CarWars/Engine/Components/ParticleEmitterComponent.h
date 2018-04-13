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
    void SetEmitScale(glm::vec3 _emitScale);

    GLuint GetVao() const;
    size_t GetParticleCount() const;

    void SetEmitCount(size_t _emitCount);

    bool IsLockedToEntity() const;
    glm::mat4 GetModelMatrix();

    float GetInitialSpeed() const;

    glm::vec2 GetInitialScale() const;
    void SetInitialScale(glm::vec2 scale);
    glm::vec2 GetFinalScale() const;
    void SetFinalScale(glm::vec2 scale);

    Texture* GetTexture() const;
    glm::vec4 GetInitialColor() const;
    glm::vec4 GetFinalColor() const;
    float GetEmissiveness() const;
    
    bool IsSprite() const;
    int GetSpriteColumns() const;
    int GetSpriteRows() const;
    glm::vec2 GetSpriteSize() const;
    float GetAnimationCycles() const;
    void SetAnimationCycles(float _animationCycles);

    float GetLifetimeSeconds() const;
    void SetLifetime(Time _lifetime);
    void SetSpawnRate(float _spawnRate);
    void SetDirections(glm::vec3 direction);

    Transform transform;

private:

    void UpdateBuffers();
    void InitializeBuffers();

    size_t emitCount;
    size_t emitOnSpawn;
    float emitConeMinAngle;
    float emitConeMaxAngle;
    glm::vec3 emitScale;

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
