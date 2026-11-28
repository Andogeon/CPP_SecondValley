#pragma once

#include "EngineMinimal.h"
#include "Object.h"
#include "EngineSystem.h"

class UEngine : public UObject, public TSingleton<UEngine>
{
	DECLARE_SINGLETON(UEngine)

public:
	int Initialize();
	void Deinitialize();

	void RegisterEngineSystem(IEngineSystem* _EngineSystem);

	virtual void Tick(float _DeltaTime) override;
	virtual void TG_PostUpdateWork(float _DeltaTime) override;
	void Render();

private:
	std::vector<IEngineSystem*>		__vecEngineSystems;
};

