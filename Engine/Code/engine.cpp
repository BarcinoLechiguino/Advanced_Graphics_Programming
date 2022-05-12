//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include <imgui.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include "globals.h"
#include "file_manager.h"
#include "app.h"

#include "engine.h"

const VertexV3V2 vertices[] = {
    { vec3(-0.5, -0.5, 0.0), vec2(0.0, 0.0) },
    { vec3( 0.5, -0.5, 0.0), vec2(1.0, 0.0) },
    { vec3( 0.5,  0.5, 0.0), vec2(1.0, 1.0) },
    { vec3(-0.5,  0.5, 0.0), vec2(0.0, 1.0) }
};

const u16 indices[] = {
    0, 1, 2,
    0, 2, 3
};

void Engine::Init(App* app)
{
    // VERTEX BUFFER
    glGenBuffers(1, &app->embeddedVertices);
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // INDICES BUFFER
    glGenBuffers(1, &app->embeddedElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // VAO
    glGenVertexArrays(1, &app->vao);
    glBindVertexArray(app->vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)12);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
    
    glBindVertexArray(0);

    // BUFFER STRUCTS
    VertexBufferLayout VBL = {};
    VBL.attributes.push_back(VertexBufferAttribute{ 0, 3, 0 });                 // 3D Positions
    VBL.attributes.push_back(VertexBufferAttribute{ 2, 2, 3 * sizeof(float) }); // Tex Coords
    VBL.stride = 5 * sizeof(float);

    // MODEL STRUCTS 
    /*Submesh submesh = {};
    submesh.VBL = VBL;
    submesh.vertices.swap(vertices);
    submesh.indices.swap(indices);
    Mesh* myMesh;
    myMesh->submeshes.push_back(submesh);*/

    // PROGRAMS
    app->texturedGeometryProgramIdx     = LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
    Program& texturedGeometryProgram    = app->programs[app->texturedGeometryProgramIdx];
    //app->programUniformTexture        = glGetUniformLocation(texturedGeometryProgram.handle, "uTexture");
    
    app->texturedMeshProgramIdx         = LoadProgram(app, "shaders.glsl", "TEXTURED_PATRICK");
    Program& texturedMeshProgram        = app->programs[app->texturedMeshProgramIdx];
    texturedMeshProgram.VIL.attributes.push_back({0, 3});     // Position
    texturedMeshProgram.VIL.attributes.push_back({ 2, 2 });   // Tex Coord
    app->texturedMeshProgram_uTexture   = glGetUniformLocation(texturedMeshProgram.handle, "uTexture");

    // SHADER
    GLint   attributeCount      = 0;
    char    attributeName[128];
    GLsizei attributeNameLength = 0;
    GLint   attributeSize       = 0;
    GLenum  attributeType;
    GLint   attributeLocation   = 0;

    glGetProgramiv(texturedGeometryProgram.handle, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    for (GLint i = 0; i < attributeCount; ++i)
    {
        glGetActiveAttrib(texturedGeometryProgram.handle, i,
                          ARRAY_COUNT(attributeName),
                          &attributeNameLength,
                          &attributeSize,
                          &attributeType,
                          attributeName);

        attributeLocation = glGetAttribLocation(texturedGeometryProgram.handle, attributeName);
        glVertexAttribPointer(attributeLocation, attributeSize, attributeType, GL_FALSE, sizeof(float) * 5, (void*)0);
    }

    // TEXTURE
    app->diceTexIdx     = LoadTexture2D(app, "dice.png");
    app->whiteTexIdx    = LoadTexture2D(app, "color_white.png");
    app->blackTexIdx    = LoadTexture2D(app, "color_black.png");
    app->normalTexIdx   = LoadTexture2D(app, "color_normal.png");
    app->magentaTexIdx  = LoadTexture2D(app, "color_magenta.png");

    app->mode = Mode_TexturedQuad;
}

void Engine::Update(App* app)
{
    // You can handle app->input keyboard/mouse here

}

void Engine::Render(App* app)
{
    switch (app->mode)
    {
    case Mode_TexturedQuad:
    {
        if (app->enableDebugGroups)
        {
            glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Shaded Model");
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, app->displaySize.x, app->displaySize.y);

        //DrawTexturedQuad(app);
        DrawMeshes(app);

        glBindVertexArray(0);
        glUseProgram(0);

        if (app->enableDebugGroups)
        {
            glPopDebugGroup();
        }
    }
    break;

    default: {};
    }
}

void Engine::Gui(App* app)
{
    ImGui::Begin("Info");
    ImGui::Text("FPS: %f", 1.0f/app->deltaTime);
    ImGui::Checkbox("Enable debug groups", &app->enableDebugGroups);
    ImGui::End();

    ImGui::Begin("OpenGL Info");
    
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "Version:");     ImGui::SameLine(); ImGui::Text("  %s", glGetString(GL_VERSION));
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "Renderer:");    ImGui::SameLine(); ImGui::Text(" %s", glGetString(GL_RENDERER));
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "Vendor:");      ImGui::SameLine(); ImGui::Text("   %s", glGetString(GL_VENDOR));
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "GLSL Ver:");    ImGui::SameLine(); ImGui::Text(" %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    if (ImGui::TreeNodeEx("Extensions", ImGuiTreeNodeFlags_None))
    {
        GLint num_extensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
        for (GLint i = 0; i < num_extensions; ++i)
        {
            ImGui::Text("Extension %i: %s", i, glGetStringi(GL_EXTENSIONS, GLuint(i)));
        }

        ImGui::TreePop();
    }
    
    ImGui::End();
}

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

    return app->programs.size() - 1;
}

Image Engine::LoadImage(const char* filename)
{
    Image img = {};
    stbi_set_flip_vertically_on_load(true);
    img.pixels = stbi_load(filename, &img.size.x, &img.size.y, &img.nchannels, 0);
    if (img.pixels)
    {
        img.stride = img.size.x * img.nchannels;
    }
    else
    {
        ELOG("Could not open file %s", filename);
    }
    return img;
}

void Engine::FreeImage(Image image)
{
    stbi_image_free(image.pixels);
}

GLuint Engine::CreateTexture2DFromImage(Image image)
{
    GLenum internalFormat = GL_RGB8;
    GLenum dataFormat = GL_RGB;
    GLenum dataType = GL_UNSIGNED_BYTE;

    switch (image.nchannels)
    {
    case 3: dataFormat = GL_RGB; internalFormat = GL_RGB8; break;
    case 4: dataFormat = GL_RGBA; internalFormat = GL_RGBA8; break;
    default: ELOG("LoadTexture2D() - Unsupported number of channels");
    }

    GLuint texHandle;
    glGenTextures(1, &texHandle);
    glBindTexture(GL_TEXTURE_2D, texHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.size.x, image.size.y, 0, dataFormat, dataType, image.pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texHandle;
}

u32 Engine::LoadTexture2D(App* app, const char* filepath)
{
    for (u32 texIdx = 0; texIdx < app->textures.size(); ++texIdx)
    {
        if (app->textures[texIdx].filepath == filepath)
        {
            return texIdx;
        }
    }

    Image image = LoadImage(filepath);

    if (image.pixels)
    {
        Texture tex = {};
        tex.handle = CreateTexture2DFromImage(image);
        tex.filepath = filepath;

        u32 texIdx = app->textures.size();
        app->textures.push_back(tex);

        FreeImage(image);
        return texIdx;
    }
    else
    {
        return UINT32_MAX;
    }
}

void Engine::DrawTexturedQuad(App* app)
{
    Program& programTexturedGeometry = app->programs[app->texturedGeometryProgramIdx];
    glUseProgram(programTexturedGeometry.handle);
    glBindVertexArray(app->vao);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUniform1i(app->programUniformTexture, 0);
    glActiveTexture(GL_TEXTURE0);
    GLuint textureHandle = app->textures[app->diceTexIdx].handle;
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

void Engine::DrawMeshes(App* app)
{
    Program& texturedMeshProgram = app->programs[app->texturedMeshProgramIdx];
    glUseProgram(texturedMeshProgram.handle);

    Model& model = app->models[app->modelIdx];
    Mesh& mesh   = app->meshes[model.meshIdx];

    for (u32 i = 0; i < mesh.submeshes.size(); ++i)
    {
        GLuint VAO = FindVAO(mesh, i, texturedMeshProgram);
        glBindVertexArray(VAO);

        u32 submeshMaterialIdx      = model.materialIndices[i];
        Material& submeshMaterial   = app->materials[submeshMaterialIdx];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, app->textures[submeshMaterial.albedoTexIdx].handle);
        glUniform1i(app->texturedMeshProgram_uTexture, 0);

        Submesh& submesh = mesh.submeshes[i];
        glDrawElements(GL_TRIANGLES, submesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)submesh.indexOffset);
    }
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
                const u32 index  = submesh.VBL.attributes[j].location;
                const u32 ncomp  = submesh.VBL.attributes[j].componentCount;
                const u32 offset = submesh.VBL.attributes[j].offset + submesh.vertexOffset;
                const u32 stride = submesh.VBL.stride;
                
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