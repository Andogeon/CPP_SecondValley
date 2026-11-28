#pragma once

#include "Level.h"
#include "T_EngineSystem.h"

#include <memory>
#include <unordered_map>

class ENGINE_API LevelEngineSystem : public TEngineSystem<LevelEngineSystem>
{
	DECLARE_SINGLETON(LevelEngineSystem)
	friend class EventEngineSystem;

public:
	virtual HRESULT Initialize() override;
	virtual void Deinitialize() override;

	virtual void Tick(float _DeltaTime) override;
	virtual void TG_PostUpdateWork(float _DeltaTime) override;
	virtual void Render() override;

	ULevel* CreateLevel(const FName& _LevelName);
	ULevel* CreateLevel(const TCHAR* _LevelName);		
	
	ULevel* CreateLevel(const FName& _LevelName, ULevel::FRuntimeObjectLoader _Loader);
	ULevel* CreateLevel(const TCHAR* _LevelName, ULevel::FRuntimeObjectLoader _Loader);
	
	bool RegisterLevel(std::unique_ptr<ULevel> _Level);
	
	bool RemoveLevel(const FName& _LevelName);
	bool RemoveLevel(const TCHAR* _LevelName);

	bool OpenLevel(const FName& _LevelName);
	bool OpenLevel(const TCHAR* _LevelName);

	ULevel* GetLevel(const FName& _LevelName) const;
	ULevel* GetLevel(const TCHAR* _LevelName) const;

	ULevel* GetCurrentLevel() const;
	const FName& GetCurrentLevelName() const;

private:
	bool OpenLevelImmediate(const FName& _LevelName);

private:
	std::unordered_map<FName, std::unique_ptr<ULevel>> __Levels;
	ULevel* __CurrentLevel = nullptr;
};
