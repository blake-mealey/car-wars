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
#include "../../Components/WeaponComponents/MachineGunComponent.h"
#include "../../Components/WeaponComponents/RailGunComponent.h"
#include "../../Components/WeaponComponents//RocketLauncherComponent.h"
#include "../Physics/CollisionGroups.h"
#include "../Physics.h"
#include "../../Components/RigidbodyComponents/RigidStaticComponent.h"
#include "../../Components/RigidbodyComponents/RigidDynamicComponent.h"
#include "../../Components/RigidbodyComponents/VehicleComponent.h"
#include "../../Components/AiComponent.h"
#include "../../Components/GuiComponents/GuiComponent.h"

using namespace nlohmann;
using namespace physx;

std::map<std::string, Mesh*> ContentManager::meshes;
std::map<std::string, Texture*> ContentManager::textures;
std::map<std::string, Material*> ContentManager::materials;
std::map<std::string, PxMaterial*> ContentManager::pxMaterials;
GLuint ContentManager::skyboxCubemap;

const std::string ContentManager::CONTENT_DIR_PATH = "./Content/";

const std::string ContentManager::MESH_DIR_PATH = CONTENT_DIR_PATH + "Meshes/";
const std::string ContentManager::TEXTURE_DIR_PATH = CONTENT_DIR_PATH + "Textures/";
const std::string ContentManager::MATERIAL_DIR_PATH = CONTENT_DIR_PATH + "Materials/";
const std::string ContentManager::PX_MATERIAL_DIR_PATH = CONTENT_DIR_PATH + "PhysicsMaterials/";
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

Mesh* ContentManager::GetMesh(const std::string filePath, unsigned pFlags) {
	Mesh* mesh = meshes[filePath];
	if (mesh != nullptr) return mesh;

	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(MESH_DIR_PATH + filePath,
        pFlags
		| aiProcess_CalcTangentSpace
		| aiProcess_Triangulate
//		| aiProcess_SortByPType             // What does this do?
        | aiProcess_GenSmoothNormals
//        | aiProcess_FlipUVs               // We probably want this!
//        | aiProcess_JoinIdenticalVertices
    );

	if (scene == nullptr) {
		std::cout << "WARNING: Failed to load mesh: " << filePath << std::endl;
		return nullptr;
	}

    // TODO: Load all meshes
	aiMesh *aiMesh = scene->mMeshes[0];

    const size_t triangleCount = aiMesh->mNumFaces;
    Triangle *triangles = new Triangle[triangleCount];
    for (size_t i = 0; i < triangleCount; ++i) {
        const aiFace &triangle = aiMesh->mFaces[i];
        triangles[i] = Triangle(triangle.mIndices[0], triangle.mIndices[1], triangle.mIndices[2]);
    }

    const size_t vertexCount = aiMesh->mNumVertices;
	glm::vec3 *vertices = new glm::vec3[vertexCount];
	glm::vec2 *uvs = new glm::vec2[vertexCount];
	glm::vec3 *normals = new glm::vec3[vertexCount];
	for (size_t i = 0; i < vertexCount; ++i) {
		vertices[i] = AssimpVectorToGlm(aiMesh->mVertices[i]);
		uvs[i] = AssimpVectorToGlm(aiMesh->mTextureCoords[0][i]);
		normals[i] = AssimpVectorToGlm(aiMesh->mNormals[i]);
	}

	mesh = new Mesh(triangleCount, vertexCount, triangles, vertices, uvs, normals);

    // TODO: Load materials/textures

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

Material* ContentManager::GetMaterial(json data) {
	Material *material;
	
	bool fromFile = data.is_string();
	std::string filePath;
	if (fromFile) {
		filePath = data.get<std::string>();
		
		material = materials[filePath];
		if (material != nullptr) return material;

		data = LoadJson(MATERIAL_DIR_PATH + filePath);
	}

	const glm::vec3 diffuseColor = JsonToVec3(data["DiffuseColor"]);
	const glm::vec3 specularColor = JsonToVec3(data["SpecularColor"]);
    const float specularity = GetFromJson<float>(data["Specularity"], 1);
    const float emissiveness = GetFromJson<float>(data["Emissiveness"], 0);

    material = new Material(diffuseColor, specularColor, specularity, emissiveness);

	if (fromFile) {
		materials[filePath] = material;
	}
    
	return material;
}

PxMaterial* ContentManager::GetPxMaterial(std::string filePath) {
    PxMaterial* material = pxMaterials[filePath];
    if (material != nullptr) return material;

    json data = LoadJson(PX_MATERIAL_DIR_PATH + filePath);
    const float staticFriction = GetFromJson<float>(data["StaticFriction"], 0.5f);
    const float dynamicFriction = GetFromJson<float>(data["DynamicFriction"], 0.5f);
    const float restitution = GetFromJson<float>(data["Restitution"], 0.6f);

    material = Physics::Instance().GetApi().createMaterial(staticFriction, dynamicFriction, restitution);

    pxMaterials[filePath] = material;
    return material;
}

std::vector<Entity*> ContentManager::LoadScene(std::string filePath, Entity *parent) {
	std::vector<Entity*> entities;

	json data = LoadJson(SCENE_DIR_PATH + filePath);
	for (json entityData : data) {
		entities.push_back(LoadEntity(entityData, parent));
	}
	return entities;
}

std::vector<Entity*> ContentManager::DestroySceneAndLoadScene(std::string filePath, Entity* parent) {
    EntityManager::DestroyScene();
    std::vector<Entity*> scene = LoadScene(filePath, parent);
    Graphics::Instance().SceneChanged();
    return scene;
}

Component* ContentManager::LoadComponent(json data) {
    while (data.is_string()) {
        data = LoadJson(COMPONENT_PREFAB_DIR_PATH + data.get<std::string>());
    }

    json prefab = data["Prefab"];
    while (!prefab.is_null()) {
        json prefabData = LoadJson(COMPONENT_PREFAB_DIR_PATH + prefab.get<std::string>());
        prefab = json(prefabData["Prefab"]);
        MergeJson(prefabData, data);
        data = prefabData;
    }

    Component *component = nullptr;
    bool supportedType = true;
    std::string type = data["Type"];
    if (type == "Mesh") component = new MeshComponent(data);
    else if (type == "Camera") component = new CameraComponent(data);
    else if (type == "PointLight") component = new PointLightComponent(data);
    else if (type == "DirectionLight") component = new DirectionLightComponent(data);
    else if (type == "SpotLight") component = new SpotLightComponent(data);
    else if (type == "RigidStatic") component = new RigidStaticComponent(data);
    else if (type == "RigidDynamic") component = new RigidDynamicComponent(data);
    else if (type == "Vehicle") component = new VehicleComponent(data);
	else if (type == "MachineGun") component = new MachineGunComponent();
	else if (type == "RailGun") component = new RailGunComponent();
    else if (type == "RocketLauncher") component = new RocketLauncherComponent();
	else if (type == "AI") component = new AiComponent(data);
	else if (type == "GUI") component = new GuiComponent(data);
    else {
        std::cout << "Unsupported component type: " << type << std::endl;
        supportedType = false;
    }
    if (supportedType) {
        component->enabled = GetFromJson<bool>(data["Enabled"], true);
        return component;
    }

    return nullptr;
}

Entity* ContentManager::LoadEntity(json data, Entity *parent) {
    while (data.is_string()) {
        data = LoadJson(ENTITY_PREFAB_DIR_PATH + data.get<std::string>());
    }

    json prefab = data["Prefab"];
	while (!prefab.is_null()) {
		json prefabData = LoadJson(ENTITY_PREFAB_DIR_PATH + prefab.get<std::string>());
        prefab = json(prefabData["Prefab"]);
        MergeJson(prefabData, data);
        data = prefabData;
	}

    // TODO: Determine whether or not the entity is static (parameter?)
    Entity *entity = EntityManager::CreateDynamicEntity(parent);

    if (!data["Tag"].is_null()) EntityManager::SetTag(entity, data["Tag"]);
    entity->transform = Transform(data);
    if (parent) entity->transform.parent = &parent->transform;

    for (const auto componentData : data["Components"]) {
        Component *component = LoadComponent(componentData);
        if (component != nullptr) {
            EntityManager::AddComponent(entity, component);
        }
    }

    json children = data["Children"];
    if (children.is_array()) {
        for (const auto childData : data["Children"]) {
            Entity *child = LoadEntity(childData, entity);
        }
    } else if (children.is_string()) {
        LoadScene(children.get<std::string>(), entity);
    }

	return entity;
}

json ContentManager::LoadJson(const std::string filePath) {
	std::ifstream file(filePath);		// TODO: Error check?
	json object;
	file >> object;
	file.close();
	return object;
}

void ContentManager::MergeJson(json &obj0, json &obj1, bool overwrite) {
    if (obj0.is_object()) {
        for (auto it = obj1.begin(); it != obj1.end(); ++it) {
            if (obj0[it.key()].is_primitive()) {
                if (overwrite || obj0[it.key()].is_null()) {
                    obj0[it.key()] = it.value();
                }
            } else {
                MergeJson(obj0[it.key()], it.value());
            }
        }
    } else if (obj0.is_array()) {
        if (obj0.size() >= 2 && obj0.size() <= 4 && obj0[0].is_number()) {
            obj0 = obj1;
        } else {
            for (json &value : obj1) {
                obj0.push_back(value);
            }
        }
    } else {
        obj0 = overwrite ? obj1 : obj0;
    }
}

glm::vec4 ContentManager::JsonToVec4(json data, glm::vec4 defaultValue) {
	if (!data.is_array() || data.size() != 4) return defaultValue;
	return glm::vec4(
		GetFromJson<float>(data[0], defaultValue.x),
		GetFromJson<float>(data[1], defaultValue.y),
		GetFromJson<float>(data[2], defaultValue.z),
		GetFromJson<float>(data[3], defaultValue.w));
}

glm::vec4 ContentManager::JsonToVec4(json data) {
	return JsonToVec4(data, glm::vec4());
}

glm::vec3 ContentManager::JsonToVec3(json data, glm::vec3 defaultValue) {
    if (!data.is_array() || data.size() != 3) return defaultValue;
    return glm::vec3(
        GetFromJson<float>(data[0], defaultValue.x),
        GetFromJson<float>(data[1], defaultValue.y),
        GetFromJson<float>(data[2], defaultValue.z));
}

glm::vec3 ContentManager::JsonToVec3(json data) {
    return JsonToVec3(data, glm::vec3());
}

glm::vec2 ContentManager::JsonToVec2(json data, glm::vec2 defaultValue) {
    if (!data.is_array() || data.size() != 2) return defaultValue;
    return glm::vec2(
        GetFromJson<float>(data[0], defaultValue.x),
        GetFromJson<float>(data[1], defaultValue.y));
}

glm::vec2 ContentManager::JsonToVec2(json data) {
    return JsonToVec2(data, glm::vec2());
}

void ContentManager::LoadCollisionGroups(std::string filePath) {
    json data = LoadJson(COLLISION_GROUPS_DIR_PATH + filePath);

    for (auto group : data) {
        CollisionGroups::AddCollisionGroup(group["Name"], group["CollidesWith"]);
    }
}

void ContentManager::LoadSkybox(std::string directoryPath) {
    glGenTextures(1, &skyboxCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);

    int width, height, nrChannels;
    for (size_t i = 0; i < 6; i++) {
        const std::string filePath = SKYBOX_DIR_PATH + directoryPath + SKYBOX_FACE_NAMES[i] + ".png";
        unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
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
