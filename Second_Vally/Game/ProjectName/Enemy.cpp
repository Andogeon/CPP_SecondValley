#include "pch.h"
#include "UE/CameraEngineSystem.h"
#include "Enemy.h"
#include "UE/Collider.h"
#include "Bullet.h"
#include "Player.h"
#include "ProjectName.h"

#include "UE/Level.h"
#include "UE/LevelEngineSystem.h"
#include "UE/ResourceEngineSystem.h"
#include "UE/Math/Rotator.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>

namespace
{
	std::mt19937 GEnemyRandomEngine{ std::random_device{}() };
	std::uniform_int_distribution<int> GEnemyMoveSpeed{ 70, 219 };
	std::uniform_int_distribution<int> GEnemyAcceleration{ 500, 800 };
	std::bernoulli_distribution GEnemyPattern{ 0.5 };
	std::uniform_real_distribution<double> GEnemyShortPause{ 0.08, 0.18 };
	std::uniform_real_distribution<double> GEnemyLongPause{ 0.25, 0.45 };
	std::uniform_real_distribution<float> GEnemyShotCooldown{ 0.8f, 2.2f };
	std::uniform_real_distribution<float> GEnemyMediumCharge{ 0.55f, 0.9f };
	std::uniform_real_distribution<float> GEnemyFullCharge{ 1.2f, 1.8f };
	std::bernoulli_distribution GEnemyShotChance{ 0.35 };
	std::uniform_int_distribution<int> GEnemyChargePattern{ 0, 99 };
}

AEnemy::AEnemy()
	: AEnemy(TEXT("Materials/Enemy"), { 50.0, 50.0 })
{
	CreateDefaultSubobject<UCollider>()->SetScale({ __Scale.X * 0.5, __Scale.Y * 0.75 });
	__MoveDistance = 50.0;
	__MoveSpeed = static_cast<double>(GEnemyMoveSpeed(GEnemyRandomEngine));
	__Acceleration = static_cast<double>(GEnemyAcceleration(GEnemyRandomEngine));
	__MoveDirection = 1.0;
	__MovementPattern = GEnemyPattern(GEnemyRandomEngine)
		? EMovementPattern::SpeedUpAndDown
		: EMovementPattern::StopAndGo;
	__CurrentSpeed = 0.0;
	__ShotCooldown = GEnemyShotCooldown(GEnemyRandomEngine);
	const int ChargeRoll = GEnemyChargePattern(GEnemyRandomEngine);
	if (ChargeRoll < 30)
	{
		__ChargePattern = EChargePattern::Instant;
		__ChargeDuration = 0.05f;
		__ChargeScale = 1.2f;
	}
	else if (ChargeRoll < 70)
	{
		__ChargePattern = EChargePattern::Medium;
		__ChargeDuration = GEnemyMediumCharge(GEnemyRandomEngine);
		__ChargeScale = 2.5f;
	}
	else
	{
		__ChargePattern = EChargePattern::Full;
		__ChargeDuration = GEnemyFullCharge(GEnemyRandomEngine);
		__ChargeScale = 5.0f;
	}
}

AEnemy::AEnemy(const TCHAR* _MaterialPath, const FVector2D& _Scale)
{
	SetMaterial(ResourceEngineSystem::GetInstance()->LoadMaterial(_MaterialPath));
	__Scale = _Scale;
}

void AEnemy::Tick(float _DeltaTime)
{
	if ((__FormationPattern < 0 || __FormationEntered) && UpdateAttack(_DeltaTime))
		return;
	// 조준·충전 중이거나 방금 발사한 프레임에는 이동과 편대 경로 시간을 갱신하지 않는다.

	const FVector2D PreviousPosition = __Pos;
	if (__FormationPattern >= 0)
	{
		UpdateFormation(_DeltaTime);
		TurnTowards(__Pos - PreviousPosition, _DeltaTime);
		return;
	}
	if (!__MovementInitialized)
	{
		__CenterPosition = __Pos;
		__MovementInitialized = true;
	}

	if (__PauseRemaining > 0.0)
	{
		__PauseRemaining -= _DeltaTime;
		return;
	}

	const double MinX = __CenterPosition.X - __MoveDistance;
	const double MaxX = __CenterPosition.X + __MoveDistance;
	const double RemainingDistance = __MoveDirection > 0.0
		? MaxX - __Pos.X
		: __Pos.X - MinX;
	const double StoppingDistance = (__CurrentSpeed * __CurrentSpeed)
		/ (2.0 * __Acceleration);

	if (RemainingDistance <= StoppingDistance)
	{
		__CurrentSpeed -= __Acceleration * _DeltaTime;
	}
	else
		__CurrentSpeed += __Acceleration * _DeltaTime;

	__CurrentSpeed = std::clamp(__CurrentSpeed, 0.0, __MoveSpeed);

	__Pos.X += __CurrentSpeed * __MoveDirection * _DeltaTime;

	if (__Pos.X < MinX)
	{
		__Pos.X = MinX;
		__CurrentSpeed = 0.0;
		__MoveDirection = 1.0;
		__PauseRemaining = __MovementPattern == EMovementPattern::SpeedUpAndDown
			? GEnemyShortPause(GEnemyRandomEngine)
			: GEnemyLongPause(GEnemyRandomEngine);
	}
	else if (__Pos.X > MaxX)
	{
		__Pos.X = MaxX;
		__CurrentSpeed = 0.0;
		__MoveDirection = -1.0;
		__PauseRemaining = __MovementPattern == EMovementPattern::SpeedUpAndDown
			? GEnemyShortPause(GEnemyRandomEngine)
			: GEnemyLongPause(GEnemyRandomEngine);
	}
	TurnTowards(__Pos - PreviousPosition, _DeltaTime);
}

void AEnemy::TurnTowards(const FVector2D& _Direction, float _DeltaTime)
{
	if (!std::isfinite(_DeltaTime) || _DeltaTime <= 0.0f
		|| !std::isfinite(_Direction.X) || !std::isfinite(_Direction.Y)
		|| _Direction.SizeSquared() < 0.000001)
		return;

	const double TargetRotation = std::atan2(_Direction.Y, _Direction.X) * 180.0 / std::numbers::pi - 90.0;
	const double Difference = std::remainder(TargetRotation - __Rotation, 360.0);
	const double Alpha = std::clamp(__TurnResponse * _DeltaTime, 0.0, 1.0);
	__Rotation = std::fmod(__Rotation + Difference * Alpha, 360.0);
	if (__Rotation < 0.0)
		__Rotation += 360.0;
}

void AEnemy::ConfigureFormation(int _Pattern, int _Slot, const FVector2D& _Origin)
{
	__FormationPattern = _Pattern;
	__FormationSlot = _Slot;
	__FormationOrigin = _Origin;
	__FormationTime = 0.0f;
	__FormationEntered = false;
	__MoveDistance = 0.0;
	__Scale = { 42.0, 42.0 };
	GetComponent<UCollider>()->SetScale({ __Scale.X * 0.5, __Scale.Y * 0.75 });
	if (_Pattern == 0)
		__FormationColor = RGB(220, 80, 55);
	else if (_Pattern == 1)
		__FormationColor = RGB(55, 150, 225);
	else
		__FormationColor = RGB(170, 75, 210);
}

void AEnemy::UpdateFormation(float _DeltaTime)
{
	__FormationTime += _DeltaTime;
	const double Slot = static_cast<double>(__FormationSlot);
	const double Phase = __FormationTime * 1.8 + Slot * 0.42;
	if (__FormationPattern == 0)
	{
		// Sequential train entry followed by a sweeping air-show arc.
		__Pos.X = __FormationOrigin.X + std::sin(Phase) * 330.0;
		const double EntryY = (std::min)(450.0, static_cast<double>(__FormationTime) * 170.0);
		__Pos.Y = __FormationOrigin.Y + EntryY + Slot * 48.0 + std::cos(Phase * 0.5) * 90.0;
	}
	else if (__FormationPattern == 1)
	{
		// Five planes orbit the center from evenly spaced directions.
		const double OrbitPhase = Phase + Slot * (6.283185307179586 / 10.0);
		const double EntryRadius = (std::max)(260.0, 1300.0 - __FormationTime * 220.0);
		const double Radius = EntryRadius + std::sin(__FormationTime * 2.0) * 55.0;
		__Pos.X = __FormationOrigin.X + std::cos(OrbitPhase) * Radius;
		__Pos.Y = __FormationOrigin.Y + std::sin(OrbitPhase) * Radius * 0.55;
	}
	else
	{
		// V-shaped formation that banks across the screen.
		const double Side = Slot < 5.0 ? -1.0 : 1.0;
		const double Rank = static_cast<int>(Slot) % 5;
		const double EntryX = (std::min)(1380.0, static_cast<double>(__FormationTime) * 400.0);
		__Pos.X = __FormationOrigin.X + EntryX + std::sin(Phase * 0.7) * 220.0 + Side * Rank * 38.0;
		__Pos.Y = 500.0 + std::cos(Phase * 0.7) * 120.0 + Rank * 34.0;
	}

	const bool bInsideScreen =
		__Pos.X >= -__Scale.X && __Pos.X <= 1920.0 + __Scale.X &&
		__Pos.Y >= -__Scale.Y && __Pos.Y <= 1080.0 + __Scale.Y;
	if (__FormationTime >= 3.5f && bInsideScreen)
		__FormationEntered = true;
}

void AEnemy::Render()
{
	HDC _DC = GetRenderTarget();
	if (_DC == nullptr)
		return;

	const int SavedDC = SaveDC(_DC);
	if (SavedDC == 0)
		return;

	if (__bCharging)
	{
		const double FacingRadians = (__Rotation + 90.0) * std::numbers::pi / 180.0;
		const FVector2D Direction(std::cos(FacingRadians), std::sin(FacingRadians));
		const FVector2D Start = CameraEngineSystem::GetInstance()->GetRenderPos(__Pos + Direction * (__Scale.Y * 0.5));
		const FVector2D End = Start + Direction * 2500.0;
		SelectObject(_DC, GetStockObject(DC_PEN));
		SetDCPenColor(_DC, __bFireWarning ? RGB(255, 0, 0) : RGB(0, 255, 0));
		// 조준 중에는 초록색, 발사 직전 0.3초 동안은 빨간색으로 표시한다.

		MoveToEx(_DC, static_cast<int>(Start.X), static_cast<int>(Start.Y), nullptr);
		LineTo(_DC, static_cast<int>(End.X), static_cast<int>(End.Y));
	}

	SelectObject(_DC, GetStockObject(DC_BRUSH));
	SelectObject(_DC, GetStockObject(NULL_PEN));
	SetDCBrushColor(_DC, __FormationColor);
	const FVector2D RenderPosition = CameraEngineSystem::GetInstance()->GetRenderPos(__Pos);
	if (!RenderMaterial()) Rectangle(_DC,
		static_cast<int>(RenderPosition.X - __Scale.X * 0.5),
		static_cast<int>(RenderPosition.Y - __Scale.Y * 0.5),
		static_cast<int>(RenderPosition.X + __Scale.X * 0.5),
		static_cast<int>(RenderPosition.Y + __Scale.Y * 0.5));

	RestoreDC(_DC, SavedDC);
}

bool AEnemy::UpdateAttack(float _DeltaTime)
{
	if (!__bCharging)
	{
		__ShotCooldown -= _DeltaTime;
		if (__ShotCooldown > 0.0f)
			return false;

		__ShotCooldown = GEnemyShotCooldown(GEnemyRandomEngine);
		if (!GEnemyShotChance(GEnemyRandomEngine))
			return false;

		if (!BeginChargingBullet(true))
			return false;
		__bCharging = true;
		__ChargeTime = 0.0f;
		__bFireWarning = false;
		__FireWarningTime = 0.0;

		const int ChargeRoll = GEnemyChargePattern(GEnemyRandomEngine);
		if (ChargeRoll < 30)
		{
			__ChargePattern = EChargePattern::Instant;
			__ChargeDuration = 0.05f;
			__ChargeScale = 1.2f;
		}
		else if (ChargeRoll < 70)
		{
			__ChargePattern = EChargePattern::Medium;
			__ChargeDuration = GEnemyMediumCharge(GEnemyRandomEngine);
			__ChargeScale = 2.5f;
		}
		else
		{
			__ChargePattern = EChargePattern::Full;
			__ChargeDuration = GEnemyFullCharge(GEnemyRandomEngine);
			__ChargeScale = 5.0f;
		}
	}

	__ChargeTime += _DeltaTime;
	const float ChargeRatio = std::clamp(__ChargeTime / __ChargeDuration, 0.0f, 1.0f);
	const APlayer* Player = GetProjectPlayer();
	FVector2D Direction = Player != nullptr ? Player->__Pos - __Pos : FVector2D(0.0, 1.0);
	if (!Direction.Normalize())
		Direction = { 0.0, 1.0 };

	const double TargetRotation = std::atan2(Direction.Y, Direction.X) * 180.0 / std::numbers::pi - 90.0;
	if (std::isfinite(_DeltaTime) && _DeltaTime > 0.0f)
	{
		const FRotator CurrentRotation(0.0, __Rotation, 0.0);
		const FRotator DesiredRotation(0.0, TargetRotation, 0.0);
		const double Alpha = std::clamp(__TurnResponse * _DeltaTime, 0.0, 1.0);
		__Rotation = FRotator::ClampAxis(FMath::Lerp(CurrentRotation, DesiredRotation, Alpha).Yaw);
		// 조준 각도를 최단 회전 방향으로 보간한다. 0°와 360° 경계도 짧은 쪽으로 회전한다.
	}

	const double FacingRadians = (__Rotation + 90.0) * std::numbers::pi / 180.0;
	const FVector2D FacingDirection(std::cos(FacingRadians), std::sin(FacingRadians));
	ACombatCharacter::UpdateChargingBullet(
		FacingDirection, 1.0f + (__ChargeScale - 1.0f) * ChargeRatio);
	// 회전하는 동안 충전 중인 탄환도 기체 앞쪽을 따라간다.

	const double AimDifference = std::abs(std::remainder(TargetRotation - __Rotation, 360.0));
	if (__bFireWarning)
	{
		__FireWarningTime += _DeltaTime;
	}
	else if (__ChargeTime >= __ChargeDuration && AimDifference <= __AimTolerance)
	{
		__bFireWarning = true;
		__FireWarningTime = 0.0;
		// 충전과 조준을 마친 시점부터 빨간 조준선을 0.3초 표시한 뒤 발사한다.
	}

	if (__bFireWarning && __FireWarningTime >= __FireWarningDuration)
	{
		ACombatCharacter::UpdateChargingBullet(FacingDirection, __ChargeScale);
		// 발사 순간에도 보간된 기체 방향을 유지하고 조준선과 같은 방향으로 발사한다.

		ReleaseChargingBullet();
		__bCharging = false;
		__ChargeTime = 0.0f;
		__bFireWarning = false;
		__FireWarningTime = 0.0;
	}

	return true;
	// 발사한 프레임까지 정지하고, 다음 프레임부터 이동을 재개한다.
}
