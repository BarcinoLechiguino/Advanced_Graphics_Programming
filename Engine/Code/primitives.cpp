#include "app.h"
#include "globals.h"

#include "primitives.h"

void Primitives::InitPrimitivesData(App* app)
{
	Utils::InitPlaneData(app);
	Utils::InitCubeData(app);
	Utils::InitSphereData(app);
}

const u32 Primitives::GetPlaneIdx()
{
	return Utils::planeIdx;
}

const u32 Primitives::GetCubeIdx()
{
	return Utils::cubeIdx;
}

const u32 Primitives::GetSphereIdx()
{
	return Utils::sphereIdx;
}


void Primitives::Utils::InitPlaneData(App* app)
{	
	const float vertices[] = {// Positions         Normals         UVs         Tangents         BiTangents
								-0.5, 0.0, -0.5,	0.0, 1.0, 0.0,	0.0, 0.0,	0.0, 0.0, 0.0,	0.0, 0.0, 0.0,          // 0
								 0.5, 0.0, -0.5,	0.0, 1.0, 0.0,	1.0, 0.0,	0.0, 0.0, 0.0,	0.0, 0.0, 0.0,          // 1
								 0.5, 0.0,  0.5,	0.0, 1.0, 0.0,	1.0, 1.0,	0.0, 0.0, 0.0,	0.0, 0.0, 0.0,          // 2
								-0.5, 0.0,  0.5,	0.0, 1.0, 0.0,	0.0, 1.0,	0.0, 0.0, 0.0,	0.0, 0.0, 0.0           // 3
							 };

    const u32 indices[]	=	{
								0, 1, 2,
								0, 2, 3
                            };

	// MODEL DATA -------------------------------------------------------------
	app->meshes.push_back(Mesh{});														// Mesh Creation: --------------------------------
	Mesh& mesh		= app->meshes.back();												// Creating the Mesh that contains prmtve submesh.
	u32 meshIdx		= (u32)app->meshes.size() - 1u;										// -----------------------------------------------

	app->models.push_back(Model{});														// Model Creation: -------------------------------
	Model& model	= app->models.back();												// Creating the model that will contain the Mesh.
	model.meshIdx	= meshIdx;															// Order: Model -> Mesh -> Submesh.
	u32 modelIdx	= (u32)app->models.size() - 1u;										// -----------------------------------------------

	model.materialIndices.push_back(app->defaultMaterialIdx);							// Material Creation: ----------------------------
	//app->materials.push_back(Material{});												// Since its a primitive, it will use the Default.
	//Material& material = app->materials.back();										// -----------------------------------------------

	VertexBufferLayout VBL = {};														// Vertex Buffer Layout Creation: ----------------
	VBL.AddAttribute(0, 3, sizeof(float));												// AddAtribute() increases the stride internally.
	VBL.AddAttribute(1, 3, sizeof(float));												// This avoids having to pass the stride/offset.
	VBL.AddAttribute(2, 2, sizeof(float));												// Should take into account when manipulating it.
	VBL.AddAttribute(3, 3, sizeof(float));												// stride += ncomp * sizeof(var) (per addition).
	VBL.AddAttribute(4, 3, sizeof(float));												// -----------------------------------------------

	Submesh submesh = {};																// Submesh Creation: -----------------------------
	submesh.VBL = VBL;																	// Adding the created VBL to the submesh.

	size_t verticesSize = ARRAY_COUNT(vertices);										// ---
	submesh.vertices.resize(verticesSize);												// Adding the vertices to the submesh's container.
	memcpy(&submesh.vertices[0], &vertices[0], verticesSize * sizeof(float));			// ---
	
	size_t indicesSize = ARRAY_COUNT(indices);											// ---
	submesh.indices.resize(indicesSize);												// Adding the indices to the submesh's container.
	memcpy(&submesh.indices[0], &indices[0], indicesSize * sizeof(u32));				// ---

	mesh.submeshes.push_back(submesh);													// -----------------------------------------------

	// VERTEX, INDEX & VAO BUFFERS --------------------------------------------
	u32 vertexBufferSize = 0;
	u32 indexBufferSize  = 0;
	for (u32 i = 0; i < mesh.submeshes.size(); ++i)
	{
		vertexBufferSize += mesh.submeshes[i].vertices.size() * sizeof(float);
		indexBufferSize  += mesh.submeshes[i].indices.size() * sizeof(u32);
	}

	glGenBuffers(1, &mesh.vertexBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &mesh.indexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	mesh.submeshes[0].vertexOffset = 0;
	mesh.submeshes[0].indexOffset  = 0;

	/*glGenBuffers(1, &mesh.vertexBufferHandle);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferHandle);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &mesh.indexBufferHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBufferHandle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, NULL, GL_STATIC_DRAW);

	u32 verticesOffset	= 0;
	u32 indicesOffset	= 0;
	for (u32 i = 0; i < mesh.submeshes.size(); ++i)
	{
		const void* verticesData = mesh.submeshes[i].vertices.data();
		const u32	verticesSize = mesh.submeshes[i].vertices.size();
		glBufferSubData(GL_ARRAY_BUFFER, verticesOffset, verticesSize, verticesData);

		const void* indicesData = mesh.submeshes[i].indices.data();
		const u32	indicesSize = mesh.submeshes[i].indices.size();
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indicesOffset, indicesSize, indicesData);
		
		mesh.submeshes[i].vertexOffset = verticesOffset;
		mesh.submeshes[i].indexOffset  = indicesOffset;
		verticesOffset += verticesSize;
		indicesOffset  += indicesSize;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/

	planeIdx = modelIdx;
}

void Primitives::Utils::InitCubeData(App* app)
{
	
}

void Primitives::Utils::InitSphereData(App* app)
{
	/*i32 H = 32;
	i32 V = 16;

	// VERTICES
	for (i32 h = 0; h < H; ++h)
	{
		for (i32 v = 0; v < V + 1; ++v)
		{
			f32 nh = (f32)h / (f32)H;
			f32 nv = (f32)v / ((f32)V) - 0.5f;
			f32 angleH = 2 * PI * nh;
			f32 angleV = -PI * nv;

			Vertex vertex;
			vertex.pos.x	= (sinf(angleH) * cosf(angleV));
			vertex.pos.y	= -sinf(angleV);
			vertex.pos.z	= (cosf(angleH) * cosf(angleV));
			vertex.norm		= vertex.pos;
			vertex.uv.x		= 1.0f;
			vertex.uv.y		= 0.0f;
			
			sphereMesh.vertices.push_back(vertex.pos.x);
			sphereMesh.vertices.push_back(vertex.pos.y);
			sphereMesh.vertices.push_back(vertex.pos.z);

			sphereMesh.vertices.push_back(vertex.norm.x);
			sphereMesh.vertices.push_back(vertex.norm.y);
			sphereMesh.vertices.push_back(vertex.norm.z);

			sphereMesh.vertices.push_back(vertex.uv.x);
			sphereMesh.vertices.push_back(vertex.uv.y);
		}
	}

	for (i32 h = 0; h < H; ++h)
	{
		for (i32 v = 0; v < V; ++v)
		{
			sphereMesh.indices.push_back(	h			* (V + 1) +  v);
			sphereMesh.indices.push_back( ((h + 1) % H) * (V + 1) +  v);
			sphereMesh.indices.push_back( ((h + 1) % H) * (V + 1) + (v + 1));
			sphereMesh.indices.push_back(	h			* (V + 1) +  v);
			sphereMesh.indices.push_back( ((h + 1) % H) * (V + 1) + (v + 1));
			sphereMesh.indices.push_back(	h			* (V + 1) + (v + 1));
		}
	}

	sphereMesh.VBL.attributes.push_back({ 0, 3, 0 });
	sphereMesh.VBL.attributes.push_back({ 1, 3, sizeof(float) * 3 });
	sphereMesh.VBL.attributes.push_back({ 2, 2, sizeof(float) * 6 });
	sphereMesh.VBL.stride = sizeof(float) * 8;

	sphereMesh.indexOffset	= 0;
	sphereMesh.vertexOffset = sizeof(float) * 8;

	VAO sphereVAO = {app->vaoQuad, app->texQuadProgramIdx};
	sphereMesh.vaos.push_back(sphereVAO);

	// VERTEX BUFFER
	glGenBuffers(1, &app->embeddedVertices);
	glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * sphereMesh.vertices.size(), &sphereMesh.vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// INDEX BUFFER
	glGenBuffers(1, &app->embeddedElements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * sphereMesh.vertices.size(), &sphereMesh.vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// VAO
	glGenVertexArrays(1, &app->vaoQuad);
	glBindVertexArray(app->vaoQuad);

	glBindBuffer(GL_ARRAY_BUFFER, app->embeddedVertices);                                                       // --------------------------------------
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);                                  // Quads do not take into account normals
	glEnableVertexAttribArray(0);                                                                               // 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)24);                                 // Therefore they will not be added to
	glEnableVertexAttribArray(1);                                                                               // the buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->embeddedElements);                                               // --------------------------------------

	glBindVertexArray(0);*/
}