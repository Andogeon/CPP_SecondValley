#pragma once

#include "define.h"

class IModuleInterface
{
public:
    virtual ~IModuleInterface() = default;
    virtual void DeinitializeProject() = 0;
    virtual bool InitializeProject() = 0;
};

inline constexpr unsigned int GameModuleApiVersion = 0x20004;
