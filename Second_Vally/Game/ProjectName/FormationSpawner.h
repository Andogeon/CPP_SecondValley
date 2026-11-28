#pragma once

#include "UE/Actor.h"
#include <array>

class AEnemy;

class AFormationSpawner final : public AActor
{
	friend class ULevel;

private:
	AFormationSpawner();

public:
	~AFormationSpawner() override = default;

protected:
	void Tick(float _DeltaTime) override;

private:
	void SpawnFormation(int _Pattern);

	float __Timer = 0.0f;
	int __NextFormation = 0;
	std::array<AEnemy*, 10> __LastFormation = {};
	bool __BossSpawned = false;
};
