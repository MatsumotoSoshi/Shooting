#include "Boss.h"
#include "BossIdleState.h"
#include "BossAttackState.h"
#include <cmath>
#include <cassert>
#include <imgui.h>

using namespace KamataEngine;

void Boss::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model != nullptr);
	assert(camera != nullptr);

	camera_ = camera;
	model_ = model;

	// 乱数生成器の初期化
	std::random_device seedGenerator;
	randomEngine_.seed(seedGenerator());

	// ワールドトランスフォーム初期化
	worldTransform_.Initialize();

	// 初期座標設定
	worldTransform_.translation_ = position;

	// 地面の高さ設定
	groundHeight_ = position.y;

	// 基本スケール設定
	worldTransform_.scale_ = baseScale_;

	// 初期状態設定
	ChangeState(std::make_unique<BossIdleState>());

	// 行列更新
	WorldTransformUpdate(worldTransform_);

	// 内部数値等初期化
	hp_ = maxHP_;
	isDown_ = false;
	downTimer_ = 0;

	firstBigAttackTriggered_ = false;
	hasCompletedBigAttack_ = false;
	normalAttacksSinceBig_ = 0;
}

void Boss::Update(const Vector3& playerPosition, Player::AttackPhase playerPhase) {
	playerPosition_ = playerPosition;
	playerPhase_ = playerPhase;

	// 通常攻撃クールタイム更新
	if (attackCooldown_ > 0) {
		--attackCooldown_;
	}

	// 大技チャージ更新
	UpdateBigAttackCharge();

	// ダウン更新
	UpdateDown();

	if (currentState_ != nullptr) {
		currentState_->Update(this);
	}

	if (pendingState_ != nullptr) {
		ChangeState(std::move(pendingState_));
	}

	ImGuiUpdate();

	WorldTransformUpdate(worldTransform_);
}

void Boss::ImGuiUpdate() {

	ImGui::Begin("Boss");

	// =========================
	// 行動状態
	// =========================

	ImGui::Text("Action State");
	const char* actionStateName = "Idle";
	switch (state_) {
	case ActionState::kIdle:
		actionStateName = "Idle";
		break;
	case ActionState::kApproach:
		actionStateName = "Approach";
		break;
	case ActionState::kAttack:
		actionStateName = "Attack";
		break;
	}
	ImGui::Text("State : %s", actionStateName);

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
	case AttackType::kLightAttack:
		attackName = "Light";
		break;

	case AttackType::kMideumAttack:
		attackName = "Medium";
		break;

	case AttackType::kHeavyAttack:
		attackName = "Heavy";
		break;
	}

	ImGui::Text("Attack Type : %s", attackName);

	// 攻撃タイマー
	ImGui::Text("Attack Timer : %d", attackTimer_);

	// =========================
	// 攻撃設定
	// =========================

	ImGui::Separator();

	ImGui::Text("Attack Settings");

	// 攻撃間隔
	ImGui::Text("Attack Interval");

	ImGui::SliderInt("Interval Min", &attackIntervalMin_, 1, 600);
	ImGui::SliderInt("Interval Max", &attackIntervalMax_, 1, 600);

	// 最小値が最大値を超えないように補正
	if (attackIntervalMin_ > attackIntervalMax_) {
		attackIntervalMax_ = attackIntervalMin_;
	}

	ImGui::SliderInt("Anticipation Frames", &anticipationFrames_, 1, 120);

	ImGui::SliderInt("Execution Frames", &executionFrames_, 1, 120);

	ImGui::SliderInt("Recovery Frames", &recoveryFrames_, 1, 120);

	ImGui::Separator();
	ImGui::Text("Big Attack Settings");

	// チャージ状態
	if (isBigAttackCharging_) {
		ImGui::Text("Big Attack : Charging");
	} else if (isBigAttack_) {
		ImGui::Text("Big Attack : Executing");
	} else {
		ImGui::Text("Big Attack : None");
	}

	// チャージゲージ
	ImGui::Text("Big Attack Gauge : %.1f / %.1f", bigAttackGauge_, bigAttackGaugeMax_);

	// チャージ時間
	ImGui::Text("Charge Timer : %d / %d", bigAttackChargeTimer_, bigAttackChargeFrames_);
														
	// ゲージ最大値
	ImGui::SliderFloat("Big Attack Gauge Max", &bigAttackGaugeMax_, 0.0f, 300.0f);

	// 特殊攻撃命中時の減少量
	ImGui::SliderFloat("Big Attack Gauge Decrease", &bigAttackGaugeDecrease_, 1.0f, 100.0f);

	// チャージ時間
	ImGui::SliderInt("Big Attack Charge Frames", &bigAttackChargeFrames_, 1, 600);

	ImGui::Separator();

	ImGui::SliderInt("HP %d", &hp_, 0, 10000);

	ImGui::End();
}

void Boss::Draw() {
	assert(model_ != nullptr);
	assert(camera_ != nullptr);

	model_->Draw(worldTransform_, *camera_);

}

void Boss::Move(const Vector3& playerPosition) {

	// プレイヤーへの移動方向
	Vector3 move = playerPosition - worldTransform_.translation_;

	// Y方向の移動を無効化
	move.y = 0.0f;

	// 移動距離
	float length = std::sqrt(move.x * move.x + move.z * move.z);

	if (length > 0.0f) {

		// プレイヤーの方向を向く
		Vector3 direction = move / length;
		worldTransform_.rotation_.y = std::atan2(direction.x, direction.z);

		// 攻撃範囲内なら移動しない
		if (length <= attackRange_) {
			WorldTransformUpdate(worldTransform_);
			return;
		}

		// 攻撃範囲を越えないよう移動量を制限
		float moveDistance = moveSpeed_;

		if (length - attackRange_ < moveDistance) {
			moveDistance = length - attackRange_;
		}

		// プレイヤー方向へ移動
		worldTransform_.translation_ += direction * moveDistance;
	}

	WorldTransformUpdate(worldTransform_);
}

void Boss::FacePlayer(const Vector3& playerPosition) {

	Vector3 direction = playerPosition - worldTransform_.translation_;
	direction.y = 0.0f;

	float length = std::sqrt(direction.x * direction.x + direction.z * direction.z);

	if (length > 0.001f) {
		direction /= length;
		worldTransform_.rotation_.y = std::atan2(direction.x, direction.z);
	}

	WorldTransformUpdate(worldTransform_);
}

void Boss::ChangeState(std::unique_ptr<BossState> newState) {
	if (currentState_ != nullptr) {
		currentState_->Exit(this);
	}
	currentState_ = std::move(newState);
	if (currentState_ != nullptr) {
		currentState_->Enter(this);
	}
}

void Boss::RequestState(std::unique_ptr<BossState> newState) {
	pendingState_ = std::move(newState);
}

void Boss::AttackMoveInitialize(AttackType type) {

	// 今回の攻撃を直前の攻撃として記録
	previousAttackType_ = type;
	hasPreviousAttack_ = true;

	// 新しい攻撃なので命中フラグをリセット
	hasHitPlayer_ = false;

	attackHitActive_ = false;

	// 攻撃種類設定
	attackType_ = type;

	// 大技後に行った通常攻撃の回数を記録
	if (!isBigAttack_) {
		++normalAttacksSinceBig_;
	}

	// 攻撃開始時に次の攻撃までの間隔を抽選
	std::uniform_int_distribution<int> intervalDistribution(attackIntervalMin_, attackIntervalMax_);

	attackCooldown_ = intervalDistribution(randomEngine_);

	// 初期段階設定
	phase_ = AttackPhase::kAnticipation;

	// タイマー初期化
	attackTimer_ = 0;

	// 突進用データ初期化
	chargeMovedDistance_ = 0.0f;
	chargeDirection_ = {};

	// 突進の場合、開始時に方向を確定
	if (attackType_ == AttackType::kHeavyAttack) {

		chargeDirection_ = playerPosition_ - worldTransform_.translation_;
		chargeDirection_.y = 0.0f;

		float length = std::sqrt(chargeDirection_.x * chargeDirection_.x + chargeDirection_.z * chargeDirection_.z);

		if (length > 0.001f) {
			chargeDirection_ /= length;
			worldTransform_.rotation_.y = std::atan2(chargeDirection_.x, chargeDirection_.z);
		}
	}
}

void Boss::AttackMoveUpdate() {
	// 攻撃していない場合
	if (phase_ == AttackPhase::kNone) {
		return;
	}

	// 経過フレーム加算
	attackTimer_++;

	int anticipationDuration = anticipationFrames_;
	int executionDuration = executionFrames_;
	int recoveryDuration = recoveryFrames_;

	if (isBigAttack_) {
		anticipationDuration = bigAttackAnticipationFrames_;
		executionDuration = bigAttackExecutionFrames_;
		recoveryDuration = bigAttackRecoveryFrames_;
	}

	switch (phase_) {

	case AttackPhase::kAnticipation:

		if (attackTimer_ >= anticipationDuration){
			phase_ = AttackPhase::kExecution;
			attackTimer_ = 0;
		}

		break;

	case AttackPhase::kExecution:

		if (attackTimer_ >= executionDuration) {

			// 攻撃処理をここに追加する

			phase_ = AttackPhase::kRecovery;
			attackTimer_ = 0;
		}

		break;

	case AttackPhase::kRecovery:

		if (attackTimer_ >= recoveryDuration) {
			phase_ = AttackPhase::kNone;
			attackTimer_ = 0;
		}

		break;

	default:
		break;
	}
}

void Boss::UpdateAttack() {
	attackHitActive_ = false;

	// 大技専用処理
	if (isBigAttack_) {

		worldTransform_.translation_.y = groundHeight_;
		worldTransform_.scale_ = baseScale_;
		worldTransform_.rotation_.x = 0.0f;

		if (phase_ == AttackPhase::kAnticipation) {

			// 大技の予備動作
			worldTransform_.scale_.y = baseScale_.y * 0.6f;
			worldTransform_.rotation_.x = -0.4f;

		} else if (phase_ == AttackPhase::kExecution) {

			// 大技の攻撃動作
			worldTransform_.scale_.y = baseScale_.y * 1.5f;
			worldTransform_.rotation_.x = 0.4f;

		} else if (phase_ == AttackPhase::kRecovery) {

			// 大技後の硬直
			worldTransform_.scale_.y = baseScale_.y * 0.8f;
		}

		AttackMoveUpdate();

		// 大技終了
		if (phase_ == AttackPhase::kNone) {

			isBigAttack_ = false;

			hasCompletedBigAttack_ = true;
			normalAttacksSinceBig_ = 0;

			worldTransform_.translation_.y = groundHeight_;
			worldTransform_.scale_ = baseScale_;
			worldTransform_.rotation_.x = 0.0f;

			state_ = ActionState::kIdle;
			RequestState(std::make_unique<BossIdleState>());
		}

		return;
	}

	// 攻撃種類ごとの予備動作・攻撃・硬直をワールド変換で表現する。
	worldTransform_.translation_.y = groundHeight_;
	worldTransform_.scale_ = baseScale_;
	worldTransform_.rotation_.x = 0.0f;

	if (phase_ == AttackPhase::kAnticipation) {
		switch (attackType_) {
		case AttackType::kLightAttack:
			// ひっかきのために上体を引く。
			worldTransform_.rotation_.x = -0.3f;
			break;
		case AttackType::kMideumAttack:
			// スタンプのために跳び上がる。
			worldTransform_.translation_.y = groundHeight_ + 1.5f;
			worldTransform_.scale_.y = baseScale_.y * 0.8f;
			break;
		case AttackType::kHeavyAttack:
			// 突進前に姿勢を低くする。
			worldTransform_.scale_.y = baseScale_.y * 0.7f;
			break;
		}
	} else if (phase_ == AttackPhase::kExecution) {
		switch (attackType_) {
		case AttackType::kLightAttack:
			// ひっかきの振り抜き。
			worldTransform_.rotation_.x = 0.45f;
			if (attackTimer_ == 0) {
				Vector3 forward = {std::sin(worldTransform_.rotation_.y), 0.0f, std::cos(worldTransform_.rotation_.y)};
				worldTransform_.translation_ += forward * 0.35f;
			}

			break;
		case AttackType::kMideumAttack: {
			// 上昇位置から着地位置まで落下する
			float fallRate = 1.0f;

			if (executionFrames_ > 0) {
				fallRate = static_cast<float>(attackTimer_ + 1) / static_cast<float>(executionFrames_);
			}

			if (fallRate > 1.0f) {
				fallRate = 1.0f;
			}

			float startHeight = groundHeight_ + 1.5f;
			float endHeight = groundHeight_ + 0.1f;

			// 落下開始から着地まで徐々に下降
			worldTransform_.translation_.y = startHeight + (endHeight - startHeight) * fallRate;

			// 落下中の見た目
			worldTransform_.scale_.y = baseScale_.y * 1.3f;

			// 落下中は本体接触判定を有効
			attackHitActive_ = true;

			break;
		}
		case AttackType::kHeavyAttack: {
			// 突進の実行時間から1フレームあたりの移動距離を計算
			float chargeSpeed = 0.0f;

			if (executionFrames_ > 0) {
				chargeSpeed = chargeDistance_ / static_cast<float>(executionFrames_);
			}

			// 残りの突進距離
			float remainingDistance = chargeDistance_ - chargeMovedDistance_;

			// 指定距離に達していなければ移動
			if (remainingDistance > 0.0f) {

				float moveDistance = chargeSpeed;

				// 残り距離を超えないよう制限
				if (moveDistance > remainingDistance) {
					moveDistance = remainingDistance;
				}

				worldTransform_.translation_ += chargeDirection_ * moveDistance;
				chargeMovedDistance_ += moveDistance;
			}
			break;
		}
		}
	}

	AttackMoveUpdate();

	if (phase_ == AttackPhase::kNone) {
		worldTransform_.translation_.y = groundHeight_;
		worldTransform_.scale_ = baseScale_;
		worldTransform_.rotation_.x = 0.0f;
		state_ = ActionState::kIdle;
		RequestState(std::make_unique<BossIdleState>());
	}
}

void Boss::UpdateAI(const Vector3& playerPosition) {

	// ダウン中はAIを停止
	if (isDown_) {
		return;
	}

	// プレイヤーとの距離を計算
	Vector3 direction = playerPosition - worldTransform_.translation_;

	// 高さは無視
	direction.y = 0.0f;

	float distance = std::sqrt(direction.x * direction.x + direction.z * direction.z);

	// 大技チャージ中・大技実行中は通常AIを行わない
	if (isBigAttackCharging_ || isBigAttack_) {
		return;
	}

	// 初回の大技
	// ボスHPが最大HPの3分の2を下回るまで発動しない
	if (!firstBigAttackTriggered_ && hp_ < maxHP_ * 2 / 3) {

		firstBigAttackTriggered_ = true;
		StartBigAttackCharge();
		return;
	}

	// プレイヤーが攻撃中なら攻撃を控える
	if (playerPhase_ == Player::AttackPhase::kAnticipation || playerPhase_ == Player::AttackPhase::kExecution) {

		if (distance > approachRange_) {
			state_ = ActionState::kApproach;
			RequestState(std::make_unique<BossApproachState>());
		} else {
			state_ = ActionState::kIdle;
			RequestState(std::make_unique<BossIdleState>());
		}

		return;
	}

	// 遠距離
	if (distance > approachRange_) {

		// クールダウン中は接近
		if (attackCooldown_ > 0) {
			if (state_ != ActionState::kApproach) {
				state_ = ActionState::kApproach;
				RequestState(std::make_unique<BossApproachState>());
			}
			return;
		}

		// 遠距離では突進か接近を選択
		std::uniform_int_distribution<int> distribution(1, 100);
		int randomValue = distribution(randomEngine_);

		int chargeProbability = 30;

		// 直前も強攻撃だった場合は突進確率を下げる
		if (hasPreviousAttack_ && previousAttackType_ == AttackType::kHeavyAttack) {
			chargeProbability = 6;
		}

		// 突進
		if (randomValue <= chargeProbability) {
			state_ = ActionState::kAttack;

			AttackMoveInitialize(AttackType::kHeavyAttack);
			RequestState(std::make_unique<BossAttackState>());
		} else {
			// 70%で接近
			state_ = ActionState::kApproach;
			RequestState(std::make_unique<BossApproachState>());
		}

		return;
	}

	// クールタイム中は接近を継続
	if (attackCooldown_ > 0) {
		state_ = ActionState::kApproach;
		RequestState(std::make_unique<BossApproachState>());
		return;
	}

	// 2回目以降の大技
	// 前回の大技後に通常攻撃を5回以上行い、HPが半分以下なら5%で抽選
	if (hasCompletedBigAttack_ && hp_ <= maxHP_ / 2 && normalAttacksSinceBig_ >= 5) {

		std::uniform_int_distribution<int> bigAttackDistribution(1, 100);

		if (bigAttackDistribution(randomEngine_) <= 5) {
			normalAttacksSinceBig_ = 0;
			StartBigAttackCharge();
			return;
		}
	}

	// 通常攻撃の選択
	AttackType selectedAttack = SelectAttack(distance);

	// 攻撃開始
	state_ = ActionState::kAttack;

	AttackMoveInitialize(selectedAttack);
	RequestState(std::make_unique<BossAttackState>());
}

float Boss::GetDistanceToPlayer(const Vector3& playerPosition) const {
	Vector3 direction = playerPosition - worldTransform_.translation_;
	direction.y = 0.0f;
	return std::sqrt(direction.x * direction.x + direction.z * direction.z);
}

Boss::AttackType Boss::SelectAttack(float distance) {

	int lightWeight = 0;
	int mediumWeight = 0;
	int heavyWeight = 0;

	// 距離に応じた基本確率
	if (distance <= attackRange_) {

		lightWeight = 70;
		mediumWeight = 30;

	} else if (distance <= approachRange_) {

		lightWeight = 10;
		mediumWeight = 45;
		heavyWeight = 45;

	} else {

		return AttackType::kHeavyAttack;
	}

	// 直前と同じ攻撃の選択確率を20%に下げる
	if (hasPreviousAttack_) {

		if (previousAttackType_ == AttackType::kLightAttack) {
			lightWeight = lightWeight * 20 / 100;
		} else if (previousAttackType_ == AttackType::kMideumAttack) {
			mediumWeight = mediumWeight * 20 / 100;
		} else if (previousAttackType_ == AttackType::kHeavyAttack) {
			heavyWeight = heavyWeight * 20 / 100;
		}
	}

	int totalWeight = lightWeight + mediumWeight + heavyWeight;

	std::uniform_int_distribution<int> distribution(1, totalWeight);
	int randomValue = distribution(randomEngine_);

	if (randomValue <= lightWeight) {
		return AttackType::kLightAttack;
	}

	randomValue -= lightWeight;

	if (randomValue <= mediumWeight) {
		return AttackType::kMideumAttack;
	}

	return AttackType::kHeavyAttack;
}

OBB Boss::GetBodyOBB() const {

	Vector3 size = {bodyOBBSize_.x * worldTransform_.scale_.x, bodyOBBSize_.y * worldTransform_.scale_.y, bodyOBBSize_.z * worldTransform_.scale_.z};

	return MakeOBB(worldTransform_.translation_, size, worldTransform_.rotation_);
}

OBB Boss::GetAttackOBB() const {

	Vector3 offset{};
	Vector3 size{};

	// 攻撃種類によって判定範囲を変更
	switch (attackType_) {

	case AttackType::kLightAttack:
		offset = lightAttackOBBOffset_;
		size = lightAttackOBBSize_;
		break;

	case AttackType::kMideumAttack:
		offset = mediumAttackOBBOffset_;
		size = mediumAttackOBBSize_;
		break;

	case AttackType::kHeavyAttack:
		offset = heavyAttackOBBOffset_;
		size = heavyAttackOBBSize_;
		break;
	}

	// ボスの向きに合わせてローカル位置を回転
	Matrix4x4 rotateY = MakeRotateYMatrix(worldTransform_.rotation_.y);
	offset = TransformConversion(offset, rotateY);

	Vector3 center{};

	if (attackType_ == AttackType::kMideumAttack) {
		// 中攻撃はボスの上下移動に影響されず、地面を基準にする。
		center = {worldTransform_.translation_.x + offset.x, groundHeight_ + mediumAttackOBBOffset_.y, worldTransform_.translation_.z + offset.z};
	} else {
		center = worldTransform_.translation_ + offset;
	}

	// サイズはワールド空間の値をそのまま使用する。
	return MakeOBB(center, size, worldTransform_.rotation_);
}

bool Boss::IsAttackHitActive() const {

	if (isBigAttack_) {
		return false;
	}

	if (attackType_ == AttackType::kMideumAttack) {
		return attackHitActive_;
	}

	return phase_ == AttackPhase::kExecution && attackTimer_ > 0;
}

bool Boss::IsBigAttackHitActive() const { return isBigAttack_ && phase_ == AttackPhase::kExecution && attackTimer_ > 0; }

void Boss::ApplyHorizontalCorrection(const Vector3& correction) {
	worldTransform_.translation_.x += correction.x;
	worldTransform_.translation_.z += correction.z;

	// Y座標は変更しない
	WorldTransformUpdate(worldTransform_);
}

void Boss::StartBigAttackCharge() {

	isBigAttackCharging_ = true;
	bigAttackGauge_ = bigAttackGaugeMax_;
	bigAttackChargeTimer_ = 0;

	// プレイヤーの方向を向く
	FacePlayer(playerPosition_);

	// チャージ中は待機ステートを利用
	state_ = ActionState::kIdle;
	RequestState(std::make_unique<BossIdleState>());
}

void Boss::UpdateBigAttackCharge() {

	if (!isBigAttackCharging_) {
		return;
	}

	// チャージ時間を進める
	++bigAttackChargeTimer_;

	// チャージ中の溜めモーション
	worldTransform_.scale_.x = baseScale_.x * 1.1f;
	worldTransform_.scale_.y = baseScale_.y * 0.65f;
	worldTransform_.scale_.z = baseScale_.z * 1.1f;

	worldTransform_.rotation_.x = -0.2f;

	// ゲージがなくなった場合は中断
	if (bigAttackGauge_ <= 0.0f) {
		CancelBigAttackCharge();
		return;
	}

	// チャージ時間終了
	if (bigAttackChargeTimer_ >= bigAttackChargeFrames_) {
		StartBigAttack();
	}
}

void Boss::CancelBigAttackCharge() {

	isBigAttackCharging_ = false;
	bigAttackChargeTimer_ = 0;
	bigAttackGauge_ = 0.0f;

	// 姿勢を戻す
	worldTransform_.scale_ = baseScale_;
	worldTransform_.rotation_.x = 0.0f;
	worldTransform_.translation_.y = groundHeight_;

	// チャージ阻止時はダウン
	StartDown();
}

void Boss::StartBigAttack() {

	isBigAttackCharging_ = false;
	isBigAttack_ = true;

	// 攻撃初期化
	phase_ = AttackPhase::kAnticipation;
	attackTimer_ = 0;

	hasHitPlayer_ = false;
	attackHitActive_ = false;

	state_ = ActionState::kAttack;

	// 既存のBossAttackStateを利用
	RequestState(std::make_unique<BossAttackState>());
}

void Boss::ReduceBigAttackGauge() {

	if (!isBigAttackCharging_) {
		return;
	}

	bigAttackGauge_ -= bigAttackGaugeDecrease_;

	if (bigAttackGauge_ < 0.0f) {
		bigAttackGauge_ = 0.0f;
	}

	// ゲージがなくなった時点でチャージ中断
	if (bigAttackGauge_ <= 0.0f) {
		CancelBigAttackCharge();
	}
}

OBB Boss::GetBigAttackOBB() const {

	Vector3 offset = bigAttackOBBOffset_;

	// ボスの向きに合わせてローカル位置を回転
	Matrix4x4 rotateY = MakeRotateYMatrix(worldTransform_.rotation_.y);

	offset = TransformConversion(offset, rotateY);

	Vector3 center = worldTransform_.translation_ + offset;

	return MakeOBB(center, bigAttackOBBSize_, worldTransform_.rotation_);
}

void Boss::StartDown() {

	isDown_ = true;

	std::uniform_int_distribution<int> downDistribution(downFramesMin_, downFramesMax_);

	downTimer_ = downDistribution(randomEngine_);

	// 既存の待機ステートを利用する
	state_ = ActionState::kIdle;
	phase_ = AttackPhase::kNone;
	attackTimer_ = 0;

	RequestState(std::make_unique<BossIdleState>());
}

void Boss::UpdateDown() {

	if (!isDown_) {
		return;
	}

	--downTimer_;

	if (downTimer_ <= 0) {
		downTimer_ = 0;
		isDown_ = false;

		state_ = ActionState::kIdle;
		RequestState(std::make_unique<BossIdleState>());
	}
}

void Boss::TakeDamage(int damage) {

	hp_ -= damage;

	if (hp_ < 0) {
		hp_ = 0;
	}
}
