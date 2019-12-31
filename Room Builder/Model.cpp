#include "Model.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <glm.hpp>
#include "Texture.h"
#include <vector>
#include "ShaderManager.h"
#include "Shader.h"
#include <unordered_map>

struct vertex
{
	glm::vec3 pos, normal, tangent;
	glm::vec2 texCoords;
};
struct Model::mesh
{
	std::vector<vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<SharedTexture> texs;
	unsigned int vao, vbo, ebo;
	struct material {
		glm::vec3 diffuseColor, emissionColor, ambientColor, specularColor;
		float opacity, shininess;
		std::string name;
	} mat;
	void init();
};

struct Model::mImpl
{
	std::vector<Model::mesh> meshes;
	std::unordered_map<std::string, SharedTexture> loadedTextures;
	std::string textureDirectory;
};

void Model::nvi_draw(renderPass p)
{
//	ShaderManager::getShader(sid)->setMat4("model", model);
	for (auto& m : mPimpl->meshes) {
		if (p != renderPass::outline) {
			for (auto& t : m.texs)
				t.bind();
			if (m.mat.name.find("reflect") != std::string::npos) {
				ShaderManager::getShader(sid)->setInt("useTex", 2);
			}
			else if (!m.texs.empty()) {
				ShaderManager::getShader(sid)->setInt("useTex", 1);
			}
			else {
				ShaderManager::getShader(sid)->setInt("useTex", 0);
				ShaderManager::getShader(sid)->setVec4("color", glm::vec4(m.mat.diffuseColor, m.mat.opacity));
			}
		}
		glBindVertexArray(m.vao);
		glDrawElements(GL_TRIANGLES, m.indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		ShaderManager::getShader(sid)->setInt("useTex", 0);
	}
}

Model::Model(const char * filename)
{
	sid = shaderID::basic;
	mPimpl = std::make_unique<mImpl>();
	if (strrchr(filename, '\\')) {
		char dir[MAX_PATH];
		size_t s = strrchr(filename, '\\') + 1 - filename;
		memcpy(dir, filename, s);
		dir[s] = '\0';
		mPimpl->textureDirectory = dir;
	}
	else
		mPimpl->textureDirectory = "";
	Assimp::Importer importer;
	auto scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
	processNode(scene->mRootNode, scene);
}


Model::~Model()
{
	for (auto& m : mPimpl->meshes)
	{
		glDeleteBuffers(1, &m.vbo);
		glDeleteBuffers(1, &m.ebo);
		glDeleteVertexArrays(1, &m.vao);
	}
}

Model::mesh Model::processMesh(void * aimesh, const void * aiscene)
{
	aiMesh * mesh = (aiMesh*)aimesh;
	aiScene * scene = (aiScene*)aiscene;
	Model::mesh m;
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
		vertex v;
		v.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
		if (mesh->mNormals != NULL) v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
		if (mesh->mTangents != NULL) v.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
		if (mesh->mTextureCoords[0] != NULL) v.texCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
		m.vertices.push_back(v);
	}
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
		m.indices.push_back(mesh->mFaces[i].mIndices[0]);
		m.indices.push_back(mesh->mFaces[i].mIndices[1]);
		m.indices.push_back(mesh->mFaces[i].mIndices[2]);
	}
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial * mat = scene->mMaterials[mesh->mMaterialIndex];
		const aiTextureType types[] = { aiTextureType::aiTextureType_DIFFUSE, aiTextureType::aiTextureType_HEIGHT, 
			aiTextureType::aiTextureType_SPECULAR, aiTextureType::aiTextureType_EMISSIVE };
		for (int i = 0; i < 1/*sizeof(types) / sizeof(aiTextureType)*/; ++i) {
			for (unsigned int j = 0; j < mat->GetTextureCount(types[i]); ++j) {
				aiString path;
				if(mat->GetTexture(types[i], j, &path) != AI_SUCCESS) printf("Could not fetch texture\n");
				std::string fullPath = mPimpl->textureDirectory;
				std::string filename = path.C_Str();
				if (filename.find('\\'))
					filename = filename.substr(filename.find_last_of('\\') + 1);
				fullPath.append(filename);
				size_t embeddedIndex = fullPath.find('*');
				if (embeddedIndex != std::string::npos) {
					int index = std::stoi(fullPath.substr(embeddedIndex + 1));
					aiTexture * tex = scene->mTextures[index];
					if (mPimpl->loadedTextures.find(fullPath) == mPimpl->loadedTextures.end()) {
						SharedTexture t(i);
						t.loadFromData(tex->mWidth, tex->mHeight, (char*)tex->pcData, GL_RGBA);
						m.texs.push_back(t);
						mPimpl->loadedTextures[fullPath] = m.texs[m.texs.size() - 1];
					}
					else {
						m.texs.emplace_back(mPimpl->loadedTextures[fullPath]);
					}
				}
				else {
					if (mPimpl->loadedTextures.find(fullPath) == mPimpl->loadedTextures.end()) {
						m.texs.emplace_back(fullPath.c_str(), i);
						mPimpl->loadedTextures[fullPath] = m.texs[m.texs.size() - 1];
					}
					else {
						m.texs.emplace_back(mPimpl->loadedTextures[fullPath]);
					}
				}
			}
		}
		aiColor3D color;
		float opacity;
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		mat->Get(AI_MATKEY_OPACITY, opacity);
		m.mat.diffuseColor = { color.r, color.g, color.b };
		m.mat.opacity = opacity;
		mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
		m.mat.specularColor = { color.r, color.g, color.b };
		mat->Get(AI_MATKEY_COLOR_EMISSIVE, color);
		m.mat.emissionColor = { color.r, color.g, color.b };
		mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
		m.mat.ambientColor = { color.r, color.g, color.b };
		aiString str;
		mat->Get(AI_MATKEY_NAME, str);
		m.mat.name = str.C_Str();
		mat->Get(AI_MATKEY_SHININESS, opacity);
		m.mat.shininess = opacity;
		printf("Mat name: %s\n", str.C_Str());
	}
	m.init();
	return m;
}

void Model::processNode(void * ainode, const void * aiscene)
{
	aiNode * node = (aiNode*)ainode;
	aiScene * scene = (aiScene*)aiscene;
	for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
		mPimpl->meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], aiscene));
	}
	for (unsigned int i = 0; i < node->mNumChildren; ++i) {
		processNode(node->mChildren[i], aiscene);
	}
}

void Model::mesh::init()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(offsetof(vertex, pos)));
//	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(offsetof(vertex, normal)));
//	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(offsetof(vertex, tangent)));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)(offsetof(vertex, texCoords)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
//	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}
