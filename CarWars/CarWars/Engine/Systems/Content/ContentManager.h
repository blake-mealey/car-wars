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
	static const std::string SCENE_DIR_PATH;

	static const std::string SKYBOX_DIR_PATH;
	static const std::string SKYBOX_FACE_NAMES[6];

	static const std::string PREFAB_DIR_PATH;
	static const std::string ENTITY_PREFAB_DIR_PATH;
	static const std::string COMPONENT_PREFAB_DIR_PATH;

	static const std::string COLLISION_GROUPS_DIR_PATH;

	static const std::string SHADERS_DIR_PATH;

	static Mesh* GetMesh(std::string filePath);
	static Texture* GetTexture(std::string filePath);
	static Material* GetMaterial(std::string filePath);

    static Component* LoadComponentPrefab(std::string filePath);
	static Entity* LoadEntityPrefab(std::string filePath);
	static std::vector<Entity*> LoadScene(std::string filePath);

	template <typename T>
	static T GetFromJson(nlohmann::json json, T defaultValue);
	static nlohmann::json LoadJson(std::string filePath);
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
	// static Texture*? GetTexture(std::string filePath);
	// static GameData*? GetDataFile(std::string filePath);

	static Component* LoadComponent(nlohmann::json data);
	static Entity* LoadEntity(nlohmann::json data);

private:
	static std::map<std::string, Mesh*> meshes;
	static std::map<std::string, Texture*> textures;
	static std::map<std::string, Material*> materials;
    static GLuint skyboxCubemap;
};

// TODO: For some reason I can't compile when this is in ContentManager.cpp?
template <typename T>
T ContentManager::GetFromJson(nlohmann::json json, T defaultValue) {
	if (json.is_null()) return defaultValue;
	return json.get<T>();
}
