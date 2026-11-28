#include "pch.h"
#include "LevelEngineSystem.h"
#include "EventEngineSystem.h"

HRESULT LevelEngineSystem::Initialize()
{
	FTickableGameObject::bCanEverTick = true;
	IRenderable::bCanEverRender = true;

	return S_OK;
}

void LevelEngineSystem::Deinitialize()
{
	EventEngineSystem::GetInstance()->ClearEvents();
	for (auto& LevelPair : __Levels)
		LevelPair.second->UnloadRuntimeObjects();

	__CurrentLevel = nullptr;
	__Levels.clear();
	EventEngineSystem::GetInstance()->ClearEvents();
}

void LevelEngineSystem::Tick(float _DeltaTime)
{
	if (__CurrentLevel != nullptr)
		__CurrentLevel->Tick(_DeltaTime);
}

void LevelEngineSystem::TG_PostUpdateWork(float _DeltaTime)
{
	if (__CurrentLevel != nullptr)
		__CurrentLevel->TG_PostUpdateWork(_DeltaTime);

}

void LevelEngineSystem::Render()
{
	if (__CurrentLevel != nullptr)
		__CurrentLevel->Render();
}

ULevel* LevelEngineSystem::CreateLevel(const FName& _LevelName)
{
	return CreateLevel(_LevelName, {});
}

ULevel* LevelEngineSystem::CreateLevel(const FName& _LevelName, ULevel::FRuntimeObjectLoader _Loader)
{	
	if (_LevelName.IsNone())
		return nullptr;

	if (ULevel* ExistingLevel = GetLevel(_LevelName))
	{
		if (_Loader)
			ExistingLevel->SetRuntimeObjectLoader(std::move(_Loader));

		return ExistingLevel;
	}

	auto Level = std::make_unique<ULevel>(_LevelName);

	Level->SetRuntimeObjectLoader(std::move(_Loader));
	ULevel* LevelPointer = Level.get();

	__Levels.emplace(_LevelName, std::move(Level));

	return LevelPointer;
}

ULevel* LevelEngineSystem::CreateLevel(const TCHAR* _LevelName)
{
	return CreateLevel(FName(_LevelName));
}

ULevel* LevelEngineSystem::CreateLevel(const TCHAR* _LevelName, ULevel::FRuntimeObjectLoader _Loader)
{
	return CreateLevel(FName(_LevelName), std::move(_Loader));
}

bool LevelEngineSystem::RegisterLevel(std::unique_ptr<ULevel> _Level)
{
	if (_Level == nullptr || _Level->GetFName().IsNone())
		return false;

	const FName LevelName = _Level->GetFName();

	if (__Levels.contains(LevelName))
		return false;

	__Levels.emplace(LevelName, std::move(_Level));

	return true;
}

bool LevelEngineSystem::RemoveLevel(const FName& _LevelName)
{
	ULevel* Level = GetLevel(_LevelName);

	if (Level == nullptr || Level == __CurrentLevel)
		return false;

	return __Levels.erase(_LevelName) > 0;
}

bool LevelEngineSystem::RemoveLevel(const TCHAR* _LevelName)
{
	return RemoveLevel(FName(_LevelName));
}

bool LevelEngineSystem::OpenLevel(const FName& _LevelName)
{
    return EventEngineSystem::GetInstance()->RequestOpenLevel(_LevelName);
}

bool LevelEngineSystem::OpenLevel(const TCHAR* _LevelName)
{
	return OpenLevel(FName(_LevelName));
}

ULevel* LevelEngineSystem::GetLevel(const FName& _LevelName) const
{
	const auto LevelIterator = __Levels.find(_LevelName);

	if (LevelIterator == __Levels.end())
		return nullptr;

	return LevelIterator->second.get();
}

ULevel* LevelEngineSystem::GetLevel(const TCHAR* _LevelName) const
{
	return GetLevel(FName(_LevelName));
}

ULevel* LevelEngineSystem::GetCurrentLevel() const
{
	return __CurrentLevel;
}

const FName& LevelEngineSystem::GetCurrentLevelName() const
{
	if (__CurrentLevel == nullptr)
		return NAME_None;

	return __CurrentLevel->GetFName();
}

bool LevelEngineSystem::OpenLevelImmediate(const FName& _LevelName)
{
    ULevel* NextLevel = GetLevel(_LevelName);
    if (!NextLevel) return false;
    if (NextLevel == __CurrentLevel) return true;
    if (__CurrentLevel) __CurrentLevel->UnloadRuntimeObjects();
    __CurrentLevel = NextLevel;
    if (!NextLevel->LoadRuntimeObjects())
    {
        NextLevel->UnloadRuntimeObjects();
        __CurrentLevel = nullptr;
        return false;
    }
    NextLevel->Activate();
    return true;
}
