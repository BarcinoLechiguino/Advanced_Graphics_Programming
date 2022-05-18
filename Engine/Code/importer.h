#ifndef __IMPORTER_H__
#define __IMPORTER_H__

#include <stb_image.h>
#include <stb_image_write.h>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "base_types.h"
#include "shader_types.h"
#include "app.h"

namespace Importer
{
	u32	 LoadTexture2D	(App* app, const char* filepath);
	u32  LoadModel		(App* app, const char* filename);

	namespace Utils
	{
		GLuint	CreateTexture2DFromImage	(Image image);
		void	FreeImage					(Image image);
		Image	LoadImage					(const char* filename);
		
		void ProcessAssimpNode				(const aiScene* scene, aiNode* node, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
		void ProcessAssimpMesh				(const aiScene* scene, aiMesh* mesh, Mesh* myMesh, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);
		void ProcessAssimpMaterial			(App* app, aiMaterial* material, Material& myMaterial, String directory);
	}
}

#endif // !__IMPORTER_H__