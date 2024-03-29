#pragma once

#include "AObjectBehavior.h"
#include "GLProgram.h"
#include "Transform.h"
#include "Time.h"
#include "Input.h"
#include "Camera.h"

struct KeyFrame {
	// Wanted positon
	glm::vec3 position;
	// Wanted rotation
	glm::vec3 rotation;
	// Get there in delta time
	float deltaTime;
};

class CameraController : public AObjectBehavior
{
private:
	GLfloat moveSpeed;
	GLfloat turnSpeed;

	void KeyControl();
	void MouseControl();

public:
	CameraController(Transform* container, GLfloat moveSpeed, GLfloat turnSpeed);

	void SetUp();
	void Update();
};

class AnimateKeyFrame : public AObjectBehavior {
private:
	size_t m_index;
	float m_deltaTime;
	std::vector<KeyFrame> m_keyframes;

public:
	AnimateKeyFrame(Transform* contanier, std::vector<KeyFrame>* keyFrames);

	void SetUp();
	void Update();
};

class PrintKeyFrame : public AObjectBehavior {
private:
	bool m_wasPressed;
public:
	PrintKeyFrame(Transform* container);

	void SetUp();
	void Update();
};

class ActivateLights : public AObjectBehavior {
private:
	bool m_wasPressed[9];
public:
	ActivateLights(Transform* container);

	void SetUp();
	void Update();
};

class HelicopterController : public AObjectBehavior
{
private:
	float moveSpeed;
	float rotSpeed;
public:
	HelicopterController(Transform* container, float vel, float rotVel);

	void SetUp();
	void Update();
};

