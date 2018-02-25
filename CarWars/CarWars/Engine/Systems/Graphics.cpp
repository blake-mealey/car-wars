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
#include "../Components/CameraComponent.h"
#include "../Components/MeshComponent.h"
#include "../Components/SpotLightComponent.h"
#include <glm/gtc/matrix_transform.inl>
#include "../Components/RigidbodyComponents/RigidbodyComponent.h"
#include "../Components/Colliders/BoxCollider.h"

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

// Initial Screen Dimensions
const size_t Graphics::SCREEN_WIDTH = 1024;
const size_t Graphics::SCREEN_HEIGHT = 768;
const size_t Graphics::SHADOW_MAP_SIZE = 1024;

// Lighting
const glm::vec3 Graphics::SKY_COLOR = glm::vec3(144.f, 195.f, 212.f) / 255.f;
const glm::vec3 Graphics::AMBIENT_COLOR = glm::vec3(0.4f);
const glm::mat4 Graphics::BIAS_MATRIX = glm::mat4(
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
);

// Singleton
Graphics::Graphics() : renderPhysicsColliders(false), bloomScale(0.1f) { }
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
		std::cout << "Error Initializing GLFW" << std::endl;
		return false;
	}

	//Create Window
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, windowTitle, NULL, NULL);
	if (window == NULL) {
		std::cout << "Error Creating Window terminate" << std::endl;
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

	//Alpha Blending
	/*glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

	// Z-Buffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Sets the sky color
	glClearColor(SKY_COLOR.r, SKY_COLOR.g, SKY_COLOR.b, 1.0f);

	glewExperimental = GL_TRUE;		// TODO: Determine whether this is necessary or not
	glewInit();
	GenerateIds();

    skyboxCube = ContentManager::GetMesh("Cube.obj");
    sunTexture = ContentManager::GetTexture("SunStrip.png");

	return true;
}

//don't use in when debugging
bool Graphics::InitializeFullScreen(char* windowTitle) {
	if (!glfwInit()) {
		std::cout << "Error Initializing GLFW" << std::endl;
		return false;
	}

	//Create Window
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	window = glfwCreateWindow(mode->width, mode->height, windowTitle, monitor, NULL);
	if (window == NULL) {
		std::cout << "Error Creating Window terminate" << std::endl;
		return false;
	}

	//GLFW Setup
	glfwMakeContextCurrent(window);
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glfwSwapInterval(1);	//Swap Buffer Every Frame (Double Buffering)

							// Input callbacks
	glfwSetMouseButtonCallback(window, Mouse::MouseButtonCallback);
	glfwSetKeyCallback(window, Keyboard::KeyboardCallback);
	//glfwSetJoystickCallback(Controller::ControllerCallback);

	// Window callbacks
	glfwSetWindowSizeCallback(window, Graphics::WindowSizeCallback);

	int xPos = (mode->width - SCREEN_WIDTH) / 2;
	int yPos = (mode->height - SCREEN_HEIGHT) / 2;
	glfwSetWindowPos(window, xPos, yPos);

	//GL Setup
	//Viewport
	glfwGetWindowSize(window, &width, &height); //check resize
	windowWidth = width;
	windowHeight = height;

	//Alpha Blending
	/*glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

	// Z-Buffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// Sets the sky color
	glClearColor(SKY_COLOR.r, SKY_COLOR.g, SKY_COLOR.b, 1.0f);

	glewExperimental = GL_TRUE;		// TODO: Determine whether this is necessary or not
	glewInit();
	GenerateIds();

	skyboxCube = ContentManager::GetMesh("Cube.obj");

	return true;
}

void Graphics::Update() {
	glfwPollEvents();			// Should this be here or in InputManager?

	// Get components
	const std::vector<Component*> pointLights = EntityManager::GetComponents(ComponentType_PointLight);
	const std::vector<Component*> directionLights = EntityManager::GetComponents(ComponentType_DirectionLight);
	const std::vector<Component*> spotLights = EntityManager::GetComponents(ComponentType_SpotLight);
	const std::vector<Component*> meshes = EntityManager::GetComponents(ComponentType_Mesh);
	const std::vector<Component*> cameraComponents = EntityManager::GetComponents(ComponentType_Camera);
    const std::vector<Component*> rigidbodyComponents = EntityManager::GetComponents({
        ComponentType_RigidDynamic,
        ComponentType_RigidStatic,
        ComponentType_Vehicle
    });

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
		depthProjectionMatrix = glm::ortho<float>(-80, 80, -80, 80, -80, 160);
		depthViewMatrix = glm::lookAt(-shadowCaster->GetDirection(), glm::vec3(0), glm::vec3(0, 1, 0));

        // Render to the shadow map framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::ShadowMap]);

        // Clear the buffer and enable front-face culling
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

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
	glCullFace(GL_BACK);

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

    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER PHYSICS COLLIDERS
    // -------------------------------------------------------------------------------------------------------------- //

    if (renderPhysicsColliders) {
        // Use wireframe polygon mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        for (size_t j = 0; j < rigidbodyComponents.size(); j++) {
            // Get enabled models
            RigidbodyComponent* rigidbody = static_cast<RigidbodyComponent*>(rigidbodyComponents[j]);
            if (!rigidbody->enabled) continue;

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

        // Reset polygon mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

    skyboxProgram->LoadUniform(UniformName::Time, StateManager::gameTime.GetTimeSeconds());

    for (Camera camera : cameras) {
        // Setup the viewport for each camera (split-screen)
        glViewport(camera.viewportPosition.x, camera.viewportPosition.y, camera.viewportSize.x, camera.viewportSize.y);

        // Load the view projection matrix into the GPU
        const glm::mat4 viewProjectionMatrix = camera.projectionMatrix * glm::mat4(glm::mat3(camera.viewMatrix));
        skyboxProgram->LoadUniform(UniformName::ViewProjectionMatrix, viewProjectionMatrix);

        // Render the skybox
        glDrawElements(GL_TRIANGLES, skyboxCube->triangleCount * 3, GL_UNSIGNED_SHORT, nullptr);
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER POST-PROCESSING EFFECTS (BLOOM)
    // -------------------------------------------------------------------------------------------------------------- //

    // Render to the glow framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::GlowEffect]);

    // Load the screen geometry (this will be used by all subsequent draw calls)
    glBindVertexArray(screenVao);

    // Use the copy shader program
    ShaderProgram *copyProgram = shaders[Shaders::Copy];
    glUseProgram(copyProgram->GetId());

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

    // Render it
    glViewport(0, 0, windowWidth, windowHeight);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Disable the depth mask and enable additive blending
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // Render each blur level
    for (size_t i = 0; i < BLUR_LEVEL_COUNT; ++i) {
        glBindTexture(GL_TEXTURE_2D, blurLevelIds[i]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Disable blending and re-enable the depth mask
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);


    // -------------------------------------------------------------------------------------------------------------- //
    // GAME GUI
    // -------------------------------------------------------------------------------------------------------------- //

    // Render Fonts

    // Unbind shader program
    glUseProgram(0);

    // Initialize stuff
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    // Set the color
    glm::vec4 color = glm::vec4(1.f, 1.f, 0.f, 1.f);
    glPixelTransferf(GL_RED_BIAS, color.r - 1.f);
    glPixelTransferf(GL_GREEN_BIAS, color.g - 1.f);
    glPixelTransferf(GL_BLUE_BIAS, color.b - 1.f);
    glPixelTransferf(GL_ALPHA_BIAS, color.a - 1.f);

    // Load the font
    FTGLPixmapFont font("./Content/Fonts/Starjedi.ttf");

    // Initialize the font
    font.FaceSize(36);

    // Render the text
    font.Render("cAR wARS", -1, FTPoint(20, 20));

    // Reset stuff
    glPopAttrib();


    // -------------------------------------------------------------------------------------------------------------- //
    // DEBUG GUI
    // -------------------------------------------------------------------------------------------------------------- //

    RenderDebugGui();

	//Swap Buffers to Display New Frame
	glfwSwapBuffers(window);
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

        ImGui::Checkbox("Render Colliders", &renderPhysicsColliders);
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
			cameras.push_back(Camera(camera->GetViewMatrix(), camera->GetProjectionMatrix()));
			if (cameras.size() == MAX_CAMERAS) break;
		}
	}
	const size_t count = cameras.size();

	// Update the camera's viewports based on the number of cameras
	const glm::vec2 windowSize = glm::vec2(windowWidth, windowHeight);
	const glm::vec2 viewportSize = GetViewportSize();
	for (size_t i = 0; i < count; ++i) {
		cameras[i].viewportPosition = glm::vec2((i % 2) * 0.5f,
			i < 2 ? (count < 3 ? 0.f : 0.5f) : 0.f) * windowSize;
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
	const glm::vec2 viewportSize = GetViewportSize();
	const float aspectRatio = viewportSize.x / viewportSize.y;
	for (Component *component : cameraComponents) {
		if (component->enabled) {
			CameraComponent *camera = static_cast<CameraComponent*>(component);
			camera->SetAspectRatio(aspectRatio);
		}
	}
}

glm::vec2 Graphics::GetViewportSize() const {
	const glm::vec2 windowSize = glm::vec2(windowWidth, windowHeight);
	return glm::vec2(cameras.size() == 1 ? 1 : 0.5, cameras.size() < 3 ? 1 : 0.5) * windowSize;
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
	shaders[Shaders::ShadowMap] = LoadShaderProgram(SHADOW_MAP_VERTEX_SHADER, SHADOW_MAP_FRAGMENT_SHADER);
	shaders[Shaders::Skybox] = LoadShaderProgram(SKYBOX_VERTEX_SHADER, SKYBOX_FRAGMENT_SHADER);
	shaders[Shaders::Screen] = LoadShaderProgram(SCREEN_VERTEX_SHADER, SCREEN_FRAGMENT_SHADER);
	shaders[Shaders::Blur] = LoadShaderProgram(BLUR_VERTEX_SHADER, BLUR_FRAGMENT_SHADER);
	shaders[Shaders::Copy] = LoadShaderProgram(COPY_VERTEX_SHADER, COPY_FRAGMENT_SHADER);

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
        std::cout << "ERROR: Screen framebuffer incomplete!" << std::endl;

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
		std::cout << "ERROR: Shadow map framebuffer incomplete!" << std::endl;

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
		std::cout << "ERROR linking shader program:" << std::endl << info << std::endl;
	}

	glDeleteShader(vertexId);
	glDeleteShader(fragmentId);

	// Return the program's ID
	return new ShaderProgram(programId);
}
