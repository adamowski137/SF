#pragma once
#include <quat/quat.hpp>
#include <vector/vector.hpp>
#include <matrix/matrix.hpp>

class Camera {
	Vector4<float> target, position;
	float yaw, pitch, radius;

	Vector4<float> right, up, front;
	bool needRecalculation;

	const float minZoom = 0.25f, maxZoom = 100.f;

	void recalculateVectors();
	float clamp(float min, float max, float val);
public:
	Camera();
	Matrix4x4<float> getViewMatrix();
	Matrix4x4<float> getInverseViewMatrix();
	Vector4<float> getUpVector();
	Vector4<float> getRightVector();
	Vector4<float> getPosition();
	void rotate(float dx, float dy);
	void zoom(float amount);
	void pan(float dx, float dy);
};
