///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef FORWARD_RENDERING

struct Light
{
	unsigned int type;
	vec3		 color;
	vec3		 direction;
	vec3		 position;
};

#define LT_DIRECTIONAL	0
#define LT_POINT		1

#define RM_ALBEDO		0
#define RM_NORMAL		1
#define RM_SHADED		2

layout(binding = 0, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uRenderMode;
	unsigned int	uLightCount;
	Light			uLight[16];
};

#if defined(VERTEX)			// ----------------------------------------

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

#elif defined(FRAGMENT)		// ----------------------------------------

in vec2	vTexCoord;
in vec3 vPosition;	
in vec3 vNormal;
in vec3 vViewDir;

uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

vec3 ShadedRender(in vec4 texColor)
{
	vec3 outputColor = vec3(0.0, 0.0, 0.0);
	
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
		
		vec3 diffuse	= dVal * light.color * texColor.xyz * 0.65;					// 
		vec3 ambient	= light.color * texColor.xyz * 0.15;						// 0.05 less on all to avoid overexposure.
		vec3 specular	= sVal * light.color * texColor.xyz * 0.05;					// 

		outputColor += (specular + diffuse + ambient) * attenuation;
	}
	
	return outputColor;
}

void main()
{
	vec4 texColor		= texture(uTexture, vTexCoord);
	vec3 outputColor	= vec3(0.0, 0.0, 0.0);

	switch (uRenderMode)
	{
		case RM_ALBEDO:	{ outputColor = texColor.xyz; }				break;
		case RM_NORMAL:	{ outputColor = vNormal;}					break;
		case RM_SHADED:	{ outputColor = ShadedRender(texColor); }	break;
		default:		{ outputColor = vec3(0.0, 0.0, 0.0); }		break;
	}

	oColor = vec4(outputColor, 1.0);
}

#endif						// ----------------------------------------

#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef GEOMETRY_PASS

struct Light
{
	unsigned int type;
	vec3		 color;
	vec3		 direction;
	vec3		 position;
};

layout(binding = 0, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	//float			uCameraNearPlane;
	//float			uCameraFarPlane;
	unsigned int	uRenderMode;
	unsigned int	uLightCount;
	Light			uLight[16];
};

#if defined(VERTEX)			// ----------------------------------------

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

layout(binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;			
out vec3 vPosition;			// ---
out vec3 vNormal;			//
out vec3 vViewDir;			// In World Space
out vec3 vTangent;			//
out vec3 vBitangent;		// ---

void main()
{
	vTexCoord	= aTexCoord;
	vPosition	= vec3(uWorldMatrix * vec4(aPosition, 1.0));
	vNormal		= vec3(uWorldMatrix * vec4(aNormal, 0.0));
	vViewDir	= uCameraPosition - vPosition;
	vTangent	= normalize(vec3(uWorldMatrix * vec4(aTangent, 0.0)));
	vBitangent	= normalize(vec3(uWorldMatrix * vec4(aBitangent, 0.0)));

	gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT)		// ----------------------------------------

in vec2 vTexCoord;			
in vec3 vPosition;			// ---
in vec3 vNormal;			//
in vec3 vViewDir;			// In World Space
in vec3 vTangent;			//
in vec3 vBitangent;			// ---

uniform sampler2D	uTexture;
//uniform sampler2D	uNormalMap;
//uniform sampler2D	uBumpMap;
//uniform float		uBumpiness;
//uniform int			noTexture;
//uniform int			noNormal;
//uniform int			noBump;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 oNormals;
layout(location = 2) out vec4 oAlbedo;
layout(location = 3) out vec4 oDepth;
layout(location = 4) out vec4 oPosition;

float near	= 0.1;
float far	= 100.0;

float LinearizeDepth(in float depth)
{
	float z			= (depth * 2.0) - 1.0;
	float linDepth	= (2.0 * near * far) / (far + near - z * (far - near));

	return linDepth;
}

vec2 ReliefMapping(in vec2 texCoords, in mat3 TBN)
{
	
}

void main()
{
	vec3 T = normalize(vTangent);
	vec3 B = normalize(vBitangent);
	vec3 N = noramlize(vNormal);

	mat3 TBN		= mat3(T, B, N);
	vec2 texCoords	= vTexCoord;

	//if (noBump == 0)
	//{
	//	texCoords = ReliefMapping(vTexCoord, TBN);
	//}

	//if (noNormal == 0)
	//{
	//	vec3 tangentSpaceNormal = texture(uNormalMap, texCoords).xyz * 2.0 - vec3(1.0);
	//	N = TBN * tangentSpaceNormal;
	//}

	oNormals	= vec4(N, 1.0);
	oAlbedo		= (noTexture == 0) ? vec4(0.5) : texture(uTexture, texCoords);
	oDepth		= vec4(vec3(LinearizeDepth(gl_FragCoord.z) / far), 1.0);
	oPosition	= vec4(vPosition, 1.0);
}

#endif						// ----------------------------------------

#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef LIGHTING_PASS

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
	//float			uCameraNearPlane;
	//float			uCameraFarPlane;
	unsigned int	uRenderMode;
	unsigned int	uLightCount;
	Light			uLight[16];
};

#if defined (VERTEX)		// ----------------------------------------

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

layout(binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
};

out vec2 vTexCoord;
out vec3 vViewPos;			// In World Space

void main()
{
	vTexCoord = aTexCoord;
	vViewPos  = vec3(uCameraPosition);

	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT)		// ----------------------------------------

in vec2 vTexCoord;
in vec3 vViewPos;

uniform sampler2D oNormals;
uniform sampler2D oAlbedo;
uniform sampler2D oDepth;
uniform sampler2D oPosition;

layout(location = 0) out vec4 oColor;

vec3 GetLightDir(in Light light, in vec3 pos, out float attenuation)
{
	vec3 dir	= vec3(0.0);
	attenuation = 1.0;

	if (light.type == LT_DIRECTIONAL)
	{
		dir = normalize(light.direction);
	}
	else if (light.type == LT_POINT)
	{
		float dist		= length(light.position - pos);
		float linear	= 0.05; // 0.09
		float quadratic = 0.01;	// 0.032
		attenuation		= 1.0 / (1.0 + linear * dist + quadratic * (dist * dist));
		dir				= normalize(light.position - pos);
	}

	return dir;
}

void main()
{
	vec3 vPosition	= texture(oPosition, vTexCoord).rgb;
	vec3 vNormal	= texture(oNormals, vTexCoord).rgb;
	vec3 vAlbedo	= texture(oAlbedo, vTexCoord).rgb;
	vec3 vViewDir	= normalize((vViewPos - vPosition));			// uCameraPosition straight up?

	vec3  ambientColor	= albedo.xyz * 0.4;							// Ambient
	vec3  specular		= vec3(1.0);								// Material Parameter
	float shininess		= 120.0;									// Material Parameter

	vec3 N				= vNormal;
	vec3 finalDiffuse	= vec3(0.0);
	vec3 finalSpecular	= vec3(0.0);

	for (int i = 0; i < uLightCount; ++i)
	{
		Light light			= uLight[i];
		float attenuation	= 1.0;
		
		vec3 L				= GetLightDir(light, vPosition, attenuation);
		vec3 R				= reflect(-normDir, N);

		float diffIntensity = max(dot(N, L), 0.0);
		vec3 diffuseColor	= (albedo.xyz * light.color * diffIntensity) * attenuation;

		float specIntensity = pow(max(dot(vViewDir, R), 0.0), shininess);
		vec3 specularColor	= (attenuation * specular * light.color * specIntensity) * attenuation;

		finalDiffuse  += diffuseColor;
		finalSpecular += specularColor;
	}

	finalDiffuse  /= uLightCount;
	finalSpecular /= uLightcount;

	oColor = vec4(ambientColor + finalDiffuse + finalSpecular, 1.0);
}

#endif						// ----------------------------------------

#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef FRAMEBUFFER

#if defined(VERTEX)			// ----------------------------------------

layout(location = 0) in vec3 aPosition;
layout(location = 2) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord	= aTexCoord;
	gl_Position = vec4(aPosition, 1.0);
}

#elif defined(FRAGMENT)		// ----------------------------------------

in vec2 vTexCoord;
uniform sampler2D uTexture;

layout(location = 0) out vec4 oColor;

void main()
{
	oColor = texture(uTexture, vTexCoord);
}

#endif						// ----------------------------------------

#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////