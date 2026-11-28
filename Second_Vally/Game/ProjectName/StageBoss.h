#pragma once

#include "Enemy.h"

class UCollider;

class AStageBoss final : public AEnemy
{
	friend class ULevel;

private:
	explicit AStageBoss(const FVector2D& _PlayerScale);

public:
	void StartRandomMovement();
	void StartEntrance(const FVector2D& _Start, const FVector2D& _Target);

	UCollider* GetLeftWingCollider() const { return __LeftWingCollider; }
	UCollider* GetBodyCollider() const { return __BodyCollider; }
	UCollider* GetRightWingCollider() const { return __RightWingCollider; }

protected:
	void Tick(float _DeltaTime) override;

private:
	enum class EMovementMode
	{
		Stationary,
		Entering,
		Random,
	};

	void ChooseRandomDirection();

	UCollider* __LeftWingCollider = nullptr;
	UCollider* __BodyCollider = nullptr;
	UCollider* __RightWingCollider = nullptr;
	EMovementMode __MovementMode = EMovementMode::Stationary;
	FVector2D __MoveDirection = {};
	FVector2D __EntryTarget = {};
	double __DirectionRemaining = 0.0;
	double __MoveSpeed = 140.0;
	double __EntrySpeed = 80.0;
};
