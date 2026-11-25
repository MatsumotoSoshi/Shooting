#include "PlayerWeapon.h"
#include <cassert>
#include <cmath>
#include <imgui.h>

using namespace KamataEngine;

void PlayerWeapon::Initialize(Model* model, Camera* camera) {
	assert(model != nullptr);
	assert(camera != nullptr);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	objectColor_.Initialize();

	WorldTransformUpdate(worldTransform_);
}

void PlayerWeapon::UpdateNormal(const WorldTransform& playerTransform, int phase, int attackTimer, int comboStep, int anticipationFrames, int executionFrames) {

	// 通常色
	objectColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});

	// コンボ終了時は次のコンボに備えて初期化
	if (comboStep == 0) {
		comboCurrentOffset_ = {};
		comboStartOffset_ = {};

		comboCurrentRotation_ = {};
		comboStartRotation_ = {};

		previousComboStep_ = 0;
	}

	// コンボ段が切り替わったとき、前段の位置と角度を引き継ぐ
	if (comboStep != previousComboStep_) {
		comboStartOffset_ = comboCurrentOffset_;
		comboStartRotation_ = comboCurrentRotation_;
		previousComboStep_ = comboStep;
	}

	// 補間用の値
	float t = 0.0f;

	if (phase == 1) {
		t = static_cast<float>(attackTimer) / static_cast<float>(anticipationFrames);
	} else if (phase == 2) {
		t = static_cast<float>(attackTimer) / static_cast<float>(executionFrames);
	}

	if (t > 1.0f) {
		t = 1.0f;
	}

	// 補間する位置と角度
	Vector3 startOffset = comboStartOffset_;
	Vector3 endOffset = comboStartOffset_;

	Vector3 startRotation = comboStartRotation_;
	Vector3 endRotation = comboStartRotation_;

	// 各段の目標位置・角度
	switch (comboStep) {

	case 1:
		// 1段目：袈裟切り
		if (phase == 1) {
			endOffset = {0.0f, anticipationHeight_, -anticipationBack_};
			endRotation.z = -0.5f;
		} else if (phase == 2) {
			startOffset = {0.0f, anticipationHeight_, -anticipationBack_};
			endOffset = {0.7f, -attackDistanceY_, attackDistanceZ_};
			startRotation.z = -0.5f;
			endRotation.z = -1.2f;
		}
		break;

	case 2:
		// 2段目：逆袈裟
		if (phase == 1) {
			// 前段の終了位置を維持
			endOffset = comboStartOffset_;
			endRotation = comboStartRotation_;
		} else if (phase == 2) {
			// 袈裟切りの終点から斜め上へ振り上げる
			startOffset = comboStartOffset_;
			endOffset = {-0.7f, attackDistanceY_ * 0.5f, attackDistanceZ_};
			startRotation = comboStartRotation_;
			endRotation.z = 1.2f;
		}
		break;

	case 3:
		// 3段目：回転斬り
		if (phase == 1) {
			// 逆袈裟の終点からプレイヤーの横へ移動
			startOffset = comboStartOffset_;
			endOffset = {attackDistanceZ_, -0.5f, 0.0f};

			startRotation = comboStartRotation_;
			endRotation.z = 0.0f;
		} else if (phase == 2) {
			// 横の構え位置を維持
			startOffset = {attackDistanceZ_, -0.5f, 0.0f};
			endOffset = startOffset;

			startRotation = comboStartRotation_;
			endRotation = comboStartRotation_;
		}
		break;
	}

	// 位置・角度を補間
	if (phase == 1 || phase == 2) {
		comboCurrentOffset_.x = startOffset.x + (endOffset.x - startOffset.x) * t;
		comboCurrentOffset_.y = startOffset.y + (endOffset.y - startOffset.y) * t;
		comboCurrentOffset_.z = startOffset.z + (endOffset.z - startOffset.z) * t;

		comboCurrentRotation_.x = startRotation.x + (endRotation.x - startRotation.x) * t;
		comboCurrentRotation_.y = startRotation.y + (endRotation.y - startRotation.y) * t;
		comboCurrentRotation_.z = startRotation.z + (endRotation.z - startRotation.z) * t;
	}

	// 回転行列
	Matrix4x4 rotateY = MakeRotateYMatrix(playerTransform.rotation_.y);

	// 武器の基本位置
	Vector3 offset = TransformConversion(localPosition_, rotateY);
	worldTransform_.translation_ = playerTransform.translation_ + offset;

	// コンボの相対位置をプレイヤーの向きに合わせる
	Vector3 attackOffset = TransformConversion(comboCurrentOffset_, rotateY);
	worldTransform_.translation_ += attackOffset;

	// 武器の角度をプレイヤーの角度に追従させる
	worldTransform_.rotation_.x = playerTransform.rotation_.x + comboCurrentRotation_.x;
	worldTransform_.rotation_.y = playerTransform.rotation_.y + comboCurrentRotation_.y;
	worldTransform_.rotation_.z = playerTransform.rotation_.z + comboCurrentRotation_.z;

	WorldTransformUpdate(worldTransform_);
}

void PlayerWeapon::UpdateSpecial(const WorldTransform& playerTransform, int phase, int attackTimer) {
	// プレイヤーの向きに合わせた回転行列
	Matrix4x4 rotateY = MakeRotateYMatrix(playerTransform.rotation_.y);

	// 特殊攻撃中は紫色の発光風演出
	if (phase != 0) {
		objectColor_.SetColor({0.8f, 0.3f, 1.0f, 1.0f});
	} else {
		objectColor_.SetColor({1.0f, 1.0f, 1.0f, 1.0f});
	}

	// 武器の基本位置
	Vector3 offset = TransformConversion(localPosition_, rotateY);

	// 基本位置をプレイヤーに合わせる
	worldTransform_.translation_ = playerTransform.translation_ + offset;

	// 攻撃による移動量
	Vector3 attackOffset = {0.0f, 0.0f, 0.0f};

	if (phase == 1) {
		// 予備動作：上方向・後方へ引く
		attackOffset.y = anticipationHeight_;
		attackOffset.z = -anticipationBack_;
	} else if (phase == 2) {
		// 攻撃動作：前方・下方向へ振り下ろす
		float t = static_cast<float>(attackTimer) / executionFrames_;

		if (t > 1.0f) {
			t = 1.0f;
		}

		attackOffset.y = anticipationHeight_ - attackDistanceY_ * t;
		attackOffset.z = -anticipationBack_ + (anticipationBack_ + attackDistanceZ_) * t;
	} else if (phase == 3) {
		// 後隙：通常位置へ戻す
		attackOffset.y = -attackDistanceY_;
		attackOffset.z = attackDistanceZ_;
	}

	// プレイヤーの向きに合わせて攻撃移動量も回転
	attackOffset = TransformConversion(attackOffset, rotateY);

	worldTransform_.translation_ += attackOffset;

	WorldTransformUpdate(worldTransform_);
}

void PlayerWeapon::ImGuiUpdate() {
	ImGui::Begin("PlayerWeapon");

	ImGui::SliderFloat3("Local Position", &localPosition_.x, -3.0f, 3.0f);

	ImGui::SliderFloat("Anticipation Height", &anticipationHeight_, 0.0f, 3.0f);
	ImGui::SliderFloat("Anticipation Back", &anticipationBack_, 0.0f, 3.0f);

	ImGui::SliderFloat("Attack Distance Y", &attackDistanceY_, 0.0f, 5.0f);
	ImGui::SliderFloat("Attack Distance Z", &attackDistanceZ_, 0.0f, 5.0f);

	ImGui::Separator();
	ImGui::Text("Attack OBB");
	ImGui::SliderFloat3("Attack OBB Size", &attackOBBSize_.x, 0.05f, 2.0f);

	ImGui::End();

}

void PlayerWeapon::Draw() {
	assert(model_ != nullptr);
	assert(camera_ != nullptr);

	model_->Draw(worldTransform_, *camera_, &objectColor_);
}

OBB PlayerWeapon::GetAttackOBB() const {
	Vector3 size = {attackOBBSize_.x * worldTransform_.scale_.x, attackOBBSize_.y * worldTransform_.scale_.y, attackOBBSize_.z * worldTransform_.scale_.z};

	return MakeOBB(worldTransform_.translation_, size, worldTransform_.rotation_);
}