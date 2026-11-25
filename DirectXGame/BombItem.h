#pragma once
#include "Item.h"
#include <KamataEngine.h>

class BombItem : public Item {
public:
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void OnHit() override;



private:
	void Explode();


};