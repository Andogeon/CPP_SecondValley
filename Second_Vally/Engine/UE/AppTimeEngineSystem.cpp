#include "pch.h"
#include "AppTimeEngineSystem.h"
#include "LaunchEngineLoop.h"
#include <algorithm>

HRESULT AppTimeEngineSystem::Initialize()
{
	if (!QueryPerformanceFrequency(&__Frequency))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (!QueryPerformanceCounter(&__PreviousCounter))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	__DeltaTime = 0.0;
	__RawDeltaTime = 0.0;
	__CurrentTime = 0.0;
	__FramesPerSecond = 0.0;
	__AverageFrameTime = 0.0;

	FTickableGameObject::bCanEverTick = true;
	IRenderable::bCanEverRender = true;

	return S_OK;
}

void AppTimeEngineSystem::Deinitialize()
{
	__Frequency = {};
	__PreviousCounter = {};
	__DeltaTime = 0.0;
	__RawDeltaTime = 0.0;
	__CurrentTime = 0.0;
	__FramesPerSecond = 0.0;
	__AverageFrameTime = 0.0;
}

void AppTimeEngineSystem::Tick(float _DeltaTime)
{
	UNREFERENCED_PARAMETER(_DeltaTime);
}

void AppTimeEngineSystem::TG_PostUpdateWork(float _DeltaTime)
{
	UNREFERENCED_PARAMETER(_DeltaTime);

	__RawDeltaTime = MeasureDeltaTime();

	__CurrentTime += __RawDeltaTime;

	constexpr double MaxDeltaTime = 0.1;
	__DeltaTime = std::clamp(__RawDeltaTime, 0.0, MaxDeltaTime);

	FEngineLoop::GetInstance()->SetDeltaTime(__DeltaTime);
}

void AppTimeEngineSystem::Render()
{
	#ifdef _DEBUG
	UpdateFramesPerSecond();

	char WindowTitle[96] = {};
	sprintf_s(
		WindowTitle,
		"QPC FPS: %.1f  Play Time: %.2fs",
		__FramesPerSecond,
		__CurrentTime);

	SetWindowTextA(FEngineLoop::GetInstance()->GetHandle(), WindowTitle);
	#endif
}

void AppTimeEngineSystem::UpdateFramesPerSecond()
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

double AppTimeEngineSystem::MeasureDeltaTime()
{
	LARGE_INTEGER CurrentCounter = {};

	if (!QueryPerformanceCounter(&CurrentCounter))
		return 0.0;

	const double DeltaTime =
		static_cast<double>(CurrentCounter.QuadPart - __PreviousCounter.QuadPart) /
		static_cast<double>(__Frequency.QuadPart);

	__PreviousCounter = CurrentCounter;

	return DeltaTime;
}
