#include "pch.h"
#include "FormationSpawner.h"
#include "Enemy.h"
#include "Player.h"
#include "ProjectName.h"
#include "StageBoss.h"

#include "UE/Level.h"
#include "UE/LevelEngineSystem.h"

#include <algorithm>

AFormationSpawner::AFormationSpawner()
{
	__Scale = { 0.0, 0.0 };
	__Timer = 0.5f;
}

void AFormationSpawner::Tick(float _DeltaTime)
{
	if (__NextFormation < 3)
	{
		__Timer -= _DeltaTime;
		if (__Timer <= 0.0f)
		{
			SpawnFormation(__NextFormation++);
			__Timer = 4.5f;
			return;
		}
	}

	if (__NextFormation < 3 || __BossSpawned)
		return;

	ULevel* Level = LevelEngineSystem::GetInstance()->GetCurrentLevel();
	APlayer* Player = GetProjectPlayer();
	if (Level == nullptr || Player == nullptr)
		return;

	for (const auto& Actor : Level->GetActors(EActorGroup::Monster))
	{
		const AEnemy* Plane = dynamic_cast<const AEnemy*>(Actor.get());
		if (Plane != nullptr &&
			std::find(__LastFormation.begin(), __LastFormation.end(), Plane) != __LastFormation.end() &&
			!Plane->HasEnteredFormation())
			return;
	}

	AStageBoss* Boss = Level->SpawnActor<AStageBoss>(EActorGroup::Monster, Player->__Scale);
	Boss->StartEntrance({ 960.0, -Boss->__Scale.Y * 0.5 }, { 960.0, 300.0 });
	__BossSpawned = true;
}

void AFormationSpawner::SpawnFormation(int _Pattern)
{
	ULevel* Level = LevelEngineSystem::GetInstance()->GetCurrentLevel();
	if (Level == nullptr)
		return;

	const FVector2D Origins[] =
	{
		{ 960.0, -300.0 },
		{ 960.0, 420.0 },
		{ -420.0, 560.0 }
	};

	for (int Slot = 0; Slot < 10; ++Slot)
	{
		AEnemy* Plane = Level->SpawnActor<AEnemy>(EActorGroup::Monster);
		if (Plane == nullptr)
			continue;
		Plane->ConfigureFormation(_Pattern, Slot, Origins[_Pattern]);
		Plane->__Pos = Origins[_Pattern];
		if (_Pattern == 2)
			__LastFormation[Slot] = Plane;
	}
}
