#pragma once

#include "T_EngineSystem.h"
#include <array>
#include <cstdint>

enum class EKeyState : uint8_t
{
	None,
	Pressed,
	Repeat,
	Released
};

enum class EKey : uint16_t
{
	Invalid = 0,

	Escape,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,
	PrintScreen,
	ScrollLock,
	Pause,

	Grave,
	Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
	Minus,
	Equals,
	Backspace,
	Tab,
	Q, W, E, R, T, Y, U, I, O, P,
	LeftBracket,
	RightBracket,
	Backslash,
	CapsLock,
	A, S, D, F, G, H, J, K, L,
	Semicolon,
	Apostrophe,
	Enter,
	LeftShift,
	Z, X, C, V, B, N, M,
	Comma,
	Period,
	Slash,
	RightShift,
	LeftControl,
	LeftWindows,
	LeftAlt,
	SpaceBar,
	RightAlt,
	RightWindows,
	Application,
	RightControl,

	Insert,
	Delete,
	Home,
	End,
	PageUp,
	PageDown,
	Left,
	Right,
	Up,
	Down,

	NumLock,
	NumpadDivide,
	NumpadMultiply,
	NumpadSubtract,
	NumpadAdd,
	NumpadDecimal,
	Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
	Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,

	Hangul,
	Hanja,
	OEM102,

	BrowserBack,
	BrowserForward,
	BrowserRefresh,
	BrowserStop,
	BrowserSearch,
	BrowserFavorites,
	BrowserHome,
	VolumeMute,
	VolumeDown,
	VolumeUp,
	MediaNextTrack,
	MediaPreviousTrack,
	MediaStop,
	MediaPlayPause,
	LaunchMail,
	LaunchMediaSelect,
	LaunchApplication1,
	LaunchApplication2,

	LeftMouseButton,
	RightMouseButton,
	MiddleMouseButton,
	ThumbMouseButton,
	ThumbMouseButton2,
	MouseX,
	MouseY,
	MouseWheelAxis,

	Count
};

struct FKeyStateData
{
	EKeyState State = EKeyState::None;
	bool bPrevActive = false;
};

class ENGINE_API InputEngineSystem : public TEngineSystem<InputEngineSystem>
{
	DECLARE_SINGLETON(InputEngineSystem)

public:
	HRESULT Initialize() override;
	void Deinitialize() override;

	void Tick(float _DeltaTime) override;
	void TG_PostUpdateWork(float _DeltaTime) override;
	void Render() override;

	bool IsKeyPressed(EKey _Key) const;
	bool IsKeyRepeat(EKey _Key) const;
	bool IsKeyReleased(EKey _Key) const;

	EKeyState GetKeyState(EKey _Key) const;
	bool GetMousePosition(FVector2D& _Position) const;

private:
	static int ToVirtualKey(EKey _Key);

	static constexpr size_t KeyCount = static_cast<size_t>(EKey::Count);

	std::array<FKeyStateData, KeyCount> __KeyStates = {};
	FVector2D __MousePosition = {};
	bool __bMousePositionValid = false;
};
