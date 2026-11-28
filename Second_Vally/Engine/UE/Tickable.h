#pragma once

#include "define.h"

class FTickableGameObject
{
public:
	virtual ~FTickableGameObject() = default;

    virtual void Tick(float _DeltaTime) PURE;
    virtual void TG_PostUpdateWork(float _DeltaTime) PURE;
    bool IsTickable() const { return bCanEverTick; }

protected:
    bool    bCanEverTick = true;
};
