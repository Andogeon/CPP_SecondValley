#include "pch.h"
#include "Collider.h"
#include "Actor.h"

#include <atomic>
#include <cmath>
#include <numbers>

namespace
{
	std::atomic<uint64_t> GNextColliderID{ 1 };
}

UCollider::UCollider()
	: __ID(GNextColliderID.fetch_add(1, std::memory_order_relaxed))
{
	bCanEverTick = false;
}

AActor* UCollider::GetActor() const
{
	return dynamic_cast<AActor*>(GetOwner());
}

FVector2D UCollider::GetPos() const
{
	const AActor* Actor = GetActor();
	if (Actor == nullptr)
		return __Offset;

	const double Radian = std::remainder(Actor->__Rotation, 360.0) * std::numbers::pi / 180.0;
	const double Cos = std::cos(Radian);
	const double Sin = std::sin(Radian);
	return Actor->__Pos + FVector2D(Cos * __Offset.X - Sin * __Offset.Y,
		Sin * __Offset.X + Cos * __Offset.Y);
}

FVector2D UCollider::GetScale() const
{
	const AActor* Actor = GetActor();
	return __bUseActorScale && Actor != nullptr ? Actor->__Scale : __Scale;
}

double UCollider::GetRotation() const
{
	const AActor* Actor = GetActor();
	return (Actor != nullptr ? Actor->__Rotation : 0.0) + __RotationOffset;
}

std::array<FVector2D, 2> UCollider::GetAxes() const
{
	const double Radian = std::remainder(GetRotation(), 360.0) * std::numbers::pi / 180.0;
	const double Cos = std::cos(Radian);
	const double Sin = std::sin(Radian);
	return { FVector2D(Cos, Sin), FVector2D(-Sin, Cos) };
}

std::array<FVector2D, 4> UCollider::GetCorners() const
{
	const FVector2D Center = GetPos();
	const FVector2D Scale = GetScale();
	const auto Axes = GetAxes();
	const FVector2D X = Axes[0] * (Scale.X * 0.5);
	const FVector2D Y = Axes[1] * (Scale.Y * 0.5);
	return { Center - X - Y, Center + X - Y, Center + X + Y, Center - X + Y };
}

bool UCollider::IsEnabled() const
{
	const AActor* Actor = GetActor();
	return __bEnabled && Actor != nullptr && Actor->IsEnabled();
}
