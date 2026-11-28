#pragma once
#include "Object.h"
#include "ActorComponent.h"
#include "Renderable.h"
#include "UE/Math/Vector2D.h"

#include <memory>
#include <vector>

class Material;
class UCollider;

class ENGINE_API AActor : public UObject, public IRenderable
{
	friend class ULevel;

protected:
	AActor() = default;

public:
	virtual ~AActor() = default;
	AActor(const AActor&) = delete;
	AActor& operator=(const AActor&) = delete;
	AActor(AActor&&) = delete;
	AActor& operator=(AActor&&) = delete;
	
    template <typename ComponentType, typename... Args>
	ComponentType* CreateDefaultSubobject(Args&&... args);

	template <typename ComponentType>
	ComponentType* GetComponent() const;
	const std::vector<std::unique_ptr<UActorComponent>>& GetComponents() const { return __Components; }

	virtual void OnCollisionEnter(UCollider* _Other);
	virtual void OnCollisionStay(UCollider* _Other);
	virtual void OnCollisionExit(UCollider* _Other);

	void SetEnabled(bool _Enabled) { __bEnabled = _Enabled; }
	bool IsEnabled() const { return __bEnabled; }

	virtual void Render() override;
    void SetMaterial(std::shared_ptr<Material> _Material);
    std::shared_ptr<Material> GetMaterial() const { return __Material; }

protected:
    bool RenderMaterial() const;
	virtual void Tick(float _DeltaTime) override;
	virtual void TG_PostUpdateWork(float _DeltaTime) override;

protected:
	virtual void PreInitializeComponents();
	virtual void PostInitializeComponents();

	virtual void BeginPlay();
	virtual void EndPlay();
	
	virtual void Destroy();		
	virtual void Destroyed();	

public:
	FVector2D	__Pos = { 0.0, 0.0 };
	FVector2D	__Scale = { 100.f, 100.f };
	double		__Rotation = 0.0;

protected:
	std::vector<std::unique_ptr<UActorComponent>> __Components;

private:
	bool __bEnabled = true;
    std::shared_ptr<Material> __Material;
};

#include "Actor.inl"
