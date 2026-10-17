#include "core/Camera.hpp"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

glm::vec3 Camera::getPosition()
{
	return target + glm::vec3(
		radius * cosf(pitch) * sinf(yaw),
		radius * sinf(pitch),
		radius * cosf(pitch) * cosf(yaw)
	);
}

void Camera::recalculateVectors()
{
	position = this->getPosition();

	front = glm::normalize(position - target);

	right = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), front));

	up = glm::normalize(glm::cross(front, right));

	needRecalculation = false;
}

float Camera::clamp(float minVal, float maxVal, float val)
{
	return val < minVal ? minVal : val > maxVal ? maxVal : val;
}

Camera::Camera() : target{0.f, 0.f, 0.f}, yaw(0.f), pitch(0.f), radius{5.f}
{
	this->recalculateVectors();
}

glm::mat4 Camera::getViewMatrix()
{
	if (needRecalculation)
		this->recalculateVectors();
	return glm::lookAtRH(position, target, up);
}

glm::mat4 Camera::getInverseViewMatrix()
{
	return glm::inverse(getViewMatrix());
}

glm::vec3 Camera::getUpVector()
{
	if (needRecalculation)
		this->recalculateVectors();
	return this->up;
}

glm::vec3 Camera::getRightVector()
{
	if (needRecalculation)
		this->recalculateVectors();
	return this->right;
}

void Camera::rotate(float dx, float dy)
{
	yaw += dx;
	pitch -= dy;

	yaw = fmodf(yaw, 2 * 3.14159265f);
	pitch = clamp(-3.14159265f / 2 + 0.01f, 3.14159265f / 2 - 0.01f, pitch);

	needRecalculation = true;
}

void Camera::zoom(float amount)
{
	radius -= amount;
	radius = clamp(minZoom, maxZoom, radius);

	needRecalculation = true;
}

void Camera::pan(float dx, float dy)
{
	if (needRecalculation)
		this->recalculateVectors();

	target += right * dx * radius + up * dy * radius;
	needRecalculation = true;
}
