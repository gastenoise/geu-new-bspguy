#include "Skybox.h"
#include "Bsp.h"
#include "Settings.h"
#include "util.h"
#include "log.h"
#include "stb_image.h"

static const GLenum s_cubemapTargets[SKY_NUM_FACES] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X, // ft
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // bk
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y, // up
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, // dn
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // lf
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  // rt
};

static const char* s_skySuffixes[SKY_NUM_FACES] = {
	"ft", "bk", "up", "dn", "lf", "rt"
};

Skybox::Skybox()
	: skyname(""), cubemapTexId(0), loaded(false), width(0), height(0)
{
	for (int i = 0; i < SKY_NUM_FACES; i++)
	{
		faces[i] = FaceImage();
	}
}

Skybox::~Skybox()
{
	clear();
}

void Skybox::deleteGLTexture()
{
	std::lock_guard<std::mutex> lock(loadMutex);

	if (cubemapTexId != 0)
	{
		glDeleteTextures(1, &cubemapTexId);
		cubemapTexId = 0;
	}

	loaded = false;
}

void Skybox::clear()
{
	std::lock_guard<std::mutex> lock(loadMutex);

	if (cubemapTexId != 0)
	{
		glDeleteTextures(1, &cubemapTexId);
		cubemapTexId = 0;
	}

	for (int i = 0; i < SKY_NUM_FACES; i++)
	{
		if (faces[i].data)
		{
			stbi_image_free(faces[i].data);
			faces[i].data = nullptr;
		}
		faces[i].filePath.clear();
		faces[i].w = 0;
		faces[i].h = 0;
		faces[i].channels = 0;
	}

	loaded = false;
	skyname.clear();
}

bool Skybox::resolveFacePath(Bsp* map, const std::string& name, const std::string& suffix, const std::string& customDir, std::string& outPath)
{
	static const std::vector<std::string> extensions = {
		".tga", ".png", ".bmp", ".jpg", ".jpeg",
		".TGA", ".PNG", ".BMP", ".JPG", ".JPEG"
	};

	std::vector<std::string> baseNames = {
		name + suffix,
		name + "_" + suffix,
		toLowerCase(name) + suffix,
		toLowerCase(name) + "_" + suffix
	};

	for (const auto& base : baseNames)
	{
		for (const auto& ext : extensions)
		{
			std::string filename = base + ext;

			// 1. Check custom skybox directory if provided
			if (!customDir.empty())
			{
				std::string candidate = customDir + "/" + filename;
				if (fileExists(candidate))
				{
					outPath = candidate;
					return true;
				}
				candidate = customDir + "/gfx/env/" + filename;
				if (fileExists(candidate))
				{
					outPath = candidate;
					return true;
				}
				candidate = customDir + "/env/" + filename;
				if (fileExists(candidate))
				{
					outPath = candidate;
					return true;
				}
			}

			// 2. Check game directory (gfx/env/ and env/)
			if (!g_settings.gamedir.empty())
			{
				std::string candidate = g_settings.gamedir + "gfx/env/" + filename;
				if (fileExists(candidate))
				{
					outPath = candidate;
					return true;
				}
				candidate = g_settings.gamedir + "env/" + filename;
				if (fileExists(candidate))
				{
					outPath = candidate;
					return true;
				}
				candidate = g_settings.gamedir + filename;
				if (fileExists(candidate))
				{
					outPath = candidate;
					return true;
				}
			}

			// 3. Search via FindPathInAssets
			if (FindPathInAssets(map, "gfx/env/" + filename, outPath))
			{
				return true;
			}
			if (FindPathInAssets(map, "env/" + filename, outPath))
			{
				return true;
			}
			if (FindPathInAssets(map, filename, outPath))
			{
				return true;
			}
		}
	}

	return false;
}

bool Skybox::loadFacesForSkyname(Bsp* map, const std::string& name, const std::string& customDir)
{
	std::string paths[SKY_NUM_FACES];

	for (int i = 0; i < SKY_NUM_FACES; i++)
	{
		if (!resolveFacePath(map, name, s_skySuffixes[i], customDir, paths[i]))
		{
			return false;
		}
	}

	int firstW = 0, firstH = 0;

	for (int i = 0; i < SKY_NUM_FACES; i++)
	{
		int w = 0, h = 0, channels = 0;
		unsigned char* data = stbi_load(paths[i].c_str(), &w, &h, &channels, 4);
		if (!data)
		{
			print_log(PRINT_RED, "Failed to load skybox image: {}\n", paths[i]);
			for (int j = 0; j < i; j++)
			{
				if (faces[j].data)
				{
					stbi_image_free(faces[j].data);
					faces[j].data = nullptr;
				}
			}
			return false;
		}

		if (i == 0)
		{
			firstW = w;
			firstH = h;
		}

		faces[i].data = data;
		faces[i].w = w;
		faces[i].h = h;
		faces[i].channels = 4;
		faces[i].filePath = paths[i];
	}

	this->width = firstW;
	this->height = firstH;
	this->skyname = name;
	return true;
}

bool Skybox::load(Bsp* map, const std::string& skyName, const std::string& customDir)
{
	std::lock_guard<std::mutex> lock(loadMutex);

	// Free any previous CPU data
	for (int i = 0; i < SKY_NUM_FACES; i++)
	{
		if (faces[i].data)
		{
			stbi_image_free(faces[i].data);
			faces[i].data = nullptr;
		}
	}

	std::string targetSky = skyName.empty() ? "desert" : skyName;

	// Attempt primary skyname
	if (loadFacesForSkyname(map, targetSky, customDir))
	{
		print_log(PRINT_GREEN, "Loaded skybox '{}' ({}x{})\n", targetSky, width, height);
		return true;
	}

	// Fallback to "desert" if target was different
	if (toLowerCase(targetSky) != "desert")
	{
		print_log(PRINT_RED | PRINT_GREEN | PRINT_INTENSITY, "Skybox '{}' not found, falling back to 'desert'...\n", targetSky);
		if (loadFacesForSkyname(map, "desert", customDir))
		{
			print_log(PRINT_GREEN | PRINT_INTENSITY, "Loaded fallback skybox 'desert' ({}x{})\n", width, height);
			return true;
		}
	}

	print_log(PRINT_RED | PRINT_GREEN | PRINT_INTENSITY, "No 3D skybox images found for '{}' or fallback 'desert'. Using flat sky.\n", targetSky);
	return false;
}

void Skybox::upload()
{
	std::lock_guard<std::mutex> lock(loadMutex);

	if (cubemapTexId != 0)
	{
		glDeleteTextures(1, &cubemapTexId);
		cubemapTexId = 0;
	}

	bool hasAllFaces = true;
	for (int i = 0; i < SKY_NUM_FACES; i++)
	{
		if (!faces[i].data)
		{
			hasAllFaces = false;
			break;
		}
	}

	if (!hasAllFaces)
	{
		loaded = false;
		return;
	}

	glGenTextures(1, &cubemapTexId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexId);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (int i = 0; i < SKY_NUM_FACES; i++)
	{
		glTexImage2D(s_cubemapTargets[i], 0, GL_RGBA, faces[i].w, faces[i].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, faces[i].data);
		stbi_image_free(faces[i].data);
		faces[i].data = nullptr;
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	loaded = true;
}

void Skybox::bind(GLuint unit)
{
	if (loaded && cubemapTexId != 0)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexId);
	}
}
