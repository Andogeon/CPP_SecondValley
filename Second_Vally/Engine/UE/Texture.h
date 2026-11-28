#pragma once

#include "AssetResource.h"
#include <Windows.h>
#include <memory>

class ENGINE_API Texture final : public AssetResource
{
public:
    Texture();
    ~Texture() override;
    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
    bool Draw(HDC _DC, const RECT& _Destination, COLORREF _Tint = RGB(255, 255, 255), float _Opacity = 1.0f, double _Rotation = 0.0) const;
    bool DrawTransparent(HDC _DC, const RECT& _Destination, COLORREF _ColorKey,
        COLORREF _Tint = RGB(255, 255, 255), float _Opacity = 1.0f, double _Rotation = 0.0) const;

protected:
    bool LoadResource(const std::filesystem::path& _FilePath, FString& _Error) override;

private:
    struct Implementation;
    std::unique_ptr<Implementation> __Implementation;
};
