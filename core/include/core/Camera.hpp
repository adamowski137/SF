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
	void setPose(float newYaw, float newPitch, float newRadius, const glm::vec3& newTarget = glm::vec3(0.0f));
	void reset(float initialYaw = -3.14159265f / 4.0f, float initialPitch = -3.14159265f / 6.0f, float initialRadius = 5.0f, const glm::vec3& initialTarget = glm::vec3(0.0f));
	float getRadius() const { return radius; }
	float getYaw() const { return yaw; }
	float getPitch() const { return pitch; }
	glm::vec3 getTarget() const { return target; }
};
