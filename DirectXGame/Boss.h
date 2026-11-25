#pragma once
#include "Global.h"
#include <KamataEngine.h>
#include <random>
#include "BossState.h"
#include "Player.h"

class Boss {
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	// 更新
	void Update(const KamataEngine::Vector3& playerPosition, Player::AttackPhase playerPhase);

	// 描画
	void Draw();

	// ImGui調整用
	void ImGuiUpdate();

	// プレイヤーの座標取得
	const KamataEngine::Vector3& GetPosition() const { return worldTransform_.translation_; }
	const KamataEngine::Vector3& GetPlayerPosition() const { return playerPosition_; }

	// ワールドトランスフォーム取得
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	// 移動
	void Move(const KamataEngine::Vector3& playerPosition);

	// プレイヤーの方向を向く
	void FacePlayer(const KamataEngine::Vector3& playerPosition);

	// 移動速度
	float moveSpeed_ = 0.15f;

	// 状態遷移関数
	void ChangeState(std::unique_ptr<BossState> newState);
	void RequestState(std::unique_ptr<BossState> newState);
	void UpdateAI(const KamataEngine::Vector3& playerPosition);
	float GetDistanceToPlayer(const KamataEngine::Vector3& playerPosition) const;
	void UpdateAttack();

	enum class ActionState {
		kIdle,     // 待機
		kApproach, // 接近
		kAttack,   // 攻撃
	};

	// 攻撃段階
	enum class AttackPhase {
		kNone,         // 攻撃していない
		kAnticipation, // 攻撃前の予備動作
		kExecution,    // 攻撃動作
		kRecovery,     // 攻撃後の硬直
	};

	// 攻撃種類
	enum class AttackType {
		kLightAttack,    // 弱攻撃
		kMideumAttack,   // 中攻撃
		kHeavyAttack,    // 強攻撃
	};

	// ボスの行動状態
	ActionState state_ = ActionState::kIdle;

	// 攻撃状態
	AttackPhase phase_ = AttackPhase::kNone;

	// 現在の攻撃種類
	AttackType attackType_ = AttackType::kLightAttack;

	// 乱数生成器
	std::mt19937 randomEngine_;

	// プレイヤーとの距離による行動範囲
	float attackRange_ = 3.0f;
	float approachRange_ = 7.0f;

	// 突進設定
	float chargeDistance_ = 20.0f;
	KamataEngine::Vector3 chargeDirection_{};
	float chargeMovedDistance_ = 0.0f;

	// 接近開始までの待機設定
	int approachDelayFrames_ = 60;
	int approachDelayTimer_ = 0;

	// 攻撃選択
	AttackType SelectAttack(float distance);

	// 攻撃経過フレーム
	int attackTimer_ = 0;

	// 攻撃間隔
	// 攻撃間隔のランダム設定
	int attackIntervalMin_ = 120;
	int attackIntervalMax_ = 240;

	// 次の攻撃までの残り時間
	int attackCooldown_ = 0;

	// 攻撃段階の時間
	int anticipationFrames_ = 30;
	int executionFrames_ = 10;
	int recoveryFrames_ = 40;

	// 攻撃遷移初期化
	void AttackMoveInitialize(AttackType type);

	// 攻撃遷移更新
	void AttackMoveUpdate();

	// ボス本体のOBBを取得
	OBB GetBodyOBB() const;

	// ボス攻撃のOBBを取得
	OBB GetAttackOBB() const;

	// プレイヤーに命中済みか
	bool HasHitPlayer() const { return hasHitPlayer_; }

	// 現在のフレームで攻撃判定が有効か
	bool IsAttackHitActive() const;

	// プレイヤーに命中したことを記録
	void RegisterPlayerHit() { hasHitPlayer_ = true; }

	// 水平方向の位置補正
	void ApplyHorizontalCorrection(const KamataEngine::Vector3& correction);

	// 大技チャージ中か
	bool IsBigAttackCharging() const { return isBigAttackCharging_; }

	// 大技の攻撃判定が有効か
	bool IsBigAttackHitActive() const;

	// 大技専用OBBを取得
	OBB GetBigAttackOBB() const;

	// プレイヤーの特殊攻撃が命中したときにチャージゲージを減らす
	void ReduceBigAttackGauge();

	// ボスHP
	int hp_ = 10000;
	int maxHP_ = 10000;

	// HP取得
	int GetHP() const { return hp_; }

	// ダメージ処理
	void TakeDamage(int damage);

	// 大技ジャストガード成功時の反撃ダメージ
	int bigAttackCounterDamage_ = 500;

	// 反撃ダメージ取得
	int GetBigAttackCounterDamage() const { return bigAttackCounterDamage_; }

private:
	// 3Dモデル
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Vector3 playerPosition_{};
	KamataEngine::Vector3 baseScale_{2.0f, 2.0f, 2.0f};
	float groundHeight_ = 0.0f;

	// 状態遷移メンバ変数
	std::unique_ptr<BossState> currentState_;
	std::unique_ptr<BossState> pendingState_;

	Player::AttackPhase playerPhase_ = Player::AttackPhase::kNone;

	// 直前の攻撃履歴
	AttackType previousAttackType_ = AttackType::kLightAttack;
	bool hasPreviousAttack_ = false;

	// 同じ攻撃中の重複命中防止
	bool hasHitPlayer_ = false;

	// ボス本体のOBB
	KamataEngine::Vector3 bodyOBBSize_ = {1.0f, 1.0f, 1.0f};

	// 攻撃種類ごとのOBBサイズ
	KamataEngine::Vector3 lightAttackOBBSize_ = {2.2f, 1.8f, 2.6f};
	KamataEngine::Vector3 mediumAttackOBBSize_ = {3.5f, 1.0f, 3.5f};
	KamataEngine::Vector3 heavyAttackOBBSize_ = {2.2f, 1.6f, 3.2f};

	// 攻撃OBBのローカル位置
	KamataEngine::Vector3 lightAttackOBBOffset_ = {0.0f, 0.0f, 1.5f};
	KamataEngine::Vector3 mediumAttackOBBOffset_ = {0.0f, 0.0f, 0.0f};
	KamataEngine::Vector3 heavyAttackOBBOffset_ = {0.0f, 0.0f, 1.5f};

	// 中攻撃の着地フレームだけ有効にする判定フラグ
	bool attackHitActive_ = false;

	// =========================
	// 大技チャージ設定
	// =========================

	// 大技チャージ中か
	bool isBigAttackCharging_ = false;

	// 大技実行中か
	bool isBigAttack_ = false;

	// 大技チャージゲージ
	float bigAttackGaugeMax_ = 300.0f;
	float bigAttackGauge_ = 300.0f;

	// 特殊攻撃1回あたりのゲージ減少量
	float bigAttackGaugeDecrease_ = 100.0f;

	// チャージ時間
	int bigAttackChargeFrames_ = 600;
	int bigAttackChargeTimer_ = 0;

	// 大技専用の攻撃時間
	int bigAttackAnticipationFrames_ = 60;
	int bigAttackExecutionFrames_ = 20;
	int bigAttackRecoveryFrames_ = 60;

	// 大技専用OBB
	KamataEngine::Vector3 bigAttackOBBSize_ = {5.0f, 2.0f, 5.0f};
	KamataEngine::Vector3 bigAttackOBBOffset_ = {0.0f, 0.0f, 0.0f};

	// 大技チャージ・実行処理
	void StartBigAttackCharge();
	void UpdateBigAttackCharge();
	void CancelBigAttackCharge();
	void StartBigAttack();


	// 初回大技を開始したか
	bool firstBigAttackTriggered_ = false;

	// 大技を一度でも完了したか
	bool hasCompletedBigAttack_ = false;

	// 大技後の通常攻撃回数
	int normalAttacksSinceBig_ = 0;

	// ボスダウン
	bool isDown_ = false;
	int downTimer_ = 0;
	int downFramesMin_ = 600;
	int downFramesMax_ = 900;

	// 大技チャージ中断時のダウン処理
	void StartDown();

	// ダウン更新
	void UpdateDown();
};
