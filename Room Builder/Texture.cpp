#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>
#include <unordered_map>


Texture::Texture(const char * filename, int id, bool flip) : id(id), flipVertically(flip)
{
	loadFromFile(filename);
}

Texture::Texture(int w, int h, int c, int id) : width(w), height(h), channels(c), id(id)
{
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, tex);
	int format = GL_RGBA;
	if (c == 1) format = GL_RED;
	else if (c == 3) format = GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::Texture(int id) : tex(~0), id(id), flipVertically(true)
{
}

void Texture::loadFromFile(const char * filename)
{
	if (tex != ~0) glDeleteTextures(1, &tex);
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, tex);
	stbi_set_flip_vertically_on_load(flipVertically);
	stbi_uc * data = stbi_load(filename, &width, &height, &channels, 0);
	if (data == NULL) printf("Failed to load texture at: %s\n", filename);
	int format = GL_RGBA;
	switch (channels) {
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
}

void Texture::loadFromData(unsigned int width, unsigned int height, const char * data, int format)
{
	if (tex != ~0) glDeleteTextures(1, &tex);
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, tex);
	stbi_uc * fileData = (unsigned char*)data;
	this->width = width;
	this->height = height;
	if (height == 0) {
		stbi_set_flip_vertically_on_load(flipVertically);
		fileData = stbi_load_from_memory((unsigned char*)data, width, &this->width, &this->height, &this->channels, 0);
		if (fileData == NULL) printf("Could not load file\n");
		switch (this->channels) {
		case 1:
			format = GL_RED;
			break;
		case 3:
			format = GL_RGB;
			break;
		case 4:
			format = GL_RGBA;
			break;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, format, this->width, this->height, 0, format, GL_UNSIGNED_BYTE, fileData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	if (height == 0)
		stbi_image_free(fileData);
}


void Texture::bind()
{
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, tex);
//	glActiveTexture(GL_TEXTURE0);
}

Texture::~Texture()
{
	if(tex != ~0) glDeleteTextures(1, &tex);
}

struct stImpl
{
	std::unordered_map<unsigned int, unsigned int> texCount;
};

std::unique_ptr<stImpl> SharedTexture::pimpl = std::make_unique<stImpl>();

SharedTexture::SharedTexture(int id) : Texture(id)
{
}

SharedTexture::SharedTexture(int w, int h, int c, int id) : Texture(w, h, c, id)
{
	pimpl->texCount[tex] = 1;
}

SharedTexture::SharedTexture(const char * filename, int id, bool flip) : Texture(filename, id, flip)
{
	pimpl->texCount[tex] = 1;
}

SharedTexture::SharedTexture(const SharedTexture & other) : Texture(other.id)
{
	tex = other.tex;
	width = other.width;
	height = other.height;
	channels = other.channels;
	id = other.id;
	flipVertically = other.flipVertically;
	pimpl->texCount[tex]++;
}

SharedTexture & SharedTexture::operator=(const SharedTexture & other)
{
	if (tex != ~0) {
		if (--pimpl->texCount[tex] == 0)
			glDeleteTextures(1, &tex);
	}
	tex = other.tex;
	width = other.width;
	height = other.height;
	channels = other.channels;
	id = other.id;
	flipVertically = other.flipVertically;
	pimpl->texCount[tex]++;
	return *this;
}

SharedTexture::~SharedTexture()
{
	if (tex != ~0) {
		if (--pimpl->texCount[tex] == 0)
			glDeleteTextures(1, &tex);
	}
}

CubeTexture::CubeTexture(const char * filenames[6], int id, bool flip) : Texture(id)
{
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	stbi_set_flip_vertically_on_load(flip);
	for (int i = 0; i < 6; ++i) {
		stbi_uc * data = stbi_load(filenames[i], &this->width, &this->height, &this->channels, 0);
		int format = this->channels == 4 ? GL_RGBA : (this->channels == 3 ? GL_RGB : GL_RED);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, this->width, this->height, 0, format, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
}

void CubeTexture::bind()
{
	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
}

CubeTexture::~CubeTexture()
{
	glDeleteTextures(1, &tex);
}
CubeTexture::CubeTexture(int id) : Texture(id) {}
