#include "Player.h"
#include <cmath>
#include <cassert>
#include <imgui.h>
#include "PlayerWeapon.h"

using namespace KamataEngine;

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model != nullptr);
	assert(camera != nullptr);

	camera_ = camera;
	model_ = model;

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;

	// 大きさ
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 内部数値等初期化
	hp_ = maxHP_;
	energyGauge_ = 0;
	isOverFlow_ = false;
	overflowTimer_ = 0;
	isJustGuardActive_ = false;
	justGuardTimer_ = 0;

	// 行列更新
	WorldTransformUpdate(worldTransform_);

	 weapon_ = new PlayerWeapon();

	Model* weaponModel = Model::CreateFromOBJ("block");
	assert(weaponModel != nullptr);

	weapon_->Initialize(weaponModel, camera_);
}

void Player::Update() {

	JustGuardUpdate();
	UpdateOverflow();

	Input* input = Input::GetInstance();

	// 特殊攻撃
	if (phase_ == AttackPhase::kNone && !isJustGuardActive_ && input->PushKey(DIK_LSHIFT) && input->TriggerKey(DIK_SPACE)) {

		if (!isOverFlow_ && energyGauge_ >= 50) {
			energyGauge_ -= 50;
			AttackMoveInitialize(AttackType::kSpecial);
		}
	}
	// 通常攻撃の開始
	else if (phase_ == AttackPhase::kNone && !isJustGuardActive_ && input->TriggerKey(DIK_SPACE)) {

		AttackMoveInitialize(AttackType::kNormal);
	}
	// 通常攻撃のコンボ入力
	else if (attackType_ == AttackType::kNormal && phase_ == AttackPhase::kRecovery && comboStep_ < kMaxComboStep && input->TriggerKey(DIK_SPACE)) {

		comboInputQueued_ = true;
	}
	// ジャストガード開始
	else if (phase_ == AttackPhase::kNone && !isJustGuardActive_ && input->TriggerKey(DIK_LSHIFT)) {

		// 入力した時点でジャストガード受付開始
		isJustGuardActive_ = true;
		justGuardTimer_ = justGuardFrames_;
	}

	Move();

	AttackMoveUpdate();

	// 3段目の回転斬り
	if (comboStep_ == 3) {

		// 回転斬りの開始角度
		float startAngle = comboStartRotationY_;

		if (phase_ == AttackPhase::kAnticipation) {
			worldTransform_.rotation_.y = startAngle;
		} else if (phase_ == AttackPhase::kExecution) {

			float t = static_cast<float>(attackTimer_) / static_cast<float>(executionFrames_);

			if (t > 1.0f) {
				t = 1.0f;
			}

			// 360度回転
			worldTransform_.rotation_.y = startAngle + t * 2.0f * static_cast<float>(M_PI);
		} else if (phase_ == AttackPhase::kRecovery) {
			// 一回転した角度を維持
			worldTransform_.rotation_.y = startAngle + 2.0f * static_cast<float>(M_PI);
		}
	}

	// 攻撃種類に応じて武器更新を分ける
	if (attackType_ == AttackType::kSpecial) {
		weapon_->UpdateSpecial(worldTransform_, static_cast<int>(phase_), attackTimer_);
	} else {
		weapon_->UpdateNormal(worldTransform_, static_cast<int>(phase_), attackTimer_, comboStep_, anticipationFrames_, executionFrames_);
	}

	ImGuiUpdate();

	WorldTransformUpdate(worldTransform_);
}

void Player::ImGuiUpdate() {

	ImGui::Begin("Player");

	// =========================
	// 行動状態
	// =========================

	ImGui::Text("Action State");

	// 攻撃段階
	const char* phaseName = "";

	switch (phase_) {
	case AttackPhase::kNone:
		phaseName = "None";
		break;

	case AttackPhase::kAnticipation:
		phaseName = "Anticipation";
		break;

	case AttackPhase::kExecution:
		phaseName = "Execution";
		break;

	case AttackPhase::kRecovery:
		phaseName = "Recovery";
		break;
	}

	ImGui::Text("Phase : %s", phaseName);

	// 攻撃種類
	const char* attackName = "";

	switch (attackType_) {
	case AttackType::kNormal:
		attackName = "Normal";
		break;

	case AttackType::kSpecial:
		attackName = "Special";
		break;
	}

	ImGui::Text("Attack Type : %s", attackName);

	// 攻撃タイマー
	ImGui::Text("Attack Timer : %d", attackTimer_);

	// =========================
	// ジャストガード
	// =========================

	ImGui::Separator();

	ImGui::Text("Just Guard");

	ImGui::Text("Active : %s", isJustGuardActive_ ? "True" : "False");

	ImGui::Text("Timer : %d", justGuardTimer_);

	// =========================
	// 移動設定
	// =========================

	ImGui::Separator();

	ImGui::Text("Movement Settings");

	ImGui::SliderFloat("Move Speed", &moveSpeed_, 0.0f, 1.0f);

	// =========================
	// 攻撃設定
	// =========================

	ImGui::Separator();

	ImGui::Text("Attack Settings");

	ImGui::SliderInt("Anticipation Frames", &anticipationFrames_, 1, 120);

	ImGui::SliderInt("Execution Frames", &executionFrames_, 1, 120);

	ImGui::SliderInt("Recovery Frames", &recoveryFrames_, 1, 120);

	// =========================
	// ジャストガード設定
	// =========================

	ImGui::Separator();

	ImGui::Text("Just Guard Settings");

	ImGui::SliderInt("Just Guard Frames", &justGuardFrames_, 1, 60);

	// =========================
	// エネルギーゲージ
	// =========================

	ImGui::Separator();

	ImGui::Text("energy Gauge");

	ImGui::SliderInt("energyGauge", &energyGauge_, 0, kMaxGuage);

	ImGui::Separator();

	ImGui::Text("HP %d", hp_);

	ImGui::End();

	if (weapon_ != nullptr) {
		weapon_->ImGuiUpdate();
	}
}

void Player::Draw() {
	assert(model_ != nullptr);
	assert(camera_ != nullptr);

	// メンバーのObjectColorを使用して描画
	model_->Draw(worldTransform_, *camera_);

	weapon_->Draw();
}

void Player::Move() {
	// 入力受付
	Input* input = Input::GetInstance();

	// 移動方向
	Vector3 move{0.0f, 0.0f, 0.0f};

	// WASD操作
	if (input->PushKey(DIK_W)) {
		move.z += 1.0f;
	}

	if (input->PushKey(DIK_S)) {
		move.z -= 1.0f;
	}

	if (input->PushKey(DIK_A)) {
		move.x -= 1.0f;
	}

	if (input->PushKey(DIK_D)) {
		move.x += 1.0f;
	}

	// 移動入力がある場合
	float length = std::sqrt(move.x * move.x + move.z * move.z);

	if (length > 0.0f) {
		// 移動ベクトルの正規化
		move /= length;

		// カメラのY軸回転に合わせて移動ベクトルを変換
		if (camera_ != nullptr) {
			Matrix4x4 rotY = MakeRotateYMatrix(camera_->rotation_.y);
			move = TransformConversion(move, rotY);
		}

		// 移動結果処理
		worldTransform_.translation_ += move * moveSpeed_;

		// 移動している方向へプレイヤーを旋回
		worldTransform_.rotation_.y = std::atan2(move.x, move.z);
	}

	// 行列更新
	WorldTransformUpdate(worldTransform_);
}

void Player::AttackMoveInitialize(AttackType type) {

	// 新しい攻撃を開始するため命中フラグをリセット
	hasHitBoss_ = false;

	attackType_ = type;

	phase_ = AttackPhase::kAnticipation;

	attackTimer_ = 0;

	// 通常攻撃の初段
	if (type == AttackType::kNormal) {
		comboStep_ = 1;
		comboInputQueued_ = false;
	} else {
		// 特殊攻撃はコンボ段数に含めない
		comboInputQueued_ = false;
	}
}

void Player::AttackMoveUpdate() {
	// 攻撃していない場合
	if (phase_ == AttackPhase::kNone) {
		return;
	}

	// 経過フレーム加算
	attackTimer_++;

	switch (phase_) {

	case AttackPhase::kAnticipation:

		if (attackTimer_ >= anticipationFrames_) {
			phase_ = AttackPhase::kExecution;
			attackTimer_ = 0;
		}

		break;

	case AttackPhase::kExecution:

		if (attackTimer_ >= executionFrames_) {

			// 攻撃処理をここに追加する

			phase_ = AttackPhase::kRecovery;
			attackTimer_ = 0;
		}

		break;

	case AttackPhase::kRecovery:

		if (attackTimer_ >= recoveryFrames_) {

			// 次段の入力が予約されている場合
			if (attackType_ == AttackType::kNormal && comboInputQueued_ && comboStep_ < kMaxComboStep) {

				++comboStep_;

				// 次のコンボ段では再び命中判定を有効にする
				hasHitBoss_ = false;

				// 3段目開始時のプレイヤーの向きを保存
				if (comboStep_ == 3) {
					comboStartRotationY_ = worldTransform_.rotation_.y;
				}

				phase_ = AttackPhase::kAnticipation;
				attackTimer_ = 0;
				comboInputQueued_ = false;

			} else {

				// コンボ終了
				phase_ = AttackPhase::kNone;
				attackTimer_ = 0;
				comboStep_ = 0;
				comboInputQueued_ = false;
			}
		}

		break;

	default:
		break;
	}

}

void Player::JustGuardUpdate() {

	// ジャストガード中でなければ終了
	if (!isJustGuardActive_) {
		return;
	}

	// 受付時間を減らす
	--justGuardTimer_;

	if (justGuardTimer_ <= 0) {
		justGuardTimer_ = 0;
		isJustGuardActive_ = false;
	}
}

bool Player::CheckJustGuard(bool enemyAttackActive, int gaugeAmount, bool isBigAttack) {

	if (!isJustGuardActive_) {
		return false;
	}

	if (!enemyAttackActive) {
		return false;
	}

	if (isBigAttack) {

		// 大技のジャストガードは必ずオーバーフローにする
		if (energyGauge_ <= kMaxGuage) {
			energyGauge_ = kMaxGuage + 1;
		}

		isOverFlow_ = true;
		overflowTimer_ = overflowFrames_;

	} else if (!isOverFlow_) {

		// 通常攻撃のジャストガードによるゲージ獲得
		energyGauge_ += gaugeAmount;

		// 100を超えた場合はオーバーフロー
		if (energyGauge_ > kMaxGuage) {
			isOverFlow_ = true;
			overflowTimer_ = overflowFrames_;
		}
	}

	// オーバーフロー中でもジャストガード自体は成立する
	isJustGuardActive_ = false;
	justGuardTimer_ = 0;

	return true;
}

OBB Player::GetBodyOBB() const {
	Vector3 center = worldTransform_.translation_;

	Matrix4x4 rotateY = MakeRotateYMatrix(worldTransform_.rotation_.y);
	Vector3 offset = TransformConversion(bodyOBBOffset_, rotateY);
	center += offset;

	Vector3 size = {bodyOBBSize_.x * worldTransform_.scale_.x, bodyOBBSize_.y * worldTransform_.scale_.y, bodyOBBSize_.z * worldTransform_.scale_.z};

	return MakeOBB(center, size, worldTransform_.rotation_);
}

void Player::ApplyHorizontalCorrection(const Vector3& correction) {
	worldTransform_.translation_.x += correction.x;
	worldTransform_.translation_.z += correction.z;

	// Y座標は変更しない
	WorldTransformUpdate(worldTransform_);
}

void Player::GainEnergyGauge(int amount) {

	// オーバーフロー中はゲージを獲得しない
	if (isOverFlow_) {
		return;
	}

	energyGauge_ += amount;

	if (energyGauge_ > kMaxGuage) {
		isOverFlow_ = true;
		overflowTimer_ = overflowFrames_;
	}
}

void Player::TakeDamage(int damage) {

	hp_ -= damage;

	if (hp_ < 0) {
		hp_ = 0;
	}
}

void Player::UpdateOverflow() {

	if (!isOverFlow_) {
		return;
	}

	// ゲージを0にする
	energyGauge_ = 0;
	
	--overflowTimer_;

	if (overflowTimer_ <= 0) {
		overflowTimer_ = 0;
		isOverFlow_ = false;
	}
}

Player::~Player() { 
	delete weapon_;
}