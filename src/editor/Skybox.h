#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <GL/glew.h>

class Bsp;

enum SkyboxFaceIndex : int
{
	SKY_FRONT = 0, // ft -> GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
	SKY_RIGHT = 1, // rt -> GL_TEXTURE_CUBE_MAP_POSITIVE_X
	SKY_BACK = 2,  // bk -> GL_TEXTURE_CUBE_MAP_POSITIVE_Z
	SKY_LEFT = 3,  // lf -> GL_TEXTURE_CUBE_MAP_NEGATIVE_X
	SKY_UP = 4,    // up -> GL_TEXTURE_CUBE_MAP_POSITIVE_Y
	SKY_DOWN = 5,  // dn -> GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
	SKY_NUM_FACES = 6
};

class Skybox
{
public:
	std::string skyname;
	GLuint cubemapTexId;
	bool loaded;
	int width;
	int height;

	Skybox();
	~Skybox();

	// Searches for and decodes the 6 skybox faces into CPU memory
	bool load(Bsp* map, const std::string& skyName, const std::string& customDir = "");

	// Uploads decoded pixel buffers to OpenGL cubemap on the main GL thread
	void upload();

	// Deletes OpenGL cubemap texture ID without touching CPU decoded buffers
	void deleteGLTexture();

	// Releases GPU texture and freed CPU buffers
	void clear();

	// Binds the OpenGL cubemap texture to the specified texture unit
	void bind(GLuint unit = 0);

	bool isLoaded() const
	{
		return loaded && cubemapTexId != 0;
	}

private:
	struct FaceImage
	{
		unsigned char* data;
		int w;
		int h;
		int channels;
		std::string filePath;

		FaceImage() : data(nullptr), w(0), h(0), channels(0), filePath("") {}
	};

	FaceImage faces[SKY_NUM_FACES];
	std::mutex loadMutex;

	bool loadFacesForSkyname(Bsp* map, const std::string& name, const std::string& customDir);
	bool resolveFacePath(Bsp* map, const std::string& name, const std::string& suffix, const std::string& customDir, std::string& outPath);
};
