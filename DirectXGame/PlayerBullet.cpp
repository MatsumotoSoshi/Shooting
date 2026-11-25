#include "PlayerBullet.h"

using namespace KamataEngine;

void PlayerBullet::Initialize(const Vector3& position, const Vector3& direction) {

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;

	worldTransform_.scale_ = {0.2f, 0.2f, 0.5f};

	velocity_ = direction * moveSpeed_;

	isAlive_ = true;

	WorldTransformUpdate(worldTransform_);
}

void PlayerBullet::Update() {

	worldTransform_.translation_ += velocity_;

	WorldTransformUpdate(worldTransform_);
}

void PlayerBullet::Draw(Model* model, Camera* camera) {

	if (isAlive_) {
		model->Draw(worldTransform_, *camera);
	}
}
