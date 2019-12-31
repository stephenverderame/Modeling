#pragma once
#include <memory>
enum textureTypeId
{
	textureType_diffuse,
	textureType_normal,
	textureType_specular,
	textureType_emission
};
class Texture
{
protected:
	unsigned int tex;
	int width, height, channels;
	int id;
	bool flipVertically;
public:
	Texture(const char * filename, int id, bool flip = true);
	Texture(int w, int h, int c, int id);
	Texture(int id = 0);
	void loadFromFile(const char * filename);
	void loadFromData(unsigned int width, unsigned int height, const char * data, int format);
	int getHeight() { return height; }
	int getWidth() { return width; }
	int getChannels() { return channels; }
	void setFlipVertical(bool f) { flipVertically = f; }
	void setId(int id) { this->id = id; }
	void bind();
	virtual ~Texture();
	Texture(const Texture & other) = delete;
	Texture& operator=(const Texture &) = delete;
};
struct stImpl;
class SharedTexture : public Texture
{
	static std::unique_ptr<stImpl> pimpl;
public:
	SharedTexture(int id = 0);
	SharedTexture(int w, int h, int c, int id);
	SharedTexture(const char * filename, int id, bool flip = true);
	SharedTexture(const SharedTexture & other);
	SharedTexture& operator=(const SharedTexture & other);
	~SharedTexture();
};

