#pragma once

#include "define.h"
#include <filesystem>
#include "UnrealString.h"

class ENGINE_API AssetResource
{
    friend class ResourceEngineSystem;

public:
    virtual ~AssetResource();
    AssetResource(const AssetResource&) = delete;
    AssetResource& operator=(const AssetResource&) = delete;
    const FString& GetKey() const { return __Key; }
    const FString& GetRelativePath() const { return __RelativePath; }

protected:
    AssetResource() = default;
    virtual bool LoadResource(const std::filesystem::path& _FilePath, FString& _Error) = 0;

private:
    FString __Key;
    FString __RelativePath;
    std::filesystem::path __FilePath;
};
