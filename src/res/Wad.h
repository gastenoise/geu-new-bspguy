#pragma once
#include <unordered_map>
#include <string>
#include "bsptypes.h"

#pragma pack(push, 1)

COLOR4 operator*(COLOR4 v, float f);
bool operator==(COLOR4 c1, COLOR4 c2);

struct WADHEADER
{
	char szMagic[4];    // should be WAD2/WAD3
	int nDir;			// number of directory entries
	int nDirOffset;		// offset into directories
};

struct WADDIRENTRY
{
	int nFilePos;				 // offset in WAD
	int nDiskSize;				 // size in file
	int nSize;					 // uncompressed size
	char nType;					 // type of entry
	bool bCompression;           // 0 if none
	short nDummy;				 // not used
	char szName[MAXTEXTURENAME]; // must be null terminated
};

struct WADTEX
{
	char szName[MAXTEXTURENAME];
	int nWidth, nHeight;
	int nOffsets[MIPLEVELS];
	std::vector<unsigned char> data;
	WADTEX();
	WADTEX(BSPMIPTEX* tex, unsigned char* palette = NULL, unsigned short colors = 256);
	~WADTEX() = default;
};

#pragma pack(pop)


class Wad
{
public:
	enum CacheMode {
		CACHE_NONE = 0,
		CACHE_ALL = 1,
		CACHE_LAZY = 2 
	};

	std::string filename = std::string();
	std::string wadname = std::string();

	bool usableTextures = false;
	bool fileLoadedInMemory = false;

	WADHEADER header = WADHEADER();
	std::vector<WADDIRENTRY> dirEntries = std::vector<WADDIRENTRY>();

	std::unordered_map<std::string, WADTEX> textureCache;
	CacheMode cacheMode = CACHE_LAZY;

	std::vector<unsigned char> fileData;

	Wad(std::string file, CacheMode mode = CACHE_LAZY);
	Wad();

	~Wad() = default;

	bool readInfo();
	bool loadFullFile();
	void unloadFile();

	void clearCache();
	void precacheAllTextures(); 

	bool hasTexture(int dirIndex);
	bool hasTexture(const std::string& name);

	bool write(const std::string& filename, const std::vector<WADTEX>& textures);
	bool write(const std::vector<WADTEX>& textures);

	WADTEX readTexture(int dirIndex, int* texturetype = NULL);
	WADTEX readTexture(const std::string& texname, int* texturetype = NULL);

	WADTEX readTextureFromFile(int dirIndex);
	WADTEX readTextureFromFile(const std::string& texname);

private:
	WADTEX readTextureFromMemory(int dirIndex);
	WADTEX readTextureFromMemory(const std::string& texname);

	int findTextureIndex(const std::string& texname);
};