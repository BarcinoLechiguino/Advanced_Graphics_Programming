#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "math_types.h"

namespace Camera
{
	mat4 TransformPosition	(const vec3& pos);
	mat4 TransformRotation	(const float angleRad, const vec3& axis);
	mat4 TransformScale		(const vec3& scaleFactor);

	mat4 TransformPositionScale(const vec3& pos, const vec3& scaleFactor);
}

#endif // !__CAMERA_H__