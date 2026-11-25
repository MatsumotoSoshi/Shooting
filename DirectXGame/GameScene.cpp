#define NONIMAX
#include "GameScene.h"
#include "InputManager.h"
#include <Windows.h>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <limits>

using namespace KamataEngine;

namespace {

float GetDistanceXZ(const Vector3& a, const Vector3& b) {

	float dx = a.x - b.x;
	float dz = a.z - b.z;

	return std::sqrt(dx * dx + dz * dz);
}

} // namespace

void GameScene::Initialize() {

	cameraController_.Initialize();

	// 地面
	groundModel_ = Model::CreateFromOBJ("Tile");
	assert(groundModel_ != nullptr);

	ground_ = new Ground();
	ground_->Initialize(groundModel_, &cameraController_.GetCamera(), {0.0f, 0.0f, 0.0f});

	// 天球
	skydomeModel_ = Model::CreateFromOBJ("Skydome");
	assert(skydomeModel_ != nullptr);

	skydome_ = new Skydome();
	skydome_->Initialize(skydomeModel_, &cameraController_.GetCamera(), {0.0f, 0.0f, 0.0f});

	// プレイヤー
	playerModel_ = Model::CreateFromOBJ("player");
	assert(playerModel_ != nullptr);

	player_ = new Player();
	player_->Initialize(playerModel_, &cameraController_.GetCamera(), {0.0f, 0.5f, 0.0f});

	// 敵モデル
	enemyModel_ = Model::CreateFromOBJ("cube");
	assert(enemyModel_ != nullptr);

	// 弾モデル
	bulletModel_ = Model::CreateFromOBJ("particle");
	assert(bulletModel_ != nullptr);

	// カメラ追従対象
	cameraController_.SetTarget(player_);
	cameraController_.Reset();

	enemySpawnTimer_ = 0;
	damageTimer_ = 0;
}

void GameScene::Update() {

	// 地面
	ground_->Update();

	// 天球
	skydome_->Update();

	// 敵の出現
	++enemySpawnTimer_;

	if (enemySpawnTimer_ >= enemySpawnInterval_) {

		enemySpawnTimer_ = 0;

		if (static_cast<int>(enemies_.size()) < maxEnemyCount_) {
			SpawnEnemy();
		}
	}

	// 入力情報を更新
	InputManager::GetInstance()->Update();

	// プレイヤー更新
	player_->Update();

	// プレイヤーが射撃要求を出したら弾を生成
	if (player_->IsShotRequested()) {

		Vector3 bulletPosition = player_->GetPosition();

		bulletPosition.y += 0.5f;

		PlayerBullet* bullet = new PlayerBullet();

		bullet->Initialize(bulletPosition, player_->GetShotDirection());

		playerBullets_.push_back(bullet);

		player_->ResetShotRequest();
	}

	// 敵の更新
	for (Enemy* enemy : enemies_) {

		Vector3 repel = GetEnemyRepel(enemy);

		enemy->Update(player_->GetPosition(), repel);
	}

	// 弾の更新
	for (PlayerBullet* bullet : playerBullets_) {
		bullet->Update();
	}

	// 当たり判定
	CheckBulletCollision();
	CheckPlayerCollision();

	// 削除
	RemoveDeadObjects();

	// ダメージ受付タイマー
	if (damageTimer_ > 0) {
		--damageTimer_;
	}

	// カメラ
	cameraController_.Update();
}

void GameScene::SpawnEnemy() {

	Vector3 playerPosition = player_->GetPosition();

	// プレイヤーの周囲から出現位置を決める
	float angle = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * 2.0f * static_cast<float>(M_PI);

	float radius = 15.0f;

	Vector3 spawnPosition = {playerPosition.x + std::cos(angle) * radius, 0.5f, playerPosition.z + std::sin(angle) * radius};

	Enemy* enemy = new Enemy();
	enemy->Initialize(spawnPosition);

	enemies_.push_back(enemy);
}

Enemy* GameScene::GetNearestEnemy() const {

	Enemy* nearestEnemy = nullptr;

	float nearestDistance = (std::numeric_limits<float>::max)();

	Vector3 playerPosition = player_->GetPosition();

	for (Enemy* enemy : enemies_) {

		if (!enemy->IsAlive()) {
			continue;
		}

		float distance = GetDistanceXZ(playerPosition, enemy->GetPosition());

		if (distance < nearestDistance) {

			nearestDistance = distance;
			nearestEnemy = enemy;
		}
	}

	return nearestEnemy;
}

Vector3 GameScene::GetEnemyRepel(const Enemy* targetEnemy) const {

	Vector3 repel = {0.0f, 0.0f, 0.0f};

	for (Enemy* enemy : enemies_) {

		if (enemy == targetEnemy || !enemy->IsAlive()) {
			continue;
		}

		Vector3 difference = targetEnemy->GetPosition() - enemy->GetPosition();

		difference.y = 0.0f;

		float distance = std::sqrt(difference.x * difference.x + difference.z * difference.z);

		// 敵同士が近い場合だけ反発
		if (distance > 0.001f && distance < 1.5f) {

			difference /= distance;

			repel.x += difference.x;
			repel.z += difference.z;
		}
	}

	return repel;
}

void GameScene::CheckBulletCollision() {

	for (PlayerBullet* bullet : playerBullets_) {

		if (!bullet->IsAlive()) {
			continue;
		}

		for (Enemy* enemy : enemies_) {

			if (!enemy->IsAlive()) {
				continue;
			}

			float distance = GetDistanceXZ(bullet->GetPosition(), enemy->GetPosition());

			// 弾と敵の衝突
			if (distance < 0.8f) {

				enemy->Destroy();
				bullet->Destroy();

				break;
			}
		}
	}
}

void GameScene::CheckPlayerCollision() {

	if (player_->GetHP() <= 0) {
		return;
	}

	if (damageTimer_ > 0) {
		return;
	}

	for (Enemy* enemy : enemies_) {

		if (!enemy->IsAlive()) {
			continue;
		}

		float distance = GetDistanceXZ(player_->GetPosition(), enemy->GetPosition());

		if (distance < 0.9f) {

			player_->TakeDamage(10);

			damageTimer_ = damageInterval_;

			break;
		}
	}
}

void GameScene::RemoveDeadObjects() {

	// 敵の削除
	for (std::list<Enemy*>::iterator iterator = enemies_.begin(); iterator != enemies_.end();) {

		if (!(*iterator)->IsAlive()) {

			delete *iterator;
			iterator = enemies_.erase(iterator);

		} else {
			++iterator;
		}
	}

	// 弾の削除
	for (std::list<PlayerBullet*>::iterator iterator = playerBullets_.begin(); iterator != playerBullets_.end();) {

		float distance = GetDistanceXZ((*iterator)->GetPosition(), player_->GetPosition());

		if (!(*iterator)->IsAlive() || distance > 40.0f) {

			delete *iterator;
			iterator = playerBullets_.erase(iterator);

		} else {
			++iterator;
		}
	}
}

void GameScene::Draw() {

	Model::PreDraw();

	ground_->Draw();
	skydome_->Draw();
	player_->Draw();

	// 敵
	for (Enemy* enemy : enemies_) {
		enemy->Draw(enemyModel_, &cameraController_.GetCamera());
	}

	// 弾
	for (PlayerBullet* bullet : playerBullets_) {
		bullet->Draw(bulletModel_, &cameraController_.GetCamera());
	}

	Model::PostDraw();
}

GameScene::~GameScene() {

	delete ground_;
	delete groundModel_;

	delete skydome_;
	delete skydomeModel_;

	delete player_;
	delete playerModel_;

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}

	enemies_.clear();

	for (PlayerBullet* bullet : playerBullets_) {
		delete bullet;
	}

	playerBullets_.clear();

	delete enemyModel_;
	delete bulletModel_;
}
