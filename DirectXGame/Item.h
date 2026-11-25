#pragma once
#include <KamataEngine.h>

class Item {
public:
	virtual ~Item() = default;

	virtual void Initialize();
	virtual void Update();
	virtual void Draw();

	// 弾が当たったとき
	virtual void OnHit();



protected:
	bool isActive_ = true;


};