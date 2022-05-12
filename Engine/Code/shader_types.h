#ifndef __SHADER_TYPES_H__
#define __SHADER_TYPES_H__

#include <vector>
#include <string>
#include <math.h>
#include <glad/glad.h>

#include "base_types.h"
#include "math_types.h"

// BUFFERS
struct VAO
{
    GLuint handle;
    GLuint programHandle;
};

struct VertexBufferAttribute
{
    u8 location;
    u8 componentCount;
    u8 offset;
};

struct VertexBufferLayout
{
    std::vector<VertexBufferAttribute>  attributes;
    u8                                  stride;
};

struct VertexShaderAttribute
{
    u8 location;
    u8 componentCount;
};

struct VertexShaderLayout
{
    std::vector<VertexShaderAttribute> attributes;
};

// VISUALS
struct Image
{
    void* pixels;
    ivec2 size;
    i32   nchannels;
    i32   stride;
};

struct Texture
{
    GLuint      handle;
    std::string filepath;
};

struct Material
{
    std::string name;
    vec3        albedo;
    vec3        emissive;
    f32         smoothness;
    u32         albedoTexIdx;
    u32         emissiveTexIdx;
    u32         normalTexIdx;
    u32         bumpTexIdx;
};

struct Submesh
{
    VertexBufferLayout  VBL;                    // Vertex Buffer Layout
    std::vector<float>  vertices;               // Create Vertex struct?
    std::vector<u32>    indices;
    u32                 vertexOffset;
    u32                 indexOffset;

    std::vector<VAO>    vaos;
};

struct Mesh
{
    std::vector<Submesh> submeshes;
    GLuint               vertexBufferHandle;
    GLuint               indexBufferHandle;
};

struct Model
{
    u32              meshIdx;
    std::vector<u32> materialIndices;
};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp;  // Hot-reloading check.

    VertexBufferLayout VIL;                 // Vertex Input Layout.
};

struct VertexV3V2
{
    vec3 pos;
    vec2 uv;
};

enum Mode
{
    Mode_TexturedQuad,
    Mode_Count
};

#endif // !__SHADER_TYPES_H__