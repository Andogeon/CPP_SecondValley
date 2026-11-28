#pragma once
#include "Actor.h"

class AController : public AActor
{
	friend class ULevel;

protected:
	AController();

public:

	virtual void Possess(class APawn* _NewPawn);
	virtual void UnPossess();
	virtual void Tick(float _DeltaTime) override;

protected:
	class APawn* __ControlledPawn;
};

