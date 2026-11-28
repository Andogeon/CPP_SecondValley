#pragma once

#include "CoreMinimal.h"

class IEngineLoop
{
public:
    virtual ~IEngineLoop() = default;

    virtual HRESULT Initialize(POINT _Resolution = {}) = 0;
    virtual void Deinitialize() = 0;
    virtual void Progress() = 0;
};
