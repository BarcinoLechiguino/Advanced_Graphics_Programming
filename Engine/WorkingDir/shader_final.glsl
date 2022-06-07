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

#define RL_SHADED		0
#define RL_ALBEDO		1
#define RL_NORMAL		2
#define RL_DEPTH		3
#define RL_POSITION		4

layout(binding = 0, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uRenderLayer;
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

float near	= 0.1;
float far	= 100.0;

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

float LinearizeDepth(in float depth)
{	
	float z			= (depth * 2.0) - 1.0;
	float linDepth	= (2.0 * near * far) / (far + near - z * (far - near));

	return linDepth;
}

void main()
{
	vec4 texColor		= texture(uTexture, vTexCoord);
	vec3 outputColor	= vec3(0.0, 0.0, 0.0);

	switch (uRenderLayer)
	{
		case RL_SHADED:		{ outputColor = ShadedRender(texColor); }						break;
		case RL_ALBEDO:		{ outputColor = texColor.xyz; }									break;
		case RL_NORMAL:		{ outputColor = vNormal; }										break;
		case RL_DEPTH:		{ outputColor = vec3(LinearizeDepth(gl_FragCoord.z) / far); }	break;
		case RL_POSITION:	{ outputColor = vPosition; }									break;
		default:			{ outputColor = vec3(0.0, 0.0, 0.0); }							break;
	}

	oColor = vec4(outputColor, 1.0);
}

#endif						// ----------------------------------------

#endif

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

#ifdef GEOMETRY_PASS

layout(binding = 0, std140) uniform GlobalParams
{
	vec3			uCameraPosition;
	unsigned int	uRenderLayer;
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
	unsigned int isCube;
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

layout(binding = 1, std140) uniform LocalParams
{
	mat4 uWorldMatrix;
	mat4 uWorldViewProjectionMatrix;
	unsigned int isCube;
};

in vec2 vTexCoord;			
in vec3 vPosition;			// ---
in vec3 vNormal;			//
in vec3 vViewDir;			// In World Space
in vec3 vTangent;			//
in vec3 vBitangent;			// ---

uniform sampler2D	uTexture;
uniform sampler2D	uNormalMap;
uniform sampler2D	uBumpMap;
uniform float		uBumpiness;

layout(location = 0) out vec4 oColor;
layout(location = 1) out vec4 oAlbedo;
layout(location = 2) out vec4 oNormals;
layout(location = 3) out vec4 oDepth;
layout(location = 4) out vec4 oPosition;

float near	= 0.1;
float far	= 1000.0;

float LinearizeDepth(in float depth)
{
	float z			= (depth * 2.0) - 1.0;
	float linDepth	= (2.0 * near * far) / (far + near - z * (far - near));

	return linDepth;
}

vec2 ReliefMapping(in vec2 texCoords, in mat3 TBN)
{
	int numSteps = 45;

	// Compute the view ray in texture space
	vec3 rayTexSpace = transpose(TBN) * normalize(vViewDir.xyz);

	// Increment
	float texSize = 256;																			// HARDCODED RELIEF TEX SIZE
	vec3 rayIncrementTexSpace;
	rayIncrementTexSpace.xy = -1 * uBumpiness * rayTexSpace.xy / abs(rayTexSpace.z * texSize);
	rayIncrementTexSpace.z = 1.0 / numSteps;

	// Sampling state
	vec3 samplePositionTexspace = vec3(texCoords, 0.0);
	float sampledDepth = texture(uBumpMap, samplePositionTexspace.xy).r;

	// Linear search
	for (int i = 0; i < numSteps && samplePositionTexspace.z < sampledDepth; ++i)
	{
		samplePositionTexspace += rayIncrementTexSpace;
		sampledDepth = texture(uBumpMap, samplePositionTexspace.xy).r;
	}

	return samplePositionTexspace.xy;
}

void main()
{
	vec3 T = normalize(vTangent);
	vec3 B = normalize(vBitangent);
	vec3 N = normalize(vNormal);

	mat3 TBN		= mat3(T, B, N);							// Normals from tangent space to world space
	vec2 texCoords	= vTexCoord;
	
	if (isCube == 0)
	{
		texCoords = ReliefMapping(vTexCoord, TBN);

		vec3 tangentSpaceNormal = texture(uNormalMap, texCoords).xyz * 2.0 - vec3(1.0);
		N = TBN * tangentSpaceNormal;
		N = normalize(uWorldMatrix * vec4(N, 0.0)).xyz;
	}

	oAlbedo		= texture(uTexture, texCoords);
	oNormals	= vec4(N, 1.0);
	oDepth		= vec4(vec3(LinearizeDepth(gl_FragCoord.z) / far), 1.0);
	oPosition	= vec4(vPosition, 1.0);
	//oColor		= oAlbedo;
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
	unsigned int	uRenderLayer;
};

layout(binding = 2, std140) uniform LightParams
{
	mat4  uWorldMatrix;
	mat4  uWorldViewProjectionMatrix;
	Light light;
};

#if defined (VERTEX)		// ----------------------------------------

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	if (light.type == LT_DIRECTIONAL)
	{
		vTexCoord	= aTexCoord;
		gl_Position = vec4(aPosition, 1.0);
	}
	else if (light.type == LT_POINT)
	{
		gl_Position = uWorldViewProjectionMatrix * vec4(aPosition, 1.0);
	}
	else
	{
		vTexCoord	= aTexCoord;
		gl_Position = vec4(aPosition, 1.0);
	}
}

#elif defined(FRAGMENT)		// ----------------------------------------

in vec2 vTexCoord;

uniform sampler2D oAlbedo;
uniform sampler2D oNormals;
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
	vec2 texCoords = vTexCoord;
	
	vec3 vPosition	= texture(oPosition, texCoords).rgb;
	vec3 vNormal	= texture(oNormals,  texCoords).rgb;
	vec3 vAlbedo	= texture(oAlbedo,	 texCoords).rgb;
	vec3 vViewDir	= normalize((uCameraPosition - vPosition));

	vec3  specularColor = vec3(1.0);								// Material Parameter
	float shininess		= 120.0;									// Material Parameter
	float attenuation	= 1.0;
	
	vec3  N	 = normalize(vNormal);									// NORMAL
	vec3  L	 = GetLightDir(light, vPosition, attenuation);			// LIGHT DIR
	vec3  R	 = reflect(-L, N);										// REFLECTED DIR
	float d0 = max(dot(N, L), 0.0);									// DIFFUSE INTENSITY
	float s0 = pow(max(dot(vViewDir, R), 0.0), shininess);			// SPECULAR INTENSITY

	vec3 ambient	= vAlbedo.xyz * 0.5;							// 0.05 less on all to avoid overexposure.
	vec3 diffuse	= light.color * vAlbedo.xyz * d0 * 0.65;		// 
	vec3 specular	= light.color * vAlbedo.xyz * s0 * 0.05;		// 

	oColor = vec4((ambient + diffuse + specular), 1.0);
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