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
#include "../../Components/LineComponent.h"

using namespace nlohmann;
using namespace physx;
using namespace std;

map<string, json> ContentManager::scenePrefabs;
map<string, json> ContentManager::entityPrefabs;
map<string, json> ContentManager::componentPrefabs;

map<string, Mesh*> ContentManager::meshes;
map<string, Texture*> ContentManager::textures;
map<string, Material*> ContentManager::materials;
map<string, PxMaterial*> ContentManager::pxMaterials;
GLuint ContentManager::skyboxCubemap;

const string ContentManager::CONTENT_DIR_PATH = "./Content/";

const string ContentManager::MESH_DIR_PATH = CONTENT_DIR_PATH + "Meshes/";
const string ContentManager::TEXTURE_DIR_PATH = CONTENT_DIR_PATH + "Textures/";
const string ContentManager::MATERIAL_DIR_PATH = CONTENT_DIR_PATH + "Materials/";
const string ContentManager::PX_MATERIAL_DIR_PATH = CONTENT_DIR_PATH + "PhysicsMaterials/";
const string ContentManager::SCENE_DIR_PATH = CONTENT_DIR_PATH + "Scenes/";

const string ContentManager::SKYBOX_DIR_PATH = CONTENT_DIR_PATH + "Skyboxes/";
const string ContentManager::SKYBOX_FACE_NAMES[6] = {"right", "left", "top", "bottom", "front", "back"};

const string ContentManager::PREFAB_DIR_PATH = CONTENT_DIR_PATH + "Prefabs/";
const string ContentManager::ENTITY_PREFAB_DIR_PATH = PREFAB_DIR_PATH + "Entities/";
const string ContentManager::COMPONENT_PREFAB_DIR_PATH = PREFAB_DIR_PATH + "Components/";

const string ContentManager::COLLISION_GROUPS_DIR_PATH = CONTENT_DIR_PATH + "CollisionGroups/";

const string ContentManager::SHADERS_DIR_PATH = CONTENT_DIR_PATH + "Shaders/";

glm::vec3 AssimpVectorToGlm(aiVector3D v) {
	return glm::vec3(v.x, v.y, v.z);
}

glm::vec2 AssimpVectorToGlm(aiVector2D v) {
	return glm::vec2(v.x, v.y);
}

Mesh* ContentManager::GetMesh(const string filePath, unsigned pFlags) {
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
		cout << "WARNING: Failed to load mesh: " << filePath << endl;
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

Texture* ContentManager::GetTexture(const string filePath) {
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
	string filePath;
	if (fromFile) {
		filePath = data.get<string>();
		
		material = materials[filePath];
		if (material != nullptr) return material;

		data = LoadJson(MATERIAL_DIR_PATH + filePath);
	}

	const glm::vec4 diffuseColor = GetColorFromJson(data["DiffuseColor"], glm::vec4(1.f));
	const glm::vec4 specularColor = GetColorFromJson(data["SpecularColor"], glm::vec4(1.f));
    const float specularity = GetFromJson<float>(data["Specularity"], 1);
    const float emissiveness = GetFromJson<float>(data["Emissiveness"], 0);

    material = new Material(diffuseColor, specularColor, specularity, emissiveness);

	if (fromFile) {
		materials[filePath] = material;
	}
    
	return material;
}

PxMaterial* ContentManager::GetPxMaterial(string filePath) {
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

vector<Entity*> ContentManager::LoadScene(string filePath, Entity *parent) {
	vector<Entity*> entities;

    json data;
    const auto it = scenePrefabs.find(filePath);
    if (it != scenePrefabs.end()) {
        data = it->second;
    } else {
        data = LoadJson(SCENE_DIR_PATH + filePath);
        scenePrefabs[filePath] = data;
    }
	
	for (json entityData : data) {
		entities.push_back(LoadEntity(entityData, parent));
	}
	return entities;
}

vector<Entity*> ContentManager::DestroySceneAndLoadScene(string filePath, Entity* parent) {
    EntityManager::DestroyScene();
    vector<Entity*> scene = LoadScene(filePath, parent);
    Graphics::Instance().SceneChanged();
    return scene;
}

Component* ContentManager::LoadComponent(json data) {
    // Get the top-level file path if applicable
    const bool fromFile = data.is_string();
    bool dataComplete = false;
    string filePath;
    if (fromFile) {
        filePath = data.get<string>();
        const auto it = componentPrefabs.find(filePath);
        if (it != componentPrefabs.end()) {
            data = it->second;
            dataComplete = true;
        }
    }

    // If we couldn't find the data in the map, construct it
    if (!dataComplete) {
        // While we are given file path strings, load the next file
        while (data.is_string()) {
            data = LoadJson(COMPONENT_PREFAB_DIR_PATH + filePath);
        }

        // While there is a nested prefab, load it
        json prefab = data["Prefab"];
        while (!prefab.is_null()) {
            json prefabData = LoadJson(COMPONENT_PREFAB_DIR_PATH + prefab.get<string>());
            prefab = json(prefabData["Prefab"]);
            MergeJson(prefabData, data);
            data = prefabData;
        }

        if (fromFile) {
            componentPrefabs[filePath] = data;
        }
    }

    // Load the component from the data
    Component *component = nullptr;
    bool supportedType = true;
    string type = data["Type"];
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
	else if (type == "Missile") component = new MissileComponent();
	else if (type == "AI") component = new AiComponent(data);
	else if (type == "GUI") component = new GuiComponent(data);
	else if (type == "Line") component = new LineComponent(data);
    else {
        cout << "Unsupported component type: " << type << endl;
        supportedType = false;
    }
    if (supportedType) {
        component->enabled = GetFromJson<bool>(data["Enabled"], true);
        return component;
    }

    return nullptr;
}

Entity* ContentManager::LoadEntity(json data, Entity *parent) {
    // Get the top-level file path if applicable
    const bool fromFile = data.is_string();
    bool dataComplete = false;
    string filePath;
    if (fromFile) {
        filePath = data.get<string>();
        const auto it = entityPrefabs.find(filePath);
        if (it != entityPrefabs.end()) {
            data = it->second;
            dataComplete = true;
        }
    }

    if (!dataComplete) {
        while (data.is_string()) {
            data = LoadJson(ENTITY_PREFAB_DIR_PATH + data.get<string>());
        }

        json prefab = data["Prefab"];
        while (!prefab.is_null()) {
            json prefabData = LoadJson(ENTITY_PREFAB_DIR_PATH + prefab.get<string>());
            prefab = json(prefabData["Prefab"]);
            MergeJson(prefabData, data);
            data = prefabData;
        }

        if (fromFile) {
            entityPrefabs[filePath] = data;
        }
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
        LoadScene(children.get<string>(), entity);
    }

	return entity;
}

json ContentManager::LoadJson(const string filePath) {
	ifstream file(filePath);		// TODO: Error check?
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

glm::vec4 ContentManager::GetColorFromJson(json data, glm::vec4 defaultValue) {
    glm::vec4 color = defaultValue;
    if (data.is_array()) {
        if (data.size() == 3) {
            color = glm::vec4(JsonToVec3(data), 255.f) / 255.f;
        } else if (data.size() == 4) {
            color = JsonToVec4(data) / 255.f;
        }
    } else if (data.is_string()) {
        string name = data.get<string>();
        if (name.find("White") != string::npos)                     color = glm::vec4(1.f, 1.f, 1.f, 1.f);
        else if (name.find("Black") != string::npos)                color = glm::vec4(0.f, 0.f, 0.f, 1.f);
        else if (name.find("Red") != string::npos)                  color = glm::vec4(206.f, 0.f, 0.f, 255.f) / 255.f;
        else if (name.find("Green") != string::npos)                color = glm::vec4(0.f, 1.f, 0.f, 1.f);
        else if (name.find("Blue") != string::npos)                 color = glm::vec4(0.f, 0.f, 1.f, 1.f);
        else if (name.find("Yellow") != string::npos)               color = glm::vec4(255.f, 233.f, 25.f, 255.f) / 255.f;
        else if (name.find("Cyan") != string::npos)                 color = glm::vec4(0.f, 1.f, 1.f, 1.f);

        if (name.find("HalfAlpha") != string::npos) color.a = 0.5f;
        else if (name.find("Alpha") != string::npos) color.a = 0.f;
    }
    return color;
}

void ContentManager::LoadCollisionGroups(string filePath) {
    json data = LoadJson(COLLISION_GROUPS_DIR_PATH + filePath);

    for (auto group : data) {
        CollisionGroups::AddCollisionGroup(group["Name"], group["CollidesWith"]);
    }
}

void ContentManager::LoadSkybox(string directoryPath) {
    glGenTextures(1, &skyboxCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);

    int width, height, nrChannels;
    for (size_t i = 0; i < 6; i++) {
        const string filePath = SKYBOX_DIR_PATH + directoryPath + SKYBOX_FACE_NAMES[i] + ".png";
        unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else {
            cout << "ERROR: Failed to load cubemap texture: " << filePath << endl;
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

GLuint ContentManager::LoadShader(string filePath, const GLenum shaderType) {
	filePath = SHADERS_DIR_PATH + filePath;

	string source;
	ifstream input(filePath.c_str());
	if (input) {
		copy(istreambuf_iterator<char>(input),
			istreambuf_iterator<char>(),
			back_inserter(source));
		input.close();
	} else {
		cout << "ERROR: Could not load shader source from file " << filePath << endl;
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
		string info(length, ' ');
		glGetShaderInfoLog(shaderId, info.length(), &length, &info[0]);
		cout << "ERROR Compiling Shader:" << endl << endl << source << endl << info << endl;
	}

	// Return the shader's ID
	return shaderId;
}
