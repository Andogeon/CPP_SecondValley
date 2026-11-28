#pragma once

#include "T_EngineSystem.h"
#include "Level.h"

#include <deque>
#include <unordered_set>

class ENGINE_API EventEngineSystem final : public TEngineSystem<EventEngineSystem>
{
    DECLARE_SINGLETON(EventEngineSystem)

public:
    HRESULT Initialize() override;
    void Deinitialize() override;
    void Tick(float _DeltaTime) override;
    void TG_PostUpdateWork(float _DeltaTime) override {}
    void Render() override {}

    AActor* RequestCreateActor(ULevel& _Level, std::unique_ptr<AActor> _Actor, EActorGroup _Group);
    bool RequestDestroyActor(ULevel& _Level, AActor* _Actor);
    bool RequestOpenLevel(const FName& _LevelName);
    bool RequestOpenLevel(const TCHAR* _LevelName);
    void CancelLevelEvents(ULevel& _Level);
    void ClearEvents();
    size_t GetPendingEventCount() const { return __Events.size() + __ProcessingEvents.size(); }
    size_t GetDisabledActorCount() const { return __DisabledActors.size(); }

private:
    enum class EEventType { CreateActor, DestroyActor, OpenLevel };
    struct FEventInfo
    {
        EEventType __Type = EEventType::CreateActor;
        ULevel* __Level = nullptr;
        std::weak_ptr<uint64_t> __Lifetime;
        uint64_t __Generation = 0;
        EActorGroup __Group = EActorGroup::Default;
        std::unique_ptr<AActor> __CreatedActor;
        AActor* __Actor = nullptr;
    };

    static bool IsValid(const FEventInfo& _Event);
    bool HasPendingActor(const ULevel& _Level, const AActor* _Actor) const;
    void Execute(FEventInfo& _Event);

private:
    std::deque<FEventInfo> __Events;
    std::deque<FEventInfo> __ProcessingEvents;
    std::vector<std::unique_ptr<AActor>> __DisabledActors;
    std::unordered_set<AActor*> __PendingDestroy;
    bool __Processing = false;
    bool __AcceptEvents = true;
};
