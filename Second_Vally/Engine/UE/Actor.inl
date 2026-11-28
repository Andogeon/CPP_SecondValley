#pragma once

#include <type_traits>
#include <utility>

template <typename ComponentType, typename... Args>
ComponentType* AActor::CreateDefaultSubobject(Args&&... args)
{
	static_assert(std::is_base_of_v<UActorComponent, ComponentType>);
	auto Component = std::make_unique<ComponentType>(std::forward<Args>(args)...);
	ComponentType* Pointer = Component.get();
	Pointer->SetOwner(this);
	__Components.push_back(std::move(Component));
	return Pointer;
}

template <typename ComponentType>
ComponentType* AActor::GetComponent() const
{
	static_assert(std::is_base_of_v<UActorComponent, ComponentType>);
	for (const auto& Component : __Components)
		if (auto* Result = dynamic_cast<ComponentType*>(Component.get()))
			return Result;
	return nullptr;
}
