#include "ParticleEmitterComponent.h"
#include "../Systems/StateManager.h"
#include "../Systems/Content/ContentManager.h"

ParticleEmitterComponent::~ParticleEmitterComponent() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

ParticleEmitterComponent::ParticleEmitterComponent(nlohmann::json data) {
    transform = Transform(data);

    lockedToEntity = ContentManager::GetFromJson<bool>(data["LockedToEntity"], false);

    initialSpeed = ContentManager::GetFromJson<float>(data["InitialSpeed"], 10.f);
    acceleration = ContentManager::JsonToVec3(data["Acceleration"], glm::vec3(0.f, -9.81f, 0.f));

    initialScale = ContentManager::JsonToVec2(data["InitialScale"], glm::vec2(1.f));
    finalScale = ContentManager::JsonToVec2(data["FinalScale"], glm::vec2(1.f));

    texture = ContentManager::GetTexture(data["Texture"]);
    initialColor = ContentManager::GetColorFromJson(data["InitialColor"], glm::vec4(1.f));
    finalColor = ContentManager::GetColorFromJson(data["FinalColor"], glm::vec4(1.f));
    emissiveness = ContentManager::GetFromJson<float>(data["Emissiveness"], 0.f);

    isSprite = ContentManager::GetFromJson<bool>(data["IsSprite"], false);
    spriteSize = ContentManager::JsonToVec2(data["SpriteSize"], glm::vec2(10.f));
    animationCycles = ContentManager::GetFromJson<float>(data["AnimationCycles"], 2.f);

    lifetime = ContentManager::GetFromJson<double>(data["Lifetime"], 3.0);
    spawnRate = ContentManager::GetFromJson<double>(data["SpawnRate"], 0.1);
    nextSpawn = StateManager::globalTime + spawnRate;

    InitializeBuffers();
}

void ParticleEmitterComponent::UpdateBuffers() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * particles.size(), particles.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleEmitterComponent::InitializeBuffers() {
    glGenBuffers(1, &vbo);
    UpdateBuffers();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<const GLvoid*>(0));                  // position
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), reinterpret_cast<const GLvoid*>(sizeof(float) * 6));  // lifetime

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

float UnitRand() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

float UnitRandNegative() {
    return (UnitRand() * 2.f) - 1.f;
}

void ParticleEmitterComponent::Update() {
    const float delta = StateManager::deltaTime.GetSeconds();

    for (auto it = particles.begin(); it != particles.end();) {
        Particle& particle = *it;
        if (particle.lifetimeSeconds > lifetime.GetSeconds()) {
            it = particles.erase(it);
        } else {
            particle.velocity = particle.velocity + delta * acceleration;
            particle.position = particle.position + delta * particle.velocity;
            particle.lifetimeSeconds += delta;
            ++it;
        }
    }

    if (spawnRate > 0.0 && GetParticleCount() < MAX_PARTICLES && StateManager::globalTime >= nextSpawn) {
        nextSpawn = StateManager::globalTime + spawnRate;
        Emit();
    }
}

void ParticleEmitterComponent::AddParticle(glm::vec3 p, glm::vec3 v) {
    Particle particle;
    particle.position = lockedToEntity ? p : p + transform.GetGlobalPosition();
    particle.velocity = v;
    particle.lifetimeSeconds = 0.f;
    particles.push_back(particle);
}

void ParticleEmitterComponent::Emit(size_t count) {
    for (size_t i = 0; i < count; ++i) {
        const glm::vec3 direction = glm::normalize(glm::vec3(UnitRandNegative(), UnitRand()*2.f, UnitRandNegative()));
        AddParticle(glm::vec3(0.f), direction * initialSpeed);
    }
}

bool ParticleEmitterComponent::IsLockedToEntity() const {
    return lockedToEntity;
}

void ParticleEmitterComponent::Sort(glm::vec3 cameraPosition) {
    glm::vec3 localCameraPosition = lockedToEntity ? glm::inverse(transform.GetTransformationMatrix()) * glm::vec4(cameraPosition, 1.f) : cameraPosition;
    std::sort(particles.begin(), particles.end(), [localCameraPosition](const Particle& lhs, const Particle& rhs) -> bool {
        return length(lhs.position - localCameraPosition) > length(rhs.position - localCameraPosition);
    });
    UpdateBuffers();
}

GLuint ParticleEmitterComponent::GetVao() const {
    return vao;
}

size_t ParticleEmitterComponent::GetParticleCount() const {
    return particles.size();
}

glm::vec2 ParticleEmitterComponent::GetInitialScale() const {
    return initialScale;
}

glm::vec2 ParticleEmitterComponent::GetFinalScale() const {
    return finalScale;
}

Texture* ParticleEmitterComponent::GetTexture() const {
    return texture;
}

glm::vec4 ParticleEmitterComponent::GetInitialColor() const {
    return initialColor;
}

glm::vec4 ParticleEmitterComponent::GetFinalColor() const {
    return finalColor;
}

float ParticleEmitterComponent::GetEmissiveness() const {
    return emissiveness;
}

bool ParticleEmitterComponent::IsSprite() const {
    return isSprite;
}

glm::vec2 ParticleEmitterComponent::GetSpriteSize() const {
    return spriteSize;
}

float ParticleEmitterComponent::GetAnimationCycles() const {
    return animationCycles;
}

float ParticleEmitterComponent::GetLifetimeSeconds() const {
    return lifetime.GetSeconds();
}

ComponentType ParticleEmitterComponent::GetType() {
    return ComponentType_ParticleEmitter;
}

void ParticleEmitterComponent::HandleEvent(Event* event) { }

void ParticleEmitterComponent::SetEntity(Entity* _entity) {
    Component::SetEntity(_entity);
    transform.parent = &_entity->transform;
}
