#include "BossAttackState.h"
#include "Boss.h"

void BossAttackState::Enter(Boss* boss) {
	boss->state_ = Boss::ActionState::kAttack;
}

void BossAttackState::Update(Boss* boss) {
	boss->UpdateAttack();
}

void BossAttackState::Exit(Boss*) {}
