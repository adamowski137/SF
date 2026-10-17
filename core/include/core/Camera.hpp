#pragma once
#include <glm/glm.hpp>

class Camera {
	glm::vec3 target, position;
	float yaw, pitch, radius;

	glm::vec3 right, up, front;
	bool needRecalculation;

	const float minZoom = 0.25f, maxZoom = 100.f;

	void recalculateVectors();
	float clamp(float min, float max, float val);
public:
	Camera();
	glm::mat4 getViewMatrix();
	glm::mat4 getInverseViewMatrix();
	glm::vec3 getUpVector();
	glm::vec3 getRightVector();
	glm::vec3 getPosition();
	void rotate(float dx, float dy);
	void zoom(float amount);
	void pan(float dx, float dy);
};
