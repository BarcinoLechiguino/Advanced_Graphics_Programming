#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "math_types.h"

namespace Transform
{
	mat4 Position		(const vec3& pos);
	mat4 Rotation		(const float angleRad, const vec3& axis);
	mat4 Scale			(const vec3& scaleFactor);

	mat4 PositionRotate(const vec3& pos, const float angleRad, const vec3& axis);
	mat4 PositionScale	(const vec3& pos, const vec3& scaleFactor);

	static const vec3 rightVector	= vec3(1.0f, 0.0f, 0.0f);
	static const vec3 upVector		= vec3(0.0f, 1.0f, 0.0f);
	static const vec3 forwardVector	= vec3(0.0f, 0.0f, 1.0f);

	static const vec3 defaultScale	= vec3(1.0f, 1.0f, 1.0f);
}

#endif // !__TRANSFORM_H__