#include "ContentManager.h"
#include <fstream>
#include <iterator>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "../../Entities/EntityManager.h"
#include "../../Components/MeshComponent.h"
#include <glm/gtx/string_cast.hpp>
#include "../../Components/CameraComponent.h"
#include "../../Components/PointLightComponent.h"
#include "../../Components/DirectionLightComponent.h"
#include "../../Components/SpotLightComponent.h"
#include "../../Components/VehicleComponent.h"
#include "../Physics/CollisionFilterShader.h"

std::map<std::string, Mesh*> ContentManager::meshes;
std::map<std::string, Texture*> ContentManager::textures;
std::map<std::string, Material*> ContentManager::materials;
GLuint ContentManager::skyboxCubemap;

const std::string ContentManager::CONTENT_DIR_PATH = "./Content/";

const std::string ContentManager::MESH_DIR_PATH = CONTENT_DIR_PATH + "Meshes/";
const std::string ContentManager::TEXTURE_DIR_PATH = CONTENT_DIR_PATH + "Textures/";
const std::string ContentManager::MATERIAL_DIR_PATH = CONTENT_DIR_PATH + "Materials/";
const std::string ContentManager::SCENE_DIR_PATH = CONTENT_DIR_PATH + "Scenes/";

const std::string ContentManager::SKYBOX_DIR_PATH = CONTENT_DIR_PATH + "Skyboxes/";
const std::string ContentManager::SKYBOX_FACE_NAMES[6] = {"right", "left", "top", "bottom", "front", "back"};

const std::string ContentManager::PREFAB_DIR_PATH = CONTENT_DIR_PATH + "Prefabs/";
const std::string ContentManager::ENTITY_PREFAB_DIR_PATH = PREFAB_DIR_PATH + "Entities/";
const std::string ContentManager::COMPONENT_PREFAB_DIR_PATH = PREFAB_DIR_PATH + "Components/";

const std::string ContentManager::COLLISION_GROUPS_DIR_PATH = CONTENT_DIR_PATH + "CollisionGroups/";

const std::string ContentManager::SHADERS_DIR_PATH = "./Engine/Shaders/";

glm::vec3 AssimpVectorToGlm(aiVector3D v) {
	return glm::vec3(v.x, v.y, v.z);
}

glm::vec2 AssimpVectorToGlm(aiVector2D v) {
	return glm::vec2(v.x, v.y);
}

Mesh* ContentManager::GetMesh(const std::string filePath) {
	Mesh* mesh = meshes[filePath];
	if (mesh != nullptr) return mesh;

	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(MESH_DIR_PATH + filePath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType);

	if (scene == nullptr) {
		std::cout << "WARNING: Failed to load mesh: " << filePath << std::endl;
		return nullptr;
	}

	aiMesh *aiMesh = scene->mMeshes[0];

	size_t vertexCount = aiMesh->mNumVertices;
	glm::vec3 *vertices = new glm::vec3[vertexCount];
	glm::vec2 *uvs = new glm::vec2[vertexCount];
	glm::vec3 *normals = new glm::vec3[vertexCount];
	for (size_t i = 0; i < vertexCount; ++i) {
		vertices[i] = AssimpVectorToGlm(aiMesh->mVertices[i]);
		uvs[i] = AssimpVectorToGlm(aiMesh->mTextureCoords[0][i]);
		normals[i] = AssimpVectorToGlm(aiMesh->mNormals[i]);
	}

	mesh = new Mesh(vertices, uvs, normals, vertexCount);

	meshes[filePath] = mesh;
	return mesh;
}

Texture* ContentManager::GetTexture(const std::string filePath) {
	Texture* texture = textures[filePath];
	if (texture != nullptr) return texture;

	int components;
	GLuint texId;
	int tWidth, tHeight;

	const auto data = stbi_load((TEXTURE_DIR_PATH + filePath).c_str(), &tWidth, &tHeight, &components, 0);

	if (data != nullptr) {
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);

		if (components == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		} else if (components == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tWidth, tHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);

		texture = new Texture(texId, tWidth, tHeight);
		textures[filePath] = texture;
	}

	return texture;
}

Material* ContentManager::GetMaterial(const std::string filePath) {
	Material* material = materials[filePath];
	if (material != nullptr) return material;

	nlohmann::json data = LoadJson(MATERIAL_DIR_PATH + filePath);
	const glm::vec3 diffuseColor = JsonToVec3(data["DiffuseColor"]);
	const glm::vec3 specularColor = JsonToVec3(data["SpecularColor"]);
    const float specularity = GetFromJson<float>(data["Specularity"], 1);
    const float emissiveness = GetFromJson<float>(data["Emissiveness"], 0);

	return new Material(diffuseColor, specularColor, specularity, emissiveness);
}

Component* ContentManager::LoadComponentPrefab(std::string filePath) {
    const nlohmann::json data = LoadJson(COMPONENT_PREFAB_DIR_PATH + filePath);
    return LoadComponent(data);
}

Entity* ContentManager::LoadEntityPrefab(std::string filePath) {
    const nlohmann::json data = LoadJson(ENTITY_PREFAB_DIR_PATH + filePath);
    return LoadEntity(data);
}

std::vector<Entity*> ContentManager::LoadScene(std::string filePath) {
	std::vector<Entity*> entities;

	nlohmann::json data = LoadJson(SCENE_DIR_PATH + filePath);
	for (nlohmann::json entityData : data) {
		entities.push_back(LoadEntity(entityData));
	}
	return entities;
}

Component* ContentManager::LoadComponent(nlohmann::json data) {
    if (data.is_string()) {
        return LoadComponentPrefab(data.get<std::string>());
    }

    Component *component = nullptr;
    bool supportedType;
    if (!data["Prefab"].is_null()) {
        component = LoadComponentPrefab(data["Prefab"]);
        supportedType = component != nullptr;
    } else {
        supportedType = true;
        std::string type = data["Type"];
        if (type == "Mesh") component = new MeshComponent(data);
        else if (type == "Camera") component = new CameraComponent(data);
        else if (type == "PointLight") component = new PointLightComponent(data);
        else if (type == "DirectionLight") component = new DirectionLightComponent(data);
        else if (type == "SpotLight") component = new SpotLightComponent(data);
        else if (type == "Vehicle") component = new VehicleComponent(data);
        else {
            std::cout << "Unsupported component type: " << type << std::endl;
            supportedType = false;
        }
    }

    if (supportedType) {
        component->enabled = GetFromJson<bool>(data["Enabled"], true);
        return component;
    }

    return nullptr;
}

Entity* ContentManager::LoadEntity(nlohmann::json data) {
    if (data.is_string()) {
        return LoadEntityPrefab(data.get<std::string>());
    }

    Entity *entity;
	if (!data["Prefab"].is_null()) {
		entity = LoadEntityPrefab(data["Prefab"]);
	} else {
		entity = EntityManager::CreateDynamicEntity();		// TODO: Determine whether or not the entity is static
	}

	for (auto it = data.begin(); it != data.end(); ++it) {
		std::string key = it.key();
		if (key == "Tag") EntityManager::SetTag(entity, it.value());
		else if (key == "Position") entity->transform.SetPosition(JsonToVec3(data["Position"]));
		else if (key == "Scale") entity->transform.SetScale(JsonToVec3(data["Scale"]));
		else if (key == "Rotate") {
			glm::vec3 rotation = JsonToVec3(data["Rotate"]);
			entity->transform.SetRotation(glm::vec3(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)));
		}
		else if (key == "Components") {
            for (auto componentData : it.value()) {
                Component *component = LoadComponent(componentData);
                if (component != nullptr) {
                    EntityManager::AddComponent(entity, component);
                }
            }
		}
		else if (key == "Children") {
			for (auto childData : it.value()) {
				Entity *child = LoadEntity(childData);
				child->transform.parent = &entity->transform;
			}
		}
		else if (key == "CylinderPart" && data["CylinderPart"]) {
			entity->transform.ConnectToCylinder();
		}
	}
	return entity;
}

nlohmann::json ContentManager::LoadJson(const std::string filePath) {
	std::ifstream file(filePath);		// TODO: Error check?
	nlohmann::json object;
	file >> object;
	file.close();
	return object;
}

glm::vec3 ContentManager::JsonToVec3(nlohmann::json data, glm::vec3 defaultValue) {
    if (!data.is_array() || data.size() != 3) return defaultValue;
    return glm::vec3(
        GetFromJson<float>(data[0], defaultValue.x),
        GetFromJson<float>(data[1], defaultValue.y),
        GetFromJson<float>(data[2], defaultValue.z));
}

glm::vec3 ContentManager::JsonToVec3(nlohmann::json data) {
    return JsonToVec3(data, glm::vec3());
}

glm::vec2 ContentManager::JsonToVec2(nlohmann::json data, glm::vec2 defaultValue) {
    if (!data.is_array() || data.size() != 2) return defaultValue;
    return glm::vec2(
        GetFromJson<float>(data[0], defaultValue.x),
        GetFromJson<float>(data[1], defaultValue.y));
}

glm::vec2 ContentManager::JsonToVec2(nlohmann::json data) {
    return JsonToVec2(data, glm::vec2());
}

void ContentManager::LoadCollisionGroups(std::string filePath) {
    nlohmann::json data = LoadJson(COLLISION_GROUPS_DIR_PATH + filePath);

    for (auto group : data) {
        CollisionGroups::AddCollisionGroup(group["Name"], group["CollidesWith"]);
    }
}

void ContentManager::LoadSkybox(std::string directoryPath) {
    glGenTextures(1, &skyboxCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);

    int width, height, nrChannels;
    for (size_t i = 0; i < 6; i++) {
        const char *filePath = (SKYBOX_DIR_PATH + directoryPath + SKYBOX_FACE_NAMES[i] + ".png").c_str();
        unsigned char *data = stbi_load(filePath, &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else {
            std::cout << "ERROR: Failed to load cubemap texture: " << filePath << std::endl;
        }
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

GLuint ContentManager::GetSkybox() {
    return skyboxCubemap;
}

GLuint ContentManager::LoadShader(std::string filePath, const GLenum shaderType) {
	filePath = SHADERS_DIR_PATH + filePath;

	std::string source;
	std::ifstream input(filePath.c_str());
	if (input) {
		copy(std::istreambuf_iterator<char>(input),
			std::istreambuf_iterator<char>(),
			std::back_inserter(source));
		input.close();
	} else {
		std::cout << "ERROR: Could not load shader source from file " << filePath << std::endl;
	}

	// Create a shader and get it's ID
	const GLuint shaderId = glCreateShader(shaderType);

	// Compile the shader
	const GLchar *sourcePointer = source.c_str();
	glShaderSource(shaderId, 1, &sourcePointer, nullptr);
	glCompileShader(shaderId);

	// Check compile status and print compilation errors
	GLint status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) {
		GLint length;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &length);
		std::string info(length, ' ');
		glGetShaderInfoLog(shaderId, info.length(), &length, &info[0]);
		std::cout << "ERROR Compiling Shader:" << std::endl << std::endl << source << std::endl << info << std::endl;
	}

	// Return the shader's ID
	return shaderId;
}
