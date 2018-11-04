#include "vector3.h"
#include <cmath>

Vector3::Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
Vector3::Vector3(const Vector3& A) {x = A.x, y = A.y, z = A.z;}

float Vector3::operator [] (const int& i) const {
	if (i == 0) return x;
	if (i == 1) return y;
	if (i == 2) return z;
	return 0;
}
Vector3 operator + (const Vector3& A, const Vector3& B) {
	return Vector3(A.x + B.x, A.y + B.y, A.z + B.z);
}
Vector3 operator - (const Vector3& A, const Vector3& B) {
	return Vector3(A.x - B.x, A.y - B.y, A.z - B.z);
}
Vector3 operator * (const Vector3& A, const float& x) {
	return Vector3(A.x * x, A.y * x, A.z * x);
}
Vector3 operator / (const Vector3& A, const float& x) {
	return Vector3(A.x / x, A.y / x, A.z / x);
}
Vector3 operator += (Vector3& A, const Vector3& B) {
	A = A + B; return A;
}
Vector3 operator -= (Vector3& A, const Vector3& B) {
	A = A - B; return A;
}
Vector3 operator *= (Vector3& A, const float& x) {
	A = A * x; return A;
}
Vector3 operator /= (Vector3& A, const float& x) {
	A = A / x; return A;
}

void Vector3::normalize() {
	float n = norm();
	if (n > EPS) *this /= n;
}

Vector3 Vector3::normalized() {
	float n = norm();
	if (n > EPS) return *this / n;
	else return *this;
}

bool Vector3::isZeroVector() const {
	return fabs(x) < EPS && fabs(y) < EPS && fabs(z) < EPS;
}

float Vector3::norm() const {
	return sqrt(x * x + y * y + z * z);
}

float Vector3::distance(const Vector3& A) const {
	return (*this - A).norm();
}

float Vector3::distToLine(const Vector3& P, const Vector3& D) const {
	Vector3 T = P - *this;
	float d = T.norm();
	float proj = T.dot(D) / D.norm();
	return sqrt(d * d - proj * proj);
}

float Vector3::dot(const Vector3& A) const {
	return A.x * x + A.y * y + A.z * z;
}

Vector3 Vector3::cross(const Vector3& A) const {
	return Vector3(A.y * z - A.z * y, - A.x * z + A.z * x,
			A.x * y - A.y * x);
}
