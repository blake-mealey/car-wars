#include "NavigationMesh.h"

#include "ContentManager.h"

NavigationMesh::NavigationMesh(nlohmann::json data) {
	columnCount = ContentManager::GetFromJson<size_t>(data["ColumnCount"], 100);
	rowCount = ContentManager::GetFromJson<size_t>(data["RowCount"], 100);

	Initialize();
}

void NavigationMesh::Initialize() {
	vertices = new NavigationVertex*[rowCount];
	for (size_t i = 0; i < rowCount; ++i) {
		vertices[i] = new NavigationVertex[columnCount];
	}

	/*for (size_t row = 0; row < rowCount; ++row) {
		for () {

		}
	}*/
}