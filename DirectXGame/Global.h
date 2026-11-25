#pragma once

#include "KamataEngine.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>

class Global {};

// AABB当たり判定
struct AABB {
	KamataEngine::Vector3 min; //!< 最小点
	KamataEngine::Vector3 max; //!< 最大点
};

// OBB当たり判定
struct OBB {
	KamataEngine::Vector3 center;          // 中心座標
	KamataEngine::Vector3 orientations[3]; // 各軸の方向
	KamataEngine::Vector3 size;            // 各軸の半分の大きさ
};

// AABBとAABB
bool IsCollisionAABB(const AABB& a, const AABB& b);

// OBBとOBB
bool IsCollisionOBB(const OBB& a, const OBB& b);

// AABBとOBB
bool IsCollisionAABBOBB(const AABB& a, const OBB& b);

// ワールドトランスフォームからOBBを作成
OBB MakeOBB(const KamataEngine::Vector3& center, const KamataEngine::Vector3& size, const KamataEngine::Vector3& rotation);

/// <summary>
///	行列計算・転送する
/// </summary>
void WorldTransformUpdate(KamataEngine::WorldTransform& worldTransform);

KamataEngine::Matrix4x4 Multiply(KamataEngine::Matrix4x4 matrix1, KamataEngine::Matrix4x4 matrix2);
// 1. X軸回転行列
KamataEngine::Matrix4x4 MakeRotateXMatrix(float radian);
// 2. Y軸回転行列
KamataEngine::Matrix4x4 MakeRotateYMatrix(float radian);
// 3. Z軸回転行列
KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian);

KamataEngine::Matrix4x4 MakeScaleMatrix(KamataEngine::Vector3 scale);
KamataEngine::Matrix4x4 MakeRotateMatrix(const KamataEngine::Vector3& rotate);
KamataEngine::Matrix4x4 MakeTranslateMatrix(KamataEngine::Vector3 translate);

KamataEngine::Matrix4x4 MakeAffineMatrix(KamataEngine::Vector3 scale, KamataEngine::Vector3 rotation, KamataEngine::Vector3 translation);

KamataEngine::Vector3 TransformConversion(const KamataEngine::Vector3& vector, const KamataEngine::Matrix4x4& m);

float Lerp(float start, float end, float t);

KamataEngine::Vector3 Lerp(KamataEngine::Vector3 start, KamataEngine::Vector3 end, float t);

// Vector3 Lerp(Vector3 start, Vector3 end, float t);

float EaseIn(float start, float end, float t);
float EaseOut(float start, float end, float t);
float EaseInOut(float start, float end, float t);

inline KamataEngine::Vector3 operator+(const KamataEngine::Vector3& a, const KamataEngine::Vector3& b) { 
	return {a.x + b.x, a.y + b.y, a.z + b.z};
}

inline KamataEngine::Vector3& operator+=(KamataEngine::Vector3& a, const KamataEngine::Vector3& b) {
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

inline KamataEngine::Vector3 operator-(const KamataEngine::Vector3& a, const KamataEngine::Vector3& b) { 
	return {a.x - b.x, a.y - b.y, a.z - b.z}; 
}

inline KamataEngine::Vector3& operator-=(KamataEngine::Vector3& a, const KamataEngine::Vector3& b) {
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

inline KamataEngine::Vector3 operator/(const KamataEngine::Vector3& v, float s) { return {v.x / s, v.y / s, v.z / s}; }

inline KamataEngine::Vector3& operator/=(KamataEngine::Vector3& v, float s) {
	v.x /= s;
	v.y /= s;
	v.z /= s;
	return v;
}

inline KamataEngine::Vector3 operator*(const KamataEngine::Vector3& v, float s) { 
	return {v.x * s, v.y * s, v.z * s}; 
}

inline KamataEngine::Vector3 operator*(const KamataEngine::Vector3& a, const KamataEngine::Vector3& b) {
	return {a.x * b.x, a.y * b.y, a.z * b.z}; 
}