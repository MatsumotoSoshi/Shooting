#pragma once
#include "BossState.h"
class BossIdleState : public BossState {
public:
	void Enter(Boss* boss) override;
	void Update(Boss* boss) override;
	void Exit(Boss* boss) override;
};

class BossApproachState : public BossState {
public:
	void Enter(Boss* boss) override;
	void Update(Boss* boss) override;
	void Exit(Boss* boss) override;
};
