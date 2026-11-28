#pragma once

#include "T_EngineSystem.h"

class AActor;

class ENGINE_API CameraEngineSystem final : public TEngineSystem<CameraEngineSystem>
{
	DECLARE_SINGLETON(CameraEngineSystem)

public:
	HRESULT Initialize() override;
	void Deinitialize() override;
	void Tick(float _DeltaTime) override;
	void TG_PostUpdateWork(float _DeltaTime) override;
	void Render() override;

	void SetObservedObject(AActor* _Actor);
	void SetObservedPosition(const FVector2D& _Position);
	void SetViewportSize(const FVector2D& _Size);
	void SetProcessTime(float _Seconds);
	void CalculateShifted(float _DeltaTime);

	AActor* GetObservedObject() const { return __ObservedObject; }
	FVector2D GetObservedPosition() const { return __CurrentObservedPosition; }
	FVector2D GetRenderPos(const FVector2D& _WorldPosition) const { return _WorldPosition - __Shifted; }
	FVector2D GetLocationPos(const FVector2D& _RenderPosition) const { return _RenderPosition + __Shifted; }
	bool IsFollowing() const { return __bFollowing; }

private:
	FVector2D __ObservedPosition = {};
	FVector2D __CurrentObservedPosition = {};
	FVector2D __PreviousObservedPosition = {};
	FVector2D __ObservedActorPosition = {};
	FVector2D __ViewportSize = {};
	FVector2D __Shifted = {};
	AActor* __ObservedObject = nullptr;
	float __ProcessTime = 5.0f;
	float __AccTime = 0.0f;
	bool __bFollowing = false;
};
