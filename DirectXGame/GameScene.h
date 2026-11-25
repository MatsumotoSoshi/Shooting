#pragma once
#include "CameraController.h"
#include "Enemy.h"
#include "Ground.h"
#include "Player.h"
#include "PlayerBullet.h"
#include "Skydome.h"
#include <KamataEngine.h>
#include <list>

class GameScene {
public:
	void Initialize();
	void Update();
	void Draw();
	~GameScene();

private:
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

	// 敵
	std::list<Enemy*> enemies_;
	KamataEngine::Model* enemyModel_ = nullptr;

	// プレイヤーの弾
	std::list<PlayerBullet*> playerBullets_;
	KamataEngine::Model* bulletModel_ = nullptr;

	// 敵出現タイマー
	int enemySpawnTimer_ = 0;
	int enemySpawnInterval_ = 120;
	int maxEnemyCount_ = 20;

	// プレイヤー接触ダメージの間隔
	int damageInterval_ = 30;
	int damageTimer_ = 0;

	// 敵の生成
	void SpawnEnemy();

	// 最寄りの敵を取得
	Enemy* GetNearestEnemy() const;

	// 敵同士の反発方向を取得
	KamataEngine::Vector3 GetEnemyRepel(const Enemy* enemy) const;

	// 敵と弾の当たり判定
	void CheckBulletCollision();

	// 敵とプレイヤーの当たり判定
	void CheckPlayerCollision();

	// 削除済みオブジェクトの整理
	void RemoveDeadObjects();
};
