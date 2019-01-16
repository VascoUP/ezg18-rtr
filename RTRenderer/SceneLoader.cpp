#include "SceneLoader.h"

enum LightType {
	SPOT, POINT, DIRECTIONAL
};

const std::string TRANSFORMS_KEY = "transforms";
const std::string TRANSFORM_KEY = "transform";

const std::string POSITION_KEY = "translation";
const std::string ROTATION_KEY = "rotation";
const std::string SCALE_KEY = "scale";

const std::string AMBIENT_KEY = "ambient";
const std::string DIFFUSE_COLOR_KEY = "diffcolor";
const std::string DIFFUSE_INTENSITY_KEY = "diffintensity";
const std::string SPECULAR_COLOR_KEY = "speccolor";
const std::string SPECULAR_INTENSITY_KEY = "specintensity";
const std::string CONSTANT_KEY = "constant";
const std::string LINEAR_KEY = "linear";
const std::string EXPONENT_KEY = "exponent";
const std::string EDGE_KEY = "edge";
const std::string RED_KEY = "red";
const std::string GREEN_KEY = "green";
const std::string BLUE_KEY = "blue";
const std::string X_KEY = "x";
const std::string Y_KEY = "y";
const std::string Z_KEY = "z";

const std::string LIGHTS_KEY = "lights";
const std::string DIRECTIONAL_LIGHT_KEY = "dlight";
const std::string POINT_LIGHTS_KEY = "plights";
const std::string POINT_LIGHT_KEY = "plight_%d";
const std::string SPOT_LIGHTS_KEY = "slights";
const std::string SPOT_LIGHT_KEY = "slight_%d";

const std::string MODELS_KEY = "models";

const std::string SHADERS_KEY = "shaders";
const std::string SHADER_KEY = "shader_%d";
const std::string VERTEX_SHADER_KEY = "vertex";
const std::string FRAGMENT_SHADER_KEY = "fragment";

void LoadLight(LightType type, nlohmann::json light, GLRenderer * meshRenderer, Transform* rootObject) {
	GLfloat diffIntensity = light[DIFFUSE_INTENSITY_KEY];
	GLfloat diffRed = light[DIFFUSE_COLOR_KEY][RED_KEY];
	GLfloat diffGreen = light[DIFFUSE_COLOR_KEY][GREEN_KEY];
	GLfloat diffBlue = light[DIFFUSE_COLOR_KEY][BLUE_KEY];
	GLfloat specIntensity = light[SPECULAR_INTENSITY_KEY];
	GLfloat specRed = light[SPECULAR_COLOR_KEY][RED_KEY];
	GLfloat specGreen = light[SPECULAR_COLOR_KEY][GREEN_KEY];
	GLfloat specBlue = light[SPECULAR_COLOR_KEY][BLUE_KEY]; 
	
	Transform *lt;
	GLfloat pitch;
	GLfloat yaw;
	GLfloat roll;
	GLfloat xPos;
	GLfloat yPos;
	GLfloat zPos;
	GLfloat constant;
	GLfloat linear;
	GLfloat exponent;
	GLfloat edge;

	switch (type) {
	case DIRECTIONAL:
		pitch = light[ROTATION_KEY][X_KEY];
		yaw = light[ROTATION_KEY][Y_KEY];
		roll = light[ROTATION_KEY][Z_KEY];
		lt = new Transform(rootObject);
		lt->Rotate(pitch, yaw, roll);
		meshRenderer->SetDirectionalLight(
			new DirectionalLight(lt, 
				4096, 4096,
				4096, 4096,
				diffIntensity, diffRed, diffGreen, diffBlue, 
				specIntensity, specRed, specGreen, specBlue));
		break;
	case POINT:
		xPos = light[POSITION_KEY][X_KEY];
		yPos = light[POSITION_KEY][Y_KEY];
		zPos = light[POSITION_KEY][Z_KEY];
		constant = light[CONSTANT_KEY];
		linear = light[LINEAR_KEY];
		exponent = light[EXPONENT_KEY];
		lt = new Transform(rootObject);
		lt->Translate(glm::vec3(xPos, yPos, zPos));
		meshRenderer->AddPointLight(
			new PointLight(lt,
				0.01f, 10.0f,
				512, 512,
				512, 512,
				constant, linear, exponent, 
				diffIntensity, diffRed, diffGreen, diffBlue, 
				specIntensity, specRed, specGreen, specBlue));
		break;
	case SPOT:
		pitch = light[ROTATION_KEY][X_KEY];
		yaw = light[ROTATION_KEY][Y_KEY];
		roll = light[ROTATION_KEY][Z_KEY];
		xPos = light[POSITION_KEY][X_KEY];
		yPos = light[POSITION_KEY][Y_KEY];
		zPos = light[POSITION_KEY][Z_KEY];
		constant = light[CONSTANT_KEY];
		linear = light[LINEAR_KEY];
		exponent = light[EXPONENT_KEY];
		edge = light[EDGE_KEY];
		lt = new Transform(rootObject);
		lt->Translate(glm::vec3(xPos, yPos, zPos));
		lt->Rotate(pitch, yaw, roll);
		meshRenderer->AddSpotLight(
			new SpotLight(lt,
				0.01f, 10.0f,
				512, 512,
				512, 512,
				edge, constant, linear, exponent, 
				diffIntensity, diffRed, diffGreen, diffBlue, 
				specIntensity, specRed, specGreen, specBlue));
		break;
	}
}

void LoadModels(nlohmann::json jsonObject, GLRenderer * meshRenderer) {
	std::vector<std::string> modelLocations = jsonObject[MODELS_KEY];
	for (size_t i = 0; i < modelLocations.size(); i++) {
		Model *model = new Model(modelLocations[i].c_str());
		model->Load();
		GLModelRenderer* modelRenderer = new GLModelRenderer();
		modelRenderer->SetRenderable(model);
		meshRenderer->AddObjectRenderer(modelRenderer);
	}
}

void LoadShaders(nlohmann::json shaders, GLRenderer* meshRenderer) {
	std::string jsonStr = shaders.dump();
	int i = 0;
	char locBuff[100] = { "\0" };
	snprintf(locBuff, sizeof(locBuff), SHADER_KEY.c_str(), i);

	nlohmann::json obj = shaders[locBuff];

	while (obj.type_name() != "null") {
		DefaultShader *shader = new DefaultShader();
		std::string vertexLocation = obj[VERTEX_SHADER_KEY];
		std::string fragmentLocation = obj[FRAGMENT_SHADER_KEY];
		shader->CreateFromFiles(vertexLocation.c_str(), fragmentLocation.c_str());
		meshRenderer->AddShader(shader);

		i++;
		snprintf(locBuff, sizeof(locBuff), POINT_LIGHT_KEY.c_str(), i);
		obj = shaders[locBuff];
	}
}

void LoadTransforms(nlohmann::json transform, GLRenderer* meshRenderer, Transform* rootObject) {

}

std::string SimulateJSONObject() {
	nlohmann::json simulateScene;
	simulateScene["ambient"] = 0.05f;

	nlohmann::json simulateDLight;
	simulateDLight["diffintensity"] = 0.6f;
	simulateDLight["specintensity"] = 0.6f;
	simulateDLight["diffcolor"]["red"] = 1.0f;
	simulateDLight["diffcolor"]["green"] = 1.0f;
	simulateDLight["diffcolor"]["blue"] = 1.0f;
	simulateDLight["speccolor"]["red"] = 1.0f;
	simulateDLight["speccolor"]["green"] = 1.0f;
	simulateDLight["speccolor"]["blue"] = 1.0f;
	simulateDLight["rotation"]["x"] = -0.5f;
	simulateDLight["rotation"]["y"] = -0.5f;
	simulateDLight["rotation"]["z"] = 0.0f;
	
	nlohmann::json simulatePLight0;
	simulatePLight0["diffintensity"] = 0.9f;
	simulatePLight0["specintensity"] = 0.9f;
	simulatePLight0["diffcolor"]["red"] = 1.0f;
	simulatePLight0["diffcolor"]["green"] = 1.0f;
	simulatePLight0["diffcolor"]["blue"] = 1.0f;
	simulatePLight0["speccolor"]["red"] = 1.0f;
	simulatePLight0["speccolor"]["green"] = 1.0f;
	simulatePLight0["speccolor"]["blue"] = 1.0f;
	simulatePLight0["translation"]["x"] = 8.0f;
	simulatePLight0["translation"]["y"] = 0.5f;
	simulatePLight0["translation"]["z"] = 0.0f;
	simulatePLight0["constant"] = 0.1f;
	simulatePLight0["linear"] = 0.05f;
	simulatePLight0["exponent"] = 0.02f;

	nlohmann::json simulatePLight1;
	simulatePLight1["diffintensity"] = 0.9f;
	simulatePLight1["specintensity"] = 0.9f;
	simulatePLight1["diffcolor"]["red"] = 1.0f;
	simulatePLight1["diffcolor"]["green"] = 1.0f;
	simulatePLight1["diffcolor"]["blue"] = 1.0f;
	simulatePLight1["speccolor"]["red"] = 1.0f;
	simulatePLight1["speccolor"]["green"] = 1.0f;
	simulatePLight1["speccolor"]["blue"] = 1.0f;
	simulatePLight1["translation"]["x"] = -2.0f;
	simulatePLight1["translation"]["y"] = 0.5f;
	simulatePLight1["translation"]["z"] = -2.0f;
	simulatePLight1["constant"] = 0.1f;
	simulatePLight1["linear"] = 0.05f;
	simulatePLight1["exponent"] = 0.02f;
	
	nlohmann::json simulateLights;
	simulateLights["dlight"] = simulateDLight;
	simulateLights["plights"]["plight_0"] = simulatePLight0;
	simulateLights["plights"]["plight_1"] = simulatePLight1;

	simulateScene["lights"] = simulateLights;

	simulateScene["models"] = { "Models/uh60.obj", "Models/Tree.obj", "Models/Tree_02.obj" };

	nlohmann::json simulatedShader;
	simulatedShader["shader_0"]["vertex"] = "Shaders/shader.vert";
	simulatedShader["shader_0"]["fragment"] = "Shaders/shader.frag";

	simulateScene["shaders"] = simulatedShader;

	return simulateScene.dump();
}

void BuildScene(GLRenderer * meshRenderer, Transform* root, GLWindow* glWindow) {
	// -- Helicopter --
	Transform *transform = new Transform(root);
	transform->SetStatic(false);
	transform->Scale(0.5f);
	transform->Translate(glm::vec3(0.0f, 10.0f, 0.0f));
	transform->Rotate(-1.57f, 3.14f, 0.0f);
	Material* mat = new Material(0.8f, 256, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 0));
	transform->AddUpdatable(new HelicopterController(transform, 7.0f, 1.0f));

	// -- Type 1 trees --
	transform = new Transform(root);
	transform->Translate(glm::vec3(10.0f, 0.0f, 0.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 1));

	transform = new Transform(root);
	transform->Translate(glm::vec3(-5.0f, 0.0f, -5.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 1));

	transform = new Transform(root);
	transform->Translate(glm::vec3(-1.0f, 0.0f, 5.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 1));

	transform = new Transform(root);
	transform->Translate(glm::vec3(2.0f, 0.0f, -3.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 1));

	transform = new Transform(root);
	transform->Translate(glm::vec3(2.0f, 0.0f, 10.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 1));

	transform = new Transform(root);
	transform->Translate(glm::vec3(-5.0f, 0.0f, -5.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 1));

	transform = new Transform(root);
	transform->Translate(glm::vec3(5.0f, 0.0f, 5.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 1));

	// -- Type 2 trees --
	transform = new Transform(root);
	transform->Translate(glm::vec3(0.0f, 0.0f, 2.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 2));

	transform = new Transform(root);
	transform->Translate(glm::vec3(3.0f, 0.0f, -1.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 2));

	transform = new Transform(root);
	transform->Translate(glm::vec3(-5.0f, 0.0f, 1.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 2));

	transform = new Transform(root);
	transform->Translate(glm::vec3(2.0f, 0.0f, -5.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 2));

	transform = new Transform(root);
	transform->Translate(glm::vec3(3.0f, 0.0f, 8.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 2));

	transform = new Transform(root);
	transform->Translate(glm::vec3(7.0f, 0.0f, -2.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 2));

	transform = new Transform(root);
	transform->Translate(glm::vec3(8.0f, 0.0f, 2.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 2));

	transform = new Transform(root);
	transform->Translate(glm::vec3(6.0f, 0.0f, -5.0f));
	mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
	meshRenderer->AddMeshRenderer(new GLObject(transform, mat, 2));

	{
		// -- Terrain --
		TerrainMesh* terrain = TerrainMesh::CreateInstance();
		GLMeshRenderer* renderer = new GLMeshRenderer();
		renderer->SetRenderable(terrain);
		meshRenderer->AddObjectRenderer(renderer);
		transform = new Transform(root);
		mat = new Material(0.2f, 16, 1.0f, 1.0f, 1.0f);
		meshRenderer->AddMeshRenderer(new GLObject(transform, mat, terrain->GetIndex()));
	}

	// Camera object is not static
	transform = new Transform(root);
	transform->SetStatic(false);
	transform->Translate(glm::vec3(0.0f, 5.0f, -5.0f));
	transform->AddUpdatable(Camera::CreateInstance(transform, glWindow));
	transform->AddUpdatable(new CameraController(transform, 5.0f, 5.0f));
	transform->AddUpdatable(new PrintKeyFram(transform));
	//std::vector<KeyFrame>* cameraKeyFrames = new std::vector<KeyFrame>();
	//KeyFrame* kf = new KeyFrame(); kf->deltaTime = 0.0f;
	//kf->position = glm::vec3(-0.996490f, 2.515669f, -3.033350f); kf->rotation = glm::vec3(0.758642f, 0.134519f, 0.000000f);
	//cameraKeyFrames->push_back(*kf); 
	//kf = new KeyFrame(); kf->deltaTime = 5.0f;
	//kf->position = glm::vec3(-1.505912f, 1.296978f, -0.267661f); kf->rotation = glm::vec3(0.178989f, -1.274950f, 0.000000f);
	//cameraKeyFrames->push_back(*kf); 
	//kf = new KeyFrame(); kf->deltaTime = 5.0f;
	//kf->position = glm::vec3(-2.715569f, 1.604647f, 2.135848f); kf->rotation = glm::vec3(0.389617f, -8.451337f, 0.000000f);
	//cameraKeyFrames->push_back(*kf); 
	//kf = new KeyFrame(); kf->deltaTime = 5.0f;
	//kf->position = glm::vec3(-7.685143f, 2.156667f, -0.090970f); kf->rotation = glm::vec3(0.585006f, -11.140243f, 0.000000f);
	//cameraKeyFrames->push_back(*kf); 
	//kf = new KeyFrame(); kf->deltaTime = 5.0f;
	//kf->position = glm::vec3(-2.473112f, 1.299403f, -3.737890f); kf->rotation = glm::vec3(-0.111726f, -10.858301f, 0.000000f);
	//cameraKeyFrames->push_back(*kf); 
	//kf = new KeyFrame(); kf->deltaTime = 5.0f;
	//kf->position = glm::vec3(10.915294f, 1.020687f, 1.661051f); kf->rotation = glm::vec3(0.231739f, -14.506601f, 0.000000f);
	//cameraKeyFrames->push_back(*kf);
	//transform->AddUpdatable(new CinematicCameraController(transform, cameraKeyFrames));
}

void SceneLoader::Load(const char* filename, GLRenderer * meshRenderer, Transform* rootObject, GLWindow* glWindow) {
	// Todo: Read from file
	std::string fileText = SimulateJSONObject();

	nlohmann::json sceneDescription = nlohmann::json::parse(fileText);
	float ambientIntensity = sceneDescription[AMBIENT_KEY];
	meshRenderer->SetAmbient(ambientIntensity);
	nlohmann::json lights = sceneDescription[LIGHTS_KEY];
	
	// -- Parse direct lights --
	nlohmann::json obj_1 = lights[DIRECTIONAL_LIGHT_KEY];

	if(obj_1.type_name() != "null") {
		LoadLight(DIRECTIONAL, obj_1, meshRenderer, rootObject);
	}

	// -- Parse point lights --
	int i = 0;
	char locBuff[100] = { "\0" };
	obj_1 = lights[POINT_LIGHTS_KEY];

	snprintf(locBuff, sizeof(locBuff), POINT_LIGHT_KEY.c_str(), i);
	nlohmann::json obj_2 = obj_1[locBuff];

	while (obj_2.type_name() != "null") {
		LoadLight(POINT, obj_2, meshRenderer, rootObject);

		i++;
		snprintf(locBuff, sizeof(locBuff), POINT_LIGHT_KEY.c_str(), i);
		obj_2 = obj_1[locBuff];
	}

	// -- Parse spot lights --
	i = 0;
	obj_1 = lights[SPOT_LIGHTS_KEY];

	snprintf(locBuff, sizeof(locBuff), SPOT_LIGHT_KEY.c_str(), i);
	obj_2 = obj_1[locBuff];

	while (obj_2.type_name() != "null") {
		LoadLight(SPOT, obj_2, meshRenderer, rootObject);

		i++;
		snprintf(locBuff, sizeof(locBuff), SPOT_LIGHT_KEY.c_str(), i);
		obj_2 = obj_1[locBuff];
	}

	// -- Parse models --
	LoadModels(sceneDescription, meshRenderer);

	// -- Parse shaders --
	LoadShaders(sceneDescription[SHADERS_KEY], meshRenderer);

	// -- Parse transforms --
	nlohmann::json transforms = sceneDescription[TRANSFORMS_KEY];
	if(transforms.type_name() != "null") LoadTransforms(transforms, meshRenderer, rootObject);

	BuildScene(meshRenderer, rootObject, glWindow);
}

void SceneLoader::Store(const char* filename, GLRenderer * meshRenderer, Transform * transform) {
	nlohmann::json simulateScene;
	simulateScene["ambient"] = meshRenderer->GetAmbient();

	DirectionalLight* dLight = meshRenderer->GetDirectionalLight();
	nlohmann::json simulateDLight;
	simulateDLight["diffintensity"] = dLight->GetDiffuseIntensity();
	simulateDLight["specintensity"] = dLight->GetSpecularIntensity();
	glm::vec3 tmpVec3 = dLight->GetDiffuseColor();
	simulateDLight["diffcolor"]["red"] = tmpVec3.r;
	simulateDLight["diffcolor"]["green"] = tmpVec3.g;
	simulateDLight["diffcolor"]["blue"] = tmpVec3.b;
	tmpVec3 = dLight->GetSpecularColor();
	simulateDLight["speccolor"]["red"] = tmpVec3.r;
	simulateDLight["speccolor"]["green"] = tmpVec3.g;
	simulateDLight["speccolor"]["blue"] = tmpVec3.b;
	tmpVec3 = dLight->GetTransform()->GetRotation();
	simulateDLight["rotation"]["x"] = tmpVec3.x;
	simulateDLight["rotation"]["y"] = tmpVec3.y;
	simulateDLight["rotation"]["z"] = tmpVec3.z;

	nlohmann::json simulatePLight0;
	simulatePLight0["diffintensity"] = 0.7f;
	simulatePLight0["specintensity"] = 0.7f;
	simulatePLight0["diffcolor"]["red"] = 0.0f;
	simulatePLight0["diffcolor"]["green"] = 1.0f;
	simulatePLight0["diffcolor"]["blue"] = 0.0f;
	simulatePLight0["speccolor"]["red"] = 1.0f;
	simulatePLight0["speccolor"]["green"] = 0.0f;
	simulatePLight0["speccolor"]["blue"] = 0.0f;
	simulatePLight0["translation"]["x"] = 0.0f;
	simulatePLight0["translation"]["y"] = 1.0f;
	simulatePLight0["translation"]["z"] = 0.0f;
	simulatePLight0["constant"] = 0.3f;
	simulatePLight0["linear"] = 0.2f;
	simulatePLight0["exponent"] = 0.1f;

	nlohmann::json simulatePLight1;
	simulatePLight1["diffintensity"] = 0.7f;
	simulatePLight1["specintensity"] = 0.7f;
	simulatePLight1["diffcolor"]["red"] = 1.0f;
	simulatePLight1["diffcolor"]["green"] = 1.0f;
	simulatePLight1["diffcolor"]["blue"] = 1.0f;
	simulatePLight1["speccolor"]["red"] = 1.0f;
	simulatePLight1["speccolor"]["green"] = 1.0f;
	simulatePLight1["speccolor"]["blue"] = 1.0f;
	simulatePLight1["translation"]["x"] = 2.0f;
	simulatePLight1["translation"]["y"] = 1.0f;
	simulatePLight1["translation"]["z"] = 4.0f;
	simulatePLight1["constant"] = 0.1f;
	simulatePLight1["linear"] = 0.05f;
	simulatePLight1["exponent"] = 0.02f;

	nlohmann::json simulateLights;
	simulateLights["dlight"] = simulateDLight;
	simulateLights["plights"]["plight_0"] = simulatePLight0;
	simulateLights["plights"]["plight_1"] = simulatePLight1;

	simulateScene["lights"] = simulateLights;

	simulateScene["models"] = { "Models/uh60.obj", "Models/Tree.obj", "Models/Tree_02.obj" };

	nlohmann::json simulatedShader;
	simulatedShader["shader_0"]["vertex"] = "Shaders/shader.vert";
	simulatedShader["shader_0"]["fragment"] = "Shaders/shader.frag";

	simulateScene["shaders"] = simulatedShader;

}
