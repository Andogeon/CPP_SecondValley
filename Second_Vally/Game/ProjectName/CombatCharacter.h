#pragma once

#include "UE/Character.h"

class ABullet;

class ACombatCharacter : public ACharacter
{
	friend class ULevel;

protected:
	ACombatCharacter() = default;

public:
	~ACombatCharacter() override = default;

protected:
	void EndPlay() override;

	bool BeginChargingBullet(bool _EnemyBullet);
	void UpdateChargingBullet(const FVector2D& _Direction, float _ScaleMultiplier);
	void ReleaseChargingBullet();
	void CancelChargingBullet();

	ABullet* GetChargingBullet() const { return __ChargingBullet; }

	float __ChargeTime = 0.0f;
	bool __bCharging = false;
	ABullet* __ChargingBullet = nullptr;
};
