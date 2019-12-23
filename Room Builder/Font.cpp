#include "Font.h"
#include <glad/glad.h>
#include <fstream>
#include <unordered_map>
#include <string>
#include "ShaderManager.h"
#include "Shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
const float rectVerts[] =
{
	0.0, 1.0,
	0.0, 0.0,
	1.0, 1.0,
	1.0, 0.0
};
struct glyph
{
	int x, y, width, height, advance, xoffset, yoffset;
};
struct fnImpl
{
	std::unordered_map<char, glyph> glyphs;
	int mapWidth, mapHeight, mapComps;
	int lineHeight;
	unsigned int mapTexture;
	unsigned int vao, vbo;
};

Font::Font(const char * fontfile)
{
	size_t folderLen = strrchr(fontfile, '\\') - fontfile + 1;
	std::unique_ptr<char[]> folder = std::make_unique<char[]>(folderLen + 1);
	memcpy(folder.get(), fontfile, folderLen);
	folder[folderLen] = '\0';
	std::ifstream fntFile(fontfile);
	std::string line;
	std::string texMap(folder.get());
	pimpl = std::make_unique<fnImpl>();
	while (std::getline(fntFile, line)) {
		size_t t = line.find("file=\"");
		size_t id = line.find("char id=");
		size_t lh = line.find("lineHeight=");
		if (t != std::string::npos) {
			texMap += line.substr(t + 6, line.find('"', t + 6) - (t + 6));
		}
		else if (lh != std::string::npos) {
			pimpl->lineHeight = std::stoi(line.substr(lh + 11, line.find(' ', lh + 11) - (lh + 11)));
		}
		if (id != std::string::npos) {
			char c = std::stoi(line.substr(id + 8, line.find(' ', id) - (id + 8)));
			size_t xPos = line.find("x=", id);
			size_t yPos = line.find("y=", xPos);
			size_t wPos = line.find("width=", yPos);
			size_t hPos = line.find("height=", wPos);
			size_t xoPos = line.find("xoffset=", wPos);
			size_t yoPos = line.find("yoffset=", xoPos);
			size_t aPos = line.find("xadvance=", yoPos);
			int x = std::stoi(line.substr(xPos + 2, line.find(' ', xPos + 2) - (xPos + 2)));
			int y = std::stoi(line.substr(yPos + 2, line.find(' ', yPos + 2) - (yPos + 2)));
			int w = std::stoi(line.substr(wPos + 6, line.find(' ', wPos + 6) - (wPos + 6)));
			int h = std::stoi(line.substr(hPos + 7, line.find(' ', hPos + 7) - (hPos + 7)));
			int xo = std::stoi(line.substr(xoPos + 8, line.find(' ', xoPos + 8) - (xoPos + 8)));
			int yo = std::stoi(line.substr(yoPos + 8, line.find(' ', yoPos + 8) - (yoPos + 8)));
			int a = std::stoi(line.substr(aPos + 9, line.find(' ', aPos + 9) - (aPos + 9)));
			pimpl->glyphs[c] = { x, y, w, h, a, xo, yo };
		}
	}
	glGenVertexArrays(1, &pimpl->vao);
	glGenBuffers(1, &pimpl->vbo);
	glBindVertexArray(pimpl->vao);
	glBindBuffer(GL_ARRAY_BUFFER, pimpl->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectVerts), rectVerts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
//	stbi_set_flip_vertically_on_load(true);
	stbi_uc * data = stbi_load(texMap.c_str(), &pimpl->mapWidth, &pimpl->mapHeight, &pimpl->mapComps, 0);
	glGenTextures(1, &pimpl->mapTexture);
	glBindTexture(GL_TEXTURE_2D, pimpl->mapTexture);
	int format = GL_RGBA;
	switch (pimpl->mapComps) {
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, format, pimpl->mapWidth, pimpl->mapHeight, 0, format, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
}


Font::~Font()
{
}

void Font::drawText(const char * txt, float x, float y, float scale)
{
	ShaderManager::getShader(shaderID::gui)->setVec2i("texData", glm::ivec2(pimpl->mapWidth, pimpl->mapHeight));
	ShaderManager::getShader(shaderID::gui)->setBool("text", true);
	glm::ivec2 screenSize;
	ShaderManager::getShader(shaderID::gui)->getVec2i("screenSize", screenSize);
	glBindTexture(GL_TEXTURE_2D, pimpl->mapTexture);
	glBindVertexArray(pimpl->vao);
	glDisable(GL_DEPTH_TEST);
	float lastX = x;
	for (size_t i = 0; i < strlen(txt); ++i) {
		if (pimpl->glyphs.find(txt[i]) != pimpl->glyphs.end()) {
			auto g = pimpl->glyphs[txt[i]];
			ShaderManager::getShader(shaderID::gui)->setVec4i("glyphData", glm::ivec4(g.x, g.y, g.width, g.height));
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(lastX, y - g.height * scale / (float)screenSize.y - g.yoffset * scale / (float)screenSize.y + pimpl->lineHeight / (float)screenSize.y, 0.0));
			model = glm::scale(model, glm::vec3(g.width / (float)screenSize.x * scale, g.height / (float)screenSize.y * scale, 1.0));
			ShaderManager::getShader(shaderID::gui)->setMat4("model", model);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			lastX += g.advance / (float)screenSize.x * scale;
		}
	}
	ShaderManager::getShader(shaderID::gui)->setBool("text", false);
	glEnable(GL_DEPTH_TEST);
}

float Font::getNDCWidth(const char * txt, float scale)
{
	glm::ivec2 screenSize;
	ShaderManager::getShader(shaderID::gui)->getVec2i("screenSize", screenSize);
	float w = 0;
	for (size_t i = 0; i < strlen(txt); ++i) {
		if (pimpl->glyphs.find(txt[i]) != pimpl->glyphs.end()) {
			auto g = pimpl->glyphs[txt[i]];
			w += g.advance / (float)screenSize.x * scale;
		}
	}
	return w + 0.005;
}
