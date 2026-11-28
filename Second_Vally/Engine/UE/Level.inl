#pragma once

#include <type_traits>
#include <utility>

template <typename ActorType, typename... Args>
ActorType* ULevel::SpawnActor(EActorGroup _Group, Args&&... _Args)
{
	static_assert(std::is_base_of_v<AActor, ActorType>);

    ToGroupIndex(_Group);
    std::unique_ptr<ActorType> Actor(new ActorType(std::forward<Args>(_Args)...));
    return static_cast<ActorType*>(RegisterActor(std::move(Actor), _Group));
}
