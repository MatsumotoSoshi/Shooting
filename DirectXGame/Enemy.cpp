#include "Enemy.h"
#include <cmath>

using namespace KamataEngine;

void Enemy::Initialize(const Vector3& position) {

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;

	worldTransform_.scale_ = {0.7f, 0.7f, 0.7f};

	isAlive_ = true;

	WorldTransformUpdate(worldTransform_);
}

void Enemy::Update(const Vector3& playerPosition, const Vector3& repel) {

	Vector3& position = worldTransform_.translation_;

	// プレイヤーへの追尾方向
	float dx = playerPosition.x - position.x;
	float dz = playerPosition.z - position.z;

	float length = std::sqrt(dx * dx + dz * dz);

	if (length > 0.001f) {

		dx /= length;
		dz /= length;

		position.x += dx * moveSpeed_;
		position.z += dz * moveSpeed_;

		// プレイヤー方向へ旋回
		worldTransform_.rotation_.y = std::atan2(dx, dz);
	}

	// 敵同士の反発
	position.x += repel.x * 0.03f;
	position.z += repel.z * 0.03f;

	WorldTransformUpdate(worldTransform_);
}

void Enemy::Draw(Model* model, Camera* camera) {

	if (isAlive_) {
		model->Draw(worldTransform_, *camera);
	}
}
