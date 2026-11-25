#include "Skydome.h" 
#include "Global.h"
#include <cassert>

using namespace KamataEngine;

void Skydome::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model != nullptr);
	assert(camera != nullptr);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	WorldTransformUpdate(worldTransform_);
}

void Skydome::Update() { 
	WorldTransformUpdate(worldTransform_);
}

void Skydome::Draw() {
	assert(model_ != nullptr);
	assert(camera_ != nullptr);
	model_->Draw(worldTransform_, *camera_);
}
