#include "Player.h"
#include <cassert>
#include <cmath>
#include <imgui.h>

using namespace KamataEngine;

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {

	assert(model != nullptr);
	assert(camera != nullptr);

	camera_ = camera;
	model_ = model;
	inputManager_ = InputManager::GetInstance();

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 十字カーソル用テクスチャ
	cursorTexture_ = TextureManager::Load("white1x1.png");

	// 横線
	cursorHorizontal_ = Sprite::Create(cursorTexture_, {0.0f, 0.0f});

	// 縦線
	cursorVertical_ = Sprite::Create(cursorTexture_, {0.0f, 0.0f});

	hp_ = maxHP_;
	shotTimer_ = 0;
	shotRequested_ = false;
	shotDirection_ = {0.0f, 0.0f, 0.0f};
	hasValidAimDirection_ = false;

	WorldTransformUpdate(worldTransform_);
}

void Player::Update() {

	shotRequested_ = false;

	// 移動
	Move();

	// マウスカーソルの方向を計算
	UpdateAimDirection();

	// 有効な照準方向がある場合だけ射撃
	Input* input = Input::GetInstance();

	if (input->PushKey(DIK_SPACE)) {
		shotRequested_ = true;
	}

	if (input->IsPressMouse(0) && hasValidAimDirection_) {

		++shotTimer_;

		if (shotTimer_ >= shotIntervalFrames_) {
			shotTimer_ = 0;
			shotRequested_ = true;
		}

	} else {
		shotTimer_ = 0;
	}

	ImGuiUpdate();

	WorldTransformUpdate(worldTransform_);
}

void Player::UpdateAimDirection() {

	hasValidAimDirection_ = false;
	shotDirection_ = {0.0f, 0.0f, 0.0f};

	Vector3 aimPosition;

	if (!GetMouseWorldPosition(aimPosition)) {
		return;
	}

	Vector3 direction = aimPosition - worldTransform_.translation_;

	direction.y = 0.0f;

	float length = std::sqrt(direction.x * direction.x + direction.z * direction.z);

	if (length <= 0.001f) {
		return;
	}

	direction /= length;

	shotDirection_ = direction;
	hasValidAimDirection_ = true;
}

void Player::Move() {
	// 入力受付
	Input* input = Input::GetInstance();

	// 移動方向
	Vector3 move{0.0f, 0.0f, 0.0f};

	// WASD操作
	if (input->PushKey(DIK_W)) {
		move.z += 1.0f;
	}

	if (input->PushKey(DIK_S)) {
		move.z -= 1.0f;
	}

	if (input->PushKey(DIK_A)) {
		move.x -= 1.0f;
	}

	if (input->PushKey(DIK_D)) {
		move.x += 1.0f;
	}

	// 移動入力がある場合
	float length = std::sqrt(move.x * move.x + move.z * move.z);

	if (length > 0.0f) {
		// 移動ベクトルの正規化
		move /= length;

		// カメラのY軸回転に合わせて移動ベクトルを変換
		if (camera_ != nullptr) {
			Matrix4x4 rotY = MakeRotateYMatrix(camera_->rotation_.y);
			move = TransformConversion(move, rotY);
		}

		// 移動結果処理
		worldTransform_.translation_ += move * moveSpeed_;

		// 移動している方向へプレイヤーを旋回
		worldTransform_.rotation_.y = std::atan2(move.x, move.z);
	}

	// 行列更新
	WorldTransformUpdate(worldTransform_);
}

void Player::Draw() {

	assert(model_ != nullptr);
	assert(camera_ != nullptr);

	model_->Draw(worldTransform_, *camera_);

	if (inputManager_ == nullptr) {
		return;
	}

	// マウス座標
	Vector2 mousePosition = inputManager_->GetMouseScreenPosition();

	// 十字カーソルのサイズ
	const float cursorLength = 24.0f;
	const float cursorThickness = 2.0f;

	// 横線の位置・サイズ
	cursorHorizontal_->SetPosition({mousePosition.x - cursorLength * 0.5f, mousePosition.y - cursorThickness * 0.5f});

	cursorHorizontal_->SetSize({cursorLength, cursorThickness});

	// 縦線の位置・サイズ
	cursorVertical_->SetPosition({mousePosition.x - cursorThickness * 0.5f, mousePosition.y - cursorLength * 0.5f});

	cursorVertical_->SetSize({cursorThickness, cursorLength});

	Sprite::PreDraw();

	// 描画
	cursorHorizontal_->Draw();
	cursorVertical_->Draw();

	Sprite::PostDraw();
}

void Player::TakeDamage(int damage) {

	if (damage <= 0 || hp_ <= 0) {
		return;
	}

	hp_ -= damage;

	if (hp_ < 0) {
		hp_ = 0;
	}
}

bool Player::GetMouseWorldPosition(Vector3& worldPosition) {

	if (camera_ == nullptr || inputManager_ == nullptr) {
		return false;
	}

	// マウス座標
	Vector2 mousePosition = inputManager_->GetMouseScreenPosition();

	// 画面サイズ
	const float screenWidth = 1280.0f;
	const float screenHeight = 720.0f;

	// NDC座標に変換
	float ndcX = (mousePosition.x / screenWidth) * 2.0f - 1.0f;

	float ndcY = 1.0f - (mousePosition.y / screenHeight) * 2.0f;

	// カメラの視野角
	// 実際のカメラ設定と合わせる必要がある
	const float fovAngleY = 0.45f;

	float aspectRatio = screenWidth / screenHeight;

	float tanHalfFov = std::tan(fovAngleY * 0.5f);

	// カメラ空間のレイ方向
	Vector3 rayDirection = {ndcX * aspectRatio * tanHalfFov, ndcY * tanHalfFov, 1.0f};

	// カメラの回転を反映
	Matrix4x4 rotateMatrix = MakeRotateMatrix(camera_->rotation_);

	rayDirection = TransformConversion(rayDirection, rotateMatrix);

	// 正規化
	float length = std::sqrt(rayDirection.x * rayDirection.x + rayDirection.y * rayDirection.y + rayDirection.z * rayDirection.z);

	if (length <= 0.001f) {
		return false;
	}

	rayDirection /= length;

	// 地面との交点
	const float groundHeight = worldTransform_.translation_.y;

	// レイが地面方向を向いていない場合
	if (rayDirection.y >= -0.001f) {
		return false;
	}

	float t = (groundHeight - camera_->translation_.y) / rayDirection.y;

	if (t <= 0.0f) {
		return false;
	}

	worldPosition = camera_->translation_ + rayDirection * t;

	return true;
}

void Player::ImGuiUpdate() {

	ImGui::Begin("Player");

	ImGui::Text("HP : %d / %d", hp_, maxHP_);

	ImGui::Separator();

	ImGui::Text("Movement Settings");

	ImGui::SliderFloat("Move Speed", &moveSpeed_, 0.0f, 1.0f);

	ImGui::Separator();

	ImGui::Text("Shooting Settings");

	ImGui::SliderInt("Shot Interval", &shotIntervalFrames_, 1, 120);

	ImGui::Text("Shot Timer : %d", shotTimer_);

	ImGui::End();
}

Player::~Player() {

	delete cursorHorizontal_;
	delete cursorVertical_;
}
