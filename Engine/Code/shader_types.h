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
    GLuint ID;
    GLuint programID;
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

};

struct Submesh
{

};

struct Mesh
{

};

struct Model
{

};

struct Program
{
    GLuint             handle;
    std::string        filepath;
    std::string        programName;
    u64                lastWriteTimestamp; // What is this for?
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