#include "Graphics.h"
#include "IO/Mouse.h"
#include "IO/Keyboard.h"

#include <iostream>
#include "Content/ContentManager.h"
#include <glm/gtx/string_cast.hpp>
#include "../Entities/EntityManager.h"
#include "../Components/CameraComponent.h"
#include "../Components/MeshComponent.h"
#include "../Components/SpotLightComponent.h"
#include <glm/gtc/matrix_transform.inl>

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
Graphics::Graphics() { }
Graphics &Graphics::Instance() {
	static Graphics instance;
	return instance;
}

void Graphics::WindowSizeCallback(GLFWwindow *window, int width, int height) {
	Instance().SetWindowDimensions(width, height);
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

	// Input callbacks
	glfwSetCursorPosCallback(window, Mouse::MousePositionCallback);
	glfwSetMouseButtonCallback(window, Mouse::MouseButtonCallback);
	glfwSetKeyCallback(window, Keyboard::KeyboardCallback);
	//glfwSetJoystickCallback(Controller::ControllerCallback);

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
	glfwSetCursorPosCallback(window, Mouse::MousePositionCallback);
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

void Graphics::Update(Time currentTime, Time deltaTime) {
	glfwPollEvents();			// Should this be here or in InputManager?

	// Get components
	const std::vector<Component*> pointLights = EntityManager::GetComponents(ComponentType_PointLight);
	const std::vector<Component*> directionLights = EntityManager::GetComponents(ComponentType_DirectionLight);
	const std::vector<Component*> spotLights = EntityManager::GetComponents(ComponentType_SpotLight);
	const std::vector<Component*> meshes = EntityManager::GetComponents(ComponentType_Mesh);
	const std::vector<Component*> cameraComponents = EntityManager::GetComponents(ComponentType_Camera);

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
		depthProjectionMatrix = glm::ortho<float>(-50, 50, -50, 50, -50, 50);
		depthViewMatrix = glm::lookAt(-shadowCaster->GetDirection(), glm::vec3(0), glm::vec3(0, 1, 0));

        // Render to the shadow map framebuffer and bind the shadow map VAO
        glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::ShadowMap]);
        glBindVertexArray(vaoIds[VAOs::ShadowMap]);

        // Clear the buffer and enable front-face culling
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);

		// Set the current shader and VAO
		ShaderProgram *shadowProgram = shaders[Shaders::ShadowMap];
		glUseProgram(shadowProgram->GetId());

		// Draw the scene
		glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
		for (size_t j = 0; j < meshes.size(); j++) {
			// Get enabled models
			MeshComponent* model = static_cast<MeshComponent*>(meshes[j]);
			if (!model->enabled) continue;

			// Load the mesh's vertices into the GPU
			Mesh *mesh = model->GetMesh();
			LoadVertices(mesh->vertices, mesh->vertexCount);

			// Load the depth model view projection matrix into the GPU
			const glm::mat4 depthModelMatrix = model->transform.GetTransformationMatrix();
			const glm::mat4 depthModelViewProjectionMatrix = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
            shadowProgram->LoadUniform(UniformName::DepthModelViewProjectionMatrix, depthModelViewProjectionMatrix);

			// Render the model
			glDrawArrays(GL_TRIANGLES, 0, model->GetMesh()->vertexCount);
		}
	}


	// -------------------------------------------------------------------------------------------------------------- //
	// RENDER WORLD
	// -------------------------------------------------------------------------------------------------------------- //

	// Render to the default framebuffer and bind the geometry VAO
	glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::Screen]);
    glBindVertexArray(vaoIds[VAOs::Geometry]);

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
	}

	// Load our lights into the GPU
    geometryProgram->LoadUniform(UniformName::AmbientColor, AMBIENT_COLOR);
	LoadLights(pointLights, directionLights, spotLights);

	// Draw the scene
	for (size_t j = 0; j < meshes.size(); j++) {
		// Get enabled models
		MeshComponent* model = static_cast<MeshComponent*>(meshes[j]);
		if (!model->enabled) continue;

		// Load the model's vertices, uvs, normals, and textures into the GPU
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
			glDrawArrays(GL_TRIANGLES, 0, model->GetMesh()->vertexCount);
		}
	}


    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER SKYBOX
    // -------------------------------------------------------------------------------------------------------------- //

    // Render to the default framebuffer and bind the skybox VAO
    glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::Screen]);
    glBindVertexArray(vaoIds[VAOs::Skybox]);

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
    LoadVertices(skyboxCube->vertices, skyboxCube->vertexCount);

    // Load the sun data into the GPU
    if (shadowCaster != nullptr) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, sunTexture->textureId);
        skyboxProgram->LoadUniform(UniformName::SunTexture, 1);
        skyboxProgram->LoadUniform(UniformName::SunSizeRadians, glm::radians(10.f));
        skyboxProgram->LoadUniform(UniformName::SunDirection, shadowCaster->GetDirection());
    }

    skyboxProgram->LoadUniform(UniformName::Time, currentTime.GetTimeSeconds());

    for (Camera camera : cameras) {
        // Setup the viewport for each camera (split-screen)
        glViewport(camera.viewportPosition.x, camera.viewportPosition.y, camera.viewportSize.x, camera.viewportSize.y);

        // Load the view projection matrix into the GPU
        const glm::mat4 viewProjectionMatrix = camera.projectionMatrix * glm::mat4(glm::mat3(camera.viewMatrix));
        skyboxProgram->LoadUniform(UniformName::ViewProjectionMatrix, viewProjectionMatrix);

        // Render the skybox
        glDrawArrays(GL_TRIANGLES, 0, skyboxCube->vertexCount);
    }

    // -------------------------------------------------------------------------------------------------------------- //
    // RENDER POST-PROCESSING EFFECTS (BLOOM)
    // -------------------------------------------------------------------------------------------------------------- //

    // Render to the glow framebuffer and bind the screen VAO
    glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::GlowEffect]);
    glBindVertexArray(vaoIds[VAOs::Screen]);

    // Load the screen geometry (this will be used by all subsequent draw calls)
    const glm::vec2 verts[4] = {
        glm::vec2(-1, -1),
        glm::vec2(1, -1),
        glm::vec2(-1, 1),
        glm::vec2(1, 1)
    };
    LoadUvs(verts, 4);

    // Use the copy shader program
    ShaderProgram *copyProgram = shaders[Shaders::Copy];
    glUseProgram(copyProgram->GetId());

    // Load the glow buffer into the GPU
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureIds[Textures::ScreenGlow]);
    copyProgram->LoadUniform(UniformName::ScreenTexture, 0);

    // Copy the glow buffer to each of the level buffers
    for (size_t i = 0; i < SCREEN_LEVEL_COUNT; ++i) {
        const float factor = 1.f / pow(2, i);
        glViewport(0, 0, windowWidth * factor, windowHeight * factor);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenLevelIds[i], 0);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Use the blur shader program
    ShaderProgram *blurProgram = shaders[Shaders::Blur];
    glUseProgram(blurProgram->GetId());
    
    // Use the first texture location
    glActiveTexture(GL_TEXTURE0);
    blurProgram->LoadUniform(UniformName::ImageTexture, 0);

    // Blur each of the level buffers
    for (size_t i = 0; i < SCREEN_LEVEL_COUNT; ++i) {
        // Get the relevant buffers
        const GLuint buffer = screenLevelIds[i];
        const GLuint blurBuffer = screenLevelBlurIds[i];
        
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
    for (size_t i = 0; i < SCREEN_LEVEL_COUNT; ++i) {
        glBindTexture(GL_TEXTURE_2D, screenLevelIds[i]);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Disable blending and re-enable the depth mask
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

	//Swap Buffers to Display New Frame
	glfwSwapBuffers(window);
}

void Graphics::LoadModel(ShaderProgram *shaderProgram, MeshComponent *model) {
	if (!model->enabled) return;

	// Load the model matrix into the GPU
	const glm::mat4 modelMatrix = model->transform.GetTransformationMatrix();
    shaderProgram->LoadUniform(UniformName::ModelMatrix, modelMatrix);

	// Get the mesh's material
	Material *mat = model->GetMaterial();

	// Load the material data into the GPU
    shaderProgram->LoadUniform(UniformName::MaterialDiffuseColor, mat->diffuseColor);
    shaderProgram->LoadUniform(UniformName::MaterialSpecularColor, mat->specularColor);
    shaderProgram->LoadUniform(UniformName::MaterialSpecularity, mat->specularity);
    shaderProgram->LoadUniform(UniformName::MaterialEmissiveness, mat->emissiveness);

	// Load the mesh into the GPU
	LoadMesh(model->GetMesh());

	// Load the texture into the GPU
	if (model->GetTexture() != nullptr) {
        shaderProgram->LoadUniform(UniformName::DiffuseTextureEnabled, true);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, model->GetTexture()->textureId);
        shaderProgram->LoadUniform(UniformName::DiffuseTexture, 0);

        shaderProgram->LoadUniform(UniformName::UvScale, model->GetUvScale());
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

    for (size_t i = 0; i < SCREEN_LEVEL_COUNT; ++i) {
        const float factor = 1.f / pow(2, i);

        glBindTexture(GL_TEXTURE_2D, screenLevelIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth * factor, windowHeight * factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, screenLevelBlurIds[i]);
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

void Graphics::LoadMesh(Mesh* mesh) {
	LoadVertices(mesh->vertices, mesh->vertexCount);
	LoadUvs(mesh->uvs, mesh->vertexCount);
	LoadNormals(mesh->normals, mesh->vertexCount);
}

void Graphics::LoadVertices(const glm::vec3 *vertices, const size_t vertexCount) {
	glBindBuffer(GL_ARRAY_BUFFER, vboIds[VBOs::Vertices]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexCount, vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Graphics::LoadUvs(const glm::vec2* uvs, const size_t vertexCount) {
	glBindBuffer(GL_ARRAY_BUFFER, vboIds[VBOs::UVs]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertexCount, uvs, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Graphics::LoadNormals(const glm::vec3* normals, const size_t vertexCount) {
	glBindBuffer(GL_ARRAY_BUFFER, vboIds[VBOs::Normals]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertexCount, normals, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Graphics::DestroyIds() {
	glDeleteVertexArrays(VAOs::Count, vaoIds);
	glDeleteBuffers(VBOs::Count, vboIds);
    glDeleteBuffers(SSBOs::Count, ssboIds);
    glDeleteFramebuffers(FBOs::Count, fboIds);
    glDeleteRenderbuffers(RBOs::Count, rboIds);
    glDeleteTextures(Textures::Count, textureIds);
    glDeleteTextures(SCREEN_LEVEL_COUNT, screenLevelIds);
    glDeleteTextures(SCREEN_LEVEL_COUNT, screenLevelBlurIds);
    for (int i = 0; i < Shaders::Count; i++) {
        glDeleteProgram(shaders[i]->GetId());
    }
}

void Graphics::GenerateIds() {
	glGenVertexArrays(VAOs::Count, vaoIds);
	glGenBuffers(VBOs::Count, vboIds);
	glGenBuffers(SSBOs::Count, ssboIds);
	for (size_t i = 0; i < SSBOs::Count; i++) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, ssboIds[i]);
	}
	glGenFramebuffers(FBOs::Count, fboIds);
	glGenRenderbuffers(RBOs::Count, rboIds);
    glGenTextures(Textures::Count, textureIds);
    glGenTextures(SCREEN_LEVEL_COUNT, screenLevelIds);
    glGenTextures(SCREEN_LEVEL_COUNT, screenLevelBlurIds);
	
    shaders[Shaders::Geometry] = LoadShaderProgram(GEOMETRY_VERTEX_SHADER, GEOMETRY_FRAGMENT_SHADER);
	shaders[Shaders::ShadowMap] = LoadShaderProgram(SHADOW_MAP_VERTEX_SHADER, SHADOW_MAP_FRAGMENT_SHADER);
	shaders[Shaders::Skybox] = LoadShaderProgram(SKYBOX_VERTEX_SHADER, SKYBOX_FRAGMENT_SHADER);
	shaders[Shaders::Screen] = LoadShaderProgram(SCREEN_VERTEX_SHADER, SCREEN_FRAGMENT_SHADER);
	shaders[Shaders::Blur] = LoadShaderProgram(BLUR_VERTEX_SHADER, BLUR_FRAGMENT_SHADER);
	shaders[Shaders::Copy] = LoadShaderProgram(COPY_VERTEX_SHADER, COPY_FRAGMENT_SHADER);

	InitializeGeometryVao();
	InitializeShadowMapVao();
	InitializeSkyboxVao();
    InitializeScreenVao();

    InitializeGlowFramebuffer();
    InitializeScreenFramebuffer();
	InitializeShadowMapFramebuffer();
}

void Graphics::InitializeGeometryVao() {
	glBindVertexArray(vaoIds[VAOs::Geometry]);

	// Vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboIds[VBOs::Vertices]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

	// UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vboIds[VBOs::UVs]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

	// Normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vboIds[VBOs::Normals]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Graphics::InitializeShadowMapVao() {
	glBindVertexArray(vaoIds[VAOs::ShadowMap]);

	// Vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vboIds[VBOs::Vertices]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Graphics::InitializeSkyboxVao() {
    glBindVertexArray(vaoIds[VAOs::Skybox]);

    // Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[VBOs::Vertices]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Graphics::InitializeScreenVao() {
    glBindVertexArray(vaoIds[VAOs::Screen]);

    // Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[VBOs::UVs]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(nullptr));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Graphics::InitializeGlowFramebuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, fboIds[FBOs::GlowEffect]);

    GLenum fboBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, fboBuffers);

    for (size_t i = 0; i < SCREEN_LEVEL_COUNT; ++i) {
        const float factor = 1.f / pow(2, i);
        
        glBindTexture(GL_TEXTURE_2D, screenLevelIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH * factor, SCREEN_HEIGHT * factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, screenLevelBlurIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH * factor, SCREEN_HEIGHT * factor, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, screenLevelIds[i], 0);
    }

//    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//        std::cout << "ERROR: Glow framebuffer incomplete!" << std::endl;

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
