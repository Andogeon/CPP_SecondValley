#pragma once

#include "CombatCharacter.h"
#include "UE/define.h"



class APlayer final : public ACombatCharacter
{
	friend class ULevel;

private:
	APlayer();

public:
	~APlayer() override = default;

protected:
	void BeginPlay() override;
	void EndPlay() override;
	void Tick(float _DeltaTime) override;

private:
	void OpenNextLevel();
	void UpdateChargeShot(float _DeltaTime);
	void CreateBullet(float _ChargeRatio);
	void UpdateChargingBullet(float _ChargeRatio);
	void UpdateBodyRotation(float _DeltaTime);
	void UpdateAim(float _DeltaTime);
	FVector2D GetAimDirection() const;
	void Render() override;

	bool __bCameraGesture = false;
	float __MoveSpeed = 300.0f;
	static constexpr float __ChargeStartDelay = 0.5f;
	static constexpr float __MaxChargeTime = 1.4f;
	static constexpr float __MaxBulletScale = 30.0f;
	static constexpr double __BodyRotateSpeed = 90.0;
	float __AimTheta = 90.0f;
	static constexpr float __AimRotateSpeed = 90.0f;
};
