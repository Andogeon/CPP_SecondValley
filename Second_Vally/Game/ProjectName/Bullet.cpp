#include "pch.h"
#include "UE/CameraEngineSystem.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Player.h"
#include "UE/Collider.h"
#include "UE/Level.h"
#include "UE/LevelEngineSystem.h"

#include <algorithm>
#include <cmath>

namespace
{
	void DrawFlameCircle(HDC _DC, double _X, double _Y, double _Radius, COLORREF _Color)
	{
		const FVector2D RenderPosition = CameraEngineSystem::GetInstance()->GetRenderPos(FVector2D(_X, _Y));
		SetDCBrushColor(_DC, _Color);
		Ellipse(_DC,
			static_cast<int>(RenderPosition.X - _Radius), static_cast<int>(RenderPosition.Y - _Radius),
			static_cast<int>(RenderPosition.X + _Radius), static_cast<int>(RenderPosition.Y + _Radius));
	}
}

ABullet::ABullet()
{
	CreateDefaultSubobject<UCollider>();
	__Scale = { 6.0, 6.0 };
	__Particles.reserve(__MaxParticles);
}

void ABullet::SetDir(const FVector2D& _Direction)
{
	FVector2D Direction = _Direction;
	if (Direction.Normalize())
		__Direction = Direction;
}

void ABullet::Tick(float _DeltaTime)
{
	if (!__bCharging)
	{
		__Pos += __Direction * (__MoveSpeed * _DeltaTime);
	}

	UpdateFire(_DeltaTime);
}

void ABullet::SetCharging(bool _Charging)
{
	__bCharging = _Charging;
	GetComponent<UCollider>()->SetEnabled(!_Charging);
}

void ABullet::SetScaleMultiplier(float _Multiplier)
{
	__Scale = FVector2D(6.0, 6.0) * _Multiplier;
}

void ABullet::SetEnemyBullet(bool _EnemyBullet)
{
	__bEnemyBullet = _EnemyBullet;
}

void ABullet::OnCollisionEnter(UCollider* _Other)
{
	if (_Other == nullptr || __bCharging || __bHit || !IsEnabled())
		return;

	AActor* OtherActor = _Other->GetActor();
	const bool bTarget = __bEnemyBullet
		? dynamic_cast<APlayer*>(OtherActor) != nullptr
		: dynamic_cast<AEnemy*>(OtherActor) != nullptr;
	if (!bTarget)
		return;

	ULevel* Level = LevelEngineSystem::GetInstance()->GetCurrentLevel();
	if (Level == nullptr || !Level->DestroyActor(this))
		return;

	__bHit = true;
	if (!__bEnemyBullet)
		Level->DestroyActor(OtherActor);
}

float ABullet::RandomRange(float _Minimum, float _Maximum)
{
	return std::uniform_real_distribution<float>(_Minimum, _Maximum)(__Random);
}

void ABullet::SpawnFireParticle()
{
	const float Radius = static_cast<float>(__Scale.X * 0.5);
	const float Angle = RandomRange(0.0f, 6.2831853f);
	const FVector2D Direction(std::cos(Angle), std::sin(Angle));
	const float Speed = RandomRange(25.0f, 75.0f) + Radius;

	FFireParticle Particle;
	Particle.__WorldPosition = __Pos + Direction * RandomRange(0.54f * Radius, 1.02f * Radius);
	Particle.__Velocity = Direction * Speed;
	Particle.__Velocity.Y -= 55.0;
	Particle.__Acceleration = FVector2D(0.0, -90.0);
	Particle.__Lifetime = RandomRange(0.18f, 0.55f);
	Particle.__Radius = RandomRange(2.0f, 5.0f + Radius * 0.35f);
	__Particles.push_back(Particle);
}

void ABullet::UpdateFire(float _DeltaTime)
{
	const float DeltaTime = std::clamp(_DeltaTime, 0.0f, 0.1f);
	__EffectTime = std::fmod(__EffectTime + DeltaTime, 100.0f);
	for (FFireParticle& Particle : __Particles)
	{
		Particle.__Age += DeltaTime;
		Particle.__WorldPosition += Particle.__Velocity * DeltaTime;
		Particle.__Velocity += Particle.__Acceleration * DeltaTime;
	}
	std::erase_if(__Particles, [](const FFireParticle& _Particle)
	{
		return _Particle.__Age >= _Particle.__Lifetime;
	});

	__EmissionRemainder += (35.0f + static_cast<float>(__Scale.X) * 1.5f) * DeltaTime;
	const int SpawnCount = static_cast<int>(__EmissionRemainder);
	__EmissionRemainder -= SpawnCount;
	for (int Index = 0; Index < SpawnCount && __Particles.size() < __MaxParticles; ++Index)
		SpawnFireParticle();
}

void ABullet::Render()
{
	HDC _DC = GetRenderTarget();
	if (_DC == nullptr)
		return;
	const int SavedDC = SaveDC(_DC);
	if (SavedDC == 0)
		return;

	SelectObject(_DC, GetStockObject(DC_BRUSH));
	SelectObject(_DC, GetStockObject(NULL_PEN));

	for (const FFireParticle& Particle : __Particles)
	{
		const float LifeRatio = Particle.__Age / Particle.__Lifetime;
		const double Radius = Particle.__Radius * (1.0f - LifeRatio);
		const double X = Particle.__WorldPosition.X;
		const double Y = Particle.__WorldPosition.Y;
		if (__bEnemyBullet)
		{
			DrawFlameCircle(_DC, X, Y, Radius,
				RGB(70, static_cast<int>(90.0f * (1.0f - LifeRatio)), 255));
			DrawFlameCircle(_DC, X, Y, Radius * 0.55,
				RGB(180, 225, 255));
		}
		else
		{
			DrawFlameCircle(_DC, X, Y, Radius,
				RGB(255, static_cast<int>(170.0f * (1.0f - LifeRatio)), 15));
			DrawFlameCircle(_DC, X, Y, Radius * 0.55,
				RGB(255, static_cast<int>(230.0f - 100.0f * LifeRatio), 65));
		}
	}

	const double Flicker = 1.0 + 0.06 * std::sin(__EffectTime * 37.0)
		+ 0.03 * std::sin(__EffectTime * 71.0);
	const double Radius = __Scale.X * 0.5 * Flicker;
	if (__bEnemyBullet)
	{
		DrawFlameCircle(_DC, __Pos.X, __Pos.Y, Radius, RGB(25, 40, 180));
		DrawFlameCircle(_DC, __Pos.X, __Pos.Y, Radius * 0.78, RGB(65, 125, 255));
		DrawFlameCircle(_DC, __Pos.X, __Pos.Y, Radius * 0.53, RGB(175, 220, 255));
		DrawFlameCircle(_DC, __Pos.X, __Pos.Y, Radius * 0.28, RGB(240, 250, 255));
	}
	else
	{
		DrawFlameCircle(_DC, __Pos.X, __Pos.Y, Radius, RGB(255, 65, 10));
		DrawFlameCircle(_DC, __Pos.X, __Pos.Y, Radius * 0.78, RGB(255, 155, 20));
		DrawFlameCircle(_DC, __Pos.X, __Pos.Y, Radius * 0.53, RGB(255, 225, 85));
		DrawFlameCircle(_DC, __Pos.X, __Pos.Y, Radius * 0.28, RGB(255, 250, 215));
	}

	RestoreDC(_DC, SavedDC);
}
