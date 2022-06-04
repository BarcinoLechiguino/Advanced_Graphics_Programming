#include "app.h"

#include "camera.h"

Camera::Camera() :
position		({ 0.0f, 0.0f, 20.0f }),
target			({ 0.0f, 0.0f, 0.0f }),
moveSpeed		(100.0f),
aspectRatio		(0.0f),
nearPlane		(0.1f),
farPlane		(1000.0f),
projectionMatrix(mat4(0.0f)),
viewMatrix		(lookAt(position, target, vec3(0.0f, 1.0f, 0.0f)))
{

}

Camera::Camera(App* app) :
position		({ 0.0f, 0.0f, 20.0f }),
target			({ 0.0f, 0.0f, 0.0f }),
moveSpeed		(100.0f),
aspectRatio		((float)app->displaySize.x / (float)app->displaySize.y),
nearPlane		(0.1f),
farPlane		(1000.0f),
projectionMatrix(mat4(0.0f)),
viewMatrix		(lookAt(position, target, vec3(0.0f, 1.0f, 0.0f)))
{

}