#pragma once
#include "Pawn.h"

class ENGINE_API ACharacter : public APawn
{
	friend class ULevel;

protected:
	ACharacter();

public:
	virtual ~ACharacter() override;

protected:
	virtual void Tick(float _DeltaTime) override;
	virtual void TG_PostUpdateWork(float _DeltaTime) override;
	virtual void Render() override;
};

