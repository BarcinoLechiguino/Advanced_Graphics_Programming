#include "camera.h"

mat4 Camera::TransformPosition(const vec3& pos)
{
	mat4 transform = translate(pos);
	return transform;
}

mat4 Camera::TransformRotation(const float angleRad, const vec3& axis)
{
	mat4 transform = rotate(angleRad, axis);
	return transform;
}

mat4 Camera::TransformScale(const vec3& scaleFactor)
{
	mat4 transform = scale(scaleFactor); 
	return transform;
}

mat4 Camera::TransformPositionScale(const vec3& pos, const vec3& scaleFactor)
{
	mat4 transform = translate(pos);
	transform = scale(scaleFactor);
	return transform;
}