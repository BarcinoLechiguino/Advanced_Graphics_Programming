///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef TEXTURED_GEOMETRY

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 2) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord	= aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = texture(uTexture, vTexCoord);
}

#endif

#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef TEXTURED_MESH

#if defined(VERTEX)	///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
//layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
//layout(location = 3) in vec3 aTangent;
//layout(location = 4) in vec3 aBitangent;

out vec2 vTexCoord;

void main()
{
	vTexCoord			= aTexCoord;
	float clippingScale = 5.0;
	gl_Position			= vec4(aPosition, clippingScale);
	gl_Position.z		= -gl_Position.z;
}

#elif defined(FRAGMENT)	///////////////////////////////////////////////

in vec2	vTexCoord;
uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = texture(uTexture, vTexCoord);
}

#endif	///////////////////////////////////////////////////////////////

#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef TEXTURED_ENTITY

struct Light
{
	unsigned int type;
	vec3		 color;
	vec3		 direction;
	vec3		 position;
};

#define LT_DIRECTIONAL	0
#define LT_POINT		1

layout(binding = 0, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uLightCount;
	Light			uLight[16];
};

#if defined(VERTEX)	///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

layout(binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vPosition;		// In Worldspace
out vec3 vNormal;		// In Worldspace
out vec3 vViewDir;		// In WorldSpace

void main()
{
	vTexCoord	= aTexCoord;
	vPosition	= vec3(uWorldMatrix * vec4(aPosition, 1.0));
	vNormal		= vec3(uWorldMatrix * vec4(aNormal, 0.0));
	vViewDir	= uCameraPosition - vPosition;
	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
}

#elif defined(FRAGMENT)	///////////////////////////////////////////////

in vec2	vTexCoord;
in vec3 vPosition;	
in vec3 vNormal;
in vec3 vViewDir;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	vec4 texColor		= texture(uTexture, vTexCoord);
	vec3 outputColor	= vec3(0.0, 0.0, 0.0);

	for (unsigned int i = 0; i < uLightCount; ++i)
	{
		Light light			= uLight[i];
		vec3  normDir		= vec3(0.0, 0.0, 0.0);
		float attenuation	= 1.0;
		
		if (light.type == LT_DIRECTIONAL)
		{ 
			normDir = normalize(light.direction); 
		}
		else if (light.type == LT_POINT)
		{ 
			vec3 distVec = (light.position - vPosition);
			
			normDir		= normalize(distVec); 
			float dist	= length(distVec);

			attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * (dist * dist));
		}

		vec3 rfltDir	= reflect(-normDir, normalize(vNormal));

		float dVal		= max(dot(normalize(vNormal), normDir), 0.0);
		float sVal		= pow(max(dot(vViewDir, rfltDir), 0.0), 0.8);
		
		vec3 diffuse	= dVal * light.color * texColor.xyz * 0.7;
		vec3 ambient	= light.color * texColor.xyz * 0.2;
		vec3 specular	= sVal * light.color * texColor.xyz * 0.1;

		outputColor += (specular + diffuse + ambient) * attenuation;
	}
	
	oColor = /*texColor;*/ vec4(outputColor, 1.0);
}

#endif	///////////////////////////////////////////////////////////////

#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef FRAMEBUFFER

#if defined(VERTEX) ///////////////////////////////////////////////////

layout(location = 0) in vec3 aPosition;
layout(location = 2) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord	= aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT) ///////////////////////////////////////////////

in vec2 vTexCoord;
uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = texture(uTexture, vTexCoord);
}

#endif ///////////////////////////////////////////////////////////////

#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

// NOTE: You can write several shaders in the same file if you want as
// long as you embrace them within an #ifdef block (as you can see above).
// The third parameter of the LoadProgram function in engine.cpp allows
// chosing the shader you want to load by name.