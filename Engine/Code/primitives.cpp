#ifndef __PRIMITIVES_H__
#define __PRIMITIVES_H__

#include <vector>

#include "base_types.h"
#include "shader_types.h"

struct Sphere
{
	void Init();

	std::vector<float>	vertices;
	std::vector<float>	normals;
	std::vector<u32>	indices;

	VertexBufferAttribute VBA;

};

struct Plane
{

};

#endif // !__PRIMITIVES_H__