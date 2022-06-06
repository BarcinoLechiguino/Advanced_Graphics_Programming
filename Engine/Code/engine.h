#ifndef __ENGINE_H__
#define __ENGINE_H__

// engine.h: 
// This file contains the functions relative to the engine.

#include "base_types.h"
#include "shader_types.h"

struct App;

namespace Engine
{
	void Init		(App* app);
	void Update		(App* app);
	void Render		(App* app);
	void DrawGui	(App* app);

	GLuint	CreateProgramFromSource		(String programSource, const char* shaderName);
	u32		LoadProgram					(App* app, const char* filepath, const char* programName);
	
	bool	UniformIsInvalid			(GLuint uniformHandle);

	GLuint	CreateVAO					(Mesh& mesh, Submesh& submesh, const Program& program);
	GLuint	FindVAO						(Mesh& mesh, u32 submeshIndex, const Program& program);

	namespace Camera
	{
		void InitCamera					(App* app);
		void InitWorldTransform			(App* app);
	}
	
	namespace Input
	{
		void GetInput(App* app);
	}

	namespace Shaders
	{
		void LoadBaseTextures			(App* app);
		void CreateDefaultMaterial		(App* app);

		void GetProgramAttributes		(App* app, GLuint programHandle);
		void InitUniformBlockBuffer		(App* app);
	}

	namespace Lights
	{
		void AddLight(App* app, LIGHT_TYPE type, vec3 color, vec3 direction, vec3 position);
	}

	namespace Renderer
	{
		void InitFramebuffer			(App* app);
		void GenerateFramebufferTexture	(GLuint& texHandle, ivec2 size, GLint type);
		void CheckFramebufferStatus		();
		
		void InitQuad					(App* app, const char* texPath);
		void InitMesh					(App* app, const char* meshPath);
		void InitEntities				(App* app);

		void RenderQuad					(App* app);
		void RenderMesh					(App* app);
		void RenderEntities				(App* app);
	}

	namespace Gui
	{
		void GeneralTab					(App* app);
		void ExtensionsTab				(App* app);
	}
}

#endif // !__ENGINE_H__