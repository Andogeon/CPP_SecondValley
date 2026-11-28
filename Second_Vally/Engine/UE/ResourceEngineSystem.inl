template<typename T>
std::shared_ptr<T> ResourceEngineSystem::LoadAsset(const FString& _RelativePath, FString* _Error)
{
    if (_Error) _Error->Empty();
    const auto Fail = [&](const FString& _Message) -> std::shared_ptr<T>
    {
        if (_Error) *_Error = _Message;
        return nullptr;
    };
    try
    {
        const FString Key = NormalizeResourcePath(_RelativePath);
        if (Key.IsEmpty()) return Fail(TEXT("Resource name must be relative to Content."));
        FString ResolveError;
        const auto FilePath = ResolveResourcePath(std::filesystem::path(_RelativePath.ToWide()), &ResolveError);
        if (FilePath.empty()) return Fail(ResolveError);
        {
            std::lock_guard<std::mutex> Lock(__ResourceMutex);
            const auto Found = __Resources.find(Key);
            if (Found != __Resources.end())
            {
                const auto Resource = std::dynamic_pointer_cast<T>(Found->second);
                if (!Resource || Found->second->__FilePath != FilePath)
                    return Fail(TEXT("Resource key already refers to another type or file."));
                return Resource;
            }
        }
        auto Resource = std::make_shared<T>();
        FString Error;
        AssetResource& Base = *Resource;
        if (!Base.LoadResource(FilePath, Error)) return Fail(Error);
        Base.__Key = Key;
        Base.__RelativePath = FString(std::filesystem::path(_RelativePath.ToWide()).lexically_normal().generic_wstring());
        Base.__FilePath = FilePath;
        std::lock_guard<std::mutex> Lock(__ResourceMutex);
        const auto Result = __Resources.emplace(Key, Resource);
        if (!Result.second)
        {
            auto Existing = std::dynamic_pointer_cast<T>(Result.first->second);
            if (!Existing || Result.first->second->__FilePath != FilePath)
                return Fail(TEXT("Resource key already refers to another type or file."));
            return Existing;
        }
        return Resource;
    }
    catch (const std::exception&)
    {
        return Fail(TEXT("Resource loading failed while reading or decoding the file."));
    }
}

template<typename T>
std::shared_ptr<T> ResourceEngineSystem::FindAsset(const FString& _Key) const
{
    FString PathKey;
    try { PathKey = NormalizeResourcePath(_Key); }
    catch (const std::exception&) { return nullptr; }
    std::lock_guard<std::mutex> Lock(__ResourceMutex);
    auto Found = __Resources.find(_Key);
    if (Found == __Resources.end() && !PathKey.IsEmpty()) Found = __Resources.find(PathKey);
    return Found == __Resources.end() ? nullptr : std::dynamic_pointer_cast<T>(Found->second);
}
