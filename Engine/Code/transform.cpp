#include "transform.h"

mat4 Transform::Position(const vec3& pos)
{
	mat4 transform = translate(pos);
	return transform;
}

mat4 Transform::Rotation(const float angleRad, const vec3& axis)
{
	mat4 transform = rotate(angleRad, axis);
	return transform;
}

mat4 Transform::Scale(const vec3& scaleFactor)
{
	mat4 transform = scale(scaleFactor);
	return transform;
}

mat4 Transform::PositionRotate(const vec3& pos, const float angleRad, const vec3& axis)
{
	mat4 transform	= translate(pos);
	transform		= rotate(angleRad, axis);
	return transform;
}

mat4 Transform::PositionScale(const vec3& pos, const vec3& scaleFactor)
{
	mat4 transform	= translate(pos);
	transform		*= scale(scaleFactor);
	return transform;
}