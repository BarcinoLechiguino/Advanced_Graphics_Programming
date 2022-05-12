#ifndef __ENGINE_H__
#define __ENGINE_H__

// engine.h: 
// This file contains the functions relative to the engine.

#include "base_types.h"
#include "shader_types.h"

struct App;

namespace Engine
{
	void Init	(App* app);
	void Update	(App* app);
	void Render	(App* app);
	void Gui	(App* app);

	GLuint	CreateProgramFromSource		(String programSource, const char* shaderName);
	u32		LoadProgram					(App* app, const char* filepath, const char* programName);
	Image	LoadImage					(const char* filename);
	void	FreeImage					(Image image);
	GLuint	CreateTexture2DFromImage	(Image image);
	u32		LoadTexture2D				(App* app, const char* filepath);
	GLuint	FindVAO						(Mesh& mesh, u32 submeshIndex, const Program& program);
	GLuint	CreateVAO					(Mesh& mesh, Submesh& submesh, const Program& program);

	void	DrawTexturedQuad			(App* app);
	void	DrawMeshes					(App* app);
}

#endif // !__ENGINE_H__