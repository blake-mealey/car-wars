#include "Graphics.h"
#include "IO/Mouse.h"
#include "IO/Keyboard.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "FTGL/ftgl.h"

#include <iostream>
#include "Content/ContentManager.h"
#include <glm/gtx/string_cast.hpp>
#include "../Entities/EntityManager.h"
#include "../Components/GuiComponents/GuiComponent.h"
#include "../Components/CameraComponent.h"
#include "../Components/MeshComponent.h"
#include "../Components/SpotLightComponent.h"
#include <glm/gtc/matrix_transform.inl>
#include "../Components/RigidbodyComponents/RigidbodyComponent.h"
#include "../Components/Colliders/BoxCollider.h"
#include "Game.h"
#include "../Components/AiComponent.h"
#include "../Components/LineComponent.h"

//#define RENDER_DOC_DEBUG_MODE

// Constants
const size_t Graphics::MAX_CAMERAS = 4;

// Shader paths
const std::string Graphics::GEOMETRY_VERTEX_SHADER = "geometry.vert";
const std::string Graphics::GEOMETRY_FRAGMENT_SHADER = "geometry.frag";
const std::string Graphics::SHADOW_MAP_VERTEX_SHADER = "shadowMap.vert";
const std::string Graphics::SHADOW_MAP_FRAGMENT_SHADER = "shadowMap.frag";
const std::string Graphics::SKYBOX_VERTEX_SHADER = "skybox.vert";
const std::string Graphics::SKYBOX_FRAGMENT_SHADER = "skybox.frag";
const std::string Graphics::SCREEN_VERTEX_SHADER = "screen.vert";
const std::string Graphics::SCREEN_FRAGMENT_SHADER = "screen.frag";
const std::string Graphics::BLUR_VERTEX_SHADER = SCREEN_VERTEX_SHADER;
const std::string Graphics::BLUR_FRAGMENT_SHADER = "blur.frag";
const std::string Graphics::COPY_VERTEX_SHADER = SCREEN_VERTEX_SHADER;
const std::string Graphics::COPY_FRAGMENT_SHADER = SCREEN_FRAGMENT_SHADER;
const std::string Graphics::NAV_VERTEX_SHADER = "navMesh.vert";
const std::string Graphics::NAV_FRAGMENT_SHADER = "navMesh.frag";
const std::string Graphics::NAV_GEOMETRY_SHADER = "navMesh.geom";
const std::string Graphics::PATH_VERTEX_SHADER = "path.vert";
const std::string Graphics::PATH_FRAGMENT_SHADER = "path.frag";
const std::string Graphics::GUI_VERTEX_SHADER = SCREEN_VERTEX_SHADER;
const std::string Graphics::GUI_FRAGMENT_SHADER = "gui.frag";

// Initial Screen Dimensions
const size_t Graphics::SCREEN_WIDTH = 1024;
const size_t Graphics::SCREEN_HEIGHT = 768;
const size_t Graphics::SHADOW_MAP_SIZE = 1024;

// Lighting
const glm::vec3 Graphics::SKY_COLOR = glm::vec3(144.f, 195.f, 212.f) / 255.f;
const glm::vec4 Graphics::AMBIENT_COLOR = glm::vec4(0.4f, 0.4f, 0.4f, 1.f);
const glm::mat4 Graphics::BIAS_MATRIX = glm::mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

// Singleton
Graphics::Graphics() : framesPerSecond(0.0), lastTime(0.0), frameCount(0), renderMeshes(true),
                       renderGuis(true), renderPhysicsColliders(false), renderPhysicsBoundingBoxes(false),
                       renderNavigationMesh(false), renderNavigationPaths(false), bloomEnabled(true),
                       bloomScale(0.1f) { }

Graphics &Graphics::Instance() {
	static Graphics instance;
	return instance;
}

void Graphics::WindowSizeCallback(GLFWwindow *window, int width, int height) {
	Instance().SetWindowDimensions(width, height);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Mouse::MouseButtonCallback(window, button, action, mods);
    ImGui_ImplGlfwGL3_MouseButtonCallback(window, button, action, mods);
}

void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Keyboard::KeyboardCallback(window, key, scancode, action, mods);
    ImGui_ImplGlfwGL3_KeyCallback(window, key, scancode, action, mods);
}

bool Graphics::Initialize(char* windowTitle) {
	if (!glfwInit()) {
		std::cerr << "Error Initializing GLFW" << std::endl;
		return false;
	}

	//Create Window
#ifdef RENDER_DOC_DEBUG_MODE
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, windowTitle, NULL, NULL);
	if (window == NULL) {
		std::cerr << "Error Creating Window terminate" << std::endl;
		return false;
	}

	//GLFW Setup
	glfwMakeContextCurrent(window);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glfwSwapInterval(1);	//Swap Buffer Every Frame (Double Buffering)

    // Setup ImGui binding
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui_ImplGlfwGL3_Init(window, false);
    //io.NavFlags |= ImGuiNavFlags_EnableKeyboard;  // Enable Keyboard Controls
    io.NavFlags |= ImGuiNavFlags_EnableGamepad;   // Enable Gamepad Controls
    ImGui::StyleColorsDark();



	// Input callbacks
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetKeyCallback(window, KeyboardCallback);
	//glfwSetJoystickCallback(Controller::ControllerCallback);
    
    glfwSetScrollCallback(window, ImGui_ImplGlfwGL3_ScrollCallback);
    glfwSetCharCallback(window, ImGui_ImplGlfwGL3_CharCallback);

	// Window callbacks
	glfwSetWindowSizeCallback(window, Graphics::WindowSizeCallback);

	// Centers window
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	int xPos = (mode->width - SCREEN_WIDTH) / 2;
	int yPos = (mode->height - SCREEN_HEIGHT) / 2;
	glfwSetWindowPos(window, xPos, yPos);

	//GL Setup
	//Viewport
	glfwGetWindowSize(window, &width, &height); //check resize
	windowWidth = width;
	windowHeight = height;

	// Sets the sky color
	glClearColor(SKY_COLOR.r, SKY_COLOR.g, SKY_COLOR.b, 1.0f);

    glLineWidth(2.f);

	glewExperimental = GL_TRUE;		// TODO: Determine whether this is necessary or not
	glewInit();
	GenerateIds();

    // Z-Buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    //Alpha Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    skyboxCube = ContentManager::GetMesh("Cube.obj");
    sunTexture = ContentManager::GetTexture("SunStrip.png");

	return true;
}

struct IsMoreOpaque {
    bool operator() (const Component* lhs, const Component* rhs) {
        const MeshComponent* lhsMesh = static_cast<const MeshComponent*>(lhs);
        const MeshComponent* rhsMesh = static_cast<const MeshComponent*>(rhs);
        return lhsMesh->GetMaterial()->diffuseColor.a > rhsMesh->GetMaterial()->diffuseColor.a;
    }
};

void Graphics::Update() {
	glfwPollEvents();			// Should this be here or in InputManager?

	// Get components
	const std::vector<Component*> pointLights = EntityManager::GetComponents(ComponentType_PointLight);
	const std::vector<Component*> directionLights = EntityManager::GetComponents(ComponentType_DirectionLight);
	const std::vector<Component*> spotLights = EntityManager::GetComponents(ComponentType_SpotLight);
	std::vector<Component*> meshes = EntityManager::GetComponents(ComponentType_Mesh);
	const std::vector<Component*> lines = EntityManager::GetComponents(ComponentType_Line);
	const std::vector<Component*> cameraComponents = EntityManager::GetComponents(ComponentType_Camera);
	const std::vector<Component*> aiComponents = EntityManager::GetComponents(ComponentType_AI);
	const std::vector<Component*> guiComponents = EntityManager::GetComponents(ComponentType_GUI);
    const std::vector<Component*> rigidbodyComponents = EntityManager::GetComponents({
        ComponentType_RigidDynamic,
        ComponentType_RigidStatic,
        ComponentType_Vehicle,
        ComponentType_PowerUpSpawner
    });

    sort(meshes.begin(), meshes.end(), IsMoreOpaque());

    // Get the active cameras and setup their viewports
    LoadCameras(cameraComponents);


	// -------------------------------------------------------------------------------------------------------------- //
	// RENDER SHADOW MAP
	// -------------------------------------------------------------------------------------------------------------- //

	// Get the shadow caster
	DirectionLightComponent* shadowCaster = nullptr;
	for (Component* component : directionLights) {
		DirectionLightComponent* light = static_cast<DirectionLightComponent*>(component);
		if (!light->enabled) continue;
		if (light->IsShadowCaster()) {
			shadowCaster = light;
			break;
		}
	}

    // Declare depth transformation matrices
	glm::mat4 depthProjectionMatrix;
	glm::mat4 depthViewMatrix;
	if (shadowCaster != nullptr) {
        // Define depth transformation matrices
		depthProjectionMatrix = glm::ortho<float>(-150, 150, -75, 75, -200, 200);
		depthViewMatrix = glm::lookAt(-shadowCaster->GetDirection(), glm::vec3(0), glm::vec3(0, 1, 0));

        // Render to the shadow map framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::ShadowMap]);

        // Clear the buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the shadow program
		ShaderProgram *shadowProgram = shaders[Shaders::ShadowMap];
		glUseProgram(shadowProgram->GetId());

		// Draw the scene
		glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
		for (size_t j = 0; j < meshes.size(); j++) {
			// Get enabled models
			MeshComponent* model = static_cast<MeshComponent*>(meshes[j]);
			if (!model->enabled) continue;

			// Load the depth model view projection matrix into the GPU
			const glm::mat4 depthModelMatrix = model->transform.GetTransformationMatrix();
			const glm::mat4 depthModelViewProjectionMatrix = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
            shadowProgram->LoadUniform(UniformName::DepthModelViewProjectionMatrix, depthModelViewProjectionMatrix);

            // Load the mesh's triangles and vertices into the GPU
            Mesh *mesh = model->GetMesh();
            glBindVertexArray(mesh->vaos[VAOs::Vertices]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eabs[EABs::Triangles]);

			// Render the model
            glDrawElements(GL_TRIANGLES, mesh->triangleCount * 3, GL_UNSIGNED_SHORT, nullptr);
		}
	}

	// -------------------------------------------------------------------------------------------------------------- //
	// RENDER WORLD
	// -------------------------------------------------------------------------------------------------------------- //

	// Render to the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::Screen]);

    // Clear the buffer and enable back-face culling
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the geometry shader program
	ShaderProgram *geometryProgram = shaders[Shaders::Geometry];
	glUseProgram(geometryProgram->GetId());

	// Load shader map into GPU
	if (shadowCaster != nullptr) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureIds[Textures::ShadowMap]);
        geometryProgram->LoadUniform(UniformName::ShadowMap, 1);
        geometryProgram->LoadUniform(UniformName::ShadowsEnabled, true);
    } else {
        geometryProgram->LoadUniform(UniformName::ShadowsEnabled, false);
    }

    // Load bloom scale into GPU
    geometryProgram->LoadUniform(UniformName::BloomScale, bloomScale);

	// Load our lights into the GPU
    geometryProgram->LoadUniform(UniformName::AmbientColor, AMBIENT_COLOR);
	LoadLights(pointLights, directionLights, spotLights);

	// Draw the scene
    if (renderMeshes) {
        for (size_t j = 0; j < meshes.size(); j++) {
            // Get enabled models
            MeshComponent* model = static_cast<MeshComponent*>(meshes[j]);
            if (!model->enabled) continue;

            // Load the model's triangles, vertices, uvs, normals, materials, and textures into the GPU
            LoadModel(geometryProgram, model);

            if (shadowCaster != nullptr) {
                // Load the depth bias model view projection matrix into the GPU
                const glm::mat4 depthModelMatrix = model->transform.GetTransformationMatrix();
                const glm::mat4 depthModelViewProjectionMatrix = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
                const glm::mat4 depthBiasMVP = BIAS_MATRIX*depthModelViewProjectionMatrix;
                geometryProgram->LoadUniform(UniformName::DepthBiasModelViewProjectionMatrix, depthBiasMVP);
            }

            for (Camera camera : cameras) {
                // Setup the viewport for each camera (split-screen)
                glViewport(camera.viewportPosition.x, camera.viewportPosition.y, camera.viewportSize.x, camera.viewportSize.y);

                // Load the model view projection matrix into the GPU
                const glm::mat4 modelViewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix * model->transform.GetTransformationMatrix();
                geometryProgram->LoadUniform(UniformName::ViewMatrix, camera.viewMatrix);
                geometryProgram->LoadUniform(UniformName::ModelViewProjectionMatrix, modelViewProjectionMatrix);

                // Render the model
                glDrawElements(GL_TRIANGLES, model->GetMesh()->triangleCount * 3, GL_UNSIGNED_SHORT, nullptr);
            }
        }

        ShaderProgram *pathProgram = shaders[Shaders::Path];
        glUseProgram(pathProgram->GetId());

        for (size_t j = 0; j < lines.size(); ++j) {
            LineComponent* line = static_cast<LineComponent*>(lines[j]);
            if (!line->enabled) continue;

            pathProgram->LoadUniform(UniformName::DiffuseColor, line->GetColor());
            pathProgram->LoadUniform(UniformName::MaterialEmissiveness, 1.f);

            // Load the vertices to the GPU
            glBindVertexArray(line->GetVaoId());

            for (Camera camera : cameras) {
                // Setup the viewport for each camera (split-screen)
                glViewport(camera.viewportPosition.x, camera.viewportPosition.y, camera.viewportSize.x, camera.viewportSize.y);

                // Load the model view projection matrix into the GPU
                const glm::mat4 viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;
                pathProgram->LoadUniform(UniformName::ViewProjectionMatrix, viewProjectionMatrix);

                // Draw the nav mesh's points
                glDrawArrays(GL_LINE_STRIP, 0, line->GetPointCount());
            }
        }

        glUseProgram(geometryProgram->GetId());
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER PHYSICS COLLIDERS
    // -------------------------------------------------------------------------------------------------------------- //

    if (renderPhysicsColliders || renderPhysicsBoundingBoxes) {
        // Use wireframe polygon mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        for (size_t j = 0; j < rigidbodyComponents.size(); j++) {
            // Get enabled models
            RigidbodyComponent* rigidbody = static_cast<RigidbodyComponent*>(rigidbodyComponents[j]);
            if (!rigidbody->enabled) continue;

            if (renderPhysicsColliders) {
                for (Collider *collider : rigidbody->colliders) {
                    // Check if the collider has a render mesh
                    Mesh *renderMesh = collider->GetRenderMesh();
                    if (!renderMesh) continue;

                    // Get the global transformation matrix of the collider
                    const glm::mat4 modelMatrix = collider->GetGlobalTransform().GetTransformationMatrix();

                    // Load the model's triangles, vertices, uvs, normals, and textures into the GPU
                    LoadModel(
                        geometryProgram,
                        modelMatrix,
                        ContentManager::GetMaterial("PhysicsCollider.json"),
                        renderMesh
                    );

                    if (shadowCaster != nullptr) {
                        // Load the depth bias model view projection matrix into the GPU
                        const glm::mat4 depthModelMatrix = modelMatrix;
                        const glm::mat4 depthModelViewProjectionMatrix = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
                        const glm::mat4 depthBiasMVP = BIAS_MATRIX*depthModelViewProjectionMatrix;
                        geometryProgram->LoadUniform(UniformName::DepthBiasModelViewProjectionMatrix, depthBiasMVP);
                    }

                    for (Camera camera : cameras) {
                        // Setup the viewport for each camera (split-screen)
                        glViewport(camera.viewportPosition.x, camera.viewportPosition.y, camera.viewportSize.x, camera.viewportSize.y);

                        // Load the model view projection matrix into the GPU
                        const glm::mat4 modelViewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix * modelMatrix;
                        geometryProgram->LoadUniform(UniformName::ViewMatrix, camera.viewMatrix);
                        geometryProgram->LoadUniform(UniformName::ModelViewProjectionMatrix, modelViewProjectionMatrix);

                        // Render the model
                        glDrawElements(GL_TRIANGLES, renderMesh->triangleCount * 3, GL_UNSIGNED_SHORT, static_cast<void*>(nullptr));
                    }
                }
            }

            if (renderPhysicsBoundingBoxes) {
                Mesh *cubeMesh = skyboxCube;
                
                PxBounds3 bounds = rigidbody->pxRigid->getWorldBounds(0.5f);
                Transform transform = Transform(nullptr,
                    Transform::FromPx(bounds.getCenter()),
                    Transform::FromPx(bounds.getDimensions()), glm::vec3(0.f), false);
                
                const glm::mat4 modelMatrix = transform.GetTransformationMatrix();
                
                LoadModel(
                    geometryProgram,
                    modelMatrix,
                    ContentManager::GetMaterial("PhysicsBoundingBox.json"),
                    cubeMesh
                );

                if (shadowCaster != nullptr) {
                    // Load the depth bias model view projection matrix into the GPU
                    const glm::mat4 depthModelMatrix = modelMatrix;
                    const glm::mat4 depthModelViewProjectionMatrix = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
                    const glm::mat4 depthBiasMVP = BIAS_MATRIX*depthModelViewProjectionMatrix;
                    geometryProgram->LoadUniform(UniformName::DepthBiasModelViewProjectionMatrix, depthBiasMVP);
                }

                for (Camera camera : cameras) {
                    // Setup the viewport for each camera (split-screen)
                    glViewport(camera.viewportPosition.x, camera.viewportPosition.y, camera.viewportSize.x, camera.viewportSize.y);

                    // Load the model view projection matrix into the GPU
                    const glm::mat4 modelViewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix * modelMatrix;
                    geometryProgram->LoadUniform(UniformName::ViewMatrix, camera.viewMatrix);
                    geometryProgram->LoadUniform(UniformName::ModelViewProjectionMatrix, modelViewProjectionMatrix);

                    // Render the model
                    glDrawElements(GL_TRIANGLES, cubeMesh->triangleCount * 3, GL_UNSIGNED_SHORT, static_cast<void*>(nullptr));
                }
            }
        }

        // Reset polygon mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER NAVIGATION MESH
    // -------------------------------------------------------------------------------------------------------------- //


    if (renderNavigationMesh) {
        Game &game = Game::Instance();
        NavigationMesh *mesh = game.GetNavigationMesh();
        if (mesh) {
            // Use the geometry shader program
            ShaderProgram *navProgram = shaders[Shaders::NavMesh];
            glUseProgram(navProgram->GetId());

            // Load the vertices to the GPU
            glBindVertexArray(mesh->vao);

            // Load the texture to the GPU
            Texture *texture = ContentManager::GetTexture("NavMeshStrip.png");
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture->textureId);
            navProgram->LoadUniform(UniformName::DiffuseTexture, 0);

            for (Camera camera : cameras) {
                // Setup the viewport for each camera (split-screen)
                glViewport(camera.viewportPosition.x, camera.viewportPosition.y, camera.viewportSize.x, camera.viewportSize.y);

                // Load the model view projection matrix into the GPU
                const glm::mat4 viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;
                navProgram->LoadUniform(UniformName::ViewProjectionMatrix, viewProjectionMatrix);

                // Draw the nav mesh's points
                glDrawArrays(GL_POINTS, 0, mesh->GetVertexCount());
            }
        }
    }

    
    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER NAVIGATION PATHS
    // -------------------------------------------------------------------------------------------------------------- //

    if (renderNavigationPaths) {
        for (Component *component : aiComponents) {
            if (!component->enabled) continue;
            AiComponent *ai = static_cast<AiComponent*>(component);

            // Use the geometry shader program
            ShaderProgram *pathProgram = shaders[Shaders::Path];
            glUseProgram(pathProgram->GetId());

            // Load the vertices to the GPU
            glBindVertexArray(ai->pathVao);

            pathProgram->LoadUniform(UniformName::DiffuseColor, glm::vec3(1.f, 0.5f, 0.f));
            pathProgram->LoadUniform(UniformName::MaterialEmissiveness, 1.f);

            for (Camera camera : cameras) {
                // Setup the viewport for each camera (split-screen)
                glViewport(camera.viewportPosition.x, camera.viewportPosition.y, camera.viewportSize.x, camera.viewportSize.y);

                // Load the model view projection matrix into the GPU
                const glm::mat4 viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;
                pathProgram->LoadUniform(UniformName::ViewProjectionMatrix, viewProjectionMatrix);

                // Draw the nav mesh's points
                glDrawArrays(GL_LINE_STRIP, 0, ai->GetPathLength());
            }
        }
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER SKYBOX
    // -------------------------------------------------------------------------------------------------------------- //

    // Render to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::Screen]);

    // Use the skybox shader program
    ShaderProgram *skyboxProgram = shaders[Shaders::Skybox];
    glUseProgram(skyboxProgram->GetId());

    // Load the skybox texture to the GPU
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, ContentManager::GetSkybox());
    skyboxProgram->LoadUniform(UniformName::SkyboxTexture, 0);

    // Load the color adjustment to the GPU
    skyboxProgram->LoadUniform(UniformName::SkyboxColor, glm::vec3(1.5f, 1.2f, 1.2f));

    // Load the skybox geometry into the GPU
    glBindVertexArray(skyboxCube->vaos[VAOs::Vertices]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxCube->eabs[EABs::Triangles]);

    // Load the sun data into the GPU
    if (shadowCaster != nullptr) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sunTexture->textureId);
        skyboxProgram->LoadUniform(UniformName::SunTexture, 1);
        skyboxProgram->LoadUniform(UniformName::SunSizeRadians, glm::radians(10.f));
        skyboxProgram->LoadUniform(UniformName::SunDirection, shadowCaster->GetDirection());
    }

    skyboxProgram->LoadUniform(UniformName::Time, StateManager::globalTime.GetSeconds());

    glDisable(GL_CULL_FACE);
    for (Camera camera : cameras) {
        // Setup the viewport for each camera (split-screen)
        glViewport(camera.viewportPosition.x, camera.viewportPosition.y, camera.viewportSize.x, camera.viewportSize.y);

        // Load the view projection matrix into the GPU
        const glm::mat4 viewProjectionMatrix = camera.projectionMatrix * glm::mat4(glm::mat3(camera.viewMatrix));
        skyboxProgram->LoadUniform(UniformName::ViewProjectionMatrix, viewProjectionMatrix);

        // Render the skybox
        glDrawElements(GL_TRIANGLES, skyboxCube->triangleCount * 3, GL_UNSIGNED_SHORT, nullptr);
    }
    glEnable(GL_CULL_FACE);




    // Load the screen geometry (this will be used by all subsequent draw calls)
    glBindVertexArray(screenVao);




    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER POST-PROCESSING EFFECTS (BLOOM)
    // -------------------------------------------------------------------------------------------------------------- //

    if (bloomEnabled) {
        // Render to the glow framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::GlowEffect]);

        // Use the copy shader program
        ShaderProgram *copyProgram = shaders[Shaders::Copy];
        glUseProgram(copyProgram->GetId());

        // Load the identity matrix as the model matrix to the GPU
        copyProgram->LoadUniform(UniformName::ModelMatrix, glm::mat4(1.f));

        // Load the glow buffer into the GPU
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureIds[Textures::ScreenGlow]);
        copyProgram->LoadUniform(UniformName::ScreenTexture, 0);

        // Copy the glow buffer to each of the level buffers
        for (size_t i = 0; i < BLUR_LEVEL_COUNT; ++i) {
            const float factor = 1.f / pow(2, i);
            glViewport(0, 0, windowWidth * factor, windowHeight * factor);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurLevelIds[i], 0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        // Use the blur shader program
        ShaderProgram *blurProgram = shaders[Shaders::Blur];
        glUseProgram(blurProgram->GetId());

        // Use the first texture location
        glActiveTexture(GL_TEXTURE0);
        blurProgram->LoadUniform(UniformName::ImageTexture, 0);

        // Blur each of the level buffers
        for (size_t i = 0; i < BLUR_LEVEL_COUNT; ++i) {
            // Get the relevant buffers
            const GLuint buffer = blurLevelIds[i];
            const GLuint blurBuffer = blurTempLevelIds[i];

            // Set the right viewport
            const float factor = 1.f / pow(2, i);
            glViewport(0, 0, windowWidth * factor, windowHeight * factor);

            // Calculate the blur offsets
            const float xOffset = 1.2f / (windowWidth * factor);
            const float yOffset = 1.2f / (windowHeight * factor);

            // Blur on the x-axis
            glBindTexture(GL_TEXTURE_2D, buffer);
            blurProgram->LoadUniform(UniformName::BlurOffset, glm::vec2(xOffset, 0.f));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurBuffer, 0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            // Blur on the y-axis
            glBindTexture(GL_TEXTURE_2D, blurBuffer);
            blurProgram->LoadUniform(UniformName::BlurOffset, glm::vec2(0.f, yOffset));
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer, 0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // COMPOSITE EFFECTS AND RENDER TO SCREEN
    // -------------------------------------------------------------------------------------------------------------- //

    // Render to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Clear the colour and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the screen shader program
    ShaderProgram *screenProgram = shaders[Shaders::Screen];
    glUseProgram(screenProgram->GetId());

    // Send the screen to the GPU
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureIds[Textures::Screen]);
    screenProgram->LoadUniform(UniformName::ScreenTexture, 0);

	// Use the identity model matrix
	screenProgram->LoadUniform(UniformName::ModelMatrix, glm::mat4(1.f));

    // Render it
    glViewport(0, 0, windowWidth, windowHeight);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (bloomEnabled) {
        // Disable the depth mask and enable additive blending
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_ONE, GL_ONE);

        // Render each blur level
        for (size_t i = 0; i < BLUR_LEVEL_COUNT; ++i) {
            glBindTexture(GL_TEXTURE_2D, blurLevelIds[i]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        // Disable blending and re-enable the depth mask
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_TRUE);
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER GAME GUI
    // -------------------------------------------------------------------------------------------------------------- //

    // Get the GUI program
    ShaderProgram *guiProgram = shaders[Shaders::GUI];

    if (renderGuis) {
        glDisable(GL_DEPTH_TEST);

        for (Camera camera : cameras) {
            // Setup the viewport for each camera (split-screen)
            glViewport(camera.viewportPosition.x, camera.viewportPosition.y, camera.viewportSize.x, camera.viewportSize.y);

            for (Component *component : guiComponents) {
                if (!component->enabled) continue;
                GuiComponent *gui = static_cast<GuiComponent*>(component);

                // Get a GUI for this camera
                Entity *guiRoot = gui->GetGuiRoot();
                if (!guiRoot || guiRoot != camera.guiRoot) continue;

                if (gui->IsMaskEnabled() || gui->IsClipEnabled()) {
                    // Enable stencil test and writing to stencil buffer and clear the stencil buffer
                    glEnable(GL_STENCIL_TEST);
                    glStencilMask(0xFF);
                    glClear(GL_STENCIL_BUFFER_BIT);

                    // Write a 1 at every pixel we write to
                    glStencilFunc(GL_ALWAYS, 1, 0xFF);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

                    // Only write to stencil buffer
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    glDepthMask(GL_FALSE);

                    // Don't write pixels when they are transparent
                    glEnable(GL_ALPHA_TEST);
                    glAlphaFunc(GL_GREATER, 0.f);

                    // Use the GUI program
                    glUseProgram(guiProgram->GetId());

                    // Set the viewport
                    glViewport(0, 0, windowWidth, windowHeight);

                    // Load the color to the GPU
                    guiProgram->LoadUniform(UniformName::DiffuseColor, glm::vec4(1.f));
                    guiProgram->LoadUniform(UniformName::IsSprite, false);

                    if (gui->IsClipEnabled()) {
                        // Send the UV scale and texture color to the GPU
                        guiProgram->LoadUniform(UniformName::DiffuseTextureEnabled, false);

                        // Send the transform to the GPU
                        const glm::mat4 modelMatrix = gui->transform.GetGuiTransformationMatrix(
                            gui->GetAnchorPoint(),
                            gui->GetScaledPosition(),
                            gui->GetScaledScale(),
                            camera.viewportPosition,
                            camera.viewportSize,
                            glm::vec2(windowWidth, windowHeight));
                        guiProgram->LoadUniform(UniformName::ModelMatrix, modelMatrix);

                        // Render it
                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    }
                    else if (gui->IsMaskEnabled()) {
                        // Send the UV scale and texture color to the GPU
                        Texture* maskTexture = gui->GetMaskTexture();
                        if (maskTexture) {
                            guiProgram->LoadUniform(UniformName::DiffuseTexture, maskTexture);
                            guiProgram->LoadUniform(UniformName::DiffuseTextureEnabled, true);
                        } else {
                            guiProgram->LoadUniform(UniformName::DiffuseTextureEnabled, false);
                        }

                        // Send the transform to the GPU
                        Transform mask = Transform(
                            gui->transform.GetLocalPosition() + gui->GetMask().GetLocalPosition(),
                            gui->GetMask().GetLocalScale(),
                            gui->GetMask().GetLocalRotation());
                        const glm::mat4 modelMatrix = mask.GetGuiTransformationMatrix(
                            gui->GetAnchorPoint(),
                            gui->GetScaledPosition(),
                            gui->GetScaledScale(),
                            camera.viewportPosition,
                            camera.viewportSize,
                            glm::vec2(windowWidth, windowHeight));
                        guiProgram->LoadUniform(UniformName::ModelMatrix, modelMatrix);

                        // Render it
                        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    }

                    // Only write with correct stencil value
                    if (gui->IsMaskEnabled() && gui->IsMaskInverted()) {
                        glStencilFunc(GL_EQUAL, 0, 0xFF);
                    } else {
                        glStencilFunc(GL_EQUAL, 1, 0xFF);
                    }

                    // No more alpha testing
                    glDisable(GL_ALPHA_TEST);

                    // Write to all buffers except stencil
                    glStencilMask(0x00);
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glDepthMask(GL_TRUE);
                }

                // RENDER THE FRAME
                Texture *frameTexture = gui->GetTexture();
                if (frameTexture) {
                    // Use the GUI program
                    glUseProgram(guiProgram->GetId());

                    // Send the screen to the GPU
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, frameTexture->textureId);
                    guiProgram->LoadUniform(UniformName::DiffuseTexture, 0);

                    // Send the UV scale and texture color to the GPU
                    guiProgram->LoadUniform(UniformName::UvScale, gui->GetUvScale());
                    guiProgram->LoadUniform(UniformName::DiffuseColor, gui->GetTextureColor());
                    guiProgram->LoadUniform(UniformName::DiffuseTextureEnabled, true);
                    guiProgram->LoadUniform(UniformName::MaterialEmissiveness, gui->GetEmissiveness());

                    guiProgram->LoadUniform(UniformName::IsSprite, gui->IsSprite());
                    if (gui->IsSprite()) {
                        guiProgram->LoadUniform(UniformName::TextureSize, glm::vec2(frameTexture->width, frameTexture->height));
                        guiProgram->LoadUniform(UniformName::SpriteSize, gui->GetSpriteSize());
                        guiProgram->LoadUniform(UniformName::SpriteOffset, gui->GetSpriteOffset());
                    }

                    // Send the transform to the GPU
                    const glm::mat4 modelMatrix = gui->transform.GetGuiTransformationMatrix(
                        gui->GetAnchorPoint(),
                        gui->GetScaledPosition(),
                        gui->GetScaledScale(),
                        camera.viewportPosition,
                        camera.viewportSize,
                        glm::vec2(windowWidth, windowHeight));
                    guiProgram->LoadUniform(UniformName::ModelMatrix, modelMatrix);

                    // Render it
                    glViewport(0, 0, windowWidth, windowHeight);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                }

                // RENDER THE FONT

                // Unbind shader program
                glUseProgram(0);

                // Set stuff
                glPushAttrib(GL_ALL_ATTRIB_BITS);

                // Set the color
                const glm::vec4 color = gui->GetFontColor();
                glPixelTransferf(GL_RED_BIAS, color.r - 1.f);
                glPixelTransferf(GL_GREEN_BIAS, color.g - 1.f);
                glPixelTransferf(GL_BLUE_BIAS, color.b - 1.f);
                glPixelTransferf(GL_ALPHA_BIAS, color.a - 1.f);

                // Render the text
                FTFont *font = gui->GetFont();

                // Font dimensions
                const glm::vec2 fontDims = gui->GetFontDimensions();

                // Get the scale and position of the GUI
                const glm::vec2 anchorPoint = gui->GetAnchorPoint();
                const glm::vec3 scale = gui->transform.GetGlobalScale() +
                    glm::vec3(camera.viewportSize * gui->GetScaledScale(), 0.f);
                const glm::vec3 position = gui->transform.GetGlobalPosition() +
                    glm::vec3(camera.viewportSize * gui->GetScaledPosition(), 0.f);

                const glm::vec3 fontPosition = position;
                glm::vec2 fontScreenPosition = camera.viewportPosition +
                    glm::vec2(fontPosition.x, camera.viewportSize.y - fontPosition.y - fontDims.y);

                glm::vec2 alignmentXOffset = glm::vec2(scale.x - fontDims.x, 0.f);
                switch (gui->GetTextXAlignment()) {
                case TextXAlignment::Left:
                    alignmentXOffset *= 0.f;
                    break;
                case TextXAlignment::Centre:
                    alignmentXOffset *= 0.5f;
                    break;
                case TextXAlignment::Right:
                    alignmentXOffset *= 1.f;
                    break;
                }

                glm::vec2 alignmentYOffset = -glm::vec2(0.f, scale.y - fontDims.y);
                switch (gui->GetTextYAlignment()) {
                case TextYAlignment::Top:
                    alignmentYOffset *= 0.f;
                    break;
                case TextYAlignment::Centre:
                    alignmentYOffset *= 0.5f;
                    break;
                case TextYAlignment::Bottom:
                    alignmentYOffset *= 1.f;
                    break;
                }

                fontScreenPosition += alignmentXOffset + alignmentYOffset - glm::vec2(scale.x, -scale.y) * anchorPoint;

                font->Render(gui->GetText().c_str(), -1, FTPoint(fontScreenPosition.x, fontScreenPosition.y));

                // Reset stuff
                glPopAttrib();

                if (gui->IsMaskEnabled() || gui->IsClipEnabled()) {
                    glDisable(GL_STENCIL_TEST);
                }
            }
        }

        glDisable(GL_STENCIL_TEST);
        glEnable(GL_DEPTH_TEST);
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER DEBUG GUI
    // -------------------------------------------------------------------------------------------------------------- //

    RenderDebugGui();

	//Swap Buffers to Display New Frame
	glfwSwapBuffers(window);
}

void Graphics::SceneChanged() {
    UpdateViewports(EntityManager::GetComponents(ComponentType_Camera));
}

void Graphics::RenderDebugGui() {
    ImGui_ImplGlfwGL3_NewFrame();

    static bool showSceneGraph = true;
    if (showSceneGraph) {
        ImGui::Begin("Scene Graph", &showSceneGraph);
        ImGui::PushItemWidth(-100);
        EntityManager::GetRoot()->RenderDebugGui();
        ImGui::End();
    }

    static bool showGraphicsMenu = true;
    if (showGraphicsMenu) {
        ImGui::Begin("Graphics", &showGraphicsMenu);
        ImGui::PushItemWidth(-100);

        frameCount++;
        if (StateManager::globalTime - lastTime >= 1.0) {
            framesPerSecond = double(frameCount);
            frameCount = 0;
            lastTime = StateManager::globalTime;
        }

        ImGui::LabelText("FPS", "%.0f", framesPerSecond);
        ImGui::LabelText("ms/frame", "%.3f", 1000.0 / framesPerSecond);

        ImGui::LabelText("Entity Count", "%d", EntityManager::GetEntityCount());
        ImGui::LabelText("Component Count", "%d", EntityManager::GetComponentCount());
        ImGui::LabelText("Mesh Count", "%d", EntityManager::GetComponentCount(ComponentType_Mesh));
        ImGui::LabelText("GUI Count", "%d", EntityManager::GetComponentCount(ComponentType_GUI));
        ImGui::LabelText("Vehicle Count", "%d", EntityManager::GetComponentCount(ComponentType_Vehicle));
        ImGui::LabelText("Rigid Dynamic Count", "%d", EntityManager::GetComponentCount(ComponentType_RigidDynamic));
        ImGui::LabelText("Rigid Static Count", "%d", EntityManager::GetComponentCount(ComponentType_RigidStatic));

        ImGui::Checkbox("Render Meshes", &renderMeshes);
        ImGui::Checkbox("Render GUIs", &renderGuis);
        ImGui::Checkbox("Render Colliders", &renderPhysicsColliders);
        ImGui::Checkbox("Render Bounding Boxes", &renderPhysicsBoundingBoxes);
        ImGui::Checkbox("Render Nav Mesh", &renderNavigationMesh);
        ImGui::Checkbox("Render Nav Paths", &renderNavigationPaths);
        ImGui::Checkbox("Bloom Enabled", &bloomEnabled);
        ImGui::DragFloat("Bloom Scale", &bloomScale, 0.01f);

        ImGui::End();
    }

    static bool show_demo_window = false;
    if (show_demo_window) {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // Render debug GUI
    glViewport(0, 0, windowWidth, windowHeight);
    ImGui::Render();
}

void Graphics::LoadModel(ShaderProgram *shaderProgram, MeshComponent *model) {
	if (!model->enabled) return;

    LoadModel(
        shaderProgram,
        model->transform.GetTransformationMatrix(),
        model->GetMaterial(),
        model->GetMesh(),
        model->GetTexture(),
        model->GetUvScale()
    );
}

void Graphics::LoadModel(ShaderProgram *shaderProgram, glm::mat4 modelMatrix, Material *material, Mesh* mesh, Texture *texture, glm::vec2 uvScale) {
    // Load the model matrix into the GPU
    shaderProgram->LoadUniform(UniformName::ModelMatrix, modelMatrix);

    // Load the material data into the GPU
    shaderProgram->LoadUniform(UniformName::MaterialDiffuseColor, material->diffuseColor);
    shaderProgram->LoadUniform(UniformName::MaterialSpecularColor, material->specularColor);
    shaderProgram->LoadUniform(UniformName::MaterialSpecularity, material->specularity);
    shaderProgram->LoadUniform(UniformName::MaterialEmissiveness, material->emissiveness);

    // Load the mesh into the GPU
    glBindVertexArray(mesh->vaos[VAOs::Geometry]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eabs[EABs::Triangles]);

    // Load the texture into the GPU
    if (texture != nullptr) {
        shaderProgram->LoadUniform(UniformName::DiffuseTextureEnabled, true);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->textureId);
        shaderProgram->LoadUniform(UniformName::DiffuseTexture, 0);

        shaderProgram->LoadUniform(UniformName::UvScale, uvScale);
    } else {
        shaderProgram->LoadUniform(UniformName::DiffuseTextureEnabled, false);
    }
}

void Graphics::LoadCameras(std::vector<Component*> cameraComponents) {
	// Find up to MAX_CAMERAS enabled cameras
	const size_t lastCount = cameras.size();
	cameras.clear();
	for (Component *component: cameraComponents) {
		if (component->enabled) {
			CameraComponent *camera = static_cast<CameraComponent*>(component);
			cameras.push_back(Camera(camera->GetViewMatrix(), camera->GetProjectionMatrix(), camera->GetGuiRoot()));
			if (cameras.size() == MAX_CAMERAS) break;
		}
	}
	const size_t count = cameras.size();

	// Update the camera's viewports based on the number of cameras
	const glm::vec2 windowSize = glm::vec2(windowWidth, windowHeight);
	for (size_t i = 0; i < count; ++i) {
        const glm::vec2 viewportSize = GetViewportSize(i);

        glm::vec2 scale = glm::vec2(0.f);
        if (count == 2) {
            scale = glm::vec2(0.f, 0.5f * (1 - i));
        } else if (count == 3) {
            scale = i == 0 ? glm::vec2(0.f, 0.5f) : glm::vec2(0.5f * (i % 2), 0.f);
        } else if (count == 4) {
            scale = glm::vec2(0.5f * (i % 2), i < 2 ? 0.5f : 0.f);
        }

		cameras[i].viewportPosition = scale * windowSize;
		cameras[i].viewportSize = viewportSize;
	}

	// If camera count changed, update aspect ratios for next frame
	if (cameras.size() != lastCount) {
		UpdateViewports(cameraComponents);
	}
}

GLFWwindow* Graphics::GetWindow() const {
	return window;
}

void Graphics::SetWindowDimensions(size_t width, size_t height) {
	windowWidth = width;
	windowHeight = height;

    glBindTexture(GL_TEXTURE_2D, textureIds[Textures::Screen]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, textureIds[Textures::ScreenGlow]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindRenderbuffer(GL_RENDERBUFFER, rboIds[RBOs::Depth]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, windowWidth, windowHeight);

    for (size_t i = 0; i < BLUR_LEVEL_COUNT; ++i) {
        const float factor = 1.f / pow(2, i);

        glBindTexture(GL_TEXTURE_2D, blurLevelIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth * factor, windowHeight * factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, blurTempLevelIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth * factor, windowHeight * factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

	UpdateViewports(EntityManager::GetComponents(ComponentType_Camera));
}

void Graphics::UpdateViewports(std::vector<Component*> cameraComponents) const {
    int count = 0;
	for (Component *component : cameraComponents) {
		if (component->enabled) {
            const glm::vec2 viewportSize = GetViewportSize(count++);
            const float aspectRatio = viewportSize.x / viewportSize.y;
			CameraComponent *camera = static_cast<CameraComponent*>(component);
			camera->SetAspectRatio(aspectRatio);
		}
	}
}

glm::vec2 Graphics::GetWindowSize() const {
	return glm::vec2(windowWidth, windowHeight);
}

glm::vec2 Graphics::GetViewportSize(int index) const {
	const glm::vec2 windowSize = glm::vec2(windowWidth, windowHeight);

    glm::vec2 scale = glm::vec2(1.f);
    if (cameras.size() == 2) {
        scale = glm::vec2(1.f, 0.5f);
    } else if (cameras.size() == 3) {
        scale = index == 0 ? glm::vec2(1.f, 0.5f) : glm::vec2(0.5f, 0.5f);
    } else if (cameras.size() == 4) {
        scale = glm::vec2(0.5f, 0.5f);
    }

    return windowSize * scale;
}

void Graphics::LoadLights(std::vector<Component*> _pointLights,
	std::vector<Component*> _directionLights, std::vector<Component*> _spotLights) {

	// Get the point light data which can be directly passed to the shader	
	std::vector<PointLight> pointLights;
	for (Component *component : _pointLights) {
		if (component->enabled)
			pointLights.push_back(static_cast<PointLightComponent*>(component)->GetData());
	}

	// Get the direction light data which can be directly passed to the shader
	std::vector<DirectionLight> directionLights;
	for (Component *component : _directionLights) {
		if (component->enabled)
			directionLights.push_back(static_cast<DirectionLightComponent*>(component)->GetData());
	}

	// Get the spot light data which can be directly passed to the shader
	std::vector<SpotLight> spotLights;
	for (Component *component : _spotLights) {
		if (component->enabled)
			spotLights.push_back(static_cast<SpotLightComponent*>(component)->GetData());
	}

	LoadLights(pointLights, directionLights, spotLights);
}

void Graphics::LoadLights(std::vector<PointLight> pointLights, std::vector<DirectionLight> directionLights, std::vector<SpotLight> spotLights) {
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIds[SSBOs::PointLights]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, pointLights.size() * sizeof(PointLight), pointLights.data(), GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIds[SSBOs::DirectionLights]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, directionLights.size() * sizeof(DirectionLight), directionLights.data(), GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboIds[SSBOs::SpotLights]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, spotLights.size() * sizeof(SpotLight), spotLights.data(), GL_DYNAMIC_COPY);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Graphics::DestroyIds() {
    glDeleteVertexArrays(1, &screenVao);
    glDeleteBuffers(1, &screenVbo);
    glDeleteBuffers(SSBOs::Count, ssboIds);
    glDeleteFramebuffers(FBOs::Count, fboIds);
    glDeleteRenderbuffers(RBOs::Count, rboIds);
    glDeleteTextures(Textures::Count, textureIds);
    glDeleteTextures(BLUR_LEVEL_COUNT, blurLevelIds);
    glDeleteTextures(BLUR_LEVEL_COUNT, blurTempLevelIds);
    for (int i = 0; i < Shaders::Count; i++) {
        glDeleteProgram(shaders[i]->GetId());
    }
}

void Graphics::GenerateIds() {
    glGenVertexArrays(1, &screenVao);
    glGenBuffers(1, &screenVbo);
	glGenBuffers(SSBOs::Count, ssboIds);
	for (size_t i = 0; i < SSBOs::Count; i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, ssboIds[i]);
	}
	glGenFramebuffers(FBOs::Count, fboIds);
	glGenRenderbuffers(RBOs::Count, rboIds);
    glGenTextures(Textures::Count, textureIds);
    glGenTextures(BLUR_LEVEL_COUNT, blurLevelIds);
    glGenTextures(BLUR_LEVEL_COUNT, blurTempLevelIds);
	
    shaders[Shaders::Geometry] = LoadShaderProgram(GEOMETRY_VERTEX_SHADER, GEOMETRY_FRAGMENT_SHADER);
    shaders[Shaders::GUI] = LoadShaderProgram(GUI_VERTEX_SHADER, GUI_FRAGMENT_SHADER);
	shaders[Shaders::ShadowMap] = LoadShaderProgram(SHADOW_MAP_VERTEX_SHADER, SHADOW_MAP_FRAGMENT_SHADER);
	shaders[Shaders::Skybox] = LoadShaderProgram(SKYBOX_VERTEX_SHADER, SKYBOX_FRAGMENT_SHADER);
	shaders[Shaders::Screen] = LoadShaderProgram(SCREEN_VERTEX_SHADER, SCREEN_FRAGMENT_SHADER);
	shaders[Shaders::Blur] = LoadShaderProgram(BLUR_VERTEX_SHADER, BLUR_FRAGMENT_SHADER);
	shaders[Shaders::Copy] = LoadShaderProgram(COPY_VERTEX_SHADER, COPY_FRAGMENT_SHADER);
	shaders[Shaders::NavMesh] = LoadShaderProgram(NAV_VERTEX_SHADER, NAV_FRAGMENT_SHADER, NAV_GEOMETRY_SHADER);
	shaders[Shaders::Path] = LoadShaderProgram(PATH_VERTEX_SHADER, PATH_FRAGMENT_SHADER);

    InitializeScreenVao();
    InitializeScreenVbo();

    InitializeGlowFramebuffer();
    InitializeScreenFramebuffer();
	InitializeShadowMapFramebuffer();
}

void Graphics::InitializeScreenVbo() {
    const glm::vec2 verts[4] = {
        glm::vec2(-1, -1),
        glm::vec2(1, -1),
        glm::vec2(-1, 1),
        glm::vec2(1, 1)
    };

    glBindBuffer(GL_ARRAY_BUFFER, screenVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 4, verts, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Graphics::InitializeScreenVao() {
    glBindVertexArray(screenVao);

    // Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, screenVbo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Graphics::InitializeGlowFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::GlowEffect]);

    GLenum fboBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, fboBuffers);

    for (size_t i = 0; i < BLUR_LEVEL_COUNT; ++i) {
        const float factor = 1.f / pow(2, i);
        
        glBindTexture(GL_TEXTURE_2D, blurLevelIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH * factor, SCREEN_HEIGHT * factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, blurTempLevelIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH * factor, SCREEN_HEIGHT * factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, blurLevelIds[i], 0);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::InitializeScreenFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::Screen]);

    GLenum fboBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, fboBuffers);

    // Normal colour buffer
    glBindTexture(GL_TEXTURE_2D, textureIds[Textures::Screen]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureIds[Textures::Screen], 0);
    
    // Highlights colour buffer
    glBindTexture(GL_TEXTURE_2D, textureIds[Textures::ScreenGlow]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textureIds[Textures::ScreenGlow], 0);

    // Depth buffer
    glBindRenderbuffer(GL_RENDERBUFFER, rboIds[RBOs::Depth]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboIds[RBOs::Depth]);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR: Screen framebuffer incomplete!" << std::endl;

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Graphics::InitializeShadowMapFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::ShadowMap]);

	// Add depth texture
	glBindTexture(GL_TEXTURE_2D, textureIds[Textures::ShadowMap]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, textureIds[Textures::ShadowMap], 0);

	// No draw buffers
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "ERROR: Shadow map framebuffer incomplete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShaderProgram* Graphics::LoadShaderProgram(std::string vertexShaderFile, std::string fragmentShaderFile) const {
	// Load and compile shaders from source
	const GLuint vertexId = ContentManager::LoadShader(vertexShaderFile, GL_VERTEX_SHADER);
	const GLuint fragmentId = ContentManager::LoadShader(fragmentShaderFile, GL_FRAGMENT_SHADER);

	// Link the shaders into a program
	const GLuint programId = glCreateProgram();
	glAttachShader(programId, vertexId);
	glAttachShader(programId, fragmentId);
	glLinkProgram(programId);

	// Check link status and print errors
	GLint status;
	glGetProgramiv(programId, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);
		std::string info(length, ' ');
		glGetProgramInfoLog(programId, info.length(), &length, &info[0]);
		std::cerr << "ERROR linking shader program:" << std::endl << info << std::endl;
	}

	glDeleteShader(vertexId);
	glDeleteShader(fragmentId);

	// Return the program's ID
	return new ShaderProgram(programId);
}

// TODO: Fix this uglyness lol
ShaderProgram* Graphics::LoadShaderProgram(std::string vertexShaderFile, std::string fragmentShaderFile, std::string geometryShaderFile) const {
    // Load and compile shaders from source
    const GLuint vertexId = ContentManager::LoadShader(vertexShaderFile, GL_VERTEX_SHADER);
    const GLuint fragmentId = ContentManager::LoadShader(fragmentShaderFile, GL_FRAGMENT_SHADER);
    const GLuint geometryId = ContentManager::LoadShader(geometryShaderFile, GL_GEOMETRY_SHADER);

    // Link the shaders into a program
    const GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexId);
    glAttachShader(programId, fragmentId);
    glAttachShader(programId, geometryId);
    glLinkProgram(programId);

    // Check link status and print errors
    GLint status;
    glGetProgramiv(programId, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &length);
        std::string info(length, ' ');
        glGetProgramInfoLog(programId, info.length(), &length, &info[0]);
        std::cerr << "ERROR linking shader program:" << std::endl << info << std::endl;
    }

    glDeleteShader(vertexId);
    glDeleteShader(fragmentId);
    glDeleteShader(geometryId);

    // Return the program's ID
    return new ShaderProgram(programId);
}

