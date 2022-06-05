#ifndef __APP_H__
#define __APP_H__

#include <map>

#include "base_types.h"
#include "math_types.h"
#include "shader_types.h"
#include "input.h"
#include "camera.h"

struct App
{
public:
    f32  deltaTime;                                                     // Loop
    bool isRunning;                                                     // ----

    Input input;                                                        // Input

public:
    ivec2 displaySize;                                                  // Window (?)

    Camera camera;

    mat4 worldMatrix;
    mat4 worldViewProjMatrix;

public:                                                                 // OpenGL Vars -----------------------------
    RENDER_MODE renderMode;
    SHADER_MODE shaderMode;                                                   // Shader mode.

    u32         texQuadProgramIdx;                                      // Index of a given geometry program.
    u32         texMeshProgramIdx;                                      // Index of a given mesh program.
    u32         texEntityProgramIdx;                                    // Index of a given entity program.
    
    u32         quadTexIdx;                                             // Buffer index of the quad texture.
    
    u32         diceTexIdx;                                             // Buffer index of the dice texture.
    u32         whiteTexIdx;                                            // Buffer index of the white texture.
    u32         blackTexIdx;                                            // Buffer index of the black texture.
    u32         normalTexIdx;                                           // Buffer index of the normal texture.
    u32         magentaTexIdx;                                          // Buffer index of the magenta texture.

    GLuint      embeddedVertices;                                       // Embedded geometry (in-editor simple meshes such 
    GLuint      embeddedElements;                                       // as a screen filling quad, a cube, a sphere...).

    GLuint      texQuadProgramUniformTexture;                           // Location of the texture uniform in the textured quad shader
    GLuint      texMeshProgramUniformTexture;
    GLuint      texEntityProgramUniformTexture;

    GLuint      vaoQuad;                                                // VAO object to link our screen filling quad with our textured quad shader
    
    Buffer      cbuffer;
    GLint       maxUniformBufferSize;
    GLint       uniformBlockAlignment;

    u32         globalParamsOffset;
    u32         globalParamsSize;

    GLuint      framebufferHandle;

public:                                                                 // Containers ------------------------------
    std::vector<Entity>     entities;                                   // Will store all active entities.
    std::vector<Light>      lights;                                     // Will store all active lights.
    std::vector<Texture>    textures;                                   // Will store all active textures.
    std::vector<Material>   materials;                                  // Will store all active materials.
    std::vector<Mesh>       meshes;                                     // Will store all active meshes.
    std::vector<Model>      models;                                     // Will store all active models.
    std::vector<Program>    programs;                                   // Will store all active programs.

    u32 activeLights;

    u32 defaultMaterialIdx;
    
    u32 modelIdx;

public:                                                                 // Debug -----------------------------------
    bool enableDebugGroups;                                             // Enable/Disable debugging with an external tool.
    char gpuName[64];                                                   // GPU name string.
    char openGlVersion[64];                                             // OpenGL version string.
};

#endif // !__APP_H__