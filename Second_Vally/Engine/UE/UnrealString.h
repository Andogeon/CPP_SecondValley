#pragma once

#include "UE/CoreTypes.h"

#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ESearchCase
{
    enum Type
    {
        CaseSensitive,
        IgnoreCase
    };
}

namespace ESearchDir
{
    enum Type
    {
        FromStart,
        FromEnd
    };
}

class FString
{
public:
    using CharType = TCHAR;
    using SizeType = std::wstring::size_type;

    FString() = default;
    FString(const FString&) = default;
    FString(FString&&) noexcept = default;
    FString& operator=(const FString&) = default;
    FString& operator=(FString&&) noexcept = default;

    CORE_API FString(const TCHAR* _Text);
    CORE_API FString(std::wstring _Text);
    CORE_API FString(std::wstring_view _Text);

    CORE_API FString(const char* _Utf8Text);
    CORE_API FString(const std::string& _Utf8Text);
    CORE_API FString(std::string_view _Utf8Text);

    [[nodiscard]] CORE_API const TCHAR* c_str() const noexcept;
    [[nodiscard]] CORE_API const TCHAR* operator*() const noexcept;

    [[nodiscard]] CORE_API int32 Len() const noexcept;
    [[nodiscard]] CORE_API bool IsEmpty() const noexcept;

    CORE_API void Empty(int32 _Slack = 0);
    CORE_API void Reset(int32 _NewReservedSize = 0);
    CORE_API void Reserve(int32 _CharacterCount);

    FString& Append(const FString& _Other);
    FString& Append(const TCHAR* _Text);
    FString& Append(const TCHAR* _Text, int32 _Count);

    [[nodiscard]] bool Contains(
        const FString& _SubStr,
        ESearchCase::Type _SearchCase = ESearchCase::IgnoreCase) const;
    [[nodiscard]] bool Contains(
        const TCHAR* _SubStr,
        ESearchCase::Type _SearchCase = ESearchCase::IgnoreCase) const;

    [[nodiscard]] bool StartsWith(
        const FString& _Prefix,
        ESearchCase::Type _SearchCase = ESearchCase::IgnoreCase) const;
    [[nodiscard]] bool StartsWith(
        const TCHAR* _Prefix,
        ESearchCase::Type _SearchCase = ESearchCase::IgnoreCase) const;

    [[nodiscard]] bool EndsWith(
        const FString& _Suffix,
        ESearchCase::Type _SearchCase = ESearchCase::IgnoreCase) const;
    [[nodiscard]] bool EndsWith(
        const TCHAR* _Suffix,
        ESearchCase::Type _SearchCase = ESearchCase::IgnoreCase) const;

    [[nodiscard]] int32 Find(
        const FString& _SubStr,
        ESearchCase::Type _SearchCase = ESearchCase::IgnoreCase,
        ESearchDir::Type _SearchDir = ESearchDir::FromStart,
        int32 _StartPosition = INDEX_NONE) const;
    [[nodiscard]] int32 Find(
        const TCHAR* _SubStr,
        ESearchCase::Type _SearchCase = ESearchCase::IgnoreCase,
        ESearchDir::Type _SearchDir = ESearchDir::FromStart,
        int32 _StartPosition = INDEX_NONE) const;

    [[nodiscard]] FString Replace(
        const TCHAR* _From,
        const TCHAR* _To,
        ESearchCase::Type _SearchCase = ESearchCase::IgnoreCase) const;
    int32 ReplaceInline(
        const TCHAR* _SearchText,
        const TCHAR* _ReplacementText,
        ESearchCase::Type _SearchCase = ESearchCase::IgnoreCase);

    [[nodiscard]] bool Equals(
        const FString& _Other,
        ESearchCase::Type _SearchCase = ESearchCase::CaseSensitive) const;

    [[nodiscard]] FString ToUpper() const;
    [[nodiscard]] FString ToLower() const;
    [[nodiscard]] FString TrimStartAndEnd() const;

    int32 ParseIntoArray(
        std::vector<FString>& _OutArray,
        const TCHAR* _Delimiter,
        bool _CullEmpty = true) const;

    [[nodiscard]] std::vector<TCHAR> GetCharArray() const;
    [[nodiscard]] FString Left(int32 _Count) const;
    [[nodiscard]] FString Right(int32 _Count) const;
    [[nodiscard]] FString Mid(int32 _Start, int32 _Count = INT32_MAX) const;

    [[nodiscard]] CORE_API const std::wstring& ToWide() const noexcept;
    [[nodiscard]] CORE_API std::string ToUtf8() const;

    FString& operator+=(const FString& _Other);
    FString& operator+=(const TCHAR* _Other);

    friend FString operator+(FString _Left, const FString& _Right)
    {
        _Left += _Right;
        return _Left;
    }

    friend bool operator==(const FString& _Left, const FString& _Right) noexcept
    {
        return _Left.__Data == _Right.__Data;
    }

    friend bool operator!=(const FString& _Left, const FString& _Right) noexcept
    {
        return !(_Left == _Right);
    }

    [[nodiscard]] static FString FromUtf8(std::string_view _Utf8Text);

    template <typename... Args>
    [[nodiscard]] static FString Printf(const TCHAR* _Format, Args... _Values);

private:
    static std::wstring Utf8ToWide(std::string_view _Utf8Text);
    static std::string WideToUtf8(std::wstring_view _WideText);

    std::wstring __Data;
};

#include "UE/UnrealString.inl"

namespace std
{
    template <>
    struct hash<FString>
    {
        [[nodiscard]] std::size_t operator()(const FString& _Value) const noexcept
        {
            return std::hash<std::wstring>{}(_Value.ToWide());
        }
    };
}
