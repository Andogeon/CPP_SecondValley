#pragma once

#include "T_EngineSystem.h"

#include <chrono>

class FAppTimeEngineSystem : public TEngineSystem<FAppTimeEngineSystem>
{
	DECLARE_SINGLETON(FAppTimeEngineSystem)

private:
	using FClock = std::chrono::steady_clock;

public:
	virtual HRESULT Initialize() override;
	virtual void Deinitialize() override;

	virtual void Tick(float _DeltaTime) override;
	virtual void TG_PostUpdateWork(float _DeltaTime) override;
	virtual void Render() override;

public:
	double GetCurrentTime() const { return __CurrentTime; }
	double GetDeltaTime() const { return __DeltaTime; }

private:
	double MeasureDeltaTime();

	void UpdateFramesPerSecond();

	FClock::time_point __PreviousTime;

	double __CurrentTime = 0.0;

	double __DeltaTime = 0.0;

	double __RawDeltaTime = 0.0;

	double __FramesPerSecond = 0.0;

	double __AverageFrameTime = 0.0;

};
