#pragma once

#include "Actor.h"
#include "NameTypes.h"
#include "Object.h"

#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

enum class EActorGroup : uint8_t
{
	Default,
	Player,
	Monster,
	Effect,
	UI,
	ProjectilePlayer,
	ProjectileEnemy,

	Count
};

class ENGINE_API ULevel : public UObject, public IRenderable
{
	friend class LevelEngineSystem;
	friend class EventEngineSystem;

public:
	using FRuntimeObjectLoader = std::function<bool(ULevel&)>;
	explicit ULevel(const FName& _LevelName);
	virtual ~ULevel() override;

	ULevel(const ULevel&) = delete;
	ULevel& operator=(const ULevel&) = delete;
	ULevel(ULevel&&) = delete;
	ULevel& operator=(ULevel&&) = delete;

public:
	template <typename ActorType, typename... Args>
	
	
	ActorType* SpawnActor(EActorGroup _Group, Args&&... _Args);

	bool DestroyActor(AActor* _Actor);
	bool ContainsActor(const AActor* _Actor) const;
	void ClearActors();
	
	void SetRuntimeObjectLoader(FRuntimeObjectLoader _Loader);
	bool LoadRuntimeObjects();
	void UnloadRuntimeObjects();

	const std::vector<std::unique_ptr<AActor>>& GetActors(EActorGroup _Group) const;
	bool IsActive() const;

	virtual void Tick(float _DeltaTime) override;
	virtual void TG_PostUpdateWork(float _DeltaTime) override;
	virtual void Render() override;

private:
	AActor* RegisterActor(std::unique_ptr<AActor> _Actor, EActorGroup _Group);
	AActor* AddActorImmediate(std::unique_ptr<AActor> _Actor, EActorGroup _Group);
	std::unique_ptr<AActor> ExtractActor(AActor* _Actor);
	void Activate();
	void Deactivate();

	static constexpr size_t GroupCount = static_cast<size_t>(EActorGroup::Count);
	static size_t ToGroupIndex(EActorGroup _Group);

private:
	std::array<std::vector<std::unique_ptr<AActor>>, GroupCount> __ActorGroups;
	FRuntimeObjectLoader __RuntimeObjectLoader;
	bool __bActive = false;
	bool __bClearingActors = false;
	std::shared_ptr<uint64_t> __EventLifetime = std::make_shared<uint64_t>(0);
};

#include "Level.inl"
