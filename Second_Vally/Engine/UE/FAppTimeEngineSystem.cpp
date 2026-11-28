#include "pch.h"
#include "FAppTimeEngineSystem.h"
#include "LaunchEngineLoop.h"
#include <algorithm>

HRESULT FAppTimeEngineSystem::Initialize()
{
	__PreviousTime = FClock::now();
	__CurrentTime = 0.0;
	__DeltaTime = 0.0;
	__RawDeltaTime = 0.0;
	__FramesPerSecond = 0.0;
	__AverageFrameTime = 0.0;

	FTickableGameObject::bCanEverTick = true;
	IRenderable::bCanEverRender = true;

	return S_OK;
}

void FAppTimeEngineSystem::Deinitialize()
{
	__PreviousTime = {};
	__CurrentTime = 0.0;
	__DeltaTime = 0.0;
	__RawDeltaTime = 0.0;
	__FramesPerSecond = 0.0;
	__AverageFrameTime = 0.0;
}

double FAppTimeEngineSystem::MeasureDeltaTime()
{
	const FClock::time_point CurrentTime = FClock::now();

	const double DeltaTime =
		std::chrono::duration<double>(CurrentTime - __PreviousTime).count();

	__PreviousTime = CurrentTime;

	return DeltaTime;
}

void FAppTimeEngineSystem::Tick(float _DeltaTime)
{
	UNREFERENCED_PARAMETER(_DeltaTime);
}

void FAppTimeEngineSystem::TG_PostUpdateWork(float _DeltaTime)
{
	UNREFERENCED_PARAMETER(_DeltaTime);

	__RawDeltaTime = MeasureDeltaTime();

	__CurrentTime += __RawDeltaTime;

	constexpr double MaxDeltaTime = 0.1;
	__DeltaTime = std::clamp(__RawDeltaTime, 0.0, MaxDeltaTime);

	FEngineLoop::GetInstance()->SetDeltaTime(__DeltaTime);
}

void FAppTimeEngineSystem::Render()
{
	#ifdef _DEBUG
	UpdateFramesPerSecond();

	char WindowTitle[96] = {};
	sprintf_s(
		WindowTitle,
		"Chrono FPS: %.1f  Play Time: %.2fs",
		__FramesPerSecond,
		__CurrentTime);

	SetWindowTextA(FEngineLoop::GetInstance()->GetHandle(), WindowTitle);
	#endif
}

void FAppTimeEngineSystem::UpdateFramesPerSecond()
{

	if (__RawDeltaTime <= 0.0)
		return;

	if (__AverageFrameTime <= 0.0)
	{
		__AverageFrameTime = __RawDeltaTime;
	}
	else
	{
		__AverageFrameTime =
			__AverageFrameTime * 0.75 + __RawDeltaTime * 0.25;
	}

	__FramesPerSecond = 1.0 / __AverageFrameTime;
}
