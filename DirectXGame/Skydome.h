#pragma once
#include <KamataEngine.h>

class Skydome {
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position = {0.0f, 0.0f, 0.0f});

	// 更新
	void Update();

	// 描画
	void Draw();

	// 位置取得
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

	// ワールドトランスフォーム取得
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

private:
	// 3Dモデル
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;
};
