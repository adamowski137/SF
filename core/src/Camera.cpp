#include "core/Camera.hpp"
#include <cmath>

Vector4<float> Camera::getPosition()
{
	return target + Vector4<float>(
		radius * cosf(pitch) * sinf(yaw),
		radius * sinf(pitch),
		radius * cosf(pitch) * cosf(yaw)
	);
}

void Camera::recalculateVectors()
{
	position = this->getPosition();

	// RH style
	front = (position - target);
	front.normalize();

	right = Vector4<float>(0.f, 1.f, 0.f).cross(front);
	right.normalize();

	up = front.cross(right);
	up.normalize();

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

Matrix4x4<float> Camera::getViewMatrix()
{
	if (needRecalculation)
		this->recalculateVectors();
	return Matrix4x4<float>::viewRH(up, right, front, position);
}

Matrix4x4<float> Camera::getInverseViewMatrix()
{
	if (needRecalculation)
		this->recalculateVectors();
	return Matrix4x4<float>::inverseViewRH(up, right, front, position);
}

Vector4<float> Camera::getUpVector()
{
	if (needRecalculation)
		this->recalculateVectors();
	return this->up;
}

Vector4<float> Camera::getRightVector()
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
