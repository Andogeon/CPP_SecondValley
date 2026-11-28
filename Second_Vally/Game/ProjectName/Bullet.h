#pragma once

#include "UE/Actor.h"

#include <random>
#include <vector>

class ABullet final : public AActor
{
	friend class ULevel;

private:
	ABullet();

public:
	~ABullet() override = default;

	void SetDir(const FVector2D& _Direction);
	void SetCharging(bool _Charging);
	void SetScaleMultiplier(float _Multiplier);
	void SetEnemyBullet(bool _EnemyBullet);
	void OnCollisionEnter(UCollider* _Other) override;
	void Render() override;

protected:
	void Tick(float _DeltaTime) override;

private:
	struct FFireParticle
	{
		FVector2D __WorldPosition;
		FVector2D __Velocity;
		FVector2D __Acceleration;
		float __Age = 0.0f;
		float __Lifetime = 0.0f;
		float __Radius = 0.0f;
	};

	void UpdateFire(float _DeltaTime);
	void SpawnFireParticle();
	float RandomRange(float _Minimum, float _Maximum);

	std::vector<FFireParticle> __Particles;
	std::mt19937 __Random{ std::random_device{}() };
	float __EmissionRemainder = 0.0f;
	float __EffectTime = 0.0f;
	static constexpr size_t __MaxParticles = 128;

	FVector2D __Direction = { 0.0, -1.0 };
	float __MoveSpeed = 500.0f;
	bool __bCharging = false;
	bool __bEnemyBullet = false;
	bool __bHit = false;
};
