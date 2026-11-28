#pragma once


class FTickableGameObject
{
public:
	virtual ~FTickableGameObject() = default;

    virtual void Tick(float _DeltaTime) = 0;
    virtual void TG_PostUpdateWork(float _DeltaTime) = 0;
    bool IsTickable() const { return bCanEverTick; }

protected:
    bool    bCanEverTick = true;
};
