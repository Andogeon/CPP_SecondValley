#pragma once

#include "EngineSystem.h"
#include "T_Singleton.h"

template<typename T>
class TEngineSystem : public IEngineSystem, public TSingleton<T>
{
protected:
    TEngineSystem();
    virtual ~TEngineSystem() override = default;
};

#include "T_EngineSystem.inl"
