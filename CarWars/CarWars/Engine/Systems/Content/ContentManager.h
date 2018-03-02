#pragma once

#include <string>
#include "Mesh.h"
#include <map>
#include "Texture.h"
#include "Material.h"
#include "json/json.hpp"
#include "../../Entities/Entity.h"

struct Texture;

class ContentManager {
public:
	static const std::string CONTENT_DIR_PATH;

	static const std::string MESH_DIR_PATH;
	static const std::string TEXTURE_DIR_PATH;
	static const std::string MATERIAL_DIR_PATH;
	static const std::string PX_MATERIAL_DIR_PATH;
	static const std::string SCENE_DIR_PATH;

	static const std::string SKYBOX_DIR_PATH;
	static const std::string SKYBOX_FACE_NAMES[6];

	static const std::string PREFAB_DIR_PATH;
	static const std::string ENTITY_PREFAB_DIR_PATH;
	static const std::string COMPONENT_PREFAB_DIR_PATH;

	static const std::string COLLISION_GROUPS_DIR_PATH;

	static const std::string SHADERS_DIR_PATH;

	static Mesh* GetMesh(std::string filePath, unsigned pFlags=0);
	static Texture* GetTexture(std::string filePath);
	static Material* GetMaterial(nlohmann::json data);
	static physx::PxMaterial* GetPxMaterial(std::string filePath);

    static Component* LoadComponentPrefab(std::string filePath);
	static std::vector<Entity*> LoadScene(std::string filePath);

	template <typename T>
	static T GetFromJson(nlohmann::json json, T defaultValue);
	static nlohmann::json LoadJson(std::string filePath);
	static void MergeJson(nlohmann::json &obj0, nlohmann::json &obj1, bool overwrite=true);
	static glm::vec4 JsonToVec4(nlohmann::json data, glm::vec4 defaultValue);
	static glm::vec4 JsonToVec4(nlohmann::json data);
    static glm::vec3 JsonToVec3(nlohmann::json data, glm::vec3 defaultValue);
	static glm::vec3 JsonToVec3(nlohmann::json data);
	static glm::vec2 JsonToVec2(nlohmann::json data, glm::vec2 defaultValue);
	static glm::vec2 JsonToVec2(nlohmann::json data);

    static void LoadCollisionGroups(std::string filePath);

    static void LoadSkybox(std::string directoryPath);
    static GLuint GetSkybox();

	static GLuint LoadShader(std::string filePath, GLenum shaderType);

	// TODO: These guys
	// static Audio*? GetAudio(std::string filePath);

    template <class T>
    static T* LoadComponent(nlohmann::json data);
	static Component* LoadComponent(nlohmann::json data);
	static Entity* LoadEntity(nlohmann::json data);

private:
	static std::map<std::string, Mesh*> meshes;
	static std::map<std::string, Texture*> textures;
	static std::map<std::string, Material*> materials;
	static std::map<std::string, physx::PxMaterial*> pxMaterials;
    static GLuint skyboxCubemap;
};

template <typename T>
T ContentManager::GetFromJson(nlohmann::json json, T defaultValue) {
    if (json.is_null()) return defaultValue;
    return json.get<T>();
}

template <class T>
T* ContentManager::LoadComponent(nlohmann::json data) {
    T *component = new T(data);
    component->enabled = GetFromJson<bool>(data["Enabled"], true);
    return component;
}