#pragma once
#include "Object.h"

class ENGINE_API UActorComponent : public UObject
{
public:
	UActorComponent() = default;
	virtual ~UActorComponent() = default;

public:
	void Tick(float _DeltaTime) override { TickComponent(_DeltaTime); }
	void TG_PostUpdateWork(float _DeltaTime) override {}
	virtual void TickComponent(float _DeltaTime);
	virtual void BeginPlay();
	virtual void EndPlay();	
	virtual void InitializeComponent();

	FORCEINLINE UObject* GetOwner() const { return __Owner; }
	FORCEINLINE void SetOwner(UObject* NewOwner) { __Owner = NewOwner; }

private:
	UObject* __Owner = nullptr;
};

