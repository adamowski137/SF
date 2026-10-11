#pragma once

#include <cmath>

template<typename T>
class Vector4 {
public:
	T x, y, z, w;
	Vector4(T a = 0, T b = 0, T c = 0, T d = 0);

	Vector4<T> operator+(const Vector4<T>& other) const;
	Vector4<T>& operator+=(const Vector4<T>& other);
	Vector4<T> operator-(const Vector4<T>& other) const;
	Vector4<T>& operator-=(const Vector4<T>& other);

	Vector4<T> operator*(const T& coeff) const;
	Vector4<T>& operator*=(const T& coeff);
	Vector4<T> operator/(const T& coeff) const;
	Vector4<T>& operator/=(const T& coeff);

	Vector4<T> operator*(const Vector4<T>& other) const;

	Vector4<T> operator-() const;

	T dot_product(const Vector4<T>& other) const;
	Vector4<T> cross(const Vector4<T>& other);
	float length() const;
	T length_sq() const;

	void normalize();

};

template<typename T>
inline Vector4<T>::Vector4(T a, T b, T c, T d) : x(a), y(b), z(c), w(d)
{

}

template<typename T>
inline Vector4<T> Vector4<T>::operator+(const Vector4<T>& other) const
{
	return Vector4<T>(this->x + other.x, this->y + other.y, this->z + other.z, this->w + other.w);
}

template<typename T>
inline Vector4<T>& Vector4<T>::operator+=(const Vector4<T>& other)
{
	this->x += other.x;
	this->y += other.y;
	this->z += other.z;
	this->w += other.w;
	return *this;
}

template<typename T>
inline Vector4<T> Vector4<T>::operator-(const Vector4<T>& other) const
{
	return Vector4<T>(this->x - other.x, this->y - other.y, this->z - other.z, this->w - other.w);
}

template<typename T>
inline Vector4<T>& Vector4<T>::operator-=(const Vector4<T>& other)
{
	this->x -= other.x;
	this->y -= other.y;
	this->z -= other.z;
	this->w -= other.w;
	return *this;
}

template<typename T>
inline Vector4<T> Vector4<T>::operator*(const T& coeff) const
{
	return Vector4<T>(this->x * coeff, this->y * coeff, this->z * coeff, this->w * coeff);
}

template<typename T>
inline Vector4<T>& Vector4<T>::operator*=(const T& coeff)
{
	this->x *= coeff;
	this->y *= coeff;
	this->z *= coeff;
	this->w *= coeff;
	return *this;
}

template<typename T>
inline Vector4<T> Vector4<T>::operator*(const Vector4<T>& other) const
{
	return Vector4<T>(this->x * other.x, this->y * other.y, this->z * other.z, this->w * other.w);
}

template<typename T>
inline Vector4<T> Vector4<T>::operator/(const T& coeff) const
{
	return Vector4<T>(this->x / coeff, this->y / coeff, this->z / coeff, this->w / coeff);
}

template<typename T>
inline Vector4<T>& Vector4<T>::operator/=(const T& coeff)
{
	this->x /= coeff;
	this->y /= coeff;
	this->z /= coeff;
	this->w /= coeff;
	return *this;
}

template<typename T>
inline Vector4<T> Vector4<T>::operator-() const
{
	return Vector4<T>(-this->x, -this->y, -this->z, -this->w);
}

template<typename T>
inline T Vector4<T>::dot_product(const Vector4<T>& other) const
{
	return this->x * other.x + this->y * other.y + this->z * other.z + this->w * other.w;
}

template<typename T>
inline Vector4<T> Vector4<T>::cross(const Vector4<T>& v)
{
	return Vector4<T>(this->y * v.z - this->z * v.y,
		this->z * v.x - this->x * v.z,
		this->x * v.y - this->y * v.x);
}

template<typename T>
inline float Vector4<T>::length() const
{
	return sqrtf(length_sq());
}

template<typename T>
inline T Vector4<T>::length_sq() const
{
	return dot_product(*this);
}

template<typename T>
inline void Vector4<T>::normalize()
{
	*this /= (T)length();
}
