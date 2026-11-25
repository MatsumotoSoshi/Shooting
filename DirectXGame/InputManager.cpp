#include "InputManager.h"

using namespace KamataEngine;

InputManager* InputManager::GetInstance() {
	static InputManager instance;
	return &instance;
}

void InputManager::Update() {
	Input* input = Input::GetInstance();

	// --- 移動 ---
	moveLeftHeld_ = input->PushKey(DIK_A) || input->PushKey(DIK_LEFT);

	moveRightHeld_ = input->PushKey(DIK_D) || input->PushKey(DIK_RIGHT);

	// --- ジャンプ ---
	jumpHeld_ = input->PushKey(DIK_SPACE);
	jumpTriggered_ = input->TriggerKey(DIK_SPACE);

	// --- アクション ---
	weaponSwitchTriggered_ = input->TriggerKey(DIK_Q);
	pauseTriggered_ = input->TriggerKey(DIK_ESCAPE);
	interactTriggered_ = input->TriggerKey(DIK_F);

	attackUpHeld_ = input->PushKey(DIK_W);
	attackDownHeld_ = input->PushKey(DIK_S);

	// --- マウス ---
	mouseMovement_ = input->GetMousePosition();

	mouseX_ = static_cast<int>(mouseMovement_.x);

	mouseY_ = static_cast<int>(mouseMovement_.y);

	primaryMouseTriggered_ = input->IsTriggerMouse(0);

	primaryMouseHeld_ = input->IsPressMouse(0);

	secondaryMouseHeld_ = input->IsPressMouse(1);

	secondaryMouseTriggered_ = input->IsTriggerMouse(1);

	// --- コントローラー ---
	// 現在フレームと前フレームの状態をここで一度だけ取得する。
	input->GetJoystickState(0, controllerState_);
	input->GetJoystickStatePrevious(0, previousControllerState_);
}

float InputManager::GetMoveAxisX() const {
	if (moveLeftHeld_ == moveRightHeld_) {
		return 0.0f; // どちらも押していない、または両方押している
	}

	return moveRightHeld_ ? 1.0f : -1.0f;
}

bool InputManager::PushLeftKey() const {
	// 左キー/Aキー/左スティックが押されているかどうか
	if (moveLeftHeld_) {
		return true;
	}

	return controllerState_.Gamepad.sThumbLX < -kStickDeadZone_;
}

bool InputManager::PushRightKey() const {
	// 右キー/Dキー/左スティックが押されているかどうか
	if (moveRightHeld_) {
		return true;
	}

	return controllerState_.Gamepad.sThumbLX > kStickDeadZone_;
}

bool InputManager::TriggerLeftKey() const {
	// Aキー/左キーの立ち上がり
	if (Input::GetInstance()->TriggerKey(DIK_A) || Input::GetInstance()->TriggerKey(DIK_LEFT)) {
		return true;
	}

	const bool nowLeft = controllerState_.Gamepad.sThumbLX < -kStickDeadZone_;

	const bool prevLeft = previousControllerState_.Gamepad.sThumbLX < -kStickDeadZone_;

	// 左スティックが左方向へ入った瞬間だけtrueにする。
	return nowLeft && !prevLeft;
}

bool InputManager::TriggerRightKey() const {
	// Dキー/右キーの立ち上がり
	if (Input::GetInstance()->TriggerKey(DIK_D) || Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
		return true;
	}

	const bool nowRight = controllerState_.Gamepad.sThumbLX > kStickDeadZone_;

	const bool prevRight = previousControllerState_.Gamepad.sThumbLX > kStickDeadZone_;

	// 右スティックが右方向へ入った瞬間だけtrueにする。
	return nowRight && !prevRight;
}

bool InputManager::TriggerAttackKey() const {
	// 攻撃：Q / マウス左 / RT
	if (Input::GetInstance()->PushKey(DIK_Q) || primaryMouseHeld_) {
		return true;
	}

	// RTトリガー
	return controllerState_.Gamepad.bRightTrigger > kTriggerThreshold_;
}

bool InputManager::TriggerAttackKeyEdge() const {
	// Qキーの立ち上がり
	if (Input::GetInstance()->TriggerKey(DIK_Q)) {
		return true;
	}

	// マウス左クリックの立ち上がり
	if (primaryMouseTriggered_) {
		return true;
	}

	// RTトリガーの立ち上がり判定
	return controllerState_.Gamepad.bRightTrigger > kTriggerThreshold_ && previousControllerState_.Gamepad.bRightTrigger <= kTriggerThreshold_;
}

bool InputManager::TriggerDecisionKey() const {
	// 決定：Space / マウス左 / A
	if (jumpHeld_ || primaryMouseHeld_) {
		return true;
	}

	// コントローラーAボタン
	return (controllerState_.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
}

bool InputManager::TriggerDecisionKeyEdge() const {
	// Spaceの立ち上がり
	if (jumpTriggered_) {
		return true;
	}

	// マウス左クリックの立ち上がり
	if (primaryMouseTriggered_) {
		return true;
	}

	// Aボタンの立ち上がり
	return IsControllerButtonTriggered(XINPUT_GAMEPAD_A);
}

bool InputManager::IsMenuUpTriggered() const {
	// Wキー / 上キー
	if (Input::GetInstance()->TriggerKey(DIK_W) || Input::GetInstance()->TriggerKey(DIK_UP)) {
		return true;
	}

	// D-Pad上
	if (IsControllerButtonTriggered(XINPUT_GAMEPAD_DPAD_UP)) {
		return true;
	}

	// 左スティック、右スティックのどちらかが
	// 上方向へ入った瞬間
	const bool nowUp = controllerState_.Gamepad.sThumbLY > kStickDeadZone_ || controllerState_.Gamepad.sThumbRY > kStickDeadZone_;

	const bool prevUp = previousControllerState_.Gamepad.sThumbLY > kStickDeadZone_ || previousControllerState_.Gamepad.sThumbRY > kStickDeadZone_;

	return nowUp && !prevUp;
}

bool InputManager::IsMenuDownTriggered() const {
	// Sキー / 下キー
	if (Input::GetInstance()->TriggerKey(DIK_S) || Input::GetInstance()->TriggerKey(DIK_DOWN)) {
		return true;
	}

	// D-Pad下
	if (IsControllerButtonTriggered(XINPUT_GAMEPAD_DPAD_DOWN)) {
		return true;
	}

	// 左スティック、右スティックのどちらかが
	// 下方向へ入った瞬間
	const bool nowDown = controllerState_.Gamepad.sThumbLY < -kStickDeadZone_ || controllerState_.Gamepad.sThumbRY < -kStickDeadZone_;

	const bool prevDown = previousControllerState_.Gamepad.sThumbLY < -kStickDeadZone_ || previousControllerState_.Gamepad.sThumbRY < -kStickDeadZone_;

	return nowDown && !prevDown;
}

bool InputManager::IsControllerButtonTriggered(WORD button) const {

	// 現在押されていて、前フレームでは押されていない
	return (controllerState_.Gamepad.wButtons & button) && !(previousControllerState_.Gamepad.wButtons & button);
}

bool InputManager::MoveStickUp() const {
	const bool nowUp = controllerState_.Gamepad.sThumbLY > kStickDeadZone_ || controllerState_.Gamepad.sThumbRY > kStickDeadZone_;

	const bool prevUp = previousControllerState_.Gamepad.sThumbLY > kStickDeadZone_ || previousControllerState_.Gamepad.sThumbRY > kStickDeadZone_;

	// どちらかのスティックが上方向へ入った瞬間を判定
	return nowUp && !prevUp;
}

bool InputManager::MoveStickDown() const {
	const bool nowDown = controllerState_.Gamepad.sThumbLY < -kStickDeadZone_ || controllerState_.Gamepad.sThumbRY < -kStickDeadZone_;

	const bool prevDown = previousControllerState_.Gamepad.sThumbLY < -kStickDeadZone_ || previousControllerState_.Gamepad.sThumbRY < -kStickDeadZone_;

	// どちらかのスティックが下方向へ入った瞬間を判定
	return nowDown && !prevDown;
}

bool InputManager::MoveStickLeft() const {
	const bool nowLeft = controllerState_.Gamepad.sThumbLX < -kStickDeadZone_ || controllerState_.Gamepad.sThumbRX < -kStickDeadZone_;

	const bool prevLeft = previousControllerState_.Gamepad.sThumbLX < -kStickDeadZone_ || previousControllerState_.Gamepad.sThumbRX < -kStickDeadZone_;

	// どちらかのスティックが左方向へ入った瞬間を判定
	return nowLeft && !prevLeft;
}

bool InputManager::MoveStickRight() const {
	const bool nowRight = controllerState_.Gamepad.sThumbLX > kStickDeadZone_ || controllerState_.Gamepad.sThumbRX > kStickDeadZone_;

	const bool prevRight = previousControllerState_.Gamepad.sThumbLX > kStickDeadZone_ || previousControllerState_.Gamepad.sThumbRX > kStickDeadZone_;

	// どちらかのスティックが右方向へ入った瞬間を判定
	return nowRight && !prevRight;
}

Vector2 InputManager::GetMouseScreenPosition() const {
	// マウスカーソルのスクリーン座標を取得
	return mouseMovement_;
}

bool InputManager::TriggerMouseLeft() const {
	// マウス左クリックの立ち上がり判定
	return primaryMouseTriggered_;
}

bool InputManager::IsMenuConfirmTriggered() const {
	// Spaceキーの立ち上がり
	if (jumpTriggered_) {
		return true;
	}

	// コントローラーAボタンの立ち上がり
	return IsControllerButtonTriggered(XINPUT_GAMEPAD_A);
}
