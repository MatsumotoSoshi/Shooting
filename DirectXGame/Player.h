#pragma once
#include "Global.h"
#include "InputManager.h"
#include <KamataEngine.h>

class Player {
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// デストラクタ
	~Player();

	// 移動
	void Move();

	// ImGui調整用
	void ImGuiUpdate();

	// プレイヤーの座標取得
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

	// ワールドトランスフォーム取得
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	// 移動速度
	float moveSpeed_ = 0.2f;

	// HP
	int hp_ = 100;
	int maxHP_ = 100;

	// ダメージ
	void TakeDamage(int damage);

	// HP取得
	int GetHP() const { return hp_; }

	// 弾の発射要求
	bool IsShotRequested() const { return shotRequested_; }

	// 発射要求を解除
	void ResetShotRequest() { shotRequested_ = false; }

	// 弾の発射方向
	const KamataEngine::Vector3& GetShotDirection() const { return shotDirection_; }

private:
	// 3Dモデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 入力管理
	InputManager* inputManager_ = nullptr;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// 射撃設定
	int shotIntervalFrames_ = 30;
	int shotTimer_ = 0;

	// 弾の発射要求
	bool shotRequested_ = false;

	// 弾の方向
	KamataEngine::Vector3 shotDirection_ = {0.0f, 0.0f, 0.0f};

	// 照準方向が有効か
	bool hasValidAimDirection_ = false;

	// マウスカーソルの方向を計算
	void UpdateAimDirection();

	// マウス位置から地面上の照準位置を取得
	bool GetMouseWorldPosition(KamataEngine::Vector3& worldPosition);

	// マウスカーソル十字
	KamataEngine::Sprite* cursorHorizontal_ = nullptr;
	KamataEngine::Sprite* cursorVertical_ = nullptr;

	uint32_t cursorTexture_ = 0;
};
