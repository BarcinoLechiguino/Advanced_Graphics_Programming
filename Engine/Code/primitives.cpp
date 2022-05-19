#include "primitives.h"

void Primitives::InitPrimitivesData()
{
	Utils::InitPlaneData();
	Utils::InitCubeData();
	Utils::InitSphereData();
}

const MeshData& Primitives::GetPlaneData()
{
	return Utils::planeData;
}

const MeshData& Primitives::GetCubeData()
{
	return Utils::cubeData;
}

const MeshData& Primitives::GetSphereData()
{
	return Utils::sphereData;
}


void Primitives::Utils::InitPlaneData()
{	
	
}

void Primitives::Utils::InitCubeData()
{
	
}

void Primitives::Utils::InitSphereData()
{
	sphereData.vertices.push_back(Vertex());
}