#include "Shader.h"

GLuint Shader::GetUniformLocation(const char * uniform)
{
	return glGetUniformLocation(shaderID, uniform);
}

std::string Shader::ReadFile(const char* file) {
	std::string code = "";
	std::ifstream fileStream(file, std::ios::in);

	if (!fileStream.is_open()) {
		printf("Failed to read %s! File does not exist", file);
		return "";
	}

	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		code.append(line + "\n");
	}
	fileStream.close();

	return code;
}

Shader::Shader()
{
	shaderID = 0;
}

void Shader::UseShader() {
	if (!shaderID) {
		printf("No initialized shader has been found");
		return;
	}
	glUseProgram(shaderID);
}

void Shader::ClearShader() {
	ShaderCompiler::ClearShader(&shaderID);
}

Shader::~Shader()
{
	ClearShader();
}


StandardShader::StandardShader() :
	Shader()
{}

bool StandardShader::CreateFromString(const char* vertexCode, const char* fragmentCode, const char* geometryCode) {
	bool success = true;
	shaderID = ShaderCompiler::CreateStandardShader(vertexCode, fragmentCode, geometryCode);
	// Fall back on the Error Shader if a problem has happened
	if (!shaderID) {
		ErrorShader* errShader = ErrorShader::GetInstance();
		shaderID = errShader->GetShaderID();
		success = false;
	}

	if (shaderID) {
		GetShaderUniforms();
	}

	return success;
}

bool StandardShader::CreateFromFiles(const char* vertexFile, const char* fragmentFile, const char* geometryFile) {
	bool success = true;
	std::string vertexString = ReadFile(vertexFile);
	std::string fragmentString = ReadFile(fragmentFile);

	if (vertexString == "" || fragmentString == "") {
		ErrorShader* errShader = ErrorShader::GetInstance();
		shaderID = errShader->GetShaderID();
		return false;
	}

	const char* vertexCode = vertexString.c_str();
	const char* fragmentCode = fragmentString.c_str();
	const char* geometryCode = nullptr;

	std::string geometryString = "";
	if (geometryFile != nullptr) {
		geometryString = ReadFile(geometryFile);
		if (geometryString != "") {
			geometryCode = geometryString.c_str();
		}
	}

	return CreateFromString(vertexCode, fragmentCode, geometryCode);
}

LightedShader::LightedShader()
	: StandardShader()
{
	pointLightCount = 0;
	spotLightCount = 0;

	uniformModel = 0;
	uniformCameraPosition = 0;
	uniformTexture = 0;
	uniformAmbientIntensity = 0;
	uniformDirectionalLight.uniformDiffuseColor = 0;
	uniformDirectionalLight.uniformDiffuseFactor = 0;
	uniformDirectionalLight.uniformSpecularColor = 0;
	uniformDirectionalLight.uniformSpecularFactor = 0;
	uniformDirectionalLight.uniformDirection = 0;
	uniformMaterial.uniformSpecularIntensity = 0;
	uniformMaterial.uniformShininess = 0;
	uniformMaterial.uniformAlbedo = 0;
	uniformPointLightCount = 0;
	uniformSpotLightCount = 0;
}

void LightedShader::GetShaderUniforms()
{
	uniformModel = GetUniformLocation("u_modelMatrix");

	uniformCameraPosition = GetUniformLocation("u_cameraPosition");
	uniformAmbientIntensity = GetUniformLocation("u_ambientFactor");

	// -- Material Uniforms -- 
	uniformMaterial.uniformSpecularIntensity = GetUniformLocation("u_material.specularIntensity");
	uniformMaterial.uniformShininess = GetUniformLocation("u_material.shininess");
	uniformMaterial.uniformAlbedo = GetUniformLocation("u_material.albedo");
	uniformTexture = GetUniformLocation("u_material.albedoTexture");

	// -- Directional Light Uniforms --
	uniformDirectionalLight.uniformDiffuseColor = GetUniformLocation("u_directionalLight.light.diffuseColor");
	uniformDirectionalLight.uniformDiffuseFactor = GetUniformLocation("u_directionalLight.light.diffuseFactor");
	uniformDirectionalLight.uniformSpecularColor = GetUniformLocation("u_directionalLight.light.specularColor");
	uniformDirectionalLight.uniformSpecularFactor = GetUniformLocation("u_directionalLight.light.specularFactor");
	uniformDirectionalLight.uniformDirection = GetUniformLocation("u_directionalLight.direction");

	// -- Point Lights Uniforms --
	uniformPointLightCount = GetUniformLocation("u_pointLightsCount");
	for (size_t i = 0; i < MAX_POINT_LIGHTS; i++) {
		char locBuff[100] = { "\0" };
		snprintf(locBuff, sizeof(locBuff), "u_pointLights[%d].light.diffuseColor", i);
		uniformPointLights[i].uniformDiffuseColor = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_pointLights[%d].light.diffuseFactor", i);
		uniformPointLights[i].uniformDiffuseFactor = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_pointLights[%d].light.specularColor", i);
		uniformPointLights[i].uniformSpecularColor = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_pointLights[%d].light.specularFactor", i);
		uniformPointLights[i].uniformSpecularFactor = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_pointLights[%d].position", i);
		uniformPointLights[i].uniformPosition = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_pointLights[%d].constant", i);
		uniformPointLights[i].uniformConstant = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_pointLights[%d].linear", i);
		uniformPointLights[i].uniformLinear = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_pointLights[%d].exponent", i);
		uniformPointLights[i].uniformExponent = GetUniformLocation(locBuff);
	}

	// -- Spot Light Uniforms --
	uniformSpotLightCount = GetUniformLocation("u_spotLightsCount");
	for (size_t i = 0; i < MAX_SPOT_LIGHTS; i++) {
		char locBuff[100] = { "\0" };
		snprintf(locBuff, sizeof(locBuff), "u_spotLights[%d].light.light.diffuseColor", i);
		uniformSpotLights[i].uniformDiffuseColor = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_spotLights[%d].light.light.diffuseFactor", i);
		uniformSpotLights[i].uniformDiffuseFactor = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_spotLights[%d].light.light.specularColor", i);
		uniformSpotLights[i].uniformSpecularColor = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_spotLights[%d].light.light.specularFactor", i);
		uniformSpotLights[i].uniformSpecularFactor = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_spotLights[%d].light.position", i);
		uniformSpotLights[i].uniformPosition = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_spotLights[%d].light.constant", i);
		uniformSpotLights[i].uniformConstant = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_spotLights[%d].light.linear", i);
		uniformSpotLights[i].uniformLinear = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_spotLights[%d].light.exponent", i);
		uniformSpotLights[i].uniformExponent = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_spotLights[%d].direction", i);
		uniformSpotLights[i].uniformDirection = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_spotLights[%d].edge", i);
		uniformSpotLights[i].uniformEdge = GetUniformLocation(locBuff);
	}
}

GLuint LightedShader::GetModelLocation() {
	return uniformModel;
}

void LightedShader::SetCameraPosition(glm::vec3 * cPosition)
{
	glUniform3f(uniformCameraPosition, cPosition->x, cPosition->y, cPosition->z);
}

void LightedShader::SetAmbientIntensity(GLfloat aIntensity)
{
	glUniform1f(uniformAmbientIntensity, aIntensity);
}

void LightedShader::SetDirectionalLight(DirectionalLight * light)
{
	light->UseLight(uniformDirectionalLight.uniformDirection, uniformDirectionalLight.uniformDiffuseColor, uniformDirectionalLight.uniformDiffuseFactor, uniformDirectionalLight.uniformSpecularColor, uniformDirectionalLight.uniformSpecularFactor);
}

void LightedShader::SetPointLights(PointLight **pLight, unsigned int lightCount, unsigned int textureUnit, unsigned int offset)
{
	if (lightCount > MAX_POINT_LIGHTS)
		lightCount = MAX_POINT_LIGHTS;
	glUniform1i(uniformPointLightCount, lightCount);
	for (size_t i = 0; i < lightCount; i++) {
		pLight[i]->UseLight(uniformPointLights[i].uniformPosition, uniformPointLights[i].uniformConstant,
			uniformPointLights[i].uniformLinear, uniformPointLights[i].uniformExponent, uniformPointLights[i].uniformDiffuseColor,
			uniformPointLights[i].uniformDiffuseFactor, uniformPointLights[i].uniformSpecularColor, uniformPointLights[i].uniformSpecularFactor);
		pLight[i]->GetStaticShadowMap()->Read(GL_TEXTURE0 + textureUnit + i);
	}
}

void LightedShader::SetSpotLights(SpotLight **sLight, unsigned int lightCount, unsigned int textureUnit, unsigned int offset)
{
	if (lightCount > MAX_SPOT_LIGHTS)
		lightCount = MAX_SPOT_LIGHTS;
	glUniform1i(uniformSpotLightCount, lightCount);
	for (size_t i = 0; i < lightCount; i++) {
		sLight[i]->UseLight(uniformSpotLights[i].uniformDirection, uniformSpotLights[i].uniformEdge,
			uniformSpotLights[i].uniformPosition, uniformSpotLights[i].uniformConstant, uniformSpotLights[i].uniformLinear, uniformSpotLights[i].uniformExponent,
			uniformSpotLights[i].uniformDiffuseColor, uniformSpotLights[i].uniformDiffuseFactor,
			uniformSpotLights[i].uniformSpecularColor, uniformSpotLights[i].uniformSpecularFactor);
		sLight[i]->GetStaticShadowMap()->Read(GL_TEXTURE0 + textureUnit + i);
	}
}

void LightedShader::SetMaterial(Material * mat)
{
	mat->UseMaterial(uniformMaterial.uniformSpecularIntensity, uniformMaterial.uniformShininess, uniformMaterial.uniformAlbedo);
}

void LightedShader::SetTexutre(GLuint textureUnit)
{
	glUniform1i(uniformTexture, textureUnit);
}


DefaultShader::DefaultShader()
	: LightedShader()
{
	pointLightCount = 0;
	spotLightCount = 0;

	uniformView = 0;
	uniformProjection = 0;

	uniformDirectionalSM.uniformStaticShadowMap = 0;
	uniformDirectionalSM.uniformDynamicShadowMap = 0;

	uniformSkybox = 0;
	uniformWorldReflection = 0;
	uniformReflectionFactor = 0;
	uniformRefractionFactor = 0;
	uniformIORValues = 0;
	uniformFresnelValues = 0;
}

GLuint DefaultShader::GetWorldReflection() const
{
	return uniformWorldReflection;
}

void DefaultShader::GetShaderUniforms()
{
	LightedShader::GetShaderUniforms();

	uniformView = GetUniformLocation("u_viewMatrix");
	uniformProjection = GetUniformLocation("u_projectionMatrix");

	// -- Directional shadow maps --
	uniformDirectionalLightTransform = GetUniformLocation("u_directionalLightTransform");
	uniformDirectionalSM.uniformStaticShadowMap = GetUniformLocation("u_directionalSM.static_shadowmap");
	uniformDirectionalSM.uniformDynamicShadowMap = GetUniformLocation("u_directionalSM.dynamic_shadowmap");

	// -- Omni shadow maps --
	for (size_t i = 0; i < MAX_POINT_LIGHTS + MAX_SPOT_LIGHTS; i++) {
		char locBuff[100] = { "\0" };
		snprintf(locBuff, sizeof(locBuff), "u_omniSM[%d].static_shadowmap", i);
		uniformOmniSM[i].uniformStaticShadowMap = GetUniformLocation(locBuff);
		snprintf(locBuff, sizeof(locBuff), "u_omniSM[%d].farPlane", i);
		uniformOmniSM[i].uniformFarPlane = GetUniformLocation(locBuff);
	}
	
	// -- Reflections --
	uniformSkybox = GetUniformLocation("u_skybox");
	uniformWorldReflection = GetUniformLocation("u_worldReflection");
	uniformReflectionFactor = GetUniformLocation("u_reflectionFactor");
	uniformRefractionFactor = GetUniformLocation("u_refractionFactor");
	uniformIORValues = GetUniformLocation("u_IoRValues");
	uniformFresnelValues = GetUniformLocation("u_fresnelValues");
}

void DefaultShader::SetProjectionMatrix(glm::mat4 * pMatrix)
{
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(*pMatrix));
}

void DefaultShader::SetViewMatrix(glm::mat4 * vMatrix)
{
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(*vMatrix));
}

void DefaultShader::SetPointLights(PointLight **pLight, unsigned int lightCount, unsigned int textureUnit, unsigned int offset)
{
	if (lightCount > MAX_POINT_LIGHTS)
		lightCount = MAX_POINT_LIGHTS;
	glUniform1i(uniformPointLightCount, lightCount);
	for (size_t i = 0; i < lightCount; i++) {
		pLight[i]->UseLight(uniformPointLights[i].uniformPosition, uniformPointLights[i].uniformConstant,
			uniformPointLights[i].uniformLinear, uniformPointLights[i].uniformExponent, uniformPointLights[i].uniformDiffuseColor,
			uniformPointLights[i].uniformDiffuseFactor, uniformPointLights[i].uniformSpecularColor, uniformPointLights[i].uniformSpecularFactor);
		pLight[i]->GetStaticShadowMap()->Read(GL_TEXTURE0 + textureUnit + i);
		glUniform1i(uniformOmniSM[i + offset].uniformStaticShadowMap, textureUnit + i);
		glUniform1f(uniformOmniSM[i + offset].uniformFarPlane, pLight[i]->GetFarPlane());
	}
}

void DefaultShader::SetSpotLights(SpotLight **sLight, unsigned int lightCount, unsigned int textureUnit, unsigned int offset)
{
	if (lightCount > MAX_SPOT_LIGHTS)
		lightCount = MAX_SPOT_LIGHTS;
	glUniform1i(uniformSpotLightCount, lightCount);
	for (size_t i = 0; i < lightCount; i++) {
		sLight[i]->UseLight(uniformSpotLights[i].uniformDirection, uniformSpotLights[i].uniformEdge,
			uniformSpotLights[i].uniformPosition, uniformSpotLights[i].uniformConstant, uniformSpotLights[i].uniformLinear, uniformSpotLights[i].uniformExponent,
			uniformSpotLights[i].uniformDiffuseColor, uniformSpotLights[i].uniformDiffuseFactor,
			uniformSpotLights[i].uniformSpecularColor, uniformSpotLights[i].uniformSpecularFactor);
		sLight[i]->GetStaticShadowMap()->Read(GL_TEXTURE0 + textureUnit + i);
		glUniform1i(uniformOmniSM[i + offset].uniformStaticShadowMap, textureUnit + i);
		glUniform1f(uniformOmniSM[i + offset].uniformFarPlane, sLight[i]->GetFarPlane());
	}
}

void DefaultShader::SetDirectionalStaticSM(GLuint textureUnit)
{
	glUniform1i(uniformDirectionalSM.uniformStaticShadowMap, textureUnit);
}

void DefaultShader::SetDirectionalDynamicSM(GLuint textureUnit)
{
	glUniform1i(uniformDirectionalSM.uniformDynamicShadowMap, textureUnit);
}

void DefaultShader::SetDirectionalLightTransform(glm::mat4 * lTransform)
{
	glUniformMatrix4fv(uniformDirectionalLightTransform, 1, GL_FALSE, glm::value_ptr(*lTransform));
}

void DefaultShader::SetSkybox(GLuint textureUnit)
{
	glUniform1i(uniformSkybox, textureUnit);
}

void DefaultShader::SetWorldReflection(GLuint textureUnit) {
	glUniform1i(uniformWorldReflection, textureUnit);
}

void DefaultShader::SetReflectionFactor(GLfloat factor) {
	glUniform1f(uniformReflectionFactor, factor);
}

void DefaultShader::SetRefractionFactor(GLfloat factor) {
	glUniform1f(uniformRefractionFactor, factor);
}

void DefaultShader::SetFresnelValues(GLfloat bias, GLfloat power, GLfloat scale) {
	glUniform3f(uniformFresnelValues, bias, power, scale);
}

void DefaultShader::SetIORValue(GLfloat x, GLfloat y, GLfloat z) {
	glUniform3f(uniformIORValues, x, y, z);
}


CubeMapRenderShader::CubeMapRenderShader() :
	LightedShader()
{}

void CubeMapRenderShader::GetShaderUniforms()
{
	LightedShader::GetShaderUniforms();

	// -- Light matrices -- 
	for (size_t i = 0; i < 6; i++) {
		char locBuff[100] = { "\0" };
		snprintf(locBuff, sizeof(locBuff), "u_viewProjectionMatrices[%d]", i);
		uniformViewProjectionMatrices[i] = GetUniformLocation(locBuff);
	}
}

void CubeMapRenderShader::SetViewProjectMatrices(std::vector<glm::mat4> viewProjectionMatrices) {
	for (size_t i = 0; i < 6; i++) {
		glUniformMatrix4fv(uniformViewProjectionMatrices[i], 1, GL_FALSE, glm::value_ptr(viewProjectionMatrices[i]));
	}
}


DirectionalShadowMapShader::DirectionalShadowMapShader() :
	StandardShader()
{
	uniformModel = 0;
	uniformDirectionalLightTransform = 0;
}

void DirectionalShadowMapShader::GetShaderUniforms()
{
	uniformModel = GetUniformLocation("u_modelMatrix");
	uniformDirectionalLightTransform = GetUniformLocation("u_directionalLightTransform");
	uniformTexture = GetUniformLocation("u_texture");
}

GLuint DirectionalShadowMapShader::GetModelLocation()
{
	return uniformModel;
}

void DirectionalShadowMapShader::SetModel(glm::mat4 * mMatrix)
{
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(*mMatrix));
}

void DirectionalShadowMapShader::SetDirectionalLightTransform(glm::mat4 * lTransform)
{
	glUniformMatrix4fv(uniformDirectionalLightTransform, 1, GL_FALSE, glm::value_ptr(*lTransform));
}

void DirectionalShadowMapShader::SetTexture(GLuint textureUnit)
{
	glUniform1i(uniformTexture, textureUnit);
}


OmnidirectionalShadowMapShader::OmnidirectionalShadowMapShader() :
	StandardShader() 
{
	uniformModel = 0;
	uniformLightPos = 0;
	uniformFarPlane = 0;
}

void OmnidirectionalShadowMapShader::GetShaderUniforms() {

	uniformModel = GetUniformLocation("u_modelMatrix");
	uniformLightPos = GetUniformLocation("u_lightPos");
	uniformFarPlane = GetUniformLocation("u_farPlane");

	for (size_t i = 0; i < 6; i++) {
		char locBuff[100] = { "\0" };
		snprintf(locBuff, sizeof(locBuff), "u_viewProjectionMatrices[%d]", i);
		uniformLightMatrices[i] = GetUniformLocation(locBuff);
	}

	uniformTexture = GetUniformLocation("u_texture");
}

GLuint OmnidirectionalShadowMapShader::GetModelLocation()
{
	return uniformModel;
}

void OmnidirectionalShadowMapShader::SetModel(glm::mat4 * mMatrix)
{
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(*mMatrix));
}

void OmnidirectionalShadowMapShader::SetLightPosition(glm::vec3* lPos) {
	glUniform3f(uniformLightPos, lPos->x, lPos->y, lPos->z);
}

void OmnidirectionalShadowMapShader::SetFarPlane(GLfloat far) {
	glUniform1f(uniformFarPlane, far);
}

void OmnidirectionalShadowMapShader::SetLightMatrices(std::vector<glm::mat4> lightMatrices) {
	for (size_t i = 0; i < 6; i++) {
		glUniformMatrix4fv(uniformLightMatrices[i], 1, GL_FALSE, glm::value_ptr(lightMatrices[i]));
	}
}

void OmnidirectionalShadowMapShader::SetTexture(GLuint unit) {
	glUniform1i(uniformTexture, unit);
}

SkyBoxShader::SkyBoxShader() :
	StandardShader()
{
	uniformSkyBox = 0;
	uniformViewProjectionMatrix = 0;
}

void SkyBoxShader::GetShaderUniforms()
{
	uniformSkyBox = GetUniformLocation("u_skybox");
	uniformViewProjectionMatrix = GetUniformLocation("u_viewProjectionMatrix");
}

void SkyBoxShader::SetSkyBox(GLuint textureUnit)
{
	glUniform1i(uniformSkyBox, textureUnit);
}

void SkyBoxShader::SetViewProjectionMatrix(glm::mat4* viewProjectionMatrix)
{
	glUniformMatrix4fv(uniformViewProjectionMatrix, 1, GL_FALSE, glm::value_ptr(*viewProjectionMatrix));
}


CustomShader::CustomShader() :
	StandardShader()
{}

GLuint CustomShader::GetShaderID()
{
	return shaderID;
}

void CustomShader::GetShaderUniforms()
{}


ComputeShader::ComputeShader() :
	Shader() {}

bool ComputeShader::CreateFromString(const char * computeCode)
{
	shaderID = ShaderCompiler::CreateSingleShader(computeCode, GL_COMPUTE_SHADER);
	// Fall back on the Error Shader if a problem has happened
	if (!shaderID)
		return false;

	//GetShaderUniforms();

	return true;
}