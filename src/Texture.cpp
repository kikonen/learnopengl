#include "Texture.h"

#include <mutex>
#include <glad/glad.h>

#include "Shader.h"

const int UNIT_IDS[] = {
	GL_TEXTURE0,
	GL_TEXTURE1,
	GL_TEXTURE2,
	GL_TEXTURE3,
	GL_TEXTURE4,
	GL_TEXTURE5,
	GL_TEXTURE6,
	GL_TEXTURE7,
	GL_TEXTURE8,
	GL_TEXTURE9,
	GL_TEXTURE10,
	GL_TEXTURE11,
	GL_TEXTURE12,
	GL_TEXTURE13,
	GL_TEXTURE14,
	GL_TEXTURE15,
	GL_TEXTURE16,
	GL_TEXTURE17,
	GL_TEXTURE18,
	GL_TEXTURE19,
	GL_TEXTURE20,
	GL_TEXTURE21,
	GL_TEXTURE22,
	GL_TEXTURE23,
	GL_TEXTURE24,
	GL_TEXTURE25,
	GL_TEXTURE26,
	GL_TEXTURE27,
	GL_TEXTURE28,
	GL_TEXTURE29,
	// NOTE KI 30 == depthMap
	GL_TEXTURE30,
	// NOTE KI 31 == skybox
	GL_TEXTURE31,
};

static std::map<std::string, Texture*> textures;
static std::map<std::string, Texture*> normals;

static std::mutex textures_lock;

Texture* Texture::getTexture(const std::string& path, int textureMode, bool normalMap)
{
	std::lock_guard<std::mutex> lock(textures_lock);

	std::string cacheKey = path + "_" + std::to_string(textureMode);
	std::map<std::string, Texture*>& cache = normalMap ? normals : textures;

	Texture* tex = cache[path];
	if (!tex) {
		tex = new Texture(path, normalMap, textureMode);
		cache[path] = tex;
	}
	return tex;
}

Texture::Texture(const std::string& path, int textureMode, bool normalMap)
	: path(path),
	textureMode(textureMode),
	normalMap(normalMap)
{
}

Texture::~Texture()
{
	delete image;
	image = NULL;
}

void Texture::prepare()
{
	if (prepared) {
		return;
	}
	prepared = true;

	if (!image) {
		return;
	}

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//	float borderColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
//	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	if (image->channels == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
	}

	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::bind(Shader* shader)
{
	glActiveTexture(GL_TEXTURE0 + unitIndex);
	glBindTexture(GL_TEXTURE_2D, textureID);
}

int Texture::load() {
	Image* tmp = Image::getImage(path);
	int res = tmp->load(true);
	if (!res) {
		image = tmp;
	}
	return res;
}
