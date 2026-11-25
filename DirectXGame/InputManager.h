#pragma once
#define NOMINMAX
#include "Global.h"
#include <KamataEngine.h>
#include <Xinput.h>

/// <summary>
/// 実際の入力を扱う唯一のクラス。KamataEngine::Inputへの直接アクセスをここに集約し、
/// Player/敵/UIMenuなど他のシステムは全てこのクラス経由で入力状態を問い合わせる。
/// ボタンAABB・カーソルスプライト・フェードといったUI要素は一切扱わない(それはUIMenuの役目)。
/// 毎フレーム先頭で1回だけUpdate()を呼び、そのフレーム中の問い合わせは全てキャッシュされた値を返す。
/// </summary>
class InputManager {
public:
	static InputManager* GetInstance();

	void Update();

	// --- 移動 ---
	bool IsMoveLeftHeld() const { return moveLeftHeld_; }
	bool IsMoveRightHeld() const { return moveRightHeld_; }

	// -1.0 / 0.0 / 1.0 (左右同時押しは0)
	float GetMoveAxisX() const;

	// 左キー/Aキー/左スティックが押されているかどうか
	bool PushLeftKey() const;

	// 右キー/Dキー/左スティックが押されているかどうか
	bool PushRightKey() const;

	bool TriggerLeftKey() const;
	bool TriggerRightKey() const;

	// --- ジャンプ ---
	bool IsJumpHeld() const { return jumpHeld_; }           // 接地中に押され続けていればジャンプ
	bool IsJumpTriggered() const { return jumpTriggered_; } // 立ち上がり

	// --- アクション ---
	bool IsWeaponSwitchTriggered() const { return weaponSwitchTriggered_; }
	bool IsPauseTriggered() const { return pauseTriggered_; }
	bool IsInteractTriggered() const { return interactTriggered_; }

	bool IsAttackUpHeld() const { return attackUpHeld_; }
	bool IsAttackDownHeld() const { return attackDownHeld_; }

	// 攻撃：Q / マウス左 / RT
	bool TriggerAttackKey() const;

	// 攻撃：Q / マウス左 / RT の立ち上がり
	bool TriggerAttackKeyEdge() const;

	// --- 決定 ---
	// Space / マウス左 / A
	bool TriggerDecisionKey() const;

	// Space / マウス左 / A の立ち上がり
	bool TriggerDecisionKeyEdge() const;

	// --- メニュー操作 ---
	// W / 上キー / 左スティック上 / 右スティック上 / D-Pad上
	bool IsMenuUpTriggered() const;

	// S / 下キー / 左スティック下 / 右スティック下 / D-Pad下
	bool IsMenuDownTriggered() const;

	// --- コントローラー ---
	// 汎用：コントローラーボタンの立ち上がり判定
	bool IsControllerButtonTriggered(WORD button) const;

	// --- スティック ---
	bool MoveStickUp() const;
	bool MoveStickDown() const;
	bool MoveStickLeft() const;
	bool MoveStickRight() const;

	// --- マウス ---
	int GetMouseX() const { return mouseX_; }
	int GetMouseY() const { return mouseY_; }

	KamataEngine::Vector2 GetMouseScreenPosition() const;

	bool IsPrimaryTriggered() const { return primaryMouseTriggered_; }
	bool IsPrimaryHeld() const { return primaryMouseHeld_; }

	bool IsBlockHeld() const { return secondaryMouseHeld_; }
	bool IsSecondaryTriggered() const { return secondaryMouseTriggered_; }

	// マウス左クリックの立ち上がり判定
	bool TriggerMouseLeft() const;

	// --- メニュー決定 ---
	// Space / Controller Aのみ。
	// マウス左クリックはUIMenu側でAABB判定する。
	bool IsMenuConfirmTriggered() const;

private:
	InputManager() = default;
	~InputManager() = default;

	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;

	// --- キーボード ---
	bool moveLeftHeld_ = false;
	bool moveRightHeld_ = false;

	bool jumpHeld_ = false;
	bool jumpTriggered_ = false;

	bool weaponSwitchTriggered_ = false;
	bool pauseTriggered_ = false;
	bool interactTriggered_ = false;

	bool attackUpHeld_ = false;
	bool attackDownHeld_ = false;

	// --- マウス ---
	KamataEngine::Vector2 mouseMovement_{};
	int mouseX_ = 0;
	int mouseY_ = 0;

	bool primaryMouseTriggered_ = false;
	bool primaryMouseHeld_ = false;

	bool secondaryMouseHeld_ = false;
	bool secondaryMouseTriggered_ = false;
	 
	// --- コントローラー ---
	XINPUT_STATE controllerState_{};
	XINPUT_STATE previousControllerState_{};

	static constexpr SHORT kStickDeadZone_ = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;

	static constexpr BYTE kTriggerThreshold_ = XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
};