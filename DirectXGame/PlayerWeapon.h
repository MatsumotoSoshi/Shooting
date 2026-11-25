#pragma once
#include "Global.h"
#include <KamataEngine.h>

class PlayerWeapon {
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

	// 通常攻撃更新
	void UpdateNormal(const KamataEngine::WorldTransform& playerTransform, int phase, int attackTimer, int comboStep, int anticipationFrames, int executionFrames);

	// 特殊攻撃更新
	void UpdateSpecial(const KamataEngine::WorldTransform& playerTransform, int phase, int attackTimer);

	// 描画
	void Draw();

	// ImGui更新
	void ImGuiUpdate();

	// 攻撃判定用OBBを取得
	OBB GetAttackOBB() const;

private:
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	KamataEngine::WorldTransform worldTransform_;

	// 前段から引き継いだ開始位置
	KamataEngine::Vector3 comboStartOffset_ = {};

	// 現在の武器位置
	KamataEngine::Vector3 comboCurrentOffset_ = {};

	// 前段から引き継いだ開始角度
	KamataEngine::Vector3 comboStartRotation_ = {};

	// 現在の武器角度
	KamataEngine::Vector3 comboCurrentRotation_ = {};

	// 現在のコンボ段
	int previousComboStep_ = 0;

	// プレイヤーからの相対位置
	KamataEngine::Vector3 localPosition_ = {0.0f, 1.5f, 0.0f};

	// 予備動作で持ち上げる高さ
	float anticipationHeight_ = 1.0f;

	// 予備動作で後ろに引く距離
	float anticipationBack_ = 0.5f;

	// 攻撃距離
	float attackDistanceY_ = 2.0f;
	float attackDistanceZ_ = 1.5f;

	// 攻撃時間
	int executionFrames_ = 10;
	int anticipationFrames_ = 10;

	// 武器の当たり判定サイズ（半分の大きさ）
	KamataEngine::Vector3 attackOBBSize_ = {0.35f, 0.8f, 0.35f};

	// 武器の色演出
	KamataEngine::ObjectColor objectColor_;

};