#include "pch.h"
#include "Engine.h"

int UEngine::Initialize()
{
	for (auto EngineSystem : __vecEngineSystems)
	{
		if (FAILED(EngineSystem->Initialize()))
			return E_FAIL;
	}

	return S_OK;
}

void UEngine::Deinitialize()
{
	for (auto EngineSystem : __vecEngineSystems)
	{
		EngineSystem->Deinitialize();
	}
}

void UEngine::RegisterEngineSystem(IEngineSystem* _EngineSystem)
{
	if (nullptr == _EngineSystem)
		return;

	for (auto EngineSystem : __vecEngineSystems)
	{
		if (EngineSystem == _EngineSystem)
			return;
	}

	__vecEngineSystems.push_back(_EngineSystem);
}

void UEngine::Tick(float _DeltaTime)
{
	for (auto EngineSystem : __vecEngineSystems)
	{
		if (EngineSystem->IsTickable())
			EngineSystem->Tick(_DeltaTime);
	}
}

void UEngine::TG_PostUpdateWork(float _DeltaTime)
{
	for (auto EngineSystem : __vecEngineSystems)
	{
		if (EngineSystem->IsTickable())
			EngineSystem->TG_PostUpdateWork(_DeltaTime);
	}
}

void UEngine::Render()
{
	for (auto EngineSystem : __vecEngineSystems)
	{
		if (EngineSystem->IsRenderable())
		EngineSystem->Render();
	}
}
