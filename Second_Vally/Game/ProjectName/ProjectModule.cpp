#include "pch.h"
#include "ProjectName.h"
#include "UE/ModuleInterface.h"

class FProjectModule final : public IModuleInterface
{
public:
    void DeinitializeProject() override { ::DeinitializeProject(); }
    bool InitializeProject() override { return ::InitializeProject(); }
};

extern "C" __declspec(dllexport) unsigned int GetModuleApiVersion() { return GameModuleApiVersion; }
extern "C" __declspec(dllexport) IModuleInterface* InitializeModule() { return new FProjectModule(); }
