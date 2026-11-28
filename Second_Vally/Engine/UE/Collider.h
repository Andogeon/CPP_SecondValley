#pragma once

#include "ActorComponent.h"
#include "Math/Vector2D.h"
#include <array>
#include <cstdint>

class AActor;

class ENGINE_API UCollider : public UActorComponent
{
	friend class CollisionEngineSystem;

public:
	UCollider();
	UCollider(const UCollider&) = delete;
	UCollider& operator=(const UCollider&) = delete;
	UCollider(UCollider&&) = delete;
	UCollider& operator=(UCollider&&) = delete;

	uint64_t GetID() const { return __ID; }
	AActor* GetActor() const;
	FVector2D GetPos() const;
	FVector2D GetScale() const;
	double GetRotation() const;
	double GetRotationOffset() const { return __RotationOffset; }
	void SetRotationOffset(double _Rotation) { __RotationOffset = _Rotation; }
	std::array<FVector2D, 2> GetAxes() const;
	std::array<FVector2D, 4> GetCorners() const;
	void SetOffset(const FVector2D& _Offset) { __Offset = _Offset; }
	void SetScale(const FVector2D& _Scale) { __Scale = _Scale; __bUseActorScale = false; }
	void SetUseActorScale(bool _Enabled) { __bUseActorScale = _Enabled; }

	void SetEnabled(bool _Enabled) { __bEnabled = _Enabled; }
	bool IsEnabled() const;

	size_t GetCollisionCount() const { return __CollisionCount; }
	bool IsColliding() const { return __CollisionCount != 0; }

private:
	uint64_t __ID;
	FVector2D __Offset = { 0.0, 0.0 };
	FVector2D __Scale = { 100.0, 100.0 };
	double __RotationOffset = 0.0;
	size_t __CollisionCount = 0;
	bool __bEnabled = true;
	bool __bUseActorScale = true;
};
