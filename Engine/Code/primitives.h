#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

#include "shader_types.h"

namespace Primitives
{
	void InitPrimitivesData	();

	const MeshData& GetPlaneData	();
	const MeshData& GetCubeData		();
	const MeshData& GetSphereData	();

	namespace Utils
	{
		void InitPlaneData	();
		void InitCubeData	();
		void InitSphereData	();

		static MeshData planeData;
		static MeshData cubeData;
		static MeshData sphereData;
	}
}

#endif // !__PRIMITIVES_H__