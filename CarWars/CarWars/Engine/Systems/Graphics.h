#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>

#include "System.h"
#include <vector>

#include "glm/glm.hpp"

//#include "Content/Mesh.h"
#include "Content/Texture.h"
#include "../Components/Component.h"
#include "Content/ShaderProgram.h"
#include "../Components/PointLightComponent.h"
#include "../Components/DirectionLightComponent.h"
#include "Content/SpotLight.h"

#define BLUR_LEVEL_COUNT 6

struct Triangle;
class Material;
class MeshComponent;
class Mesh;

struct Camera {
	Camera(glm::mat4 _viewMatrix, glm::mat4 _projectionMatrix, Entity *_guiRoot) : viewMatrix(_viewMatrix), projectionMatrix(_projectionMatrix), guiRoot(_guiRoot) {}
	
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec2 viewportPosition;
	glm::vec2 viewportSize;
	Entity *guiRoot;
};

struct EABs {
    enum { Triangles=0, Count };
};

struct VAOs {
	enum { Geometry=0, Vertices, UVs, Count };
};

struct VBOs {
	enum { Vertices=0, UVs, Normals, Count };
};

struct SSBOs {
	enum { PointLights=0, DirectionLights, SpotLights, Count };
};

struct FBOs {
	enum { Screen=0, ShadowMap, GlowEffect, Count };
};

struct RBOs {
    enum { Depth=0, Count };
};

struct Textures {
	enum { Screen=0, ScreenGlow, ShadowMap, Count };
};

struct Shaders {
	enum { Geometry=0, GUI, ShadowMap, Skybox, Screen, Blur, Copy, NavMesh, Path, Count };
};

class Graphics : public System {
public:
	// Access the singleton instance
	static Graphics& Instance();

	// Constants
	static const std::string GEOMETRY_VERTEX_SHADER;
	static const std::string GEOMETRY_FRAGMENT_SHADER;
	static const std::string SHADOW_MAP_VERTEX_SHADER;
	static const std::string SHADOW_MAP_FRAGMENT_SHADER;
    static const std::string SKYBOX_VERTEX_SHADER;
    static const std::string SKYBOX_FRAGMENT_SHADER;
    static const std::string SCREEN_VERTEX_SHADER;
    static const std::string SCREEN_FRAGMENT_SHADER;
    static const std::string BLUR_VERTEX_SHADER;
    static const std::string BLUR_FRAGMENT_SHADER;
    static const std::string COPY_VERTEX_SHADER;
    static const std::string COPY_FRAGMENT_SHADER;
    static const std::string NAV_VERTEX_SHADER;
    static const std::string NAV_GEOMETRY_SHADER;
    static const std::string NAV_FRAGMENT_SHADER;
    static const std::string PATH_VERTEX_SHADER;
    static const std::string PATH_FRAGMENT_SHADER;
    static const std::string GUI_VERTEX_SHADER;
    static const std::string GUI_FRAGMENT_SHADER;

	static const size_t MAX_CAMERAS;

	static const size_t SCREEN_WIDTH;
	static const size_t SCREEN_HEIGHT;
	static const size_t SHADOW_MAP_SIZE;

	static const glm::vec3 SKY_COLOR;
	static const glm::vec3 AMBIENT_COLOR;

	static const glm::mat4 BIAS_MATRIX;

	// System calls
	bool Initialize(char* windowTitle);
	bool InitializeFullScreen(char* windowTitle); // don't use when debugging
	void Update() override;

    void SceneChanged();
    
    // Debug gui
    void RenderDebugGui();

    // System accessors
	GLFWwindow* GetWindow() const;

	static void WindowSizeCallback(GLFWwindow *window, int width, int height);
	void SetWindowDimensions(size_t width, size_t height);
	void UpdateViewports(std::vector<CameraComponent>& cameraComponents) const;

	glm::vec2 GetWindowSize() const;
	glm::vec2 GetViewportSize() const;

private:
	// No instantiation or copying
	Graphics();
	Graphics(const Graphics&) = delete;
	Graphics& operator= (const Graphics&) = delete;

	void LoadModel(ShaderProgram* shaderProgram, MeshComponent* model);
    void Graphics::LoadModel(ShaderProgram *shaderProgram, glm::mat4 modelMatrix, Material *material, Mesh* mesh, Texture *texture = nullptr, glm::vec2 uvScale = glm::vec2(1.f));

	void LoadCameras(std::vector<CameraComponent>& cameraComponents);
	std::vector<Camera> cameras;
	
	GLFWwindow* window;
	size_t windowWidth;
	size_t windowHeight;
	
    Mesh *skyboxCube;
    Texture *sunTexture;

    GLuint screenVao;
    GLuint screenVbo;
	GLuint ssboIds[SSBOs::Count];
	GLuint fboIds[FBOs::Count];
	GLuint rboIds[RBOs::Count];
	GLuint textureIds[Textures::Count];
	ShaderProgram* shaders[Shaders::Count];

    GLuint blurLevelIds[BLUR_LEVEL_COUNT];
    GLuint blurTempLevelIds[BLUR_LEVEL_COUNT];

    bool renderMeshes;
    bool renderPhysicsColliders;
    bool renderPhysicsBoundingBoxes;
    bool renderNavigationMesh;
    bool renderNavigationPaths;
    float bloomScale;

	void LoadLights(std::vector<PointLightComponent>& _pointLights, std::vector<DirectionLightComponent>& _directionLights, std::vector<SpotLightComponent>& _spotLights);
	void LoadLights(std::vector<PointLight>& pointLights, std::vector<DirectionLight>& directionLights, std::vector<SpotLight>& spotLights);

	void DestroyIds();
	void GenerateIds();

    void InitializeScreenVbo();
	void InitializeScreenVao();

    void InitializeGlowFramebuffer();
    void InitializeScreenFramebuffer();
	void InitializeShadowMapFramebuffer();
	ShaderProgram* LoadShaderProgram(std::string vertexShaderFile, std::string fragmentShaderFile) const;
    ShaderProgram* LoadShaderProgram(std::string vertexShaderFile, std::string fragmentShaderFile, std::string geometryShaderFile) const;
};
