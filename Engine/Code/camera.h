#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "math_types.h"

struct App;

struct Camera
{
	// CONSTRUCTOR
	Camera();
	Camera(App* app);

	// METHODS
	inline const vec3	GetPosition		() const { return position; }
	inline const vec3	GetTarget		() const { return target; }
	inline const float	GetMoveSpeed	() const { return moveSpeed; }

	inline const float	GetAspectRatio	() const { return aspectRatio; }
	inline const float	GetNearPlane	() const { return nearPlane; }
	inline const float	GetFarPlane		() const { return farPlane; }

	inline const mat4	GetProjMatrix	() const { return projectionMatrix; }
	inline const mat4	GetViewMatrix	() const { return viewMatrix; }
	
	inline void SetPosition		(vec3 newPosition)		{ position			= newPosition; }
	inline void SetTarget		(vec3 newTarget)		{ target			= newTarget; }
	inline void SetAspectRatio	(float newAspectRatio)	{ aspectRatio		= newAspectRatio; }
	inline void SetViewMatrix	(mat4 newViewMatrix)	{ viewMatrix		= newViewMatrix; }
	inline void SetProjMatrix	(float radians)			{ projectionMatrix	= glm::perspective(radians, aspectRatio, nearPlane, farPlane); }

	// VARIABLES
	vec3	position;
	vec3	target;
	float	moveSpeed;

	float	aspectRatio;
	float	nearPlane;
	float	farPlane;

	mat4	projectionMatrix;
	mat4	viewMatrix;
};

#endif // !__CAMERA_H__