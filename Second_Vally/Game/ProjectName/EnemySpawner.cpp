#include "pch.h"
#include "EnemySpawner.h"
#include "Enemy.h"

#include "UE/Level.h"
#include "UE/LevelEngineSystem.h"

#include <random>

namespace
{
	std::mt19937 GSpawnerRandom{ std::random_device{}() };
	std::uniform_int_distribution<int> RandomX{ 0, 1919 };
	std::uniform_int_distribution<int> RandomY{ 0, 539 };
}

AEnemySpawner::AEnemySpawner()
{
	__Scale = { 0.0, 0.0 };
}

void AEnemySpawner::Tick(float _DeltaTime)
{
	if (__SpawnedCount >= __TargetCount)
		return;

	__SpawnTimer -= _DeltaTime;
	if (__SpawnTimer > 0.0f)
		return;

	ULevel* Level = LevelEngineSystem::GetInstance()->GetCurrentLevel();
	if (Level == nullptr)
		return;

	AEnemy* Enemy = Level->SpawnActor<AEnemy>(EActorGroup::Monster);
	if (Enemy == nullptr)
		return;

	Enemy->__Pos = {
		static_cast<double>(RandomX(GSpawnerRandom)),
		static_cast<double>(RandomY(GSpawnerRandom))
	};
	++__SpawnedCount;
	__SpawnTimer = 0.12f;
}
