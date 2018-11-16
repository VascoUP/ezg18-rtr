#pragma once

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

#include <GL\glew.h>

#include "ShaderCompiler.h"
#include "ErrorShader.h"
#include "Material.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Commons.h"

// This class holds the information needed to run simple shader program
class Shader
{
private:
	// ID of the shader
	GLuint shaderID;

	// -- Transformation --
	// Projection matrix
	GLuint uniformProjection;
	// View matrix
	GLuint uniformView;
	// Model matrix
	GLuint uniformModel;

	// -- Camera --
	// Camera Position
	GLuint uniformCameraPosition;

	// -- Ambiente Light --
	// Ambient light intensity
	GLuint uniformAmbientIntensity;

	// -- Directional light --
	struct {
		// Directional light diffuse color
		GLuint uniformDiffuseColor;
		// Directional light diffuse factor
		GLuint uniformDiffuseFactor;
		// Directional light diffuse color
		GLuint uniformSpecularColor;
		// Directional light diffuse factor
		GLuint uniformSpecularFactor;
		// Directional light direction
		GLuint uniformDirection;
	} uniformDirectionalLight;

	int pointLightCount;
	GLuint uniformPointLightCount;
	struct {
		GLuint uniformDiffuseColor;
		GLuint uniformDiffuseFactor;
		GLuint uniformSpecularColor;
		GLuint uniformSpecularFactor;

		GLuint uniformPosition;
		GLuint uniformConstant;
		GLuint uniformLinear;
		GLuint uniformExponent;
	} uniformPointLights[MAX_POINT_LIGHTS];

	// -- Material --
	struct  {
		// Specular intensity
		GLuint uniformSpecularIntensity;
		// Shininess
		GLuint uniformShininess;
		// Albedo
		GLuint uniformAlbedo;
	} uniformMaterial;

public:
	// Constructor
	Shader();

	// Given the vertex and fragment shader code it creates a shader program
	bool CreateFromString(const char* vertexCode, const char* fragmentCode);
	// Given the vertex and fragment shader files it creates a shader program
	bool CreateFromFiles(const char* vertexFile, const char* fragmentFile);
	// Reads shader code from a file
	std::string ReadFile(const char* file);
	// Getter for uniformProjection
	GLuint GetProjectionLocation();
	// Getter for uniformView
	GLuint GetViewLocation();
	// Getter for uniformModel
	GLuint GetModelLocation();

	// Getter for uniformCameraPosition
	GLuint GetCameraPositionLocation();

	// Getter for uniformAmbientIntensity
	GLuint GetAmbienteIntensityLocation();

	void SetDirectionalLight(DirectionalLight* light);

	void SetPointLights(PointLight* pLight, unsigned int lightCount);
	
	void SetMaterial(Material* mat);

	// Activates this shader whenever it's called
	void UseShader();
	// Frees the memory allocated by the shader program
	void ClearShader();

	// Destructor
	~Shader();
};

