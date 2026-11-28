#include "pch.h"
#include "ModuleManager.h"
#include "ThirdParty/WIL/include/wil/resource.h"

namespace
{
    IModuleInterface* ActiveGameModule = nullptr;
}

FModuleManager::FModuleManager() = default;

FModuleManager::~FModuleManager()
{
    if (ActiveGameModule == __GameModule)
        ActiveGameModule = nullptr;
    delete __GameModule;
    if (__Library)
        FreeLibrary(static_cast<HMODULE>(__Library));
}

bool FModuleManager::LoadGameModule(const std::filesystem::path& _ModuleFile)
{
    if (__Library || ActiveGameModule)
        return false;

    std::error_code Error;
    const std::filesystem::path ModuleFile = std::filesystem::absolute(_ModuleFile, Error);
    if (Error || !std::filesystem::is_regular_file(ModuleFile, Error))
        return false;

    wil::unique_hmodule Library(LoadLibraryExW(ModuleFile.c_str(), nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS));
    if (!Library)
        return false;

    using FGetModuleApiVersion = unsigned int (*)();
    using FInitializeModule = IModuleInterface* (*)();
    const auto GetVersion = reinterpret_cast<FGetModuleApiVersion>(
        GetProcAddress(Library.get(), "GetModuleApiVersion"));
    const auto InitializeModule = reinterpret_cast<FInitializeModule>(
        GetProcAddress(Library.get(), "InitializeModule"));
    if (!GetVersion || GetVersion() != GameModuleApiVersion || !InitializeModule)
        return false;

    IModuleInterface* Module = InitializeModule();
    if (!Module)
        return false;

    __Library = Library.release();
    __GameModule = Module;
    __LoadedModuleFile = ModuleFile.lexically_normal();
    ActiveGameModule = Module;
    return true;
}

IModuleInterface* FModuleManager::GetGameModule()
{
    return ActiveGameModule;
}

const std::filesystem::path& FModuleManager::GetLoadedModuleFile() const
{
    return __LoadedModuleFile;
}
