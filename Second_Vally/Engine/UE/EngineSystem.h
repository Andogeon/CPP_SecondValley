#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "Renderable.h"

class IEngineSystem : public FTickableGameObject, public IRenderable
{
public:
	virtual ~IEngineSystem() = default;
	virtual HRESULT Initialize() = 0;
	virtual void Deinitialize() = 0;
};