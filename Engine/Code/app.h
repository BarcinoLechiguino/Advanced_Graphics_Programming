#ifndef __APP_H__
#define __APP_H__

#include "base_types.h"
#include "shader_types.h"
#include "input.h"

struct App
{
public:
    f32  deltaTime;                                                     // Loop
    bool isRunning;                                                     // ----

    Input input;                                                        // Input

    ivec2 displaySize;                                                  // Window (?)

public:                                                                 // OpenGL Vars -----------------------------
    Mode mode;                                                          // Rendering mode.

    u32 texturedGeometryProgramIdx;                                     // Index of a given program.
    u32 diceTexIdx;                                                     // Buffer index of the dice texture.
    u32 whiteTexIdx;                                                    // Buffer index of the white texture.
    u32 blackTexIdx;                                                    // Buffer index of the black texture.
    u32 normalTexIdx;                                                   // Buffer index of the normal texture.
    u32 magentaTexIdx;                                                  // Buffer index of the magenta texture.

    GLuint embeddedVertices;                                            // Embedded geometry (in-editor simple meshes such 
    GLuint embeddedElements;                                            // as a screen filling quad, a cube, a sphere...).

    GLuint programUniformTexture;                                       // Location of the texture uniform in the textured quad shader
    
    std::vector<Texture>    textures;
    std::vector<Material>   materials;
    std::vector<Mesh>       meshes;
    std::vector<Model>      models;
    std::vector<Program>    programs;

    // program indices
    u32 texturedGeometryProgramIdx;
    u32 texturedMeshProgramIdx;

    GLuint vao;                                                         // VAO object to link our screen filling quad with our textured quad shader

public:                                                                 // Containers ------------------------------
    std::vector<Texture>  textures;                                     // Will store all active textures.
    std::vector<Program>  programs;                                     // Will store all active programs.

public:                                                                 // Debug -----------------------------------
    bool enableDebugGroups;                                             // Enable/Disable debugging with an external tool.
    char gpuName[64];                                                   // GPU name string.
    char openGlVersion[64];                                             // OpenGL version string.
};

#endif // !__APP_H__