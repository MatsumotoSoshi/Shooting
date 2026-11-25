#include "BossIdleState.h"
#include "Boss.h"

void BossIdleState::Enter(Boss* boss) {
	boss->state_ = Boss::ActionState::kIdle;
}

void BossIdleState::Update(Boss* boss) {
	boss->UpdateAI(boss->GetPlayerPosition());
}

void BossIdleState::Exit(Boss*) {}

void BossApproachState::Enter(Boss* boss) {
	boss->state_ = Boss::ActionState::kApproach;
}

void BossApproachState::Update(Boss* boss) {

	// 共通AI判断
	boss->UpdateAI(boss->GetPlayerPosition());

	// 攻撃へ移行した場合は移動しない
	if (boss->state_ == Boss::ActionState::kAttack) {
		return;
	}

	// Idleへ移行した場合は移動しない
	if (boss->state_ == Boss::ActionState::kIdle) {
		boss->approachDelayTimer_ = 0;
		return;
	}

	// 接近状態の場合
	if (boss->state_ == Boss::ActionState::kApproach) {

		float distance = boss->GetDistanceToPlayer(boss->GetPlayerPosition());

		// 接近条件内に戻ったら待機時間をリセットして移動
		if (distance <= boss->approachRange_) {
			boss->approachDelayTimer_ = 0;
			boss->Move(boss->GetPlayerPosition());
			return;
		}

		// プレイヤーの方向を向く
		boss->FacePlayer(boss->GetPlayerPosition());

		// 接近開始まで待機
		if (boss->approachDelayTimer_ < boss->approachDelayFrames_) {
			++boss->approachDelayTimer_;
			return;
		}

		// 待機時間終了後に接近
		boss->Move(boss->GetPlayerPosition());
	}
}

void BossApproachState::Exit(Boss*) {}
