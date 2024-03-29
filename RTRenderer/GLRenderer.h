#pragma once

#include <vector>

#include <GL\glew.h>

#include "Commons.h"

#include "Mesh.h"
#include "Model.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"
#include "Material.h"
#include "Light.h"
#include "CubeMap.h"
#include "SkyBox.h"

class GLObject
{
private:
	Transform* m_transform;
	Material* m_material;
	size_t m_modelIndex;
public:
	GLObject(Transform *transform, Material* material, size_t modelIndex);

	bool FilterPass(RenderFilter filter);
	void UseMaterial(LightedShader* shader);
	size_t GetModelIndex() const;
	glm::mat4 GetTransformMatrix() const;

	~GLObject();
};

class GLObjectRenderer {
protected:
	bool m_useInstanciation = false;
	
	IRenderable* m_renderable;
	std::vector<GLObject*> m_objects;
public:
	virtual void Render(RenderFilter filter, GLuint uniformModel, LightedShader* shader = nullptr) = 0;
	virtual void IncrementVertices() = 0;

	void AddMeshRenderer(GLObject* meshRenderer);
	void SetIndex(size_t index) { m_renderable->SetIndex(index); }
	void Clear();

	~GLObjectRenderer();
};

class GLModelRenderer 
	: public GLObjectRenderer
{
public:
	void SetRenderable(Model* renderable);
	void Render(RenderFilter filter, GLuint uniformModel, LightedShader* shader = nullptr);
	void IncrementVertices() override;
};

class GLMeshRenderer
	: public GLObjectRenderer
{
public:
	void SetRenderable(Mesh* renderable);
	void Render(RenderFilter filter, GLuint uniformModel, LightedShader* shader = nullptr);
	void IncrementVertices() override;
};

class GLRenderer;

class GLCubeMapRenderer {
private:
	CubeMapRenderShader* m_cubemapShader;
	GLModelRenderer* m_refractModel;
	GLModelRenderer* m_reflectModel;
	Transform* m_refractTransform;
	Transform* m_reflectTransform;
	CubeMap* m_refract;
	CubeMap* m_reflect;
	
public:
	GLCubeMapRenderer();

	void Initialize(Transform* transform);

	Transform* GetRefractTransform() {
		return m_refractTransform;
	};
	Transform* GetReflectTransform() {
		return m_reflectTransform;
	};

	void CubeMapPass(GLRenderer* glRenderer);
	void RenderModels(RenderFilter filter, GLuint uniformModel, LightedShader* shader = nullptr);
	void Render(DefaultShader* shader, GLuint uniformModel, GLuint textureUnit);
	
	~GLCubeMapRenderer();
};

class GLRenderer
{
private:
	friend class GLCubeMapRenderer;

	std::vector<GLObjectRenderer*> m_renderables;

	DefaultShader* m_shader;
	DirectionalShadowMapShader* m_directionalSMShader;
	OmnidirectionalShadowMapShader* m_omnidirectionalSMShader;

	GLCubeMapRenderer* m_cubemapRenderer;

	SkyBox* m_skybox;

	bool m_directionalLightPassDone = false;

	GLfloat m_ambientIntensity;

	DirectionalLight* m_directionalLight;
	size_t m_pointLightsCount = 0;
	PointLight* m_pointLights[MAX_POINT_LIGHTS];
	size_t m_spotLightsCount = 0;
	SpotLight* m_spotLights[MAX_SPOT_LIGHTS];
public:
	GLRenderer(Transform* transform);

	GLCubeMapRenderer* GetCubemapRenderer();
	GLfloat GetAmbient();
	void SetAmbient(GLfloat intensity);
	DirectionalLight* GetDirectionalLight();
	Light* GetPointLightAt(size_t index);
	Light* GetSpotLightAt(size_t index);
	void SetDirectionalLight(DirectionalLight* light);
	void AddPointLight(PointLight* light);
	void AddSpotLight(SpotLight* light);
	void AddObjectRenderer(GLObjectRenderer* renderer);
	void AddMeshRenderer(GLObject * meshRenderer);
	void AddShader(DefaultShader* shader);
	void Render(GLWindow* glWindow, Transform* root, RenderFilter filter);
	void BakeStage(GLWindow* glWindow);

	~GLRenderer();

private:
	bool DynamicMeshes();
	void RenderScene(RenderFilter filter, GLuint uniformModel, LightedShader* shader = nullptr);
	void DirectionalSMPass(RenderFilter filter);
	void OmnidirectionalSMPass(PointLight* light, RenderFilter filter);
	void CubeMapPass(Transform* transport, CubeMapRenderShader* shader, CubeMap* cubemap);
	void RenderPass(RenderFilter filter);
};
