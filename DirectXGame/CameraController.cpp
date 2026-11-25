#include "CameraController.h"
#include "Global.h"
#include "Player.h"
#include <cmath>

using namespace KamataEngine;

void CameraController::Initialize() {

	camera_.Initialize();

	// 水平方向の距離を算出
	float horizontalDistance = std::sqrt(targetOffset_.x * targetOffset_.x + targetOffset_.z * targetOffset_.z);

	// カメラの角度（斜め見下ろし角度）
	// X軸回転: 見下ろし角度（ピッチ）
	// Y軸回転: 水平方向の向き（ヨー）
	camera_.rotation_ = {
		std::atan2(targetOffset_.y, horizontalDistance),
		std::atan2(-targetOffset_.x, -targetOffset_.z),
		0.0f
	};

	camera_.UpdateMatrix();
}

void CameraController::Reset() {

	if (target_ == nullptr) {
		return;
	}

	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	destination_ = targetWorldTransform.translation_ + targetOffset_;
	camera_.translation_ = destination_;

	camera_.UpdateMatrix();
}

void CameraController::Update() {

	if (target_ == nullptr) {
		return;
	}

	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	destination_ = targetWorldTransform.translation_ + targetOffset_;

	camera_.translation_ = Lerp(
		camera_.translation_, destination_, 
		kInterpolationRate);

	camera_.UpdateMatrix();
}