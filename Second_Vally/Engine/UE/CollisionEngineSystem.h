#pragma once

#include "Level.h"
#include "T_EngineSystem.h"

#include <array>
#include <map>
#include <utility>
#include <vector>

class UCollider;

enum class ECollisionPhase : uint8_t
{
	Enter,
	Stay,
	Exit
};

class ENGINE_API CollisionEngineSystem : public TEngineSystem<CollisionEngineSystem>
{
	DECLARE_SINGLETON(CollisionEngineSystem)
	friend class ULevel;

public:
	HRESULT Initialize() override;
	void Deinitialize() override;
	void Tick(float _DeltaTime) override;
	void TG_PostUpdateWork(float _DeltaTime) override;
	void Render() override;

	void RequestInspection(EActorGroup _Left, EActorGroup _Right);

	void SetInspection(EActorGroup _Left, EActorGroup _Right, bool _Enabled);
	bool IsInspection(EActorGroup _Left, EActorGroup _Right) const;
	void InspectionOff();

private:
	static constexpr size_t GroupCount = static_cast<size_t>(EActorGroup::Count);
	static_assert(GroupCount <= 32, "Collision channel masks support up to 32 groups.");
	using FPairKey = std::pair<uint64_t, uint64_t>;
	using FColliderGroups = std::array<std::vector<UCollider*>, GroupCount>;

	struct FCollisionInfo
	{
		UCollider* Left;
		UCollider* Right;
		EActorGroup LeftGroup;
		EActorGroup RightGroup;
		bool bWasColliding = false;
	};

	void Update(ULevel& _Level);
	void ChannelInspection(EActorGroup _Left, EActorGroup _Right, const FColliderGroups& _Groups);
	static bool IsCollision(const UCollider& _Left, const UCollider& _Right);
	static void Notify(FCollisionInfo& _Info, ECollisionPhase _Phase);
	void RemoveActor(AActor* _Actor);

	std::array<uint32_t, GroupCount> __Channels = {};

	std::map<FPairKey, FCollisionInfo> __CollisionInfo;
};
