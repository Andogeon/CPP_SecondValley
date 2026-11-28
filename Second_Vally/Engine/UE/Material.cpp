#include "pch.h"
#include "Material.h"
#include "Texture.h"
#include "ResourceEngineSystem.h"
#include <cmath>
#include <numbers>
#include <fstream>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "msimg32.lib")

Material::~Material() = default;

void Material::SetTexture(std::shared_ptr<Texture> _Texture)
{
    __Texture = std::move(_Texture);
}

bool Material::SetOpacity(float _Opacity)
{
    if (!std::isfinite(_Opacity) || _Opacity < 0.0f || _Opacity > 1.0f) return false;
    __Opacity = _Opacity;
    return true;
}

bool Material::SetColorKey(COLORREF _ColorKey)
{
    __UseColorKey = true;
    __ColorKey = _ColorKey;
    return true;
}

bool Material::LoadResource(const std::filesystem::path& _FilePath, FString& _Error)
{
    _Error = TEXT("Invalid Material 1 file.");
    std::ifstream Input(_FilePath);
    if (!Input) return false;
    if (Input.peek() == 0xEF)
    {
        char BOM[3] = {};
        if (!Input.read(BOM, 3) || static_cast<unsigned char>(BOM[1]) != 0xBB ||
            static_cast<unsigned char>(BOM[2]) != 0xBF) return false;
    }
    std::string Line;
    if (!std::getline(Input, Line)) return false;
    std::istringstream Header(Line);
    std::string Tag;
    int Version = 0;
    if (!(Header >> Tag >> Version) || Tag != "Material" || Version != 1 || !(Header >> std::ws).eof()) return false;
    std::string TexturePath;
    COLORREF Tint = RGB(255, 255, 255);
    float Opacity = 1.0f;
    bool HasTexture = false, HasTint = false, HasOpacity = false;
    bool HasColorKey = false;
    COLORREF ColorKey = RGB(255, 0, 255);
    while (std::getline(Input, Line))
    {
        std::istringstream Row(Line);
        if (!(Row >> Tag)) continue;
        if (Tag.starts_with("//")) continue;
        if (Tag == "Texture" && !HasTexture)
        {
            HasTexture = true;
            Row >> std::ws;
            if (Row.peek() != '"' || !(Row >> std::quoted(TexturePath))) return false;
        }
        else if (Tag == "Tint" && !HasTint)
        {
            HasTint = true;
            int R = 0, G = 0, B = 0;
            if (!(Row >> R >> G >> B) || R < 0 || R > 255 || G < 0 || G > 255 || B < 0 || B > 255) return false;
            Tint = RGB(R, G, B);
        }
        else if (Tag == "ColorKey" && !HasColorKey)
        {
            HasColorKey = true;
            int R = 0, G = 0, B = 0;
            if (!(Row >> R >> G >> B) || R < 0 || R > 255 || G < 0 || G > 255 || B < 0 || B > 255) return false;
            ColorKey = RGB(R, G, B);
        }
        else if (Tag == "Opacity" && !HasOpacity)
        {
            HasOpacity = true;
            if (!(Row >> Opacity) || !std::isfinite(Opacity) || Opacity < 0.0f || Opacity > 1.0f) return false;
        }
        else return false;
        if (!(Row >> std::ws).eof()) return false;
    }
    if (Input.bad()) return false;
    if (HasColorKey && TexturePath.empty())
    {
        _Error = TEXT("ColorKey requires a texture.");
        return false;
    }
    std::shared_ptr<Texture> LoadedTexture;
    if (!TexturePath.empty())
    {
        if (TexturePath.size() > 32767 || TexturePath.find('\0') != std::string::npos) return false;
        const int Length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
            TexturePath.data(), static_cast<int>(TexturePath.size()), nullptr, 0);
        if (!Length) return false;
        std::wstring WidePath(static_cast<size_t>(Length), L'\0');
        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, TexturePath.data(),
            static_cast<int>(TexturePath.size()), WidePath.data(), Length) != Length) return false;
        const FString RelativePath(std::filesystem::path(WidePath).generic_wstring());
        LoadedTexture = ResourceEngineSystem::GetInstance()->LoadTexture(
            RelativePath, &_Error);
        if (!LoadedTexture) return false;
    }
    __Texture = std::move(LoadedTexture);
    __Tint = Tint;
    __Opacity = Opacity;
    __UseColorKey = HasColorKey;
    __ColorKey = ColorKey;
    _Error.Empty();
    return true;
}

bool Material::Draw(HDC _DC, const RECT& _Destination, double _Rotation) const
{
    const long long Width = static_cast<long long>(_Destination.right) - _Destination.left;
    const long long Height = static_cast<long long>(_Destination.bottom) - _Destination.top;
    if (!_DC || !std::isfinite(_Rotation) || Width <= 0 || Height <= 0 || Width > INT_MAX || Height > INT_MAX) return false;
    if (__UseColorKey)
        return __Texture && __Texture->DrawTransparent(_DC, _Destination, __ColorKey, __Tint, __Opacity, _Rotation);
    if (__Texture) return __Texture->Draw(_DC, _Destination, __Tint, __Opacity, _Rotation);
    if (__Opacity == 0.0f) return true;
    struct FRestoreDC
    {
        HDC DC;
        int Saved = 0;
        ~FRestoreDC() { if (Saved) RestoreDC(DC, Saved); }
    } Restore{ _DC };
    const double Rotation = std::remainder(_Rotation, 360.0);
    if (Rotation != 0.0)
    {
        Restore.Saved = SaveDC(_DC);
        if (!Restore.Saved || !SetGraphicsMode(_DC, GM_ADVANCED)) return false;
        const double Radian = Rotation * std::numbers::pi / 180.0;
        const double Cos = std::cos(Radian), Sin = std::sin(Radian);
        const double CenterX = _Destination.left + Width * 0.5;
        const double CenterY = _Destination.top + Height * 0.5;
        const XFORM Transform = { static_cast<FLOAT>(Cos), static_cast<FLOAT>(Sin),
            static_cast<FLOAT>(-Sin), static_cast<FLOAT>(Cos),
            static_cast<FLOAT>(CenterX - Cos * CenterX + Sin * CenterY),
            static_cast<FLOAT>(CenterY - Sin * CenterX - Cos * CenterY) };
        if (!ModifyWorldTransform(_DC, &Transform, MWT_LEFTMULTIPLY)) return false;
    }
    if (__Opacity == 1.0f)
    {
        HBRUSH Brush = CreateSolidBrush(__Tint);
        if (!Brush) return false;
        const bool Drawn = FillRect(_DC, &_Destination, Brush) != 0;
        DeleteObject(Brush);
        return Drawn;
    }
    BITMAPINFO Info = {};
    Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    Info.bmiHeader.biWidth = 1;
    Info.bmiHeader.biHeight = -1;
    Info.bmiHeader.biPlanes = 1;
    Info.bmiHeader.biBitCount = 32;
    Info.bmiHeader.biCompression = BI_RGB;
    void* Pixel = nullptr;
    HBITMAP Bitmap = CreateDIBSection(_DC, &Info, DIB_RGB_COLORS, &Pixel, nullptr, 0);
    HDC Source = CreateCompatibleDC(_DC);
    bool Drawn = false;
    if (Bitmap && Source && Pixel)
    {
        *static_cast<DWORD*>(Pixel) = (GetRValue(__Tint) << 16) | (GetGValue(__Tint) << 8) | GetBValue(__Tint);
        HGDIOBJ Previous = SelectObject(Source, Bitmap);
        if (Previous && Previous != HGDI_ERROR)
        {
            BLENDFUNCTION Blend = { AC_SRC_OVER, 0, static_cast<BYTE>(__Opacity * 255.0f), 0 };
            Drawn = AlphaBlend(_DC, _Destination.left, _Destination.top,
                _Destination.right - _Destination.left, _Destination.bottom - _Destination.top,
                Source, 0, 0, 1, 1, Blend) != FALSE;
            SelectObject(Source, Previous);
        }
    }
    if (Source) DeleteDC(Source);
    if (Bitmap) DeleteObject(Bitmap);
    return Drawn;
}
