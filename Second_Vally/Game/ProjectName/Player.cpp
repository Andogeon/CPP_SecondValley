#include "pch.h"
#include "Player.h"
#include "UE/Collider.h"
#include "UE/CameraEngineSystem.h"
#include "Bullet.h"

#include "UE/InputEngineSystem.h"
#include "UE/LevelEngineSystem.h"
#include "UE/Level.h"
#include "UE/LaunchEngineLoop.h"
#include "UE/define.h"
#include "UE/ResourceEngineSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
	double RayToActor(const FVector2D& _Origin, const FVector2D& _Direction, const AActor& _Actor)
	{
		const double MinX = _Actor.__Pos.X - _Actor.__Scale.X * 0.5;
		const double MaxX = _Actor.__Pos.X + _Actor.__Scale.X * 0.5;
		const double MinY = _Actor.__Pos.Y - _Actor.__Scale.Y * 0.5;
		const double MaxY = _Actor.__Pos.Y + _Actor.__Scale.Y * 0.5;
		double Enter = 0.0;
		double Exit = (std::numeric_limits<double>::max)();

		const auto Slab = [&](double _Origin, double _Direction, double _Minimum, double _Maximum)
		{
			if (std::abs(_Direction) < 0.000001)
				return _Origin >= _Minimum && _Origin <= _Maximum;
			const double Inverse = 1.0 / _Direction;
			double A = (_Minimum - _Origin) * Inverse;
			double B = (_Maximum - _Origin) * Inverse;
			if (A > B)
				std::swap(A, B);
			Enter = (std::max)(Enter, A);
			Exit = (std::min)(Exit, B);
			return Enter <= Exit;
		};

		if (!Slab(_Origin.X, _Direction.X, MinX, MaxX) ||
			!Slab(_Origin.Y, _Direction.Y, MinY, MaxY) || Exit <= 1.0)
			return (std::numeric_limits<double>::max)();
		return Enter > 1.0 ? Enter : Exit;
	}
}

APlayer::APlayer()
{
	__Pos = { 960.0f, 540.0f };
	__Scale = { 100.0f, 100.0f };
	CreateDefaultSubobject<UCollider>()->SetScale({ __Scale.X * 0.5, __Scale.Y * 0.75 });
    SetMaterial(ResourceEngineSystem::GetInstance()->LoadMaterial(TEXT("Materials/Player")));
}

void APlayer::BeginPlay()
{
	ACombatCharacter::BeginPlay();
	CameraEngineSystem::GetInstance()->SetObservedObject(this);
}

void APlayer::EndPlay()
{
	CameraEngineSystem* Camera = CameraEngineSystem::GetInstance();
	if (Camera->GetObservedObject() == this)
		Camera->SetObservedObject(nullptr);
	__bCameraGesture = false;
	ACombatCharacter::EndPlay();
}

void APlayer::Tick(float _DeltaTime)
{
	if (KEY_PRESSED(EKey::F8))
		OpenNextLevel();

	const float Horizontal =
		(KEY_REPEAT(EKey::Right) ? 1.0f : 0.0f) -
		(KEY_REPEAT(EKey::Left) ? 1.0f : 0.0f);

	const float Vertical =
		(KEY_REPEAT(EKey::Down) ? 1.0f : 0.0f) -
		(KEY_REPEAT(EKey::Up) ? 1.0f : 0.0f);

	__Pos.X += Horizontal * __MoveSpeed * static_cast<float>(_DeltaTime);
	__Pos.Y += Vertical * __MoveSpeed * static_cast<float>(_DeltaTime);

	UpdateBodyRotation(_DeltaTime);
	UpdateAim(_DeltaTime);
	UpdateChargeShot(_DeltaTime);
}

void APlayer::UpdateBodyRotation(float _DeltaTime)
{
	const double RotationInput =
		((KEY_PRESSED(EKey::S) || KEY_REPEAT(EKey::S)) ? 1.0 : 0.0) -
		((KEY_PRESSED(EKey::A) || KEY_REPEAT(EKey::A)) ? 1.0 : 0.0);
	__Rotation = std::fmod(__Rotation + RotationInput * __BodyRotateSpeed * _DeltaTime, 360.0);
	if (__Rotation < 0.0)
		__Rotation += 360.0;
}

void APlayer::UpdateAim(float _DeltaTime)
{
	const float AimInput =
		(KEY_REPEAT(EKey::Z) ? 1.0f : 0.0f) -
		(KEY_REPEAT(EKey::X) ? 1.0f : 0.0f);

	__AimTheta += AimInput * __AimRotateSpeed * _DeltaTime;
	if (__AimTheta >= 360.0f)
		__AimTheta -= 360.0f;
	if (__AimTheta < 0.0f)
		__AimTheta += 360.0f;
}

FVector2D APlayer::GetAimDirection() const
{
	constexpr float DegreesToRadians = 0.0174532925f;
	const float Radian = __AimTheta * DegreesToRadians;
	return FVector2D(std::cos(Radian), -std::sin(Radian));
}

void APlayer::UpdateChargeShot(float _DeltaTime)
{
	if (GetForegroundWindow() != FEngineLoop::GetInstance()->GetHandle())
	{
		CancelChargingBullet();
		__bCameraGesture = false;
		return;
	}

	const bool bSpaceDown = KEY_PRESSED(EKey::SpaceBar) || KEY_REPEAT(EKey::SpaceBar);
	FVector2D MousePosition;
	if (bSpaceDown && KEY_PRESSED(EKey::RightMouseButton)
		&& InputEngineSystem::GetInstance()->GetMousePosition(MousePosition))
		__bCameraGesture = true;
	if (__bCameraGesture)
	{
		CancelChargingBullet();
		if (!bSpaceDown)
			__bCameraGesture = false;
		return;
	}

	if (KEY_PRESSED(EKey::SpaceBar))
	{
		__bCharging = true;
		__ChargeTime = 0.0f;
	}

	if (!__bCharging)
		return;

	if (KEY_PRESSED(EKey::SpaceBar) || KEY_REPEAT(EKey::SpaceBar))
	{
		__ChargeTime = std::clamp(
			__ChargeTime + _DeltaTime, 0.0f, __ChargeStartDelay + __MaxChargeTime);

		if (__ChargeTime >= __ChargeStartDelay)
			UpdateChargingBullet(std::clamp(
				(__ChargeTime - __ChargeStartDelay) / __MaxChargeTime, 0.0f, 1.0f));
	}
	else if (KEY_RELEASED(EKey::SpaceBar))
	{
		const float ChargeRatio = std::clamp(
			(__ChargeTime - __ChargeStartDelay) / __MaxChargeTime, 0.0f, 1.0f);
		CreateBullet(ChargeRatio);
		__bCharging = false;
		__ChargeTime = 0.0f;
	}
}

void APlayer::CreateBullet(float _ChargeRatio)
{
	UpdateChargingBullet(_ChargeRatio);
	if (GetChargingBullet() == nullptr)
		return;
	ReleaseChargingBullet();
}

void APlayer::UpdateChargingBullet(float _ChargeRatio)
{
	if (!BeginChargingBullet(false))
		return;
	const float ScaleMultiplier = 1.0f + (__MaxBulletScale - 1.0f) * _ChargeRatio;
	const FVector2D AimDirection = GetAimDirection();
	ACombatCharacter::UpdateChargingBullet(AimDirection, ScaleMultiplier);
}

void APlayer::Render()
{
	HDC _DC = GetRenderTarget();
	if (_DC == nullptr)
		return;
	const int SavedDC = SaveDC(_DC);
	if (SavedDC == 0)
		return;

	HPEN GuidePen = CreatePen(PS_DOT, 1, RGB(255, 40, 40));
	HGDIOBJ PreviousPen = SelectObject(_DC, GuidePen);
	HGDIOBJ PreviousBrush = SelectObject(_DC, GetStockObject(NULL_BRUSH));

	const FVector2D Direction = GetAimDirection();
	double GuideLength = 2000.0;
	if (ULevel* Level = LevelEngineSystem::GetInstance()->GetCurrentLevel())
	{
		for (size_t GroupIndex = 0; GroupIndex < static_cast<size_t>(EActorGroup::Count); ++GroupIndex)
		{
			for (const std::unique_ptr<AActor>& Actor : Level->GetActors(static_cast<EActorGroup>(GroupIndex)))
			{
				if (Actor == nullptr || Actor.get() == this || Actor.get() == __ChargingBullet)
					continue;
				GuideLength = (std::min)(GuideLength, RayToActor(__Pos, Direction, *Actor));
			}
		}
	}
	const FVector2D RenderPosition = CameraEngineSystem::GetInstance()->GetRenderPos(__Pos);
	const FVector2D GuideEnd = CameraEngineSystem::GetInstance()->GetRenderPos(__Pos + Direction * GuideLength);
	MoveToEx(_DC, static_cast<int>(RenderPosition.X), static_cast<int>(RenderPosition.Y), nullptr);
	LineTo(_DC, static_cast<int>(GuideEnd.X), static_cast<int>(GuideEnd.Y));

	SelectObject(_DC, PreviousBrush);
	SelectObject(_DC, PreviousPen);
	DeleteObject(GuidePen);
	SelectObject(_DC, GetStockObject(DC_BRUSH));
	SelectObject(_DC, GetStockObject(NULL_PEN));
	SetDCBrushColor(_DC, RGB(55, 145, 210));
	if (!RenderMaterial()) Rectangle(_DC,
		static_cast<int>(RenderPosition.X - __Scale.X * 0.5),
		static_cast<int>(RenderPosition.Y - __Scale.Y * 0.5),
		static_cast<int>(RenderPosition.X + __Scale.X * 0.5),
		static_cast<int>(RenderPosition.Y + __Scale.Y * 0.5));
	RestoreDC(_DC, SavedDC);
}

void APlayer::OpenNextLevel()
{
	LevelEngineSystem* LevelManager = LevelEngineSystem::GetInstance();
	if (nullptr == LevelManager)
		return;

	const bool bIsMainLevel =
		LevelManager->GetCurrentLevel() == LevelManager->GetLevel(TEXT("MainLevel"));
	const bool bIsSecondLevel =
		LevelManager->GetCurrentLevel() == LevelManager->GetLevel(TEXT("SecondLevel"));
	LevelManager->OpenLevel(
		bIsMainLevel ? TEXT("SecondLevel") :
		bIsSecondLevel ? TEXT("Level3") : TEXT("MainLevel"));
}
