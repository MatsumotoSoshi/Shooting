#pragma once
#include <KamataEngine.h>
#include "CameraController.h"
#include "Ground.h"
#include "Player.h"
#include "Boss.h"
#include "Skydome.h"

class GameScene {
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();
	
	// 描画
	void Draw();

	// デストラクタ
	~GameScene();

private:
	// カメラコントローラー
	CameraController cameraController_;

	// 地面
	Ground* ground_ = nullptr;
	KamataEngine::Model* groundModel_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;
	KamataEngine::Model* skydomeModel_ = nullptr;

	// プレイヤー
	Player* player_ = nullptr;
	KamataEngine::Model* playerModel_ = nullptr;

	// ボス
	Boss* boss_ = nullptr;
	KamataEngine::Model* bossModel_ = nullptr;
};