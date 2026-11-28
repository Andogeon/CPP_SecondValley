#pragma once

#include "AssetResource.h"
#include <Windows.h>
#include <memory>

class Texture;

class ENGINE_API Material final : public AssetResource
{
public:
    Material() = default;
    ~Material() override;
    void SetTexture(std::shared_ptr<Texture> _Texture);
    std::shared_ptr<Texture> GetTexture() const { return __Texture; }
    void SetTint(COLORREF _Tint) { __Tint = _Tint; }
    COLORREF GetTint() const { return __Tint; }
    bool SetOpacity(float _Opacity);
    float GetOpacity() const { return __Opacity; }
    bool SetColorKey(COLORREF _ColorKey);
    void ClearColorKey() { __UseColorKey = false; }
    bool UsesColorKey() const { return __UseColorKey; }
    COLORREF GetColorKey() const { return __ColorKey; }
    bool Draw(HDC _DC, const RECT& _Destination, double _Rotation = 0.0) const;

protected:
    bool LoadResource(const std::filesystem::path& _FilePath, FString& _Error) override;

private:
    std::shared_ptr<Texture> __Texture;
    COLORREF __Tint = RGB(255, 255, 255);
    float __Opacity = 1.0f;
    bool __UseColorKey = false;
    COLORREF __ColorKey = RGB(255, 0, 255);
};
