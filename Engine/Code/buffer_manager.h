#ifndef __BUFFER_MANAGER_H__
#define __BUFFER_MANAGER_H__

#include "base_types.h"
#include "shader_types.h"

namespace BufferManager
{
	Buffer	CreateBuffer	(u32 size, GLenum type, GLenum usage);
	void	BindBuffer		(const Buffer& buffer);
	void	UnbindBuffer	(const Buffer& buffer);
	void	MapBuffer		(Buffer& buffer, GLenum access);
	void	UnmapBuffer		(Buffer& buffer);

	bool	IsPowerOfTwo	(u32 value);
	
	u32		Align			(u32 value, u32 alignment);
	void	AlignHead		(Buffer& buffer, u32 alignment);
	void	PushAlignedData	(Buffer& buffer, const void* data, u32 size, u32 alignment);

	void	BindBufferRange(size_t varSize, u32 binding, u32 bufferHandle, u32 blockOffset);
}

#define BINDING(b) b

#define CreateConstantBuffer(size)		BufferManager::CreateBuffer(size, GL_UNIFORM_BUFFER, GL_STREAM_DRAW);
#define CreateStaticVertexBuffer(size)	BufferManager::CreateBuffer(size, GL_ARRAY_BUFFER, GL_STATIC_DRAW);
#define CreateStaticIndexBuffer(size)	BufferManager::CreateBuffer(size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW);

#define PushData(buffer, data, size)	BufferManager::PushAlignedData(buffer, data, size, 1);
#define PushFloat(buffer, value)		{ f32 v = value; BufferManager::PushAlignedData(buffer, &v, sizeof(v), 4); }
#define PushUInt(buffer, value)			{ u32 v = value; BufferManager::PushAlignedData(buffer, &v, sizeof(v), 4); }
#define PushVec2(buffer, value)			BufferManager::PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushVec3(buffer, value)			BufferManager::PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushVec4(buffer, value)			BufferManager::PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushMat3(buffer, value)			BufferManager::PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))
#define PushMat4(buffer, value)			BufferManager::PushAlignedData(buffer, value_ptr(value), sizeof(value), sizeof(vec4))

#endif // !__BUFFER_MANAGER_H__