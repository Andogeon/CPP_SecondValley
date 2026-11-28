#pragma once

#include "UE/Character.h"

class APlayer final : public ACharacter
{
public:
	APlayer();
	virtual ~APlayer() override = default;

protected:
	virtual void Tick(float _DeltaTime) override;
	virtual void TG_PostUpdateWork(float _DeltaTime) override;
	virtual void Render() override;

public:
	virtual void OnCollisionEnter(UCollider* _Other) override;
	virtual void OnCollisionStay(UCollider* _Other) override;
	virtual void OnCollisionExit(UCollider* _Other) override;

private:
	float		 __MoveSpeed = 300.0f;
};
