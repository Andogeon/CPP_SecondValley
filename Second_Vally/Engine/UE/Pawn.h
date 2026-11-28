#pragma once
#include "Actor.h"

class ENGINE_API APawn : public AActor
{
	friend class ULevel;

protected:
	APawn();

public:
	virtual ~APawn() override;

protected:
	virtual void Tick(float _DeltaTime) override;
	virtual void TG_PostUpdateWork(float _DeltaTime) override;
	virtual void Render() override;
};

