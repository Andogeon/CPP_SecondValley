#pragma once

#include "T_EngineSystem.h"
#include "Math/Vector.h"
#include "AssetResource.h"

#include <filesystem>
#include <mutex>
#include <vector>
#include <memory>
#include <unordered_map>

class Texture;
class Sound;
class Material;

class ENGINE_API ResourceEngineSystem final : public TEngineSystem<ResourceEngineSystem>
{
    DECLARE_SINGLETON(ResourceEngineSystem)

public:
	HRESULT Initialize() override;
	void Deinitialize() override;
	void Tick(float _DeltaTime) override { UNREFERENCED_PARAMETER(_DeltaTime); }
	void TG_PostUpdateWork(float _DeltaTime) override { UNREFERENCED_PARAMETER(_DeltaTime); }
	void Render() override {}

	void AddContentRoot(const FString& _ContentRoot);

	std::shared_ptr<Texture> LoadTexture(const FString& _RelativePath, FString* _Error = nullptr);
	std::shared_ptr<Sound> LoadSound(const FString& _RelativePath, FString* _Error = nullptr);
	std::shared_ptr<Material> LoadMaterial(const FString& _RelativePath, FString* _Error = nullptr);
	std::shared_ptr<Texture> FindTexture(const FString& _Key) const;
	std::shared_ptr<Sound> FindSound(const FString& _Key) const;
	std::shared_ptr<Material> FindMaterial(const FString& _Key) const;
	std::shared_ptr<Material> CreateMaterial(const FString& _Key, std::shared_ptr<Texture> _Texture = nullptr,
		COLORREF _Tint = RGB(255, 255, 255), float _Opacity = 1.0f);
	void ClearResources();

private:
    template<typename T>
    std::shared_ptr<T> LoadAsset(const FString& _RelativePath, FString* _Error);
    template<typename T>
    std::shared_ptr<T> FindAsset(const FString& _Key) const;
    static FString NormalizeResourcePath(const FString& _RelativePath);
    std::filesystem::path ResolveResourcePath(const std::filesystem::path& _RelativePath, FString* _Error = nullptr) const;
    mutable std::mutex __ResourceMutex;
    std::unordered_map<FString, std::shared_ptr<AssetResource>> __Resources;

private:
	mutable std::mutex __ContentRootMutex;
	std::vector<std::filesystem::path> __ContentRoots;
};

#include "ResourceEngineSystem.inl"
