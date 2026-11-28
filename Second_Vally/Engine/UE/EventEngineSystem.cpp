#include "pch.h"
#include "EventEngineSystem.h"
#include "LevelEngineSystem.h"

#include <algorithm>

HRESULT EventEngineSystem::Initialize()
{
    ClearEvents();
    __AcceptEvents = true;
    bCanEverTick = true;
    bCanEverRender = false;
    return S_OK;
}

void EventEngineSystem::Deinitialize()
{
    __AcceptEvents = false;
    ClearEvents();
}

bool EventEngineSystem::IsValid(const FEventInfo& _Event)
{
    const auto Lifetime = _Event.__Lifetime.lock();
    return _Event.__Level && Lifetime &&
        (_Event.__Type == EEventType::OpenLevel || *Lifetime == _Event.__Generation);
}

AActor* EventEngineSystem::RequestCreateActor(ULevel& _Level, std::unique_ptr<AActor> _Actor, EActorGroup _Group)
{
    if (!__AcceptEvents || !_Actor || _Level.__bClearingActors || _Group >= EActorGroup::Count) return nullptr;
    FEventInfo Event;
    Event.__Type = EEventType::CreateActor;
    Event.__Level = &_Level;
    Event.__Lifetime = _Level.__EventLifetime;
    Event.__Generation = *_Level.__EventLifetime;
    Event.__Group = _Group;
    Event.__Actor = _Actor.get();
    Event.__CreatedActor = std::move(_Actor);
    AActor* Result = Event.__Actor;
    __Events.push_back(std::move(Event));
    return Result;
}

bool EventEngineSystem::HasPendingActor(const ULevel& _Level, const AActor* _Actor) const
{
    const auto Match = [&](const FEventInfo& _Event)
    {
        return _Event.__Type == EEventType::CreateActor && _Event.__Level == &_Level &&
            _Event.__Actor == _Actor && IsValid(_Event);
    };
    return std::any_of(__Events.begin(), __Events.end(), Match) ||
        std::any_of(__ProcessingEvents.begin(), __ProcessingEvents.end(), Match);
}

bool EventEngineSystem::RequestDestroyActor(ULevel& _Level, AActor* _Actor)
{
    if (!__AcceptEvents || !_Actor || _Level.__bClearingActors ||
        (!_Level.ContainsActor(_Actor) && !HasPendingActor(_Level, _Actor))) return false;
    if (__PendingDestroy.contains(_Actor)) return true;
    FEventInfo Event;
    Event.__Type = EEventType::DestroyActor;
    Event.__Level = &_Level;
    Event.__Lifetime = _Level.__EventLifetime;
    Event.__Generation = *_Level.__EventLifetime;
    Event.__Actor = _Actor;
    __Events.push_back(std::move(Event));
    __PendingDestroy.insert(_Actor);
    return true;
}

bool EventEngineSystem::RequestOpenLevel(const FName& _LevelName)
{
    ULevel* Level = LevelEngineSystem::GetInstance()->GetLevel(_LevelName);
    if (!__AcceptEvents || !Level || Level->__bClearingActors) return false;
    FEventInfo Event;
    Event.__Type = EEventType::OpenLevel;
    Event.__Level = Level;
    Event.__Lifetime = Level->__EventLifetime;
    Event.__Generation = *Level->__EventLifetime;
    __Events.push_back(std::move(Event));
    return true;
}

bool EventEngineSystem::RequestOpenLevel(const TCHAR* _LevelName)
{
    return RequestOpenLevel(FName(_LevelName));
}

void EventEngineSystem::CancelLevelEvents(ULevel& _Level)
{
    const auto Cancel = [&](std::deque<FEventInfo>& _Events)
    {
        std::deque<FEventInfo> Pending;
        Pending.swap(_Events);
        for (auto& Event : Pending)
        {
            if (Event.__Level != &_Level || Event.__Type == EEventType::OpenLevel)
                _Events.push_back(std::move(Event));
            else if (Event.__Type == EEventType::DestroyActor) __PendingDestroy.erase(Event.__Actor);
        }
    };
    Cancel(__Events);
    Cancel(__ProcessingEvents);
}

void EventEngineSystem::ClearEvents()
{
    const bool AcceptEvents = __AcceptEvents;
    __AcceptEvents = false;
    __Events.clear();
    __ProcessingEvents.clear();
    __PendingDestroy.clear();
    __DisabledActors.clear();
    __AcceptEvents = AcceptEvents;
}

void EventEngineSystem::Execute(FEventInfo& _Event)
{
    if (_Event.__Type == EEventType::DestroyActor) __PendingDestroy.erase(_Event.__Actor);
    if (!IsValid(_Event)) return;
    switch (_Event.__Type)
    {
    case EEventType::CreateActor:
        _Event.__Level->AddActorImmediate(std::move(_Event.__CreatedActor), _Event.__Group);
        break;
    case EEventType::DestroyActor:
        if (auto Actor = _Event.__Level->ExtractActor(_Event.__Actor))
            __DisabledActors.push_back(std::move(Actor));
        break;
    case EEventType::OpenLevel:
        LevelEngineSystem::GetInstance()->OpenLevelImmediate(_Event.__Level->GetFName());
        break;
    }
}

void EventEngineSystem::Tick(float _DeltaTime)
{
    if (__Processing || !__AcceptEvents) return;
    __Processing = true;
    struct FProcessingGuard
    {
        bool& __Value;
        ~FProcessingGuard() { __Value = false; }
    } Guard{ __Processing };
    std::vector<std::unique_ptr<AActor>> Disabled;
    Disabled.swap(__DisabledActors);
    Disabled.clear();
    __ProcessingEvents.swap(__Events);
    while (!__ProcessingEvents.empty())
    {
        FEventInfo Event = std::move(__ProcessingEvents.front());
        __ProcessingEvents.pop_front();
        Execute(Event);
    }
}
