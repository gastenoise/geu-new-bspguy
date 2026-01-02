#pragma once
#include <vector>
#include <string>
#include <map>

#include "vectors.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "primitives.h"

#pragma pack(push, 1)

// CSM v3 header (canonical format)
struct csm_header
{
    unsigned int ident;
    unsigned int version;
    unsigned int header_size;
    unsigned int flags;

    char pathes[1024];           // search paths (v3)
    unsigned int lmGroups;
    unsigned int dtGroups;       // v3
    vec3 model_mins;
    vec3 model_maxs;

    unsigned int mat_ofs;
    unsigned int mat_size;

    unsigned int faces_ofs;
    unsigned int face_size;
    unsigned int faces_count;

    unsigned int vertex_ofs;
    unsigned int vertex_size;
    unsigned int vertex_count;

    // CSM_VERSION == 3
    unsigned int sides_ofs;
    unsigned int side_size;   // sizeof(csm_side)
    unsigned int sides_count;

    unsigned int points_ofs;
    unsigned int point_size;  // sizeof(vec3)
    unsigned int points_count;

    csm_header()
    {
        memset(this, 0, sizeof(csm_header));
    }
};

struct csm_color_t
{
    csm_color_t() { r = g = b = a = 255; }
    unsigned char r, g, b, a;
};

struct csm_vertex
{
    vec3 point;
    vec3 normal;
    COLOR4 color;
    csm_vertex(vec3 p, vec3 n, COLOR4 c)
    {
        color = c;
        point = p;
        normal = n;
    }
    csm_vertex()
    {
        point = normal = vec3();
        color = COLOR4();
    }
};

struct csm_uv_t
{
    csm_uv_t() { uv[0] = uv[1] = uv[2] = vec2(); }
    vec2 uv[3];
};

struct csm_face
{
    unsigned short material;
    unsigned short flags;
    unsigned int index[3];
    int lmGroup;
    int dtGroup;
    csm_uv_t tc[2];

    csm_face()
    {
        memset(this, 0, sizeof(csm_face));
        lmGroup = -1;
        dtGroup = -1;
    }
};

struct csm_side
{
    unsigned int firstpoint;
    unsigned short numpoints;
    unsigned short material;
    int brushGroup;
    unsigned int reserved;
    vec4 vecs[2];
    csm_side() : firstpoint(0), numpoints(0), material(0), brushGroup(-1), reserved(0)
    {
        memset(vecs, 0, sizeof(vecs));
    }
};

#pragma pack(pop)

// Mesh container for rendering
struct CSM_MDL_MESH
{
    VertexBuffer* buffer;
    unsigned int matid;
    std::vector<modelVert> verts;
    Texture* texture;

    CSM_MDL_MESH()
    {
        buffer = nullptr;
        matid = 0;
        texture = nullptr;
        verts = std::vector<modelVert>();
    }

    ~CSM_MDL_MESH()
    {
        delete buffer;
        buffer = nullptr;
        matid = 0;
        texture = nullptr;
        verts.clear();
    }
};

// Side mesh for v3 sides
struct CSM_SIDE_MESH
{
    VertexBuffer* buffer;
    unsigned int matid;
    std::vector<modelVert> verts;
    Texture* texture;

    CSM_SIDE_MESH()
    {
        buffer = nullptr;
        matid = 0;
        texture = nullptr;
        verts = std::vector<modelVert>();
    }

    ~CSM_SIDE_MESH()
    {
        delete buffer;
        buffer = nullptr;
        matid = 0;
        texture = nullptr;
        verts.clear();
    }
};

#define IDCSMMODHEADER (('M'<<24)+('S'<<16)+('C'<<8)+'I')
#define IDCSM_VERSION 3

class CSMFile {
private:
    bool readed;
    std::vector<Texture*> mat_textures;
    std::vector<CSM_MDL_MESH*> model;
    std::vector<CSM_SIDE_MESH*> sideModel;
    std::string csmFilePath;

    bool read_v3(std::ifstream& file);
    bool read_v2(std::ifstream& file);

    void parseMaterialsFromString(const std::string& materialsStr);
    std::string getStringFromMaterials();
    void upload();
    void uploadSides();

    // load textures for materials
    void loadTextures();
    Texture* loadTextureForMaterial(const std::string& materialName);

public:
    CSMFile();
    CSMFile(std::string path);
    ~CSMFile();

    csm_header header;

    std::vector<std::string> materials;
    std::vector<csm_vertex> vertices;
    std::vector<csm_face> faces;

    // v3 data
    std::vector<csm_side> sides;
    std::vector<vec3> points;

    // Options
    bool showSides = false; // default off

    bool validate();
    bool read(const std::string& filePath);
    bool write(const std::string& filePath);

    void draw();

    void printInfo();
};

extern std::map<unsigned int, CSMFile*> csm_models;
CSMFile* AddNewXashCsmToRender(const std::string& path, unsigned int sum = 0);
