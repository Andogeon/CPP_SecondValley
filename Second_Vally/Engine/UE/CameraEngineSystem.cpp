#include "pch.h"
#include "CameraEngineSystem.h"
#include "Actor.h"
#include "InputEngineSystem.h"
#include "LaunchEngineLoop.h"
#include "LevelEngineSystem.h"

#include <algorithm>
#include <cmath>

HRESULT CameraEngineSystem::Initialize()
{
	bCanEverTick = true;
	bCanEverRender = false;
	__ObservedObject = nullptr;
	__bFollowing = false;
	__ProcessTime = 5.0f;
	__AccTime = 0.0f;
	__ViewportSize = {};
	const POINT Resolution = FEngineLoop::GetInstance()->GetResolution();
	SetViewportSize(FVector2D(Resolution.x, Resolution.y));
	__CurrentObservedPosition = __ViewportSize * 0.5;
	__ObservedPosition = __CurrentObservedPosition;
	__PreviousObservedPosition = __CurrentObservedPosition;
	__ObservedActorPosition = {};
	__Shifted = {};
	return S_OK;
}

void CameraEngineSystem::Deinitialize()
{
	__ObservedObject = nullptr;
	__bFollowing = false;
	__ObservedPosition = {};
	__CurrentObservedPosition = {};
	__PreviousObservedPosition = {};
	__ObservedActorPosition = {};
	__ViewportSize = {};
	__Shifted = {};
	__AccTime = 0.0f;
}

void CameraEngineSystem::Tick(float _DeltaTime)
{
}

void CameraEngineSystem::TG_PostUpdateWork(float _DeltaTime)
{
	const POINT Resolution = FEngineLoop::GetInstance()->GetResolution();
	SetViewportSize(FVector2D(Resolution.x, Resolution.y));
	ULevel* Level = LevelEngineSystem::GetInstance()->GetCurrentLevel();
	if (__ObservedObject != nullptr &&
		(Level == nullptr || !Level->ContainsActor(__ObservedObject) || !__ObservedObject->IsEnabled()))
		SetObservedObject(nullptr);

	const bool bObservedObjectMoved = __ObservedObject != nullptr &&
		(__ObservedObject->__Pos - __ObservedActorPosition).SizeSquared() > 0.000001;
	InputEngineSystem* Input = InputEngineSystem::GetInstance();
	const auto IsDown = [Input](EKey _Key)
	{
		return Input->IsKeyPressed(_Key) || Input->IsKeyRepeat(_Key);
	};
	FVector2D MousePosition;
	const bool bPointRequested = IsDown(EKey::SpaceBar) && Input->IsKeyPressed(EKey::RightMouseButton)
		&& Input->GetMousePosition(MousePosition);
	const FVector2D PanDirection(
		(IsDown(EKey::Numpad6) ? 1.0 : 0.0) - (IsDown(EKey::Numpad4) ? 1.0 : 0.0),
		(IsDown(EKey::Numpad2) ? 1.0 : 0.0) - (IsDown(EKey::Numpad8) ? 1.0 : 0.0));
	const bool bKeyboardPan = PanDirection.SizeSquared() > 0.0;
	const float DeltaTime = std::isfinite(_DeltaTime) ? (std::max)(_DeltaTime, 0.0f) : 0.0f;

	if (bPointRequested)
		SetObservedPosition(GetLocationPos(MousePosition));
	else if (bKeyboardPan)
	{
		__bFollowing = false;
		__CurrentObservedPosition += PanDirection * (500.0 * DeltaTime);
		__ObservedPosition = __CurrentObservedPosition;
		__PreviousObservedPosition = __CurrentObservedPosition;
		__AccTime = __ProcessTime;
	}
	else if (bObservedObjectMoved)
		__bFollowing = true;

	if (__bFollowing && __ObservedObject != nullptr)
	{
		__CurrentObservedPosition = __ObservedObject->__Pos;
		__ObservedPosition = __CurrentObservedPosition;
		__PreviousObservedPosition = __CurrentObservedPosition;
		__AccTime = __ProcessTime;
	}
	if (__ObservedObject != nullptr)
		__ObservedActorPosition = __ObservedObject->__Pos;
	CalculateShifted(DeltaTime);
}

void CameraEngineSystem::Render()
{
}

void CameraEngineSystem::SetObservedObject(AActor* _Actor)
{
	__ObservedObject = _Actor;
	__bFollowing = _Actor != nullptr;
	if (_Actor == nullptr)
		return;
	__ObservedActorPosition = _Actor->__Pos;
	__CurrentObservedPosition = _Actor->__Pos;
	__ObservedPosition = _Actor->__Pos;
	__PreviousObservedPosition = _Actor->__Pos;
	__AccTime = __ProcessTime;
	__Shifted = __CurrentObservedPosition - __ViewportSize * 0.5;
}

void CameraEngineSystem::SetObservedPosition(const FVector2D& _Position)
{
	if (!std::isfinite(_Position.X) || !std::isfinite(_Position.Y))
		return;
	__bFollowing = false;
	__PreviousObservedPosition = __CurrentObservedPosition;
	__ObservedPosition = _Position;
	__AccTime = 0.0f;
	CalculateShifted(0.0f);
}

void CameraEngineSystem::SetViewportSize(const FVector2D& _Size)
{
	if (!std::isfinite(_Size.X) || !std::isfinite(_Size.Y) || _Size.X <= 0.0 || _Size.Y <= 0.0)
		return;
	__ViewportSize = _Size;
	__Shifted = __CurrentObservedPosition - __ViewportSize * 0.5;
}

void CameraEngineSystem::SetProcessTime(float _Seconds)
{
	if (!std::isfinite(_Seconds) || _Seconds < 0.0f)
		return;
	__ProcessTime = _Seconds;
	__PreviousObservedPosition = __CurrentObservedPosition;
	__AccTime = 0.0f;
	CalculateShifted(0.0f);
}

void CameraEngineSystem::CalculateShifted(float _DeltaTime)
{
	const float DeltaTime = std::isfinite(_DeltaTime) ? (std::max)(_DeltaTime, 0.0f) : 0.0f;
	__AccTime = (std::min)(__AccTime + DeltaTime, __ProcessTime);
	const double Alpha = __ProcessTime > 0.0f ? __AccTime / __ProcessTime : 1.0;
	__CurrentObservedPosition = __PreviousObservedPosition
		+ (__ObservedPosition - __PreviousObservedPosition) * Alpha;
	__Shifted = __CurrentObservedPosition - __ViewportSize * 0.5;
}
