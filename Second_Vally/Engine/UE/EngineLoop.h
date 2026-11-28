#pragma once

#include "CoreMinimal.h"

class IEngineLoop
{
public:
    virtual ~IEngineLoop() = default;

    virtual HRESULT Initialize(POINT _Resolution = {}) PURE;
    virtual void Deinitialize() PURE;
    virtual void Progress() PURE;
};
