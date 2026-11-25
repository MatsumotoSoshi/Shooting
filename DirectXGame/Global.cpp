#include "Global.h"
using namespace KamataEngine;

namespace {

float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

} // namespace

// AABB当たり判定
bool IsCollisionAABB(const AABB& a, const AABB& b) {
	if ((a.min.x <= b.max.x && a.max.x >= b.min.x) && // x軸
	    (a.min.y <= b.max.y && a.max.y >= b.min.y) && // y軸
	    (a.min.z <= b.max.z && a.max.z >= b.min.z))   // // z軸

	{
		return true;
	}
	return false;
}

// OBB当たり判定
bool IsCollisionOBB(const OBB& a, const OBB& b) {

	const float epsilon = 0.000001f;

	float R[3][3];
	float AbsR[3][3];

	// Aの各軸とBの各軸の内積
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			R[i][j] = Dot(a.orientations[i], b.orientations[j]);
			AbsR[i][j] = std::fabs(R[i][j]) + epsilon;
		}
	}

	// 中心間の距離
	Vector3 centerDistance = b.center - a.center;

	// Aの座標系に変換
	float t[3];

	for (int i = 0; i < 3; i++) {
		t[i] = Dot(centerDistance, a.orientations[i]);
	}

	float ra;
	float rb;

	// Aの3軸で判定
	for (int i = 0; i < 3; i++) {

		ra = a.size.x;
		if (i == 1)
			ra = a.size.y;
		if (i == 2)
			ra = a.size.z;

		rb = b.size.x * AbsR[i][0] + b.size.y * AbsR[i][1] + b.size.z * AbsR[i][2];

		if (std::fabs(t[i]) > ra + rb) {
			return false;
		}
	}

	// Bの3軸で判定
	for (int j = 0; j < 3; j++) {

		ra = a.size.x * AbsR[0][j] + a.size.y * AbsR[1][j] + a.size.z * AbsR[2][j];

		rb = b.size.x;
		if (j == 1)
			rb = b.size.y;
		if (j == 2)
			rb = b.size.z;

		float projectedDistance = std::fabs(t[0] * R[0][j] + t[1] * R[1][j] + t[2] * R[2][j]);

		if (projectedDistance > ra + rb) {
			return false;
		}
	}

	// AとBの各軸の外積6軸で判定
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {

			int i1 = (i + 1) % 3;
			int i2 = (i + 2) % 3;

			int j1 = (j + 1) % 3;
			int j2 = (j + 2) % 3;

			float tValue = std::fabs(t[i2] * R[i1][j] - t[i1] * R[i2][j]);

			ra = (i1 == 0 ? a.size.x : (i1 == 1 ? a.size.y : a.size.z)) * AbsR[i2][j] + (i2 == 0 ? a.size.x : (i2 == 1 ? a.size.y : a.size.z)) * AbsR[i1][j];

			rb = (j1 == 0 ? b.size.x : (j1 == 1 ? b.size.y : b.size.z)) * AbsR[i][j2] + (j2 == 0 ? b.size.x : (j2 == 1 ? b.size.y : b.size.z)) * AbsR[i][j1];

			if (tValue > ra + rb) {
				return false;
			}
		}
	}

	return true;
}

// AABBとOBBの当たり判定
bool IsCollisionAABBOBB(const AABB& a, const OBB& b) {

	OBB obbA{};

	// AABBの中心
	obbA.center = {(a.min.x + a.max.x) * 0.5f, (a.min.y + a.max.y) * 0.5f, (a.min.z + a.max.z) * 0.5f};

	// AABBの半分の大きさ
	obbA.size = {(a.max.x - a.min.x) * 0.5f, (a.max.y - a.min.y) * 0.5f, (a.max.z - a.min.z) * 0.5f};

	// AABBの軸はワールド座標に固定
	obbA.orientations[0] = {1.0f, 0.0f, 0.0f};
	obbA.orientations[1] = {0.0f, 1.0f, 0.0f};
	obbA.orientations[2] = {0.0f, 0.0f, 1.0f};

	return IsCollisionOBB(obbA, b);
}

// ワールドトランスフォームからOBBを作成
OBB MakeOBB(const Vector3& center, const Vector3& size, const Vector3& rotation) {

	OBB obb{};

	obb.center = center;
	obb.size = size;

	// 回転行列
	Matrix4x4 rotateX = MakeRotateXMatrix(rotation.x);
	Matrix4x4 rotateY = MakeRotateYMatrix(rotation.y);
	Matrix4x4 rotateZ = MakeRotateZMatrix(rotation.z);

	Matrix4x4 rotateMatrix = Multiply(rotateX, Multiply(rotateY, rotateZ));

	// 各軸の方向を取得
	obb.orientations[0] = TransformConversion({1.0f, 0.0f, 0.0f}, rotateMatrix);

	obb.orientations[1] = TransformConversion({0.0f, 1.0f, 0.0f}, rotateMatrix);

	obb.orientations[2] = TransformConversion({0.0f, 0.0f, 1.0f}, rotateMatrix);

	return obb;
}

void WorldTransformUpdate(KamataEngine::WorldTransform& worldTransform) {
	// スケール、回転、平行移動からワールド行列を計算
	worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);
	// 定数バッフアへの書き込み
	worldTransform.TransferMatrix();
}

#pragma region 行列計算
Matrix4x4 Multiply(Matrix4x4 matrix1, Matrix4x4 matrix2) {
	Matrix4x4 result;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = 0.0f;
			for (int k = 0; k < 4; k++) {
				result.m[i][j] += matrix1.m[i][k] * matrix2.m[k][j];
			}
		}
	}

	return result;
}

// 1. X軸回転行列
Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 result = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, cosf(radian), sinf(radian), 0.0f, 0.0f, -sinf(radian), cosf(radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	return result;
}

// 2. Y軸回転行列
Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 result = {cosf(radian), 0.0f, -sinf(radian), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, sinf(radian), 0.0f, cosf(radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	return result;
}

// 3. Z軸回転行列
Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result = {cosf(radian), sinf(radian), 0.0f, 0.0f, -sinf(radian), cosf(radian), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
	return result;
}

//
Matrix4x4 MakeScaleMatrix(Vector3 scale) {
	Matrix4x4 result;
	return result = {scale.x, 0.0f, 0.0f, 0.0f, 0.0f, scale.y, 0.0f, 0.0f, 0.0f, 0.0f, scale.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
}

Matrix4x4 MakeTranslateMatrix(Vector3 translate) {
	Matrix4x4 result;
	return result = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, translate.x, translate.y, translate.z, 1};
}

Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotate, Vector3 translate) {
	Matrix4x4 scaleMatrix = MakeScaleMatrix(scale);
	Matrix4x4 rotateXMatrix = MakeRotateXMatrix(rotate.x);
	Matrix4x4 rotateYMatrix = MakeRotateYMatrix(rotate.y);
	Matrix4x4 rotateZMatrix = MakeRotateZMatrix(rotate.z);
	Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);

	Matrix4x4 rotateXYZMatrix = Multiply(rotateXMatrix, Multiply(rotateYMatrix, rotateZMatrix));
	Matrix4x4 worldMatrix = Multiply(scaleMatrix, Multiply(rotateXYZMatrix, translateMatrix));
	// worldMatrix = Multiply(worldMatrix, translateMatrix);

	return worldMatrix;
}

Vector3 TransformConversion(const Vector3& vector, const Matrix4x4& m) {
	Vector3 result;
	float w;
	result.x = vector.x * m.m[0][0] + vector.y * m.m[1][0] + vector.z * m.m[2][0] + m.m[3][0];
	result.y = vector.x * m.m[0][1] + vector.y * m.m[1][1] + vector.z * m.m[2][1] + m.m[3][1];
	result.z = vector.x * m.m[0][2] + vector.y * m.m[1][2] + vector.z * m.m[2][2] + m.m[3][2];
	w = vector.x * m.m[0][3] + vector.y * m.m[1][3] + vector.z * m.m[2][3] + m.m[3][3];
#ifdef _DEBUG
	assert(w != 0.0f);
#endif

	result.x /= w;
	result.y /= w;
	result.z /= w;
	return result;
}

#pragma endregion

float Lerp(float start, float end, float t) { return (1.0f - t) * start + t * end; };


Vector3 Lerp(Vector3 start, Vector3 end, float t) {
	Vector3 result;
	result.x = start.x + (end.x - start.x) * t;
	result.y = start.y + (end.y - start.y) * t;
	result.z = start.z + (end.z - start.z) * t;
	return result;
};

float EaseIn(float start, float end, float t) {
	// 加速するイージング（開始がゆっくり）
	float num = 1.0f - cosf((static_cast<float>(M_PI) / 2.0f) * t);
	return Lerp(start, end, num);
};

float EaseOut(float start, float end, float t) {
	// 減速するイージング（終了がゆっくり）
	float num = sinf((static_cast<float>(M_PI) / 2.0f) * t);
	return Lerp(start, end, num);
};

float EaseInOut(float start, float end, float t) {
	float num;
	// return -(Math.cos(Math.PI * x) - 1) / 2;
	num = -(cosf(static_cast<float>(M_PI) * t) - 1.0f) / 2;
	return Lerp(start, end, num);
};
