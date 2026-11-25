#include "GameScene.h"
#include <cassert>
#include "PlayerWeapon.h"

using namespace KamataEngine;

namespace {

// ベクトルの内積
float DotProduct(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

// OBBを指定方向に投影したときの半径
float GetOBBProjectionRadius(const OBB& obb, const Vector3& direction) {
	float radius = 0.0f;

	radius += obb.size.x * std::fabs(DotProduct(obb.orientations[0], direction));
	radius += obb.size.y * std::fabs(DotProduct(obb.orientations[1], direction));
	radius += obb.size.z * std::fabs(DotProduct(obb.orientations[2], direction));

	return radius;
}

// プレイヤーとボスの本体OBBが重なった場合に押し戻す
void ResolveBodyCollision(Player* player, Boss* boss) {

	OBB playerOBB = player->GetBodyOBB();
	OBB bossOBB = boss->GetBodyOBB();

	// 本体同士が重なっていなければ終了
	if (!IsCollisionOBB(playerOBB, bossOBB)) {
		return;
	}

	// 水平方向の中心間ベクトル
	Vector3 direction = bossOBB.center - playerOBB.center;
	direction.y = 0.0f;

	float distance = std::sqrt(direction.x * direction.x + direction.z * direction.z);

	// 中心がほぼ同じ場合の対策
	if (distance < 0.001f) {
		direction = boss->GetPosition() - player->GetPosition();
		direction.y = 0.0f;

		distance = std::sqrt(direction.x * direction.x + direction.z * direction.z);

		if (distance < 0.001f) {
			direction = {1.0f, 0.0f, 0.0f};
			distance = 1.0f;
		}
	}

	// 水平方向に正規化
	direction /= distance;

	// 両者のOBBを方向ベクトルに投影した半径
	float playerRadius = GetOBBProjectionRadius(playerOBB, direction);
	float bossRadius = GetOBBProjectionRadius(bossOBB, direction);

	// 水平方向の重なり量
	float overlap = playerRadius + bossRadius - distance;

	if (overlap <= 0.0f) {
		return;
	}

	// 少し余裕を持たせて押し戻す
	float correctionDistance = (overlap + 0.001f) * 0.5f;

	Vector3 playerCorrection = direction * -correctionDistance;
	Vector3 bossCorrection = direction * correctionDistance;

	// 両者を互いに離す
	player->ApplyHorizontalCorrection(playerCorrection);
	boss->ApplyHorizontalCorrection(bossCorrection);
}

} // namespace

void GameScene::Initialize() {
	cameraController_.Initialize();

	// 地面モデル読み込み・生成
	groundModel_ = Model::CreateFromOBJ("Tile");
	assert(groundModel_ != nullptr);
	ground_ = new Ground();
	ground_->Initialize(groundModel_, &cameraController_.GetCamera(), {0.0f, 0.0f, 0.0f});

	// 天球モデル読み込み・生成
	skydomeModel_ = Model::CreateFromOBJ("Skydome");
	assert(skydomeModel_ != nullptr);
	skydome_ = new Skydome();
	skydome_->Initialize(skydomeModel_, &cameraController_.GetCamera(), {0.0f, 0.0f, 0.0f});

	// プレイヤーモデル読み込み・生成
	playerModel_ = Model::CreateFromOBJ("player");
	assert(playerModel_ != nullptr);
	player_ = new Player();
	player_->Initialize(playerModel_, &cameraController_.GetCamera(), {0.0f, 0.5f, 0.0f});

	// ボスモデル読み込み・生成
	bossModel_ = Model::CreateFromOBJ("cube");
	assert(bossModel_ != nullptr);
	boss_ = new Boss();
	boss_->Initialize(bossModel_, &cameraController_.GetCamera(), {0.0f, 2.0f, 10.0f});

	// 追従対象を設定して初期位置へ即時移動
	cameraController_.SetTarget(player_);
	cameraController_.Reset();
}

void GameScene::Update() {

	// 地面更新
	ground_->Update();

	// 天球更新
	skydome_->Update();

	// プレイヤー更新
	player_->Update();

	// ボス更新
	boss_->Update(player_->GetPosition(), player_->phase_);


#pragma region プレイヤーの攻撃判定

	if (player_->phase_ == Player::AttackPhase::kExecution && !player_->HasHitBoss() && player_->GetWeapon() != nullptr) {

		OBB playerAttackOBB = player_->GetWeapon()->GetAttackOBB();
		OBB bossBodyOBB = boss_->GetBodyOBB();

		if (IsCollisionOBB(playerAttackOBB, bossBodyOBB)) {

			if (player_->IsSpecialAttack()) {

				// 特殊攻撃はボスに100ダメージ
				boss_->TakeDamage(100);

				// チャージ中なら大技ゲージを100減らす
				if (boss_->IsBigAttackCharging()) {
					boss_->ReduceBigAttackGauge();
				}

			} else {

				// 通常攻撃はコンボ段数に応じてダメージ
				switch (player_->comboStep_) {
				case 1:
					boss_->TakeDamage(10);
					break;

				case 2:
					boss_->TakeDamage(15);
					break;

				case 3:
					boss_->TakeDamage(25);

					// 3段目命中時にゲージを10獲得
					player_->GainEnergyGauge(10);
					break;
				}
			}

			// この攻撃中は再度命中しない
			player_->RegisterBossHit();
		}
	}

#pragma endregion

#pragma region ボスの攻撃判定

	if (!boss_->HasHitPlayer()) {

		// =========================
		// 大技
		// =========================
		if (boss_->IsBigAttackHitActive()) {

			OBB bossBigAttackOBB = boss_->GetBigAttackOBB();
			OBB playerBodyOBB = player_->GetBodyOBB();

			if (IsCollisionOBB(bossBigAttackOBB, playerBodyOBB)) {

				// 大技のジャストガード
				bool isJustGuardSuccess = player_->CheckJustGuard(true, 0, true);

				if (isJustGuardSuccess) {

					// 成功時は通常ダメージを与えず、ボスへ反撃
					boss_->TakeDamage(boss_->GetBigAttackCounterDamage());

				} else {

					// 失敗時はプレイヤーに30ダメージ
					player_->TakeDamage(30);
				}

				boss_->RegisterPlayerHit();
			}
		}

		// =========================
		// 中攻撃
		// =========================
		else if (boss_->attackType_ == Boss::AttackType::kMideumAttack) {

			if (boss_->IsAttackHitActive()) {

				OBB bossBodyOBB = boss_->GetBodyOBB();
				OBB bossAttackOBB = boss_->GetAttackOBB();
				OBB playerBodyOBB = player_->GetBodyOBB();

				if (IsCollisionOBB(bossBodyOBB, playerBodyOBB) || IsCollisionOBB(bossAttackOBB, playerBodyOBB)) {

					bool isJustGuardSuccess = player_->CheckJustGuard(true, 20);

					if (!isJustGuardSuccess) {
						player_->TakeDamage(10);
					}

					boss_->RegisterPlayerHit();
				}
			}
		}

		// =========================
		// 弱攻撃・強攻撃
		// =========================
		else if (boss_->IsAttackHitActive()) {

			OBB bossAttackOBB = boss_->GetAttackOBB();
			OBB playerBodyOBB = player_->GetBodyOBB();

			if (IsCollisionOBB(bossAttackOBB, playerBodyOBB)) {

				int damage = 0;
				int gaugeAmount = 0;

				if (boss_->attackType_ == Boss::AttackType::kLightAttack) {
					damage = 5;
					gaugeAmount = 15;
				} else if (boss_->attackType_ == Boss::AttackType::kHeavyAttack) {
					damage = 15;
					gaugeAmount = 30;
				}

				bool isJustGuardSuccess = player_->CheckJustGuard(true, gaugeAmount);

				if (!isJustGuardSuccess) {
					player_->TakeDamage(damage);
				}

				boss_->RegisterPlayerHit();
			}
		}
	}

#pragma endregion

	// 本体同士の接触・押し戻し
	ResolveBodyCollision(player_, boss_);

	// カメラ更新
	cameraController_.Update();
}

void GameScene::Draw() {

	// 3Dモデル描画前処理
	Model::PreDraw();

	// 地面描画
	ground_->Draw();

	// 天球描画
	skydome_->Draw();

	// プレイヤー描画
	player_->Draw();

	// ボス描画
	boss_->Draw();

	// 3Dモデル描画後処理
	Model::PostDraw();
}

GameScene::~GameScene() {
	delete ground_;
	delete groundModel_;
	delete skydome_;
	delete skydomeModel_;
	delete player_;
	delete playerModel_;
	delete boss_;
	delete bossModel_;
}