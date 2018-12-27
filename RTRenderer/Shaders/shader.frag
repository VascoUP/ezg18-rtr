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


// --New lighting model
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
};

uniform sampler2D u_mainTexture;
uniform sampler2D u_directionalSM;

uniform	Material u_material;
uniform vec3 u_cameraPosition;

uniform float u_ambientFactor;
uniform DirectionalLight u_directionalLight;
uniform PointLight u_pointLights[MAX_POINT_LIGHTS];
uniform int u_pointLightsCount = 0;
uniform SpotLight u_spotLights[MAX_SPOT_LIGHTS];
uniform int u_spotLightsCount = 0;

float CalculateDirectionalShadowFactor(DirectionalLight light) 
{
	// Normalized coordinates
	vec3 projCoords = vert_directionalLightSpacePos.xyz / vert_directionalLightSpacePos.w;
	projCoords = (projCoords * 0.5) + 0.5;

	//float closestDepth = texture(u_directionalSM, projCoords.xy).x;
	float currentDepth = projCoords.z;

	vec3 normal = normalize(vert_normal);
	vec3 lightDir = normalize(light.direction);

	float bias = max(0.01 * (1.0 - dot(normal, lightDir)), 0.005);

	float shadow = 0.0;//currentDepth > closestDepth ? 1.0 : 0.0;

	vec2 texelSize = 1.0 / textureSize(u_directionalSM, 0);
	for(int x = -1; x < 1; x++) {
		for(int y = -1; y < 1; y++) {
			float pcfDepth = texture(u_directionalSM, projCoords.xy + vec2(x, y) * texelSize).x;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;

	if(projCoords.z > 1.0) {
		shadow = 0.0;
	}

	return shadow;
}

float CalculateAttenuation(float dist, float falloffStart, float falloffEnd)
{
	float attenuation = (falloffEnd - dist) / (falloffEnd - falloffStart);
    return clamp(attenuation, 0.0f, 1.0f);
}

vec4 CalculateLighting(FragParams frag, vec3 matColor, float matShininess, vec3 nLightToFrag, Light light, float shadowFactor) {
    vec4 outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    //Intensity of the diffuse light. Saturate to keep within the 0-1 range.
    float NdotL = dot( frag.frag_Normal, nLightToFrag );
    float intensity = clamp( NdotL, 0.0f, 1.0f );

    // Calculate the diffuse light factoring in light color, power and the attenuation
    outColor += intensity * vec4(light.diffuseColor, 1.0f) * vec4(light.diffuseFactor, 1.0f);

    //Calculate the half vector between the light vector and the view vector.
    //This is typically slower than calculating the actual reflection vector
    // due to the normalize function's reciprocal square root
    vec3 H = normalize( nLightToFrag + frag.frag_nvToCam );

    //Intensity of the specular light
    float NdotH = dot( frag.frag_Normal, H );
    intensity = pow( clamp( NdotH, 0.0f, 1.0f ), matShininess );

    //Sum up the specular light factoring
    outColor += intensity * vec4(light.specularColor, 1.0f) * vec4(light.specularFactor, 1.0f);

    return (1.0 - shadowFactor) * outColor * vec4(matColor, 1.0f);
}
 
vec4 CalculateDirectionalLight(FragParams frag, vec3 matColor, float matShininess, DirectionalLight light) {
	return CalculateLighting(frag, u_material.albedo, u_material.shininess, -u_directionalLight.direction, u_directionalLight.light, CalculateDirectionalShadowFactor(light));
}

vec4 CalculatePointLight(FragParams frag, vec3 matColor, float matShininess, PointLight light) {
	vec3 lightToFrag = light.position - frag.frag_Position;
	float dLightToFrag = length(lightToFrag);
	lightToFrag = normalize(lightToFrag);

	vec4 plColor = CalculateLighting(frag, u_material.albedo, u_material.shininess, lightToFrag, light.light, 0.0);

	// Calculate attenuation based on distance
	float attenuation = light.exponent * dLightToFrag * dLightToFrag + light.linear * dLightToFrag + light.constant;

	return plColor / attenuation;
}


vec4 CalculatePointLights(FragParams frag, vec3 matColor, float matShininess, PointLight lights[MAX_POINT_LIGHTS], int pointLightCount) {
	vec4 plsColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	for(int i = 0; i < pointLightCount; i++) {
		plsColor += CalculatePointLight(frag, matColor, matShininess, lights[i]);
	}

	return plsColor;
}

vec4 CalculateSpotLight(FragParams frag, vec3 matColor, float matShininess, float slFactor, SpotLight light) {
	vec4 color = CalculatePointLight(frag, matColor, matShininess, light.light);
	return color * (1.0f - (1.0f - slFactor) * (1.0f / (1.0f - light.edge)));
}

vec4 CalculateSpotLights(FragParams frag, vec3 matColor, float matShininess, SpotLight lights[MAX_POINT_LIGHTS], int spotLightCount) {
	vec4 plsColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	for(int i = 0; i < spotLightCount; i++) {
		vec3 rayDirection = normalize(frag.frag_Position - lights[i].light.position);
		float slFactor = dot(rayDirection, lights[i].direction);
		if(slFactor > lights[i].edge) {
			plsColor += CalculateSpotLight(frag, matColor, matShininess, slFactor, lights[i]);
		}
	}

	return plsColor;
}

void main()
{
	vec4 textColor = texture(u_mainTexture, vert_mainTex);
	if(textColor.a <= 0.5f)
		discard;

	FragParams frag;
	frag.frag_Position = vert_pos;
	frag.frag_Normal = -vert_normal;
	frag.frag_nvToCam = normalize(u_cameraPosition - vert_pos);
	
	vec4 dlColor = CalculateDirectionalLight(frag, u_material.albedo, u_material.shininess, u_directionalLight);
	vec4 plsColor = CalculatePointLights(frag, u_material.albedo, u_material.shininess, u_pointLights, u_pointLightsCount);
	vec4 slsColor = CalculateSpotLights(frag, u_material.albedo, u_material.shininess, u_spotLights, u_spotLightsCount);
	vec4 aColor = vec4(u_ambientFactor, u_ambientFactor, u_ambientFactor, 1.0f);
	
	frag_color = textColor * clamp( dlColor + plsColor + slsColor + aColor, 0.0f, 1.0f );
}
