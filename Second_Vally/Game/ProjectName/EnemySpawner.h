#pragma once

#include "UE/Actor.h"

class AEnemySpawner final : public AActor
{
	friend class ULevel;

private:
	AEnemySpawner();

public:
	~AEnemySpawner() override = default;

protected:
	void Tick(float _DeltaTime) override;

private:
	float __SpawnTimer = 0.0f;
	int __SpawnedCount = 0;
	static constexpr int __TargetCount = 60;
};
