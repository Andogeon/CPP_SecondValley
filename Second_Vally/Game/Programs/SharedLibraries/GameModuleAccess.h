#pragma once

#include "UE/ModuleManager.h"

inline void DeinitializeProject()
{
    if (auto* Module = FModuleManager::GetGameModule()) Module->DeinitializeProject();
}

inline bool InitializeProject()
{
    if (auto* Module = FModuleManager::GetGameModule()) return Module->InitializeProject();
    return false;
}
