#pragma once
#include <KamataEngine.h>
#include "Global.h"

class PlayerWeapon;

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

	// ImGui調整用
	void ImGuiUpdate();

	// プレイヤーの座標取得
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }

	// ワールドトランスフォーム取得
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	// 移動
	void Move();

	// 移動速度
	float moveSpeed_ = 0.2f;

	// 攻撃段階
	enum class AttackPhase {
		kNone,         // 攻撃していない
		kAnticipation, // 攻撃前の予備動作
		kExecution,    // 攻撃動作
		kRecovery,     // 攻撃後の硬直
	};

	// 攻撃種類
	enum class AttackType {
		kNormal,  // 通常攻撃
		kSpecial, // 特殊攻撃
	};

	// 攻撃状態
	AttackPhase phase_ = AttackPhase::kNone;

	// 現在の攻撃種類
	AttackType attackType_ = AttackType::kNormal;

	// 攻撃経過フレーム
	int attackTimer_ = 0;

	// 攻撃段階の時間
	int anticipationFrames_ = 10;
	int executionFrames_ = 5;
	int recoveryFrames_ = 20;

	// 通常攻撃コンボ段数
	int comboStep_ = 0;

	// 次の攻撃入力を受け付けたか
	bool comboInputQueued_ = false;

	// 回転斬り開始時のプレイヤーの向き
	float comboStartRotationY_ = 0.0f;

	// コンボ最大段数
	static const int kMaxComboStep = 3;

	// 攻撃遷移初期化
	void AttackMoveInitialize(AttackType type);

	// 攻撃遷移更新
	void AttackMoveUpdate();

	// ジャストガード受付時間
	int justGuardFrames_ = 19;

	// ジャストガード残りフレーム
	int justGuardTimer_ = 0;

	// ジャストガード中か
	bool isJustGuardActive_ = false;

	// ジャストガード判定取得
	bool IsJustGuardActive() const { return isJustGuardActive_; }

	// ジャストガード更新
	void JustGuardUpdate();

	// 敵の攻撃と照合してジャストガード成功を判定
	// gaugeAmount : ボスの攻撃種類に応じたゲージ獲得量
	// isBigAttack : 大技かどうか
	bool CheckJustGuard(bool enemyAttackActive, int gaugeAmount, bool isBigAttack = false);

	// 行動可能か
	bool CanAction() const { return phase_ == AttackPhase::kNone && !isJustGuardActive_; }

	// エネルギーゲージ
	int energyGauge_ = 0;

	int kMaxGuage = 100;

	// プレイヤーHP
	int hp_ = 100;
	int maxHP_ = 100;

	// HP減少
	void TakeDamage(int damage);

	// ゲージ獲得
	void GainEnergyGauge(int amount);

	// オーバーフロー時間
	int overflowFrames_ = 1200; // 60FPSで約20秒
	int overflowTimer_ = 0;

	// HP取得
	int GetHP() const { return hp_; }

	// オーバーフロー
	bool isOverFlow_ = false;

	// プレイヤー本体のOBBを取得
	OBB GetBodyOBB() const;

	// 武器を取得
	PlayerWeapon* GetWeapon() const { return weapon_; }

	// ボスに命中済みか
	bool HasHitBoss() const { return hasHitBoss_; }

	// ボスに命中したことを記録
	void RegisterBossHit() { hasHitBoss_ = true; }

	// 水平方向の位置補正
	void ApplyHorizontalCorrection(const KamataEngine::Vector3& correction);

	// 現在の攻撃が特殊攻撃か
	bool IsSpecialAttack() const { return attackType_ == AttackType::kSpecial; }

private:
	// 3Dモデル
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	PlayerWeapon* weapon_ = nullptr;

	// ボスへの攻撃が命中済みか
	bool hasHitBoss_ = false;

	// プレイヤー本体のOBB
	KamataEngine::Vector3 bodyOBBOffset_ = {0.0f, 0.0f, 0.0f};
	KamataEngine::Vector3 bodyOBBSize_ = {0.5f, 0.9f, 0.5f};

	// オーバーフロー時間の更新
	void UpdateOverflow();

};