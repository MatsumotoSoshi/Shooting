#pragma once

class Boss;

class BossState {
public:
	virtual ~BossState() = default;

	// 状態に入ったとき
	virtual void Enter(Boss* boss) = 0;

	// 毎フレームの更新
	virtual void Update(Boss* boss) = 0;

	// 状態から出るとき
	virtual void Exit(Boss* boss) = 0;
};