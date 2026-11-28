#include "pch.h"
#include "ResourceEngineSystem.h"
#include "Engine.h"
#include "PathEngineSystem.h"
#include "Texture.h"
#include "Sound.h"
#include "Material.h"

#include <fstream>
#include <string>
#include <cwchar>

HRESULT ResourceEngineSystem::Initialize()
{
	FTickableGameObject::bCanEverTick = false;
    IRenderable::bCanEverRender = false;
    PathEngineSystem* Paths = PathEngineSystem::GetInstance();
	if (nullptr == Paths || Paths->GetProjectDirectory().empty())
		return E_UNEXPECTED;
	AddContentRoot(Paths->GetProjectContentDirectory().c_str());
	AddContentRoot(Paths->GetEngineContentDirectory().c_str());
	return S_OK;
}

void ResourceEngineSystem::Deinitialize()
{
	ClearResources();
	std::lock_guard<std::mutex> Lock(__ContentRootMutex);
	__ContentRoots.clear();
}

std::filesystem::path ResourceEngineSystem::ResolveResourcePath(const std::filesystem::path& _RelativePath, FString* _Error) const
{
    if (_Error) _Error->Empty();
    const auto Fail = [&](const wchar_t* _Message) -> std::filesystem::path
    {
        if (_Error) *_Error = _Message;
        return {};
    };
    if (_RelativePath.empty() || _RelativePath.has_root_path())
        return Fail(TEXT("Resource name must be relative to Content."));
    const auto Relative = _RelativePath.lexically_normal();
    if (Relative.filename().empty() || Relative == TEXT("."))
        return Fail(TEXT("Resource name must include a filename without its extension."));
    for (const auto& Part : Relative)
        if (Part == TEXT("..")) return Fail(TEXT("Resource name must remain inside Content."));
    std::vector<std::filesystem::path> Roots;
    {
        std::lock_guard<std::mutex> Lock(__ContentRootMutex);
        Roots = __ContentRoots;
    }
    for (const auto& Root : Roots)
    {
        std::error_code Error;
        const auto CanonicalRoot = std::filesystem::weakly_canonical(Root, Error);
        if (Error) continue;
        const auto Directory = std::filesystem::weakly_canonical(CanonicalRoot / Relative.parent_path(), Error);
        if (Error) continue;
        const auto Within = Directory.lexically_relative(CanonicalRoot);
        if (Within.empty() || Within.has_root_path()) continue;
        bool Escapes = false;
        for (const auto& Part : Within) if (Part == TEXT("..")) Escapes = true;
        if (Escapes) return Fail(TEXT("Resource directory must remain inside Content."));
        if (!std::filesystem::is_directory(Directory, Error))
        {
            if (Error && Error != std::errc::no_such_file_or_directory)
                return Fail(TEXT("Resource directory could not be inspected."));
            continue;
        }
        std::filesystem::directory_iterator Iter(Directory, Error), End;
        if (Error) return Fail(TEXT("Resource directory could not be scanned."));
        std::filesystem::path Match;
        for (; Iter != End; Iter.increment(Error))
        {
            if (Error) return Fail(TEXT("Resource directory could not be scanned."));
            if (_wcsicmp(Iter->path().stem().c_str(), Relative.filename().c_str()) != 0) continue;
            if (!Iter->is_regular_file(Error))
            {
                if (Error) return Fail(TEXT("Resource candidate could not be inspected."));
                continue;
            }
            if (!Match.empty())
                return Fail(TEXT("Multiple files have the requested resource name."));
            Match = std::filesystem::weakly_canonical(Iter->path(), Error);
            if (Error) return Fail(TEXT("Resource candidate could not be resolved."));
            const auto CandidateWithin = Match.lexically_relative(CanonicalRoot);
            if (CandidateWithin.empty() || CandidateWithin.has_root_path())
                return Fail(TEXT("Resource file must remain inside Content."));
            for (const auto& Part : CandidateWithin)
                if (Part == TEXT("..")) return Fail(TEXT("Resource file must remain inside Content."));
        }
        if (Error) return Fail(TEXT("Resource directory could not be scanned."));
        if (!Match.empty()) return Match;
    }
    return Fail(TEXT("No file has the requested resource name inside Content."));
}

FString ResourceEngineSystem::NormalizeResourcePath(const FString& _RelativePath)
{
    const std::filesystem::path Path(_RelativePath.ToWide());
    if (Path.empty() || Path.has_root_path() || _RelativePath.ToWide().find(L'\0') != std::wstring::npos) return {};
    const auto Normalized = Path.lexically_normal();
    for (const auto& Part : Normalized)
        if (Part == TEXT("..")) return {};
    if (Normalized.filename().empty() || Normalized == TEXT(".")) return {};
    return FString(Normalized.generic_wstring()).ToLower();
}

std::shared_ptr<Texture> ResourceEngineSystem::LoadTexture(const FString& _RelativePath, FString* _Error)
{
    return LoadAsset<Texture>(_RelativePath, _Error);
}

std::shared_ptr<Sound> ResourceEngineSystem::LoadSound(const FString& _RelativePath, FString* _Error)
{
    return LoadAsset<Sound>(_RelativePath, _Error);
}

std::shared_ptr<Material> ResourceEngineSystem::LoadMaterial(const FString& _RelativePath, FString* _Error)
{
    return LoadAsset<Material>(_RelativePath, _Error);
}

std::shared_ptr<Texture> ResourceEngineSystem::FindTexture(const FString& _Key) const { return FindAsset<Texture>(_Key); }
std::shared_ptr<Sound> ResourceEngineSystem::FindSound(const FString& _Key) const { return FindAsset<Sound>(_Key); }
std::shared_ptr<Material> ResourceEngineSystem::FindMaterial(const FString& _Key) const { return FindAsset<Material>(_Key); }

std::shared_ptr<Material> ResourceEngineSystem::CreateMaterial(const FString& _Key,
    std::shared_ptr<Texture> _Texture, COLORREF _Tint, float _Opacity)
{
    if (_Key.IsEmpty()) return nullptr;
    auto Resource = std::make_shared<Material>();
    if (!Resource->SetOpacity(_Opacity)) return nullptr;
    Resource->SetTexture(std::move(_Texture));
    Resource->SetTint(_Tint);
    Resource->__Key = _Key;
    std::lock_guard<std::mutex> Lock(__ResourceMutex);
    if (__Resources.find(_Key) != __Resources.end()) return nullptr;
    __Resources.emplace(_Key, Resource);
    return Resource;
}

void ResourceEngineSystem::ClearResources()
{
    std::unordered_map<FString, std::shared_ptr<AssetResource>> Resources;
    {
        std::lock_guard<std::mutex> Lock(__ResourceMutex);
        Resources.swap(__Resources);
    }
    for (const auto& Entry : Resources)
        if (const auto Audio = std::dynamic_pointer_cast<Sound>(Entry.second)) Audio->Stop();
}

void ResourceEngineSystem::AddContentRoot(const FString& _ContentRoot)
{
	if (_ContentRoot.IsEmpty())
		return;

	const std::filesystem::path Root(_ContentRoot.ToWide());
	std::lock_guard<std::mutex> Lock(__ContentRootMutex);
	for (const std::filesystem::path& ExistingRoot : __ContentRoots)
	{
		if (ExistingRoot == Root)
			return;
	}
	__ContentRoots.push_back(Root);
}
