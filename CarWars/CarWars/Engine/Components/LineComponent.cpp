#include "LineComponent.h"
#include "../Systems/Content/ContentManager.h"

using namespace nlohmann;

LineComponent::~LineComponent() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

LineComponent::LineComponent(json data) {
    color = ContentManager::JsonToVec3(data["Color"], glm::vec3(1.f, 0.f, 0.f));
    if (data["Points"].is_array()) {
        for (const json point : data["Points"]) {
            points.push_back(ContentManager::JsonToVec3(point));
        }
    }

    InitializeRenderBuffers();
}

LineComponent::LineComponent(std::vector<glm::vec3> _points, glm::vec3 _color) : points(_points), color(_color) {
    InitializeRenderBuffers();
}

ComponentType LineComponent::GetType() {
    return ComponentType_Line;
}

void LineComponent::HandleEvent(Event* event) { }

void LineComponent::RenderDebugGui() {
    Component::RenderDebugGui();
}

void LineComponent::SetPoints(std::vector<glm::vec3> _points) {
    points = _points;
    UpdateRenderBuffers();
}

void LineComponent::SetPoint(size_t index, glm::vec3 point) {
    points[index] = point;
    UpdateRenderBuffers();
}

void LineComponent::SetPoint0(glm::vec3 point) {
    SetPoint(0, point);
}

void LineComponent::SetPoint1(glm::vec3 point) {
    SetPoint(1, point);
}

void LineComponent::SetPoint2(glm::vec3 point) {
    SetPoint(2, point);
}

std::vector<glm::vec3> LineComponent::GetPoints() const {
    return points;
}

glm::vec3 LineComponent::GetPoint(size_t index) const {
    return points[index];
}

glm::vec3 LineComponent::GetPoint0() const {
    return GetPoint(0);
}

glm::vec3 LineComponent::GetPoint1() const {
    return GetPoint(1);
}

glm::vec3 LineComponent::GetPoint2() const {
    return GetPoint(2);
}

void LineComponent::SetColor(glm::vec3 _color) {
    color = _color;
}

glm::vec3 LineComponent::GetColor() const {
    return color;
}

size_t LineComponent::GetPointCount() const {
    return points.size();
}

GLuint LineComponent::GetVaoId() const {
    return vao;
}

void LineComponent::InitializeRenderBuffers() {
    glGenBuffers(1, &vbo);
    UpdateRenderBuffers();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));        // position

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void LineComponent::UpdateRenderBuffers() {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * points.size(), points.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
