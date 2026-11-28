#pragma once

#include "CombatCharacter.h"

class AEnemy : public ACombatCharacter
{
	friend class ULevel;

private:
	AEnemy();

public:
	~AEnemy() override = default;
	void ConfigureFormation(int _Pattern, int _Slot, const FVector2D& _Origin);
	bool HasEnteredFormation() const { return __FormationEntered; }

protected:
	AEnemy(const TCHAR* _MaterialPath, const FVector2D& _Scale);

	void Tick(float _DeltaTime) override;
	void Render() override;
	bool UpdateAttack(float _DeltaTime);
	void UpdateFormation(float _DeltaTime);
	void TurnTowards(const FVector2D& _Direction, float _DeltaTime);

private:
	enum class EMovementPattern
	{
		SpeedUpAndDown,
		StopAndGo,
	};
	enum class EChargePattern
	{
		Instant,
		Medium,
		Full,
	};

	static constexpr double __TurnResponse = 6.0;
	static constexpr double __AimTolerance = 3.0;
	static constexpr double __FireWarningDuration = 0.3;
	FVector2D __CenterPosition = {};
	double __MoveDistance = 50.0;
	double __MoveSpeed = 100.0;
	double __CurrentSpeed = 100.0;
	double __Acceleration = 600.0;
	double __MoveDirection = 1.0;
	double __PauseRemaining = 0.0;
	EMovementPattern __MovementPattern = EMovementPattern::SpeedUpAndDown;
	bool __MovementInitialized = false;
	float __ShotCooldown = 0.0f;
	float __ChargeDuration = 1.0f;
	float __ChargeScale = 1.0f;
	bool __bFireWarning = false;
	double __FireWarningTime = 0.0;
	EChargePattern __ChargePattern = EChargePattern::Medium;
	bool __FormationEntered = false;
	int __FormationPattern = -1;
	int __FormationSlot = 0;
	FVector2D __FormationOrigin = {};
	float __FormationTime = 0.0f;
	COLORREF __FormationColor = RGB(170, 45, 75);
};
