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

	namespace Input
	{
		void GetInput(App* app);
	}

	namespace Renderer
	{
		void InitQuad	(App* app);
		void InitMesh	(App* app);

		void RenderQuad	(App* app);
		void RenderMesh	(App* app);
	}

	namespace Gui
	{
		void InfoTab					(App* app);
		void ExtensionsTab				(App* app);
	}
}

#endif // !__ENGINE_H__