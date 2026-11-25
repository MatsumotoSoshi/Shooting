#pragma once
#include "BossState.h"
class BossAttackState : public BossState {
public:
	void Enter(Boss* boss) override;
	void Update(Boss* boss) override;
	void Exit(Boss* boss) override;
};
