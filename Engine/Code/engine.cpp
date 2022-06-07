//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include "imgui_includes.h"

#include "globals.h"
#include "app.h"
#include "buffer_manager.h"
#include "file_manager.h"
#include "importer.h"
#include "transform.h"
#include "camera.h"
#include "primitives.h"

#include "engine.h"

void Engine::Init(App* app)
{
    app->enableDebugGroups  = false;
    app->refreshFramebuffer = true;

    app->useNormalMap   = false;
    app->useBumpMap     = false;
    app->bumpiness      = 1.0f;

    app->renderMode  = RENDER_MODE::DEFERRED;
    app->renderLayer = RENDER_LAYER::SHADED;
    app->shaderMode  = SHADER_MODE::ENTITIES;

    Camera::InitCamera(app);
    Camera::InitWorldTransform(app);
    
    Renderer::InitFramebuffer(app);
    
    Shaders::LoadBaseTextures(app);
    Shaders::CreateDefaultMaterial(app);

    Primitives::InitPrimitivesData(app);
    

    const char* texPath     = "dice.png";
    const char* meshPath    = "Patrick/Patrick.obj";

    switch (app->shaderMode)
    {
    case SHADER_MODE::QUAD:     { Renderer::InitQuad(app, texPath); }  break;
    case SHADER_MODE::MESH:     { Renderer::InitMesh(app, meshPath); } break;
    case SHADER_MODE::ENTITIES: { Renderer::InitEntities(app); }       break;
    default:                    { /* NOTHING FOR NOW */ };
    }
}

void Engine::Update(App* app)
{   
    Input::GetInput(app);
    
    if (app->refreshFramebuffer)
    {
        Renderer::RefreshFramebuffer(app);
        app->refreshFramebuffer = false;
    }

    app->camera.SetViewMatrix(glm::lookAt(app->camera.position, app->camera.target, Transform::upVector));

    if (app->shaderMode == SHADER_MODE::ENTITIES)
    {
        (!Renderer::InDeferredMode(app)) ? Shaders::ForwardUniformBlockBuffer(app) : Shaders::DeferredUniformBlockBuffer(app);
    }

    Shaders::HotReloading(app);
}

void Engine::Render(App* app)
{
    if (app->enableDebugGroups)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Shaded Model");
    }
    
    //glPushMatrix();
    //glMultMatrixf((GLfloat*)&Transform::PositionScale(app->camera.position, Transform::defaultScale)[0]);

    switch (app->shaderMode)
    {
    case SHADER_MODE::QUAD:     { Renderer::RenderQuad(app); }     break;
    case SHADER_MODE::MESH:     { Renderer::RenderMesh(app); }     break;
    case SHADER_MODE::ENTITIES: { Renderer::RenderEntities(app); } break;
    default:                    { /*NOTHING AT THE MOMENT*/ };
    }

    //glPopMatrix();

    if (app->enableDebugGroups)
    {
        glPopDebugGroup();
    }

    static bool done = false;
    if (!done)
    {
        GLenum error = glGetError();
        for (; error != GL_NO_ERROR; error = glGetError())
        {
            ELOG("DETECTED ERROR: %u", error);
        }

        done = true;
    }
}

void Engine::DrawGui(App* app)
{
    Gui::GeneralTab(app);
    Gui::ExtensionsTab(app);
}

// SHADERS ---
GLuint Engine::CreateProgramFromSource(String programSource, const char* shaderName)
{
    GLchar  infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint   success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(vertexShaderDefine),
        (GLint)programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(fragmentShaderDefine),
        (GLint)programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 Engine::LoadProgram(App* app, const char* filepath, const char* programName)
{
    String programSource = FileManager::ReadTextFile(filepath);

    Program program = {};
    program.handle = CreateProgramFromSource(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = FileManager::GetFileLastWriteTimestamp(filepath);
    app->programs.push_back(program);

    //VertexShaderLayout VSL = {};
    //VSL.attributes

    return app->programs.size() - 1;
}

bool Engine::UniformIsInvalid(GLuint uniformHandle)
{
    return (uniformHandle == GL_INVALID_VALUE || uniformHandle == GL_INVALID_OPERATION);
}

GLuint Engine::CreateVAO(Mesh& mesh, Submesh& submesh, const Program& program)
{
    GLuint vaoHandle = 0;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);

    for (u32 i = 0; i < program.VIL.attributes.size(); ++i)
    {
        bool attributeWasLinked = false;
        for (u32 j = 0; j < submesh.VBL.attributes.size(); ++j)
        {
            if (program.VIL.attributes[i].location == submesh.VBL.attributes[j].location)
            {
                const u32 index     = submesh.VBL.attributes[j].location;
                const u32 ncomp     = submesh.VBL.attributes[j].componentCount;
                const u32 offset    = submesh.VBL.attributes[j].offset + submesh.vertexOffset;
                const u32 stride    = submesh.VBL.stride;

                glVertexAttribPointer(index, ncomp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                glEnableVertexAttribArray(index);

                attributeWasLinked = true;

                break;
            }
        }

        assert(attributeWasLinked);
    }

    glBindVertexArray(0);

    return vaoHandle;
}

GLuint Engine::FindVAO(Mesh& mesh, u32 submeshIndex, const Program& program)
{
    Submesh& submesh = mesh.submeshes[submeshIndex];
    for (u32 i = 0; i < (u32)submesh.vaos.size(); ++i)
    {
        if (submesh.vaos[i].programHandle == program.handle)
        {
            return submesh.vaos[i].handle;
        }
    }

    GLuint vaoHandle = CreateVAO(mesh, submesh, program);

    VAO vao = { vaoHandle, program.handle };
    submesh.vaos.push_back(vao);

    return vaoHandle;
}

// INPUT ----------------------------------------------------------------------
void Engine::Input::GetInput(App* app)
{   
    // DEBUG
    if (app->input.keys[K_SPACE] == BUTTON_PRESS) { app->enableDebugGroups = !app->enableDebugGroups; }

    // CAMERA
    vec3 position = app->camera.GetPosition();
    
    if (app->input.keys[K_W] == BUTTON_PRESSED) { position.z -= app->camera.moveSpeed * app->deltaTime; }
    if (app->input.keys[K_A] == BUTTON_PRESSED) { position.x -= app->camera.moveSpeed * app->deltaTime; }
    if (app->input.keys[K_S] == BUTTON_PRESSED) { position.z += app->camera.moveSpeed * app->deltaTime; }
    if (app->input.keys[K_D] == BUTTON_PRESSED) { position.x += app->camera.moveSpeed * app->deltaTime; }
    if (app->input.keys[K_E] == BUTTON_PRESSED) { position.y -= app->camera.moveSpeed * app->deltaTime; }
    if (app->input.keys[K_Q] == BUTTON_PRESSED) { position.y += app->camera.moveSpeed * app->deltaTime; }

    app->camera.SetPosition(position);

    // RENDER MODE
    if (app->input.keys[K_M] == BUTTON_PRESS) 
    { 
        app->renderMode = (Renderer::InDeferredMode(app)) ? RENDER_MODE::FORWARD : RENDER_MODE::DEFERRED;
    }

    // RENDER LAYER
    if (app->input.keys[K_L] == BUTTON_PRESS)
    {
        u32 newMode = (u32)app->renderLayer + 1;
        app->renderLayer = (newMode == (u32)RENDER_LAYER::DEFAULT) ? (RENDER_LAYER)0 : (RENDER_LAYER)newMode;
    }

    // MAPS
    if (app->input.keys[K_N] == BUTTON_PRESS) { app->useNormalMap = !app->useNormalMap; }
    if (app->input.keys[K_B] == BUTTON_PRESS) { app->useBumpMap = !app->useBumpMap; }
}

// CAMERA ----------------------------------------------------------------------
void Engine::Camera::InitCamera(App* app)
{
    app->camera.SetAspectRatio((float)app->displaySize.x / (float)app->displaySize.y);
    app->camera.SetProjMatrix(glm::radians(60.0f));
    app->camera.SetViewMatrix(glm::lookAt(app->camera.position, app->camera.target, Transform::upVector));
}

void Engine::Camera::InitWorldTransform(App* app)
{
    app->worldMatrix            = Transform::PositionScale(Transform::upVector, Transform::defaultScale);
    app->worldViewProjMatrix    = app->camera.GetProjMatrix() * app->camera.GetViewMatrix() * app->worldMatrix;
}

// SHADERS ----------------------------------------------------------------------
void Engine::Shaders::LoadBaseTextures(App* app)
{
    app->diceTexIdx     = Importer::LoadTexture2D(app, "dice.png");
    app->whiteTexIdx    = Importer::LoadTexture2D(app, "color_white.png");
    app->blackTexIdx    = Importer::LoadTexture2D(app, "color_black.png");
    app->normalTexIdx   = Importer::LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx  = Importer::LoadTexture2D(app, "color_magenta.png");
}

void Engine::Shaders::CreateDefaultMaterial(App* app)
{
    app->materials.push_back(Material{});
    Material& material = app->materials.back();
    app->defaultMaterialIdx = app->materials.size() - 1u;

    material.name       = "Default";
    material.albedo     = vec3(1.0f, 1.0f, 1.0f);
    material.emissive   = vec3(1.0f, 1.0f, 1.0f);
    material.smoothness = 1.0f;
    
    material.albedoTexIdx = app->whiteTexIdx;
}

void Engine::Shaders::GetProgramAttributes(App* app, GLuint programHandle, GLuint& programUniformTexture)
{
    Program& program        = app->programs[programHandle];
    programUniformTexture   = glGetUniformLocation(program.handle, "uTexture");

    GLint   attributeCount = 0;
    glGetProgramiv(program.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    for (GLint i = 0; i < attributeCount; ++i)
    {
        char    attributeName[128];
        GLsizei attributeNameLength = 0;
        GLint   attributeSize = 0;
        GLint   attributeLocation = 0;
        GLenum  attributeType;

        glGetActiveAttrib(program.handle, i,
                            ARRAY_COUNT(attributeName),
                            &attributeNameLength,
                            &attributeSize,
                            &attributeType,
                            attributeName);

        attributeLocation = glGetAttribLocation(program.handle, attributeName);
        glVertexAttribPointer(attributeLocation, attributeSize, attributeType, GL_FALSE, sizeof(float) * 5, (void*)0);          // Is this necessary?

        program.VIL.attributes.push_back({ (u8)attributeLocation, (u8)attributeSize });
    }
}

void Engine::Shaders::InitUniformBlockBuffer(App* app)
{
    glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &app->maxUniformBufferSize);
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &app->uniformBlockAlignment);
}

void Engine::Shaders::ForwardUniformBlockBuffer(App* app)
{
    BufferManager::MapBuffer(app->cbuffer, GL_WRITE_ONLY);
    app->globalParamsOffset = app->cbuffer.head;

    PushVec3(app->cbuffer, app->camera.position);
    PushUInt(app->cbuffer, (u32)app->renderLayer);
    PushUInt(app->cbuffer, (u32)app->activeLights);
    for (u32 i = 0; i < app->activeLights; ++i)
    {
        BufferManager::AlignHead(app->cbuffer, sizeof(vec4));

        Light& light = app->lights[i];
        PushUInt(app->cbuffer, light.type);
        PushVec3(app->cbuffer, light.color);
        PushVec3(app->cbuffer, light.direction);
        PushVec3(app->cbuffer, light.position);
    }

    app->globalParamsSize = app->cbuffer.head - app->globalParamsOffset;

    for (u32 i = 0; i < app->entities.size(); ++i)
    {
        BufferManager::AlignHead(app->cbuffer, app->uniformBlockAlignment);

        Entity& entity = app->entities[i];
        entity.localParamsOffset = app->cbuffer.head;

        PushMat4(app->cbuffer, entity.worldMatrix);
        PushMat4(app->cbuffer, app->camera.GetProjMatrix() * app->camera.GetViewMatrix() * entity.worldMatrix);

        entity.localParamsSize = app->cbuffer.head - entity.localParamsOffset;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Engine::Shaders::DeferredUniformBlockBuffer(App* app)
{
    BufferManager::MapBuffer(app->cbuffer, GL_WRITE_ONLY);
    app->globalParamsOffset = app->cbuffer.head;

    PushVec3(app->cbuffer, app->camera.position);
    PushUInt(app->cbuffer, (u32)app->renderLayer);

    app->globalParamsSize = app->cbuffer.head - app->globalParamsOffset;

    for (u32 i = 0; i < app->entities.size(); ++i)
    {
        BufferManager::AlignHead(app->cbuffer, app->uniformBlockAlignment);

        Entity& entity = app->entities[i];
        entity.localParamsOffset = app->cbuffer.head;

        PushMat4(app->cbuffer, entity.worldMatrix);
        PushMat4(app->cbuffer, app->camera.GetProjMatrix() * app->camera.GetViewMatrix() * entity.worldMatrix);
        PushUInt(app->cbuffer, (i == 3) ? 0 : 1);

        entity.localParamsSize = app->cbuffer.head - entity.localParamsOffset;
    }

    for (u32 i = 0; i < app->activeLights; ++i)
    {
        BufferManager::AlignHead(app->cbuffer, app->uniformBlockAlignment);

        Light& light = app->lights[i];
        light.localParamsOffset = app->cbuffer.head;

        PushMat4(app->cbuffer, light.worldMatrix);
        PushMat4(app->cbuffer, app->camera.GetProjMatrix() * app->camera.GetViewMatrix() * light.worldMatrix);

        PushUInt(app->cbuffer, light.type);
        PushVec3(app->cbuffer, light.color);
        PushVec3(app->cbuffer, light.direction);
        PushVec3(app->cbuffer, light.position);

        light.localParamsSize = app->cbuffer.head - light.localParamsOffset;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Engine::Shaders::HotReloading(App* app)
{
    for (u64 i = 0; i < app->programs.size(); ++i)
    {
        Program& program = app->programs[i];
        u64 currentTimestamp = FileManager::GetFileLastWriteTimestamp(program.filepath.c_str());
        if (currentTimestamp < program.lastWriteTimestamp)
        {
            glDeleteProgram(program.handle);
            String programSource = FileManager::ReadTextFile(program.filepath.c_str());
            const char* programName = program.programName.c_str();
            program.handle = CreateProgramFromSource(programSource, programName);
            program.lastWriteTimestamp = currentTimestamp;
        }
    }
}

// ENTITIES --------------------------------------------------------------------
void Engine::Lights::AddLight(App* app, LIGHT_TYPE type, vec3 color, vec3 direction, vec3 position, mat4 worldMatrix)
{
    Light light         = {};
    light.type          = type;
    light.color         = color;
    light.direction     = direction;
    light.position      = position;
    light.worldMatrix   = worldMatrix;

    app->lights.push_back(light);
    ++app->activeLights;
}

// RENDERER --------------------------------------------------------------------
void Engine::Renderer::InitFramebuffer(App* app)
{
    // GENERATING TEXTURES
    GenerateFramebufferTexture(app->GShadedTex,    app->displaySize,   GL_UNSIGNED_BYTE);
    GenerateFramebufferTexture(app->GAlbedoTex,    app->displaySize,   GL_UNSIGNED_BYTE);
    GenerateFramebufferTexture(app->GNormalTex,    app->displaySize,   GL_UNSIGNED_BYTE);
    GenerateFramebufferTexture(app->GDepthTex,     app->displaySize,   GL_UNSIGNED_BYTE);
    GenerateFramebufferTexture(app->GPositionTex,  app->displaySize,   GL_FLOAT);

    // DEPTH BUFFER ATTACHMENT
    glGenTextures(1, &app->depthBufferHandle);
    glBindTexture(GL_TEXTURE_2D, app->depthBufferHandle);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, app->displaySize.x, app->displaySize.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    // ATTACHING ELEMENTS TO FRAMEBUFFER
    glGenFramebuffers(1, &app->framebufferHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, app->framebufferHandle);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,  app->GShadedTex,        0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,  app->GAlbedoTex,        0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,  app->GNormalTex,        0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3,  app->GDepthTex,         0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4,  app->GPositionTex,      0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,   app->depthBufferHandle, 0);

    CheckFramebufferStatus();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::Renderer::ClearFramebuffer(App* app)
{
    GLuint drawBuffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1
    };

    glBindFramebuffer(GL_FRAMEBUFFER, app->framebufferHandle);

    glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::Renderer::FreeFramebuffer(App* app)
{   
    GLuint textures[] = {
       app->GShadedTex,
       app->GNormalTex,
       app->GAlbedoTex,
       app->GDepthTex,
       app->GPositionTex,
       app->depthBufferHandle
    };
    
    glDeleteTextures(ARRAY_COUNT(textures), textures);
    glDeleteFramebuffers(1, &app->framebufferHandle);
}

void Engine::Renderer::GenerateFramebufferTexture(GLuint& texHandle, ivec2 size, GLint type)
{
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Engine::Renderer::CheckFramebufferStatus()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
        case GL_FRAMEBUFFER_UNDEFINED:                      { ELOG("GL_FRAMEBUFFER_UNDEFINED"); }                       break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          { ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"); }           break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  { ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"); }   break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         { ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"); }          break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         { ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"); }          break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:         { ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"); }          break;
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:       { ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"); }        break;
        default:                                            { ELOG("[UNKNOWN FRAMEBUFFER ERROR]"); }
        }
    }
}

void Engine::Renderer::InitQuad(App* app, const char* texPath)
{
    const Vertex vertices[] = {// Positions              Normals              UVs             Tangents             BiTangents
                                { vec3(-0.5, -0.5, 0.0), vec3(0.0, 0.0, 1.0), vec2(0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) },          // 0
                                { vec3( 0.5, -0.5, 0.0), vec3(0.0, 0.0, 1.0), vec2(1.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) },          // 1
                                { vec3( 0.5,  0.5, 0.0), vec3(0.0, 0.0, 1.0), vec2(1.0, 1.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) },          // 2
                                { vec3(-0.5,  0.5, 0.0), vec3(0.0, 0.0, 1.0), vec2(0.0, 1.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) }           // 3
                              };

    const u32 indices[]     = {
                                0, 1, 2,
                                0, 2, 3
                              };

    // VERTEX BUFFER
    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // INDEX BUFFER
    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // VAO
    glGenVertexArrays(1, &app->vaoQuad);
    glBindVertexArray(app->vaoQuad);

    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);                                                       // --------------------------------------
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);                                  // Positions
    glEnableVertexAttribArray(0);                                                                               // 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));                // Normals
    glEnableVertexAttribArray(1);                                                                               // 
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));                // UVs
    glEnableVertexAttribArray(2);                                                                               // 
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 8));                // Tangents
    glEnableVertexAttribArray(3);                                                                               // 
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 11));               // Bitangents
    glEnableVertexAttribArray(4);                                                                               // 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);                                               // --------------------------------------

    glBindVertexArray(0);

    // PROGRAM
    app->texQuadProgramIdx      = LoadProgram(app, "shader_base.glsl", "TEXTURED_GEOMETRY");
    Program& texQuadProgram     = app->programs[app->texQuadProgramIdx];
    app->texQuadProgramUniformTexture  = glGetUniformLocation(texQuadProgram.handle, "uTexture");

    if (UniformIsInvalid(app->texQuadProgramUniformTexture))
    {
        ILOG("Program Uniform Texture was not valid!");
    }

    // TEXTURE
    app->quadTexIdx = Importer::LoadTexture2D(app, texPath);  
}

void Engine::Renderer::InitMesh(App* app, const char* meshPath)
{
    app->modelIdx                       = Importer::LoadModel(app, meshPath);
    app->texMeshProgramIdx              = LoadProgram(app, "shader_base.glsl", "TEXTURED_MESH");
    Program& texMeshProgram             = app->programs[app->texMeshProgramIdx];
    app->texMeshProgramUniformTexture   = glGetUniformLocation(texMeshProgram.handle, "uTexture");

    // SHADER
    GLint   attributeCount = 0;
    glGetProgramiv(texMeshProgram.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    for (GLint i = 0; i < attributeCount; ++i)
    {
        char    attributeName[128];
        GLsizei attributeNameLength = 0;
        GLint   attributeSize       = 0;
        GLint   attributeLocation   = 0;
        GLenum  attributeType;
        
        glGetActiveAttrib(texMeshProgram.handle, i,
            ARRAY_COUNT(attributeName),
            &attributeNameLength,
            &attributeSize,
            &attributeType,
            attributeName);

        attributeLocation = glGetAttribLocation(texMeshProgram.handle, attributeName);
        glVertexAttribPointer(attributeLocation, attributeSize, attributeType, GL_FALSE, sizeof(float) * 5, (void*)0);

        texMeshProgram.VIL.attributes.push_back({ (u8)attributeLocation, (u8)attributeSize });
    }
}

void Engine::Renderer::InitEntities(App* app)
{
    // INIT QUADS
    InitLightingQuad(app);
    InitFramebufferQuad(app);
    
    // MODEL LOADING
    u32 patrickModelIdx = Importer::LoadModel(app, "Patrick/Patrick.obj");
    u32 reliefCubeIdx   = Importer::LoadModel(app, "Cube/Cube.fbx");
    u32 sphereIdx       = Importer::LoadModel(app, "Sphere/Sphere.fbx");
    u32 planeIdx        = Primitives::GetPlaneIdx();
    Primitives::SetSphereIdx(sphereIdx);

    // TEXTURE LOADING
    app->reliefTexIdx   = Importer::LoadTexture2D(app, "Cube/toy_box_disp.png");
    
    // ENTITIES
    //                        NAME          WORLD MATRIX                                                                MODEL IDX           PRMS OFFSET PRMS SIZE
    app->entities.push_back({ "Patrick_1",  Transform::PositionScale({ 5.0f, 3.5f, -5.0f }, Transform::defaultScale),   patrickModelIdx,    0,          0       });
    app->entities.push_back({ "Patrick_2",  Transform::PositionScale({ 0.0f, 3.5f,  0.0f }, Transform::defaultScale),   patrickModelIdx,    0,          0       });
    app->entities.push_back({ "Patrick_3",  Transform::PositionScale({-5.0f, 3.5f, -5.0f }, Transform::defaultScale),   patrickModelIdx,    0,          0       });
    app->entities.push_back({ "ReliefCube", Transform::PositionScale({ 0.0f, 5.0f,  0.0f }, Transform::defaultScale),   reliefCubeIdx,      0,          0       });
    app->entities.push_back({ "Plane_1",    Transform::PositionScale({ 0.0f, 0.0f,  0.0f }, { 25.0f, 25.0f, 25.0f }),   planeIdx,           0,          0       });
    app->entities.push_back({ "Sphere_1",   Transform::PositionScale({ 2.0f, 2.0f,  0.0f }, Transform::defaultScale),   sphereIdx,          0,          0       });

    // LIGHTS
    //                    LIGHT TYPE        COLOR                 DIRECTION             POSITION 
    Lights::AddLight(app, LT_DIRECTIONAL, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f,  1.0f }, Transform::PositionScale({ 1.0f, 1.0f,  1.0f }, Transform::defaultScale));
    Lights::AddLight(app, LT_POINT,       { 0.5f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 3.0f, -2.0f }, Transform::PositionScale({ 0.0f, 3.0f, -2.0f }, Transform::defaultScale));
    //app->lights.push_back({ LT_DIRECTIONAL, { 1.0f, 1.0f, 1.0f }, { -1.0f, -1.0f, -1.0f },  { 1.0f, 1.0f,  1.0f } });
    //app->lights.push_back({ LT_POINT,       { 0.5f, 0.0f, 0.0f }, {  0.0f,  0.0f,  0.0f },  { 0.0f, 3.0f, -2.0f } });

    // SHADER
    app->forwardRenderingProgramIdx = LoadProgram(app, "shader_final.glsl", "FORWARD_RENDERING");
    Shaders::GetProgramAttributes(app, app->forwardRenderingProgramIdx, app->texEntityProgramUniformTexture);

    GLuint a = 0;
    app->deferredGeometryProgramIdx = LoadProgram(app, "shader_final.glsl", "GEOMETRY_PASS");
    Shaders::GetProgramAttributes(app, app->deferredGeometryProgramIdx, a);
    app->deferredLightingProgramIdx = LoadProgram(app, "shader_final.glsl", "LIGHTING_PASS");
    Shaders::GetProgramAttributes(app, app->deferredLightingProgramIdx, a);

    Shaders::InitUniformBlockBuffer(app);
    app->cbuffer = CreateConstantBuffer(app->maxUniformBufferSize);
}

void Engine::Renderer::InitLightingQuad(App* app)
{
    const Vertex vertices[] = {// Positions              Normals              UVs             Tangents             BiTangents
                                { vec3(-1.0, -1.0, 0.0), vec3(0.0, 0.0, 1.0), vec2(0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) },          // 0
                                { vec3( 1.0, -1.0, 0.0), vec3(0.0, 0.0, 1.0), vec2(1.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) },          // 1
                                { vec3( 1.0,  1.0, 0.0), vec3(0.0, 0.0, 1.0), vec2(1.0, 1.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) },          // 2
                                { vec3(-1.0,  1.0, 0.0), vec3(0.0, 0.0, 1.0), vec2(0.0, 1.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) }           // 3
                              };


    const u32 indices[]     = {
                                0, 1, 2,
                                0, 2, 3
                              };

    // VERTEX BUFFER
    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // INDEX BUFFER
    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // VAO
    glGenVertexArrays(1, &app->vaoQuad);
    glBindVertexArray(app->vaoQuad);

    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);                                                       // --------------------------------------
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);                                  // Positions
    glEnableVertexAttribArray(0);                                                                               // 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));                // Normals
    glEnableVertexAttribArray(1);                                                                               // 
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));                // UVs
    glEnableVertexAttribArray(2);                                                                               // 
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 8));                // Tangents
    glEnableVertexAttribArray(3);                                                                               // 
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 11));               // Bitangents
    glEnableVertexAttribArray(4);                                                                               // 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);                                               // --------------------------------------

    glBindVertexArray(0);
}

void Engine::Renderer::InitFramebufferQuad(App* app)
{
    const Vertex vertices[] = {// Positions              Normals              UVs             Tangents             BiTangents
                                { vec3(-1.0, -1.0, 0.0), vec3(0.0, 0.0, 1.0), vec2(0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) },          // 0
                                { vec3( 1.0, -1.0, 0.0), vec3(0.0, 0.0, 1.0), vec2(1.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) },          // 1
                                { vec3( 1.0,  1.0, 0.0), vec3(0.0, 0.0, 1.0), vec2(1.0, 1.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) },          // 2
                                { vec3(-1.0,  1.0, 0.0), vec3(0.0, 0.0, 1.0), vec2(0.0, 1.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0) }           // 3
                              };

    const u32 indices[]     = {
                                0, 1, 2,
                                0, 2, 3
                              };

    // VERTEX BUFFER
    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // INDEX BUFFER
    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // VAO
    glGenVertexArrays(1, &app->vaoFramebufferQuad);
    glBindVertexArray(app->vaoFramebufferQuad);

    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);                                                       // --------------------------------------
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);                                  // Positions
    glEnableVertexAttribArray(0);                                                                               // 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));                // Normals
    glEnableVertexAttribArray(1);                                                                               // 
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));                // UVs
    glEnableVertexAttribArray(2);                                                                               // 
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 8));                // Tangents
    glEnableVertexAttribArray(3);                                                                               // 
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 11));               // Bitangents
    glEnableVertexAttribArray(4);                                                                               // 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);                                               // --------------------------------------

    glBindVertexArray(0);

    // PROGRAM
    app->framebufferQuadProgramIdx          = LoadProgram(app, "shader_final.glsl", "FRAMEBUFFER");
    Program& framebufferQuadProgram         = app->programs[app->framebufferQuadProgramIdx];
    app->framebufferQuadProgramUniformTex   = glGetUniformLocation(framebufferQuadProgram.handle, "uTexture");

    if (UniformIsInvalid(app->framebufferQuadProgramUniformTex))
    {
        ILOG("Program Uniform Texture was not valid!");
    }
}

void Engine::Renderer::RenderLightingQuad(App* app)
{
    glBindVertexArray(app->vaoFramebufferQuad);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
}

void Engine::Renderer::RenderLightingSphere(App* app)
{
    Model& model = app->models[Primitives::GetSphereIdx()];
    Mesh& mesh   = app->meshes[model.meshIdx];
    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        GLuint VAO = FindVAO(mesh, i, app->programs[app->deferredLightingProgramIdx]);
        glBindVertexArray(VAO);

        Submesh& submesh = mesh.submeshes[i];
        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
    }

    glBindVertexArray(0);
}

void Engine::Renderer::RenderQuad(App* app)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    Program& programTexturedGeometry = app->programs[app->texQuadProgramIdx];
    glUseProgram(programTexturedGeometry.handle);
    glBindVertexArray(app->vaoQuad);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform1i(app->texQuadProgramUniformTexture, 0);
    glActiveTexture(GL_TEXTURE0);
    GLuint textureHandle = app->textures[app->quadTexIdx].handle;
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glUseProgram(0);
}

void Engine::Renderer::RenderMesh(App* app)
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, app->displaySize.x, app->displaySize.y);
        
    Program& texturedMeshProgram = app->programs[app->texMeshProgramIdx];
    glUseProgram(texturedMeshProgram.handle);
    
    Model& model = app->models[app->modelIdx];
    Mesh& mesh = app->meshes[model.meshIdx];

    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        GLuint VAO = FindVAO(mesh, i, texturedMeshProgram);
        glBindVertexArray(VAO);

        u32 submeshMaterialIdx = model.materialIndices[i];
        Material& submeshMaterial = app->materials[submeshMaterialIdx];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTexIdx].handle);
        glUniform1i(app->texMeshProgramUniformTexture, 0);

        Submesh& submesh = mesh.submeshes[i];
        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

void Engine::Renderer::RenderEntities(App* app)
{
    BindFramebufferForRender(app);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, app->displaySize.x, app->displaySize.y);

    GeometryPass(app);

    if (InDeferredMode(app))
    {
        LightingPass(app);
    }

    glBindVertexArray(0);
    glUseProgram(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    FramebufferPass(app);
}

void Engine::Renderer::GeometryPass(App* app)
{
    glEnable(GL_DEPTH_TEST);
    //glDisable(GL_BLEND);
    
    Program& renderProgram = (InDeferredMode(app)) ? app->programs[app->deferredGeometryProgramIdx] : app->programs[app->forwardRenderingProgramIdx];
    glUseProgram(renderProgram.handle);

    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cbuffer.handle, app->globalParamsOffset, app->globalParamsSize);

    for (u32 i = 0; i < app->entities.size(); ++i)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(1), app->cbuffer.handle, app->entities[i].localParamsOffset, app->entities[i].localParamsSize);

        Model& model = app->models[app->entities[i].modelIndex];
        Mesh& mesh   = app->meshes[model.meshIdx];
        for (u32 i = 0; i < mesh.submeshes.size(); ++i)
        {
            GLuint VAO = FindVAO(mesh, i, renderProgram);
            glBindVertexArray(VAO);

            u32 submeshMaterialIdx      = model.materialIndices[i];
            Material& submeshMaterial   = app->materials[submeshMaterialIdx];

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTexIdx].handle);
            glUniform1i(glGetUniformLocation(renderProgram.handle, "uTexture"), 0);

            // NORMAL MAP
            if (app->useNormalMap)
            {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.normalTexIdx].handle);
                glUniform1i(glGetUniformLocation(renderProgram.handle, "uNormalMap"), 1);
            }

            // RELIEF MAP
            if (app->useBumpMap)
            {
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, app->textures[app->reliefTexIdx].handle);
                glUniform1i(glGetUniformLocation(renderProgram.handle, "uBumpMap"), 2);
                glUniform1f(glGetUniformLocation(renderProgram.handle, "uBumpiness"), app->bumpiness);
            }

            Submesh& submesh = mesh.submeshes[i];
            glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

void Engine::Renderer::LightingPass(App* app)
{
    GLuint drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);
    
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glClear(GL_COLOR_BUFFER_BIT);
    
    Program& deferredLightingProgram = app->programs[app->deferredLightingProgramIdx];
    glUseProgram(deferredLightingProgram.handle);

    glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(0), app->cbuffer.handle, app->globalParamsOffset, app->globalParamsSize);

    for (u32 i = 0; i < app->activeLights; ++i)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(2), app->cbuffer.handle, app->lights[i].localParamsOffset, app->lights[i].localParamsSize);

        Light& light = app->lights[i];

        glUniform1i(glGetUniformLocation(deferredLightingProgram.handle, "oAlbedo"),    0);
        glUniform1i(glGetUniformLocation(deferredLightingProgram.handle, "oNormals"),   1);
        glUniform1i(glGetUniformLocation(deferredLightingProgram.handle, "oDepth"),     2);
        glUniform1i(glGetUniformLocation(deferredLightingProgram.handle, "oPosition"),  3);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, app->GAlbedoTex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, app->GNormalTex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, app->GDepthTex);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, app->GPositionTex);

        switch (light.type)
        {
        case LIGHT_TYPE::LT_DIRECTIONAL: { RenderLightingQuad(app); }   break;
        case LIGHT_TYPE::LT_POINT:       { RenderLightingSphere(app); } break;
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

void Engine::Renderer::FramebufferPass(App* app)                                          // THIS-HERE
{
    glUseProgram(app->programs[app->framebufferQuadProgramIdx].handle);
    glBindVertexArray(app->vaoFramebufferQuad);

    glUniform1i(app->framebufferQuadProgramUniformTex, 0);
    glActiveTexture(GL_TEXTURE0);

    switch (app->renderLayer)
    {
    case RENDER_LAYER::SHADED:   { glBindTexture(GL_TEXTURE_2D, app->GShadedTex); }     break;
    case RENDER_LAYER::ALBEDO:   { glBindTexture(GL_TEXTURE_2D, app->GAlbedoTex); }     break;
    case RENDER_LAYER::NORMAL:   { glBindTexture(GL_TEXTURE_2D, app->GNormalTex); }     break;
    case RENDER_LAYER::DEPTH:    { glBindTexture(GL_TEXTURE_2D, app->GDepthTex); }      break;
    case RENDER_LAYER::POSITION: { glBindTexture(GL_TEXTURE_2D, app->GPositionTex); }   break;
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void Engine::Renderer::BindFramebufferForRender(App* app)                       // THIS-HERE
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ClearFramebuffer(app);

    glBindFramebuffer(GL_FRAMEBUFFER, app->framebufferHandle);

    GLuint drawBuffers[] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4
    };

    glDrawBuffers(ARRAY_COUNT(drawBuffers), drawBuffers);
}

void Engine::Renderer::RefreshFramebuffer(App* app)
{
    FreeFramebuffer(app);
    InitFramebuffer(app);
}

bool Engine::Renderer::InDeferredMode(App* app)
{
    return (app->renderMode != RENDER_MODE::FORWARD);
}

// GUI -------------------------------------------------------------------------
void Engine::Gui::GeneralTab(App* app)
{
    ImGui::Begin("General");

    ImGui::TextColored(cyan,    "State:");
    ImGui::TextColored(yellow,  "FPS:");    ImGui::SameLine(); ImGui::Text(" %f", 1.0f / app->deltaTime);
    ImGui::Checkbox("Enable debug groups", &app->enableDebugGroups);
    
    ImGui::Separator();
    
    ImGui::TextColored(cyan,    "Hardware:");
    ImGui::TextColored(yellow,  "Version:");     ImGui::SameLine(); ImGui::Text("  %s", glGetString(GL_VERSION));
    ImGui::TextColored(yellow,  "Renderer:");    ImGui::SameLine(); ImGui::Text(" %s", glGetString(GL_RENDERER));
    ImGui::TextColored(yellow,  "Vendor:");      ImGui::SameLine(); ImGui::Text("   %s", glGetString(GL_VENDOR));
    ImGui::TextColored(yellow,  "GLSL Ver:");    ImGui::SameLine(); ImGui::Text(" %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    ImGui::Separator();

    ImGui::TextColored(cyan, "Camera:");

    vec3 position = app->camera.GetPosition();
    if (ImGui::DragFloat3("Position", (float*)&position, 0.1)) { app->camera.SetPosition(position); }
    //if (ImGui::DragFloat3("Rotation", (float*)&rotation, 0.1)) { app->camera.SetRotation(rotation); }

    ImGui::Separator();
    
    ImGui::TextColored(cyan, "Shaders:");

    const char* items[] = { "FORWARD", "DEFERRED" };
    static int item_current = 1;
    ImGui::Combo("Shading Pipeline", &item_current, items, IM_ARRAYSIZE(items));
    app->renderMode = (RENDER_MODE)item_current;

    const char* items2[] = { "SHADED", "ALBEDO", "NORMAL", "DEPTH", "POSITION" };
    static int item_current2 = 0;
    ImGui::Combo("Shading Layers", &item_current2, items2, IM_ARRAYSIZE(items2));
    app->renderLayer = (RENDER_LAYER)item_current2;

    //static bool deferredShading = Renderer::InDeferredMode(app);
    //ImGui::Checkbox("Deferred Shading", &deferredShading);
    //app->renderMode = (deferredShading) ? RENDER_MODE::DEFERRED : RENDER_MODE::FORWARD;

    ImGui::Checkbox("Normal Map", &app->useNormalMap);
    ImGui::Checkbox("Bump Map", &app->useBumpMap);

    ImGui::End();
}

void Engine::Gui::ExtensionsTab(App* app)
{
    ImGui::Begin("OpenGL Extensions");

    if (ImGui::TreeNodeEx("Extensions", ImGuiTreeNodeFlags_None))
    {
        GLint num_extensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
        for (GLint i = 0; i < num_extensions; ++i)
        {
            ImGui::TextColored(yellow, "Extension %i:", i); ImGui::SameLine(); ImGui::Text("%s", glGetStringi(GL_EXTENSIONS, GLuint(i)));
        }

        ImGui::TreePop();
    }

    ImGui::End();
}