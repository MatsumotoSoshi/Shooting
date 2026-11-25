#pragma once
#include <KamataEngine.h>

class Player;

class CameraController {
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();

	void Reset();

	void SetTarget(Player* target) {
		target_ = target;
	}

	KamataEngine::Camera& GetCamera() {
		return camera_;
	}

	// 注視オフセットの設定・取得
	void SetTargetOffset(const KamataEngine::Vector3& offset) {
		targetOffset_ = offset;
	}
	const KamataEngine::Vector3& GetTargetOffset() const {
		return targetOffset_;
	}

private:
	// カメラ
	KamataEngine::Camera camera_;

	Player* target_ = nullptr;

	// 注視対象からのカメラの位置（斜め手前・上空からのオフセット）
	KamataEngine::Vector3 targetOffset_ = {-20.0f, 25.0f, -20.0f};

	KamataEngine::Vector3 destination_{0.0f, 0.0f, 0.0f};
	static inline const float kInterpolationRate = 0.1f; // 補間の速度

};
