#pragma once

#include "Component.h"
#include <glm/glm.hpp>
#include <json/json.hpp>
#include <GL/glew.h>

class LineComponent : public Component {
public:
    ~LineComponent();
    explicit LineComponent(nlohmann::json data);
    explicit LineComponent(std::vector<glm::vec3> _points={}, glm::vec4 _color=glm::vec4(1.f, 0.f, 0.f, 1.f));

    ComponentType GetType() override;
    void HandleEvent(Event* event) override;

    void RenderDebugGui() override;

    void SetPoints(std::vector<glm::vec3> _points);
    void SetPoint(size_t index, glm::vec3 point);
    void SetPoint0(glm::vec3 point);
    void SetPoint1(glm::vec3 point);
    void SetPoint2(glm::vec3 point);

    std::vector<glm::vec3> GetPoints() const;
    glm::vec3 GetPoint(size_t index) const;
    glm::vec3 GetPoint0() const;
    glm::vec3 GetPoint1() const;
    glm::vec3 GetPoint2() const;

    void SetColor(glm::vec4 _color);
    glm::vec4 GetColor() const;
    size_t GetPointCount() const;

    GLuint GetVaoId() const;
private:
    void InitializeRenderBuffers();
    void UpdateRenderBuffers();

    GLuint vbo;
    GLuint vao;

    glm::vec4 color;
    std::vector<glm::vec3> points;
};
