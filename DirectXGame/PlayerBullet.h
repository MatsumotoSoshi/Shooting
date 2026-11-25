#pragma once
#include "Global.h"
#include <KamataEngine.h>

class PlayerBullet {
public:
	// 初期化
	void Initialize(const KamataEngine::Vector3& position, const KamataEngine::Vector3& direction);

	// 更新
	void Update();

	// 描画
	void Draw(KamataEngine::Model* model, KamataEngine::Camera* camera);

	// 座標取得
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

	// ワールドトランスフォーム取得
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	// 生存状態
	bool IsAlive() const { return isAlive_; }

	// 削除
	void Destroy() { isAlive_ = false; }

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Vector3 velocity_{};

	float moveSpeed_ = 0.7f;

	bool isAlive_ = true;
};
