#include "pch.h"
#include "Texture.h"
#include <objidl.h>
#include <gdiplus.h>
#include <cmath>
#include <numbers>
#include <cstddef>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "msimg32.lib")

struct Texture::Implementation
{
    ULONG_PTR __Token = 0;
    std::unique_ptr<Gdiplus::Bitmap> __Bitmap;
    HDC __ColorKeyDC = nullptr;
    HBITMAP __ColorKeyBitmap = nullptr;
    HGDIOBJ __PreviousBitmap = nullptr;
    BYTE* __ColorKeyPixels = nullptr;
    COLORREF __CachedColorKey = 0;
    COLORREF __CachedTint = 0;
    bool __HasColorKeyCache = false;

    Implementation()
    {
        Gdiplus::GdiplusStartupInput Input;
        if (Gdiplus::GdiplusStartup(&__Token, &Input, nullptr) != Gdiplus::Ok)
            __Token = 0;
    }

    void ReleaseColorKeySurface()
    {
        if (__ColorKeyDC)
        {
            GdiFlush();
            if (__PreviousBitmap) SelectObject(__ColorKeyDC, __PreviousBitmap);
        }
        if (__ColorKeyBitmap) DeleteObject(__ColorKeyBitmap);
        if (__ColorKeyDC) DeleteDC(__ColorKeyDC);
        __ColorKeyDC = nullptr;
        __ColorKeyBitmap = nullptr;
        __PreviousBitmap = nullptr;
        __ColorKeyPixels = nullptr;
        __HasColorKeyCache = false;
    }

    ~Implementation()
    {
        ReleaseColorKeySurface();
        __Bitmap.reset();
        if (__Token) Gdiplus::GdiplusShutdown(__Token);
    }

    bool PrepareColorKey(COLORREF _ColorKey, COLORREF _Tint)
    {
        if (__HasColorKeyCache && __CachedColorKey == _ColorKey && __CachedTint == _Tint)
            return true;

        const UINT Width = __Bitmap->GetWidth(), Height = __Bitmap->GetHeight();
        if (!__ColorKeyDC)
        {
            BITMAPINFO Info = {};
            Info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            Info.bmiHeader.biWidth = static_cast<LONG>(Width);
            Info.bmiHeader.biHeight = -static_cast<LONG>(Height);
            Info.bmiHeader.biPlanes = 1;
            Info.bmiHeader.biBitCount = 32;
            Info.bmiHeader.biCompression = BI_RGB;
            void* Pixels = nullptr;
            HDC DC = CreateCompatibleDC(nullptr);
            HBITMAP Bitmap = CreateDIBSection(nullptr, &Info, DIB_RGB_COLORS, &Pixels, nullptr, 0);
            HGDIOBJ Previous = DC && Bitmap ? SelectObject(DC, Bitmap) : nullptr;
            if (!DC || !Bitmap || !Pixels || !Previous || Previous == HGDI_ERROR)
            {
                if (DC && Previous && Previous != HGDI_ERROR) SelectObject(DC, Previous);
                if (Bitmap) DeleteObject(Bitmap);
                if (DC) DeleteDC(DC);
                return false;
            }
            __ColorKeyDC = DC;
            __ColorKeyBitmap = Bitmap;
            __PreviousBitmap = Previous;
            __ColorKeyPixels = static_cast<BYTE*>(Pixels);
        }

        // Render Thread에서 이전 GDI 사용을 마친 뒤 캐시의 픽셀을 변경.
        if (!GdiFlush()) return false;
        Gdiplus::Rect Area(0, 0, static_cast<INT>(Width), static_cast<INT>(Height));
        Gdiplus::BitmapData Data = {};
        if (__Bitmap->LockBits(&Area, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &Data) != Gdiplus::Ok)
            return false;

        __HasColorKeyCache = false;
        const unsigned int Tint[3] = { GetBValue(_Tint), GetGValue(_Tint), GetRValue(_Tint) };
        for (UINT Y = 0; Y < Height; ++Y)
        {
            const BYTE* Source = static_cast<const BYTE*>(Data.Scan0) + static_cast<std::ptrdiff_t>(Y) * Data.Stride;
            BYTE* Target = __ColorKeyPixels + static_cast<size_t>(Y) * Width * 4;
            for (UINT X = 0; X < Width; ++X, Source += 4, Target += 4)
            {
                const bool Keyed = RGB(Source[2], Source[1], Source[0]) == _ColorKey;
                const unsigned int Alpha = Keyed ? 0 : Source[3];
                // 원본 RGB로 ColorKey를 비교한 뒤 Tint와 픽셀 Alpha를 곱해 premultiplied BGRA 준비.
                for (UINT Channel = 0; Channel < 3; ++Channel)
                    Target[Channel] = static_cast<BYTE>((Source[Channel] * Tint[Channel] * Alpha + 32512u) / 65025u);
                Target[3] = static_cast<BYTE>(Alpha);
            }
        }
        if (__Bitmap->UnlockBits(&Data) != Gdiplus::Ok) return false;
        __CachedColorKey = _ColorKey;
        __CachedTint = _Tint;
        __HasColorKeyCache = true;
        return true;
    }
};

Texture::Texture() : __Implementation(std::make_unique<Implementation>()) {}
Texture::~Texture() = default;

bool Texture::LoadResource(const std::filesystem::path& _FilePath, FString& _Error)
{
    if (!__Implementation->__Token)
    {
        _Error = TEXT("GDI+ initialization failed.");
        return false;
    }
    Gdiplus::Bitmap Source(_FilePath.c_str());
    if (Source.GetLastStatus() != Gdiplus::Ok || !Source.GetWidth() || !Source.GetHeight() ||
        Source.GetWidth() > 16384 || Source.GetHeight() > 16384 ||
        static_cast<unsigned long long>(Source.GetWidth()) * Source.GetHeight() > 67108864)
    {
        _Error = TEXT("Invalid texture or texture exceeds the supported dimensions.");
        return false;
    }
    std::unique_ptr<Gdiplus::Bitmap> Bitmap(Source.Clone(0, 0,
        static_cast<INT>(Source.GetWidth()), static_cast<INT>(Source.GetHeight()), PixelFormat32bppARGB));
    if (!Bitmap || Bitmap->GetLastStatus() != Gdiplus::Ok)
    {
        _Error = TEXT("Texture decoding failed.");
        return false;
    }
    __Implementation->ReleaseColorKeySurface();
    __Implementation->__Bitmap = std::move(Bitmap);
    return true;
}

unsigned int Texture::GetWidth() const
{
    return __Implementation->__Bitmap ? __Implementation->__Bitmap->GetWidth() : 0;
}

bool Texture::DrawTransparent(HDC _DC, const RECT& _Destination, COLORREF _ColorKey, COLORREF _Tint, float _Opacity, double _Rotation) const
{
    const long long Width = static_cast<long long>(_Destination.right) - _Destination.left;
    const long long Height = static_cast<long long>(_Destination.bottom) - _Destination.top;
    if (!_DC || !__Implementation->__Bitmap || !std::isfinite(_Opacity) || _Opacity < 0.0f || _Opacity > 1.0f ||
        !std::isfinite(_Rotation) || Width <= 0 || Height <= 0 || Width > INT_MAX || Height > INT_MAX) return false;
    if (_Opacity == 0.0f) return true;
    if (!__Implementation->PrepareColorKey(_ColorKey, _Tint)) return false;

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

    const BLENDFUNCTION Blend = { AC_SRC_OVER, 0, static_cast<BYTE>(std::lround(_Opacity * 255.0f)), AC_SRC_ALPHA };
    // 전체 Opacity는 합성 시 한 번만 적용. Opacity만 바뀌면 픽셀 캐시를 다시 계산하지 않음.
    return AlphaBlend(_DC, _Destination.left, _Destination.top, static_cast<int>(Width), static_cast<int>(Height),
        __Implementation->__ColorKeyDC, 0, 0, static_cast<int>(GetWidth()), static_cast<int>(GetHeight()), Blend) != FALSE;
}

unsigned int Texture::GetHeight() const
{
    return __Implementation->__Bitmap ? __Implementation->__Bitmap->GetHeight() : 0;
}

bool Texture::Draw(HDC _DC, const RECT& _Destination, COLORREF _Tint, float _Opacity, double _Rotation) const
{
    const long long Width = static_cast<long long>(_Destination.right) - _Destination.left;
    const long long Height = static_cast<long long>(_Destination.bottom) - _Destination.top;
    if (!_DC || !__Implementation->__Bitmap || !std::isfinite(_Opacity) || _Opacity < 0.0f || _Opacity > 1.0f ||
        !std::isfinite(_Rotation) || Width <= 0 || Height <= 0 || Width > INT_MAX || Height > INT_MAX) return false;
    if (_Opacity == 0.0f) return true;
    Gdiplus::Graphics Graphics(_DC);
    Graphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    Gdiplus::ColorMatrix Matrix = {};
    Matrix.m[0][0] = GetRValue(_Tint) / 255.0f;
    Matrix.m[1][1] = GetGValue(_Tint) / 255.0f;
    Matrix.m[2][2] = GetBValue(_Tint) / 255.0f;
    Matrix.m[3][3] = _Opacity;
    Matrix.m[4][4] = 1.0f;
    Gdiplus::ImageAttributes Attributes;
    Attributes.SetColorMatrix(&Matrix);
    Attributes.SetWrapMode(Gdiplus::WrapModeTileFlipXY);
    const double Rotation = std::remainder(_Rotation, 360.0);
    if (Rotation == 0.0)
        return Graphics.DrawImage(__Implementation->__Bitmap.get(),
            Gdiplus::Rect(_Destination.left, _Destination.top, static_cast<INT>(Width), static_cast<INT>(Height)),
            0, 0, static_cast<INT>(GetWidth()), static_cast<INT>(GetHeight()), Gdiplus::UnitPixel, &Attributes) == Gdiplus::Ok;

    const double HalfWidth = Width * 0.5, HalfHeight = Height * 0.5;
    const double CenterX = _Destination.left + HalfWidth, CenterY = _Destination.top + HalfHeight;
    const double Radian = Rotation * std::numbers::pi / 180.0;
    const double Cos = std::cos(Radian), Sin = std::sin(Radian);
    const auto Rotate = [&](double _X, double _Y)
    {
        return Gdiplus::PointF(static_cast<Gdiplus::REAL>(CenterX + Cos * _X - Sin * _Y),
            static_cast<Gdiplus::REAL>(CenterY + Sin * _X + Cos * _Y));
    };
    const Gdiplus::PointF Points[] = {
        Rotate(-HalfWidth, -HalfHeight), Rotate(HalfWidth, -HalfHeight), Rotate(-HalfWidth, HalfHeight) };
    return Graphics.DrawImage(__Implementation->__Bitmap.get(), Points, 3, 0.0f, 0.0f,
        static_cast<Gdiplus::REAL>(GetWidth()), static_cast<Gdiplus::REAL>(GetHeight()),
        Gdiplus::UnitPixel, &Attributes) == Gdiplus::Ok;
}
