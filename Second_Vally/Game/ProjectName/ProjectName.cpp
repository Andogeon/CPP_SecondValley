#include "pch.h"
#include "ProjectName.h"

#include "Player.h"
#include "UE/LevelEngineSystem.h"
#include "UE/CollisionEngineSystem.h"

#include <random>

namespace
{
	APlayer* GProjectPlayer = nullptr;

	void ConfigureCollisionChannels()
	{
		auto* Collision = CollisionEngineSystem::GetInstance();
		Collision->InspectionOff();
		Collision->RequestInspection(EActorGroup::Player, EActorGroup::Monster);
		Collision->RequestInspection(EActorGroup::Player, EActorGroup::ProjectileEnemy);
		Collision->RequestInspection(EActorGroup::Monster, EActorGroup::ProjectilePlayer);
	}
}

bool InitializeProject()
{
	LevelEngineSystem* LevelManager = LevelEngineSystem::GetInstance();
	
	if (nullptr == LevelManager)
		return false;

	ULevel* MainLevel = LevelManager->CreateLevel(TEXT("MainLevel"), [](ULevel& _Level)
		{
			ConfigureCollisionChannels();
			
			GProjectPlayer = _Level.SpawnActor<APlayer>(EActorGroup::Player);

			return nullptr != GProjectPlayer;
		});

	if (nullptr == MainLevel)
		return false;

	return LevelManager->OpenLevel(TEXT("MainLevel"));
}

void DeinitializeProject()
{
	GProjectPlayer = nullptr;
}

APlayer* GetProjectPlayer()
{
	return GProjectPlayer;
}






//#include "pch.h"
//#include "ProjectName.h"
//
//#include "Player.h"
//#include "Enemy.h"
//#include "StageBoss.h"
//#include "EnemySpawner.h"
//#include "FormationSpawner.h"
//#include "UE/LevelEngineSystem.h"
//#include "UE/CollisionEngineSystem.h"
//
//#include <random>
//
//namespace
//{
//	APlayer* GProjectPlayer = nullptr;
//
//	void ConfigureCollisionChannels()
//	{
//		auto* Collision = CollisionEngineSystem::GetInstance();
//		Collision->InspectionOff();
//		Collision->RequestInspection(EActorGroup::Player, EActorGroup::Monster);
//		Collision->RequestInspection(EActorGroup::Player, EActorGroup::ProjectileEnemy);
//		Collision->RequestInspection(EActorGroup::Monster, EActorGroup::ProjectilePlayer);
//	}
//}
//
//bool InitializeProject()
//{
//	LevelEngineSystem* LevelManager = LevelEngineSystem::GetInstance();
//	if (nullptr == LevelManager)
//		return false;
//
//	ULevel* MainLevel = LevelManager->CreateLevel(
//		TEXT("MainLevel"),
//		[](ULevel& _Level)
//		{
//			ConfigureCollisionChannels();
//			GProjectPlayer = _Level.SpawnActor<APlayer>(EActorGroup::Player);
//			if (nullptr != GProjectPlayer)
//				GProjectPlayer->__Pos = { 480.0f, 540.0f };
//
//			if (GProjectPlayer != nullptr)
//			{
//				AStageBoss* Boss = _Level.SpawnActor<AStageBoss>(EActorGroup::Monster, GProjectPlayer->__Scale);
//				Boss->__Pos = { 960.0, 450.0 };
//			}
//
//			for (int EnemyIndex = 0; EnemyIndex < 3; ++EnemyIndex)
//			{
//				AEnemy* Enemy = _Level.SpawnActor<AEnemy>(EActorGroup::Monster);
//				Enemy->__Pos = { 480.0f + 480.0f * EnemyIndex, 270.0f };
//			}
//
//			return nullptr != GProjectPlayer;
//		});
//
//	if (nullptr == MainLevel)
//		return false;
//
//	ULevel* SecondLevel = LevelManager->CreateLevel(
//		TEXT("SecondLevel"),
//		[](ULevel& _Level)
//		{
//			ConfigureCollisionChannels();
//			GProjectPlayer = _Level.SpawnActor<APlayer>(EActorGroup::Player);
//			if (nullptr != GProjectPlayer)
//				GProjectPlayer->__Pos = { 1440.0f, 540.0f };
//
//			if (GProjectPlayer != nullptr)
//			{
//				AStageBoss* Boss = _Level.SpawnActor<AStageBoss>(EActorGroup::Monster, GProjectPlayer->__Scale);
//				Boss->__Pos = { 960.0, 450.0 };
//				Boss->StartRandomMovement();
//			}
//
//			_Level.SpawnActor<AEnemySpawner>(EActorGroup::Default);
//
//			return nullptr != GProjectPlayer;
//		});
//
//	if (nullptr == SecondLevel)
//		return false;
//
//	ULevel* Level3 = LevelManager->CreateLevel(
//		TEXT("Level3"),
//		[](ULevel& _Level)
//		{
//			ConfigureCollisionChannels();
//			GProjectPlayer = _Level.SpawnActor<APlayer>(EActorGroup::Player);
//			if (GProjectPlayer != nullptr)
//				GProjectPlayer->__Pos = { 960.0f, 850.0f };
//
//			_Level.SpawnActor<AFormationSpawner>(EActorGroup::Default);
//			return GProjectPlayer != nullptr;
//		});
//	if (Level3 == nullptr)
//		return false;
//
//	return LevelManager->OpenLevel(TEXT("MainLevel"));
//}
//
//void DeinitializeProject()
//{
//	GProjectPlayer = nullptr;
//}
//
//APlayer* GetProjectPlayer()
//{
//	return GProjectPlayer;
//}