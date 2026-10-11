#pragma once
#include <cmath>
#include <tuple>
#include "../vector/vector.hpp"

class Quat {
public:
	float q0, q1, q2, q3;
	Quat(float q0 = 0, float q1 = 0, float q2 = 0, float q3 = 0);
	Quat operator+(const Quat& other) const;
	Quat operator-(const Quat& other) const;
	Quat operator*(const Quat& other) const;
	Quat operator*(const float& coeff) const;
	Quat operator/(const float& coeff) const;

	static Quat AxisAngle(float angle, float x, float y, float z);
	std::tuple<float, float, float, float> ToAxisAngle();

	static Quat FromEuler(float yaw, float pitch, float roll);
	std::tuple<float, float, float> ToEuler() const;

	Quat conjugate() const;
	Quat inv() const;
	void normalize();
	Vector4<float> vectorMultiply(const Vector4<float>& v);

	float dot(const Quat& other) const;
};

inline Quat::Quat(float q0, float q1, float q2, float q3) : q0(q0), q1(q1), q2(q2), q3(q3)
{
}

inline Quat Quat::operator+(const Quat& other) const
{
	return Quat(q0 + other.q0, q1 + other.q1, q2 + other.q2, q3 + other.q3);
}

inline Quat Quat::operator-(const Quat& other) const
{
	return Quat(q0 - other.q0, q1 - other.q1, q2 - other.q2, q3 - other.q3);
}

inline Quat Quat::operator*(const Quat& other) const
{
	return Quat(q0 * other.q0 - q1 * other.q1 - q2 * other.q2 - q3 * other.q3,
				q0 * other.q1 + q1 * other.q0 + q2 * other.q3 - q3 * other.q2,
				q0 * other.q2 - q1 * other.q3 + q2 * other.q0 + q3 * other.q1,
				q0 * other.q3 + q1 * other.q2 - q2 * other.q1 + q3 * other.q0);
}

inline Quat Quat::operator*(const float& coeff) const
{
	return Quat(q0 * coeff, q1 * coeff, q2 * coeff, q3 * coeff);
}

inline Quat Quat::operator/(const float& coeff) const
{
	return Quat(q0 / coeff, q1 / coeff, q2 / coeff, q3 / coeff);
}

inline Quat Quat::AxisAngle(float angle, float x, float y, float z)
{
	float factor = sinf(angle / 2.f);
	Quat q;
	q.q0 = cosf(angle / 2.f);
	q.q1 = x * factor;
	q.q2 = y * factor;
	q.q3 = z * factor;

	q.normalize();
	return q;
}

inline std::tuple<float, float, float, float> Quat::ToAxisAngle()
{
	float angle = 2 * acosf(q0);
	float sindiv2 = sinf(angle / 2);

	if (std::abs(sindiv2) < 1e-6)
	{
		return { 0.0f, 1.0f, 0.0f, 0.0f };
	}

	return {angle, q1 / sindiv2, q2 / sindiv2, q3 / sindiv2 };
}

inline std::tuple<float, float, float> Quat::ToEuler() const
{
	float yaw, pitch, roll;

	float sinr_cosp = 2.0f * (q0 * q1 + q2 * q3);
	float cosr_cosp = 1.0f - 2.0f * (q1 * q1 + q2 * q2);
	roll = atan2f(sinr_cosp, cosr_cosp);

	float sinp = 2.0f * (q0 * q2 - q3 * q1);
	if (fabs(sinp) >= 1)
		pitch = copysignf(3.1415f / 2, sinp);
	else
		pitch = asinf(sinp);

	float siny_cosp = 2.0f * (q0 * q3 + q1 * q2);
	float cosy_cosp = 1.0f - 2.0f * (q2 * q2 + q3 * q3);
	yaw = atan2f(siny_cosp, cosy_cosp);

	return { yaw, pitch, roll };
}

inline Quat Quat::FromEuler(float yaw, float pitch, float roll)
{
	float cy = cosf(yaw * 0.5f);
	float sy = sinf(yaw * 0.5f);
	float cp = cosf(pitch * 0.5f);
	float sp = sinf(pitch * 0.5f);
	float cr = cosf(roll * 0.5f);
	float sr = sinf(roll * 0.5f);

	return Quat(
		cr * cp * cy + sr * sp * sy,
		sr * cp * cy - cr * sp * sy,
		cr * sp * cy + sr * cp * sy,
		cr * cp * sy - sr * sp * cy
	);
}

inline Quat Quat::conjugate() const
{
	return Quat(q0, -q1, -q2, -q3);
}

inline Quat Quat::inv() const
{
	return this->conjugate() / this->dot(*this);
}

inline void Quat::normalize()
{
	float norm = sqrtf(this->dot(*this));
	q0 /= norm;
	q1 /= norm;
	q2 /= norm;
	q3 /= norm;
}

inline Vector4<float> Quat::vectorMultiply(const Vector4<float>& v)
{
	Quat vQuat = Quat(0.f, v.x, v.y, v.z);
	Quat result = *this * vQuat * this->inv();

	return { result.q1, result.q2, result.q3 };
}

inline float Quat::dot(const Quat& other) const
{
	return q0 * other.q0 + q1 * other.q1 + q2 * other.q2 + q3 * other.q3;
}
