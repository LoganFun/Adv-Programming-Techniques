#pragma once
#include <unordered_map>
#include <unordered_set>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "./common/shader.hpp"
#include "vertexBufferObject.h"
#include "texture.h"

class CMaterial
{
public:
	int iTexture;
};

class CAssimpModel
{
public:
	bool LoadModelFromFile(char* sFilePath);

	static void FinalizeVBO();
	static void BindModelsVAO();
	void ProcessNode(aiNode* node, const aiScene* scene);

	void RenderModel();
	CAssimpModel();
private:
	std::unordered_map<std::string, std::unordered_set<unsigned int>> m_mapObjects;
	std::unordered_set<unsigned int> m_setIds;
	std::string m_strLastName;
	bool bLoaded;
	static CVertexBufferObject vboModelData;
	static uint32_t uiVAO;
	static vector<CTexture> tTextures;
	vector<int> iMeshStartIndices;
	vector<int> iMeshSizes;
	vector<int> iMaterialIndices;
	int iNumMaterials;
};