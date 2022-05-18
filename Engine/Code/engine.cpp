//
// engine.cpp : Put all your graphics stuff in this file. This is kind of the graphics module.
// In here, you should type all your OpenGL commands, and you can also type code to handle
// input platform events (e.g to move the camera or react to certain shortcuts), writing some
// graphics related GUI options, and so on.
//

#include <imgui.h>

#include "globals.h"
#include "file_manager.h"
#include "app.h"
#include "importer.h"

#include "engine.h"

void Engine::Init(App* app)
{
    app->enableDebugGroups = false;
    
    app->mode = MODE::QUAD;
    
    switch (app->mode)
    {
    case MODE::QUAD: { Renderer::InitQuad(app); }  break;
    case MODE::MESH: { Renderer::InitMesh(app); }  break;
    default:         { /* NOTHING FOR NOW */ };
    }
}

void Engine::Update(App* app)
{
    for (u64 i = 0; i < app->programs.size(); ++i)
    {
        Program& program = app->programs[i];
        u64 currentTimestamp = FileManager::GetFileLastWriteTimestamp(program.filepath.c_str());
        if (currentTimestamp < program.lastWriteTimestamp)
        {
            glDeleteProgram(program.handle);
            String programSource        = FileManager::ReadTextFile(program.filepath.c_str());
            const char* programName     = program.programName.c_str();
            program.handle              = CreateProgramFromSource(programSource, programName);
            program.lastWriteTimestamp  = currentTimestamp;
        }
    }

    // You can handle app->input keyboard/mouse here

}

void Engine::Render(App* app)
{
    if (app->enableDebugGroups)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 1, -1, "Shaded Model");
    }
    
    switch (app->mode)
    {
    case MODE::QUAD: { Renderer::RenderQuad(app); } break;
    case MODE::MESH: { Renderer::RenderMesh(app); } break;
    default:         { /*NOTHING AT THE MOMENT*/ };
    }

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
    Gui::InfoTab(app);
    Gui::ExtensionsTab(app);
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

// RENDERER --------------------------------------------------------------------
void Engine::Renderer::InitQuad(App* app)
{
    const VertexV3V2 vertices[] = {
        { vec3(-0.5, -0.5, 0.0), vec2(0.0, 0.0) },          // 0
        { vec3( 0.5, -0.5, 0.0), vec2(1.0, 0.0) },          // 1
        { vec3( 0.5,  0.5, 0.0), vec2(1.0, 1.0) },          // 2
        { vec3(-0.5,  0.5, 0.0), vec2(0.0, 1.0) }           // 3
    };

    const u16 indices[] = {
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

    glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexV3V2), (void*)12);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);

    glBindVertexArray(0);

    // PROGRAM
    app->texQuadProgramIdx      = LoadProgram(app, "shaders.glsl", "TEXTURED_GEOMETRY");
    Program& texQuadProgram     = app->programs[app->texQuadProgramIdx];
    app->programUniformTexture  = glGetUniformLocation(texQuadProgram.handle, "uTexture");

    if (UniformIsInvalid(app->programUniformTexture))
    {
        ILOG("Program Uniform Texture was not valid!");
    }

    // TEXTURE
    app->diceTexIdx = Importer::LoadTexture2D(app, "dice.png");
    
    //app->whiteTexIdx    = Importer::LoadTexture2D(app, "color_white.png");
    //app->blackTexIdx    = Importer::LoadTexture2D(app, "color_black.png");
    //app->normalTexIdx   = Importer::LoadTexture2D(app, "color_normal.png");
    //app->magentaTexIdx  = Importer::LoadTexture2D(app, "color_magenta.png");
}

void Engine::Renderer::InitMesh(App* app)
{
    app->modelIdx                       = Importer::LoadModel(app, "Patrick/Patrick.obj");
    app->texMeshProgramIdx              = LoadProgram(app, "shaders.glsl", "TEXTURED_MESH");
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

    glUniform1i(app->programUniformTexture, 0);
    glActiveTexture(GL_TEXTURE0);
    GLuint textureHandle = app->textures[app->diceTexIdx].handle;
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

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

// GUI -------------------------------------------------------------------------
void Engine::Gui::InfoTab(App* app)
{
    ImGui::Begin("Info");

    ImGui::Text("FPS: %f", 1.0f / app->deltaTime);
    ImGui::Checkbox("Enable debug groups", &app->enableDebugGroups);

    ImGui::End();
}

void Engine::Gui::ExtensionsTab(App* app)
{
    ImGui::Begin("OpenGL Info");

    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "Version:");     ImGui::SameLine(); ImGui::Text("  %s",  glGetString(GL_VERSION));
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "Renderer:");    ImGui::SameLine(); ImGui::Text(" %s",   glGetString(GL_RENDERER));
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "Vendor:");      ImGui::SameLine(); ImGui::Text("   %s", glGetString(GL_VENDOR));
    ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "GLSL Ver:");    ImGui::SameLine(); ImGui::Text(" %s",   glGetString(GL_SHADING_LANGUAGE_VERSION));

    ImGui::Separator();

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