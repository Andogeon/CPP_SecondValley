#pragma once

#include "T_EngineSystem.h"

#include <Windows.h>


class AppTimeEngineSystem : public TEngineSystem<AppTimeEngineSystem>
{
	DECLARE_SINGLETON(AppTimeEngineSystem)

public:
	HRESULT Initialize() override;
	void Deinitialize() override;

	void Tick(float _DeltaTime) override;
	void TG_PostUpdateWork(float _DeltaTime) override;
	void Render() override;

	double GetDeltaTime() const { return __DeltaTime; }

	double GetCurrentTime() const { return __CurrentTime; }

private:
	double MeasureDeltaTime();

	void UpdateFramesPerSecond();

	LARGE_INTEGER __Frequency = {};

	LARGE_INTEGER __PreviousCounter = {};

	double __DeltaTime = 0.0;

	double __RawDeltaTime = 0.0;

	double __CurrentTime = 0.0;

	double __FramesPerSecond = 0.0;

	double __AverageFrameTime = 0.0;

};
