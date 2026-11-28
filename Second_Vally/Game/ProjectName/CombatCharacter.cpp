#include "pch.h"
#include "CombatCharacter.h"

#include "Bullet.h"
#include "UE/Level.h"
#include "UE/LevelEngineSystem.h"

void ACombatCharacter::EndPlay()
{
	CancelChargingBullet();
	ACharacter::EndPlay();
}

bool ACombatCharacter::BeginChargingBullet(bool _EnemyBullet)
{
	if (__ChargingBullet != nullptr)
		return true;

	ULevel* Level = LevelEngineSystem::GetInstance()->GetCurrentLevel();
	if (Level == nullptr)
		return false;

	__ChargingBullet = Level->SpawnActor<ABullet>(
		_EnemyBullet ? EActorGroup::ProjectileEnemy : EActorGroup::ProjectilePlayer);
	if (__ChargingBullet == nullptr)
		return false;

	__ChargingBullet->SetEnemyBullet(_EnemyBullet);
	__ChargingBullet->SetCharging(true);
	return true;
}

void ACombatCharacter::UpdateChargingBullet(
	const FVector2D& _Direction, float _ScaleMultiplier)
{
	if (__ChargingBullet == nullptr)
		return;

	__ChargingBullet->SetScaleMultiplier(_ScaleMultiplier);
	__ChargingBullet->__Pos = __Pos +
		_Direction * ((__Scale.X + __ChargingBullet->__Scale.X) * 0.5);
	__ChargingBullet->SetDir(_Direction);
}

void ACombatCharacter::ReleaseChargingBullet()
{
	if (__ChargingBullet == nullptr)
		return;

	__ChargingBullet->SetCharging(false);
	__ChargingBullet = nullptr;
}

void ACombatCharacter::CancelChargingBullet()
{
	if (__ChargingBullet != nullptr)
	{
		if (ULevel* Level = LevelEngineSystem::GetInstance()->GetCurrentLevel())
			Level->DestroyActor(__ChargingBullet);
		__ChargingBullet = nullptr;
	}

	__bCharging = false;
	__ChargeTime = 0.0f;
}
