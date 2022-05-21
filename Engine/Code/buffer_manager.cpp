#include "shader_types.h"

#include "buffer_manager.h"

Buffer BufferManager::CreateBuffer(u32 size, GLenum type, GLenum usage)
{
	Buffer buffer = {};
	buffer.size = size;
	buffer.type = type;
	
	glGenBuffers(1, &buffer.handle);
	glBindBuffer(type, buffer.handle);
	glBufferData(type, buffer.size, NULL, usage);
	glBindBuffer(type, 0);

	return buffer;
}

void BufferManager::BindBuffer(const Buffer& buffer)
{
	glBindBuffer(buffer.type, buffer.handle);
}

void BufferManager::UnbindBuffer(const Buffer& buffer)
{
	glBindBuffer(buffer.type, 0);
}

void BufferManager::MapBuffer(Buffer& buffer, GLenum access)
{
	glBindBuffer(buffer.type, buffer.handle);
	buffer.data = (u8*)glMapBuffer(buffer.type, access);
	buffer.head = 0;
}

void BufferManager::UnmapBuffer(Buffer& buffer)
{
	glUnmapBuffer(buffer.type);
	glBindBuffer(buffer.type, 0);
}

bool BufferManager::IsPowerOfTwo(u32 value)
{
	return (value && !(value & (value - 1)));
}

u32 BufferManager::Align(u32 value, u32 alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

u32 BufferManager::AlignHead(Buffer& buffer, u32 alignment)
{
	assert(IsPowerOfTwo(alignment), "Alignment must be a power of 2!");
	buffer.head = Align(buffer.head, alignment);
}

void BufferManager::PushAlignedData(Buffer& buffer, const void* data, u32 size, u32 alignment)
{
	assert((buffer.data != NULL), "Buffer must be mapped first!");
	AlignHead(buffer, alignment);
	memcpy((u8*)buffer.data + buffer.head, data, size);
	buffer.head += size;
}

void BufferManager::BindBufferRange(size_t varSize, u32 binding, u32 bufferHandle, u32 blockOffset)
{
	u32 blockSize = varSize * 2;
	glBindBufferRange(GL_UNIFORM_BUFFER, BINDING(binding), bufferHandle, blockOffset, blockSize);
}