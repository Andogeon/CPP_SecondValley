#include "pch.h"
#include "Level.h"
#include "EventEngineSystem.h"
#include "CollisionEngineSystem.h"

#include <algorithm>
#include <stdexcept>

ULevel::ULevel(const FName& _LevelName)
	: UObject(_LevelName)
{
}

ULevel::~ULevel()
{
	Deactivate();
	ClearActors();
}

AActor* ULevel::RegisterActor(std::unique_ptr<AActor> _Actor, EActorGroup _Group)
{
    if (__bClearingActors) return nullptr;
    if (__bActive)
        return EventEngineSystem::GetInstance()->RequestCreateActor(*this, std::move(_Actor), _Group);
    return AddActorImmediate(std::move(_Actor), _Group);
}

AActor* ULevel::AddActorImmediate(std::unique_ptr<AActor> _Actor, EActorGroup _Group)
{
    if (!_Actor || __bClearingActors) return nullptr;
    const size_t GroupIndex = ToGroupIndex(_Group);
    AActor* Actor = _Actor.get();
    __ActorGroups[GroupIndex].push_back(std::move(_Actor));
    if (__bActive)
    {
        Actor->PreInitializeComponents();
        Actor->PostInitializeComponents();
        Actor->BeginPlay();
    }
    return Actor;
}

bool ULevel::ContainsActor(const AActor* _Actor) const
{
    for (const auto& Group : __ActorGroups)
        for (const auto& Actor : Group)
            if (Actor.get() == _Actor) return true;
    return false;
}

bool ULevel::DestroyActor(AActor* _Actor)
{
    return EventEngineSystem::GetInstance()->RequestDestroyActor(*this, _Actor);
}

std::unique_ptr<AActor> ULevel::ExtractActor(AActor* _Actor)
{
    if (!_Actor || __bClearingActors) return nullptr;
    for (auto& Group : __ActorGroups)
    {
        const auto Found = std::find_if(Group.begin(), Group.end(),
            [_Actor](const auto& _Owned) { return _Owned.get() == _Actor; });
        if (Found == Group.end()) continue;
        auto Actor = std::move(*Found);
        Group.erase(Found);
        Actor->SetEnabled(false);
        CollisionEngineSystem::GetInstance()->RemoveActor(Actor.get());
        if (__bActive) Actor->EndPlay();
        Actor->Destroy();
        Actor->Destroyed();
        return Actor;
    }
    return nullptr;
}

void ULevel::ClearActors()
{
    if (__bClearingActors) return;
    __bClearingActors = true;
    ++*__EventLifetime;
    EventEngineSystem::GetInstance()->CancelLevelEvents(*this);
    auto Groups = std::move(__ActorGroups);
    __ActorGroups = {};
    for (auto& Group : Groups)
    {
        for (const auto& Actor : Group)
        {
            Actor->SetEnabled(false);
            CollisionEngineSystem::GetInstance()->RemoveActor(Actor.get());
            if (__bActive) Actor->EndPlay();
            Actor->Destroy();
            Actor->Destroyed();
        }
    }
    Groups = {};
    __bClearingActors = false;
}

void ULevel::SetRuntimeObjectLoader(FRuntimeObjectLoader _Loader)
{
	__RuntimeObjectLoader = std::move(_Loader);
}

bool ULevel::LoadRuntimeObjects()
{
	if (!__RuntimeObjectLoader)
		return true;

	ClearActors();
	return __RuntimeObjectLoader(*this);
}

void ULevel::UnloadRuntimeObjects()
{
	Deactivate();
	ClearActors();
}

const std::vector<std::unique_ptr<AActor>>& ULevel::GetActors(EActorGroup _Group) const
{
	return __ActorGroups[ToGroupIndex(_Group)];
}

bool ULevel::IsActive() const
{
	return __bActive;
}

void ULevel::Tick(float _DeltaTime)
{
	if (!__bActive)
		return;

	for (const auto& ActorGroup : __ActorGroups)
	{
		for (const std::unique_ptr<AActor>& Actor : ActorGroup)
		{
			if (Actor->IsEnabled() && Actor->IsTickable())
				Actor->Tick(_DeltaTime);
		}
	}

}

void ULevel::TG_PostUpdateWork(float _DeltaTime)
{
	if (!__bActive)
		return;

	for (const auto& ActorGroup : __ActorGroups)
	{
		for (const std::unique_ptr<AActor>& Actor : ActorGroup)
		{
			if (Actor->IsEnabled() && Actor->IsTickable())
				Actor->TG_PostUpdateWork(_DeltaTime);
		}
	}
}

void ULevel::Render()
{
	if (!__bActive)
		return;

	for (const auto& ActorGroup : __ActorGroups)
	{
		for (const std::unique_ptr<AActor>& Actor : ActorGroup)
		{
			if (Actor->IsEnabled() && Actor->IsRenderable())
				Actor->Render();
		}
	}
}

void ULevel::Activate()
{
	if (__bActive)
		return;

	__bActive = true;

	for (const auto& ActorGroup : __ActorGroups)
	{
		for (const std::unique_ptr<AActor>& Actor : ActorGroup)
		{
			Actor->PreInitializeComponents();
			Actor->PostInitializeComponents();
			Actor->BeginPlay();
		}
	}

}

void ULevel::Deactivate()
{
	if (!__bActive)
		return;

	for (auto GroupIterator = __ActorGroups.rbegin();
		GroupIterator != __ActorGroups.rend();
		++GroupIterator)
	{
		for (auto ActorIterator = GroupIterator->rbegin();
			ActorIterator != GroupIterator->rend();
			++ActorIterator)
		{
			CollisionEngineSystem::GetInstance()->RemoveActor(ActorIterator->get());
			(*ActorIterator)->EndPlay();
		}
	}

	__bActive = false;
}

size_t ULevel::ToGroupIndex(EActorGroup _Group)
{
	const size_t GroupIndex = static_cast<size_t>(_Group);

	if (GroupIndex >= GroupCount)
		throw std::out_of_range("Invalid actor group.");

	return GroupIndex;
}
