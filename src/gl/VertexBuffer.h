#pragma once

#include <vector>
#include <GL/glew.h>
#include "ShaderProgram.h"
#include "util.h"

// VertexBuffer manages a VBO + VAO and the CPU-side vertex data pointer.
// Notes:
//  - The class does not copy the provided data by default; use takeOwnership=true
//    in setData if you want the VertexBuffer to free the memory.
class VertexBuffer
{
public:
    // Shader program used to query attribute locations and element size
    ShaderProgram* shaderProgram;

    // Number of vertices currently referenced by 'data' / stored in the VBO
    int numVerts;

    // Primitive type used by drawFull (e.g., GL_TRIANGLES)
    int primitive;

    // Frame id used to avoid drawing the same buffer multiple times per frame
    int frameId;

    // Constructor: shaderProgram may be nullptr; dat may be nullptr.
    // primitive defaults to 0 (caller should pass a GL primitive enum).
    // If takeOwnership is true, VertexBuffer will delete[] the pointer 
    // when replaced or on destruction.
    VertexBuffer(ShaderProgram* shaderProgram, void* dat, int numVerts, int primitive, bool takeOwnership);
    VertexBuffer(ShaderProgram* shaderProgram, int primitive);
    ~VertexBuffer();

    // Set the CPU-side data pointer. If takeOwnership is true, VertexBuffer will
    // delete[] the pointer when replaced or on destruction.
    void setData(void* data, int numVerts, bool takeOwnership);

    // Return CPU-side data. If data was freed but VBO exists, this will try to
    // read back the buffer contents into a newly allocated array.
    unsigned char* getData();

    // Upload vertex data to GPU. If forceReupload is true, reupload even if
    // already uploaded. hideErrors suppresses logging of GL errors.
    void upload(bool hideErrors = true, bool forceReupload = false);

    // Mark buffer as needing reupload (will upload on next draw/upload call)
    void reupload();

    // Draw a subrange [start, end). start is inclusive, end is exclusive.
    // hideErrors controls logging behavior.
    void drawRange(int primitive, int start, int end, bool hideErrors = true);

    // Convenience draws
    void draw(int primitive);
    void drawFull();

private:
    GLsizeiptr lastBufferSize = 0; // for glBufferSubData optimization
    bool ownData;            // true if buffer should delete data on destruction
    bool uploaded;           // true if data has been uploaded to GPU
    unsigned char* data;     // CPU-side pointer to vertex data (may be nullptr)
    GLuint vboId;            // 0 means not created yet
    GLuint vaoId;            // 0 means not created yet

    // Helper to add vertex attributes based on flags (implementation detail)
    void addAttributes(int attFlags);

    VertexBuffer(const VertexBuffer&) = delete; 
    VertexBuffer& operator=(const VertexBuffer&) = delete;
};
