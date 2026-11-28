#include "pch.h"
#include "Controller.h"

AController::AController()
	: __ControlledPawn(nullptr)
{
}

void AController::Possess(APawn* _NewPawn)
{
	if (__ControlledPawn == _NewPawn)
		return;

	__ControlledPawn = _NewPawn;
}

void AController::UnPossess()
{
	__ControlledPawn = nullptr;
}

void AController::Tick(float _DeltaTime)
{
	if (__ControlledPawn)
	{
		
		
	}
}
