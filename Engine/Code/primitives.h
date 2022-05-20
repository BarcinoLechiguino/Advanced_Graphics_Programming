#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

#include "shader_types.h"

struct App;

namespace Primitives
{
	void InitPrimitivesData	(App* app);

	const u32 GetPlaneIdx	();
	const u32 GetCubeIdx	();
	const u32 GetSphereIdx	();

	namespace Utils
	{
		void InitPlaneData	(App* app);
		void InitCubeData	(App* app);
		void InitSphereData	(App* app);

		static u32 planeIdx;
		static u32 cubeIdx;
		static u32 sphereIdx;
	}
}

#endif // !__PRIMITIVES_H__