#version 330

#define MAX_POINT_LIGHTS	3
#define MAX_SPOT_LIGHTS		3

in vec3 vert_normal;
in vec2 vert_mainTex;
in vec3 vert_pos;
in vec4 vert_directionalLightSpacePos;

out vec4 frag_color;

struct Light {
	vec3 diffuseColor;
	vec3 diffuseFactor;
	vec3 specularColor;
	vec3 specularFactor;
};

struct DirectionalLight 
{
	Light light;
    vec3 direction;
};

struct PointLight 
{
	Light light;
	vec3 position;
	float constant;
	float linear;
	float exponent;
};

struct SpotLight 
{
	PointLight light;
	vec3 direction;
	float edge;
};

struct FragParams {
	// Fragment position
	vec3 frag_Position;
	// Fragment normal
	vec3 frag_Normal;
	// Normalized vector (nv) from camera to fragment position
	vec3 frag_nvToCam;
};

struct Material {
	float specularIntensity;
	float shininess;
	vec3 albedo;
	sampler2D albedoTexture;
};

struct ShadowMap {
	sampler2D static_shadowmap;
	sampler2D dynamic_shadowmap;
};

struct OmniShadowMap {
	samplerCube static_shadowmap;
	float farPlane;
};


uniform vec3 u_cameraPosition;

uniform float u_ambientFactor;

uniform DirectionalLight u_directionalLight;
uniform ShadowMap u_directionalSM;

uniform OmniShadowMap u_omniSM[MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS];
uniform PointLight u_pointLights[MAX_POINT_LIGHTS];
uniform int u_pointLightsCount = 0;
uniform SpotLight u_spotLights[MAX_SPOT_LIGHTS];
uniform int u_spotLightsCount = 0;

uniform	Material u_material;

uniform samplerCube u_skybox;
uniform samplerCube u_worldReflection;
uniform float u_reflectionFactor;
uniform float u_refractionFactor;
uniform vec3 u_IoRValues;
uniform vec3 u_fresnelValues;

float CalculateOmniShadowFactor(PointLight light, int shadowMapIndex, vec3 fragToLight, float depth) {
	float shadow = 0.0;
	float samples = 3.0;
	float offset = 0.1;
	for(float x = -offset; x < offset; x += offset / (samples * 0.5))
	{
	    for(float y = -offset; y < offset; y += offset / (samples * 0.5))
	    {
			for(float z = -offset; z < offset; z += offset / (samples * 0.5))
			{
				float closestDepth = texture(u_omniSM[shadowMapIndex].static_shadowmap, fragToLight + vec3(x,y,z)).r;
				closestDepth *= u_omniSM[shadowMapIndex].farPlane;
				shadow += float(depth > closestDepth);				
			}
		}
	}

	shadow /= (samples * samples * samples);
	return float(depth < u_omniSM[shadowMapIndex].farPlane) * shadow;
}

float CalculateDirectionalShadowFactor(DirectionalLight light) 
{
	// Normalized coordinates
	vec3 projCoords = vert_directionalLightSpacePos.xyz / vert_directionalLightSpacePos.w;
	projCoords = (projCoords * 0.5) + 0.5;

	float currentDepth = projCoords.z;

	float shadow = 0.0;

	vec2 s_texelSize = 1.0 / textureSize(u_directionalSM.static_shadowmap, 0);
	vec2 d_texelSize = 1.0 / textureSize(u_directionalSM.dynamic_shadowmap, 0);
	for(int x = -1; x < 1; x++) {
		for(int y = -1; y < 1; y++) {
			float s_pcfDepth = texture(u_directionalSM.static_shadowmap, projCoords.xy + vec2(x, y) * s_texelSize).x;
			float d_pcfDepth = texture(u_directionalSM.dynamic_shadowmap, projCoords.xy + vec2(x, y) * d_texelSize).x;
			shadow += max(float(currentDepth > s_pcfDepth), float(currentDepth > d_pcfDepth));
		}
	}
	shadow /= 9.0;
	
	return float(projCoords.z < 1.0) * shadow;
}

float CalculateAttenuation(float dist, float falloffStart, float falloffEnd)
{
	float attenuation = (falloffEnd - dist) / (falloffEnd - falloffStart);
    return clamp(attenuation, 0.0, 1.0);
}

vec4 CalculateLighting(FragParams frag, vec3 matColor, float matSpecIntensity, float matShininess, vec3 nLightToFrag, Light light, float shadowFactor) {
    vec4 outColor = vec4(0.0, 0.0, 0.0, 1.0);

	// Diffuse
    float NdotL = dot( nLightToFrag, frag.frag_Normal );
    float intensity = max( NdotL, 0.0 );

    outColor += intensity * vec4(light.diffuseColor, 1.0f) * vec4(light.diffuseFactor, 1.0f);

	// Specular 
    vec3 H = normalize( nLightToFrag + frag.frag_nvToCam );
    float NdotH = max( dot( H, frag.frag_Normal ), 0.0 );
    intensity = pow( NdotH, matShininess );
	
    outColor += intensity * matSpecIntensity * vec4(light.specularColor, 1.0) * vec4(light.specularFactor, 1.0);

    return vec4((1.0 - shadowFactor) * outColor.rgb, 1.0) * vec4(matColor, 1.0);
}
 
vec4 CalculateDirectionalLight(FragParams frag, DirectionalLight light) {
	return CalculateLighting(frag, u_material.albedo, u_material.specularIntensity, u_material.shininess, -u_directionalLight.direction, u_directionalLight.light, CalculateDirectionalShadowFactor(light));
}

vec4 CalculatePointLight(FragParams frag, PointLight light, int shadowMapIndex) {
	vec3 lightToFrag = light.position - frag.frag_Position;
	float dLightToFrag = length(lightToFrag);
	vec3 nLightToFrag = normalize(lightToFrag);


	float shadowFactor = CalculateOmniShadowFactor(light, shadowMapIndex, -lightToFrag, dLightToFrag);
	vec4 plColor = CalculateLighting(frag, u_material.albedo, u_material.specularIntensity, u_material.shininess, nLightToFrag, light.light, shadowFactor);

	// Calculate attenuation based on distance
	float attenuation = light.exponent * dLightToFrag * dLightToFrag + light.linear * dLightToFrag + light.constant;

	return plColor / attenuation;
}


vec4 CalculatePointLights(FragParams frag, PointLight lights[MAX_POINT_LIGHTS], int pointLightCount) {
	vec4 plsColor = vec4(0.0, 0.0, 0.0, 1.0);
	for(int i = 0; i < pointLightCount; i++) {
		plsColor += CalculatePointLight(frag, lights[i], i);
	}

	return plsColor;
}

vec4 CalculateSpotLight(FragParams frag, float slFactor, SpotLight light, int shadowMapIndex) {
	vec4 color = CalculatePointLight(frag, light.light, shadowMapIndex);
	return color * (1.0 - (1.0 - slFactor) * (1.0 / (1.0 - light.edge)));
}

vec4 CalculateSpotLights(FragParams frag, SpotLight lights[MAX_POINT_LIGHTS], int spotLightCount) {
	vec4 plsColor = vec4(0.0, 0.0, 0.0, 1.0);
	for(int i = 0; i < spotLightCount; i++) {
		vec3 rayDirection = normalize(frag.frag_Position - lights[i].light.position);
		float slFactor = dot(rayDirection, lights[i].direction);
		if(slFactor > lights[i].edge) {
			plsColor += CalculateSpotLight(frag, slFactor, lights[i], i + u_pointLightsCount);
		}
	}

	return plsColor;
}

vec4 CalculateLigthing(FragParams frag) {
	vec4 dlColor = CalculateDirectionalLight(frag, u_directionalLight);
	vec4 plsColor = CalculatePointLights(frag, u_pointLights, u_pointLightsCount);
	vec4 slsColor = CalculateSpotLights(frag, u_spotLights, u_spotLightsCount);
	vec4 aColor = vec4(u_ambientFactor, u_ambientFactor, u_ambientFactor, 1.0);
	return dlColor + plsColor + slsColor + aColor;
}

float FresnelApproximation(float iDotN, vec3 fresnelValues)
{
    float bias = fresnelValues.x;
    float power = fresnelValues.y;
    float scale = fresnelValues.z;
    return bias + pow(1.0 - iDotN, power) * scale;
}

vec4 CalculateReflection(FragParams frag) {
	vec3 reflectVec = reflect(-frag.frag_nvToCam, frag.frag_Normal);
	vec3 refColor = texture(u_worldReflection, reflectVec).rgb;
	if(equal(refColor, vec3(0.0, 0.0, 0.0)) == bvec3(1, 1, 1)) {
		return vec4(texture(u_skybox, frag.frag_Normal).rgb, 1.0);
	}
	return vec4(refColor, 1.0);
}

vec4 CalculateRefraction(FragParams frag) {
	// -- Refraction color --
	vec3 refractColor;
	// Red
	vec4 refractTex = texture(u_worldReflection, refract(-frag.frag_nvToCam, frag.frag_Normal, u_IoRValues.x));
	refractColor.r = 
		refractTex.r * refractTex.a +
		texture(u_skybox, refract(-frag.frag_nvToCam, frag.frag_Normal, u_IoRValues.x)).r * (1.0 - refractTex.a);
	// Green
	refractTex = texture(u_worldReflection, refract(-frag.frag_nvToCam, frag.frag_Normal, u_IoRValues.y));
	refractColor.g = 
		refractTex.g * refractTex.a +
		texture(u_skybox, refract(-frag.frag_nvToCam, frag.frag_Normal, u_IoRValues.y)).g * (1.0 - refractTex.a);
	// Blue
	refractTex = texture(u_worldReflection, refract(-frag.frag_nvToCam, frag.frag_Normal, u_IoRValues.z));
	refractColor.b = 
		refractTex.b * refractTex.a +
		texture(u_skybox, refract(-frag.frag_nvToCam, frag.frag_Normal, u_IoRValues.z)).b * (1.0 - refractTex.a);
	return vec4(refractColor, 1.0);
}


void main()
{
	vec4 tColor = texture(u_material.albedoTexture, vert_mainTex);
	if(tColor.a < 0.8)
		discard;

	FragParams frag;
	frag.frag_Position = vert_pos;
	frag.frag_Normal = -vert_normal;
	frag.frag_nvToCam = normalize(u_cameraPosition - vert_pos);
	
	// -- Color from lights --
	vec4 lColor = CalculateLigthing(frag);
	
	// -- Color from reflection and refraction -- 
	if(u_reflectionFactor > 0.1) {
		vec4 rflColor = CalculateReflection(frag);
		vec4 rfrColor = CalculateRefraction(frag);
	
		float fresnelTerm = FresnelApproximation(dot(frag.frag_nvToCam, frag.frag_Normal), u_fresnelValues);

		// -- Result --
		frag_color = mix(tColor, mix(rfrColor, rflColor * lColor, fresnelTerm), u_reflectionFactor);
	} else {
		frag_color = tColor * lColor;
	}

	frag_color = clamp(frag_color, 0.0, 1.0);
}
