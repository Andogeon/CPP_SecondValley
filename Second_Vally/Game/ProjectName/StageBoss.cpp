#include "pch.h"
#include "StageBoss.h"
#include "UE/Collider.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

namespace
{
	std::mt19937 GStageBossRandom{ std::random_device{}() };
	std::uniform_real_distribution<double> GStageBossDirectionDuration{ 0.8, 2.0 };
}

AStageBoss::AStageBoss(const FVector2D& _PlayerScale)
	: AEnemy(TEXT("Materials/StageBoss"), _PlayerScale * 3.0)
{
	bCanEverTick = false;
	__LeftWingCollider = CreateDefaultSubobject<UCollider>();
	__LeftWingCollider->SetScale({ __Scale.X * 8.0 / 32.0, __Scale.Y * 9.0 / 48.0 });
	__LeftWingCollider->SetOffset({ -__Scale.X * 7.0 / 32.0, -__Scale.Y * 0.5 / 48.0 });

	__BodyCollider = CreateDefaultSubobject<UCollider>();
	__BodyCollider->SetScale({ __Scale.X * 6.0 / 32.0, __Scale.Y * 31.0 / 48.0 });
	__BodyCollider->SetOffset({ 0.0, __Scale.Y * 0.5 / 48.0 });

	__RightWingCollider = CreateDefaultSubobject<UCollider>();
	__RightWingCollider->SetScale({ __Scale.X * 8.0 / 32.0, __Scale.Y * 9.0 / 48.0 });
	__RightWingCollider->SetOffset({ __Scale.X * 7.0 / 32.0, -__Scale.Y * 0.5 / 48.0 });
}

void AStageBoss::StartRandomMovement()
{
	bCanEverTick = true;
	__MovementMode = EMovementMode::Random;
	ChooseRandomDirection();
}

void AStageBoss::StartEntrance(const FVector2D& _Start, const FVector2D& _Target)
{
	bCanEverTick = true;
	__Pos = _Start;
	__EntryTarget = _Target;
	__MovementMode = EMovementMode::Entering;
}

void AStageBoss::ChooseRandomDirection()
{
	std::array<FVector2D, 4> Directions;
	size_t Count = 0;
	const double HalfWidth = __Scale.X * 0.5;
	const double HalfHeight = __Scale.Y * 0.5;
	if (__Pos.X > HalfWidth) Directions[Count++] = { -1.0, 0.0 };
	if (__Pos.X < 1920.0 - HalfWidth) Directions[Count++] = { 1.0, 0.0 };
	if (__Pos.Y > HalfHeight) Directions[Count++] = { 0.0, -1.0 };
	if (__Pos.Y < 1080.0 - HalfHeight) Directions[Count++] = { 0.0, 1.0 };
	if (Count == 0)
	{
		__MoveDirection = {};
		return;
	}
	std::uniform_int_distribution<size_t> PickDirection{ 0, Count - 1 };
	__MoveDirection = Directions[PickDirection(GStageBossRandom)];
	__DirectionRemaining = GStageBossDirectionDuration(GStageBossRandom);
}

void AStageBoss::Tick(float _DeltaTime)
{
	if (!std::isfinite(_DeltaTime) || _DeltaTime <= 0.0f)
		return;

	const FVector2D PreviousPosition = __Pos;
	if (__MovementMode == EMovementMode::Entering)
	{
		const FVector2D Delta = __EntryTarget - __Pos;
		const double Distance = std::hypot(Delta.X, Delta.Y);
		const double Step = __EntrySpeed * _DeltaTime;
		if (Distance <= Step)
		{
			__Pos = __EntryTarget;
			StartRandomMovement();
		}
		else
			__Pos += Delta * (Step / Distance);
		TurnTowards(__Pos - PreviousPosition, _DeltaTime);
		return;
	}

	if (__MovementMode != EMovementMode::Random)
		return;

	__DirectionRemaining -= _DeltaTime;
	if (__DirectionRemaining <= 0.0)
		ChooseRandomDirection();

	const FVector2D Next = __Pos + __MoveDirection * (__MoveSpeed * _DeltaTime);
	__Pos.X = std::clamp(Next.X, __Scale.X * 0.5, 1920.0 - __Scale.X * 0.5);
	__Pos.Y = std::clamp(Next.Y, __Scale.Y * 0.5, 1080.0 - __Scale.Y * 0.5);
	if (__Pos.X != Next.X || __Pos.Y != Next.Y)
		__DirectionRemaining = 0.0;
	TurnTowards(__Pos - PreviousPosition, _DeltaTime);
}
