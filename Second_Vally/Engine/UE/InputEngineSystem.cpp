#include "pch.h"
#include "InputEngineSystem.h"
#include "LaunchEngineLoop.h"

HRESULT InputEngineSystem::Initialize()
{
	__KeyStates.fill(FKeyStateData{});
	__MousePosition = {};
	__bMousePositionValid = false;

	FTickableGameObject::bCanEverTick = true;
	IRenderable::bCanEverRender = false;

	return S_OK;
}

void InputEngineSystem::Deinitialize()
{
	__KeyStates.fill(FKeyStateData{});
	__MousePosition = {};
	__bMousePositionValid = false;
}

void InputEngineSystem::Tick(float _DeltaTime)
{
	UNREFERENCED_PARAMETER(_DeltaTime);

	const HWND Window = FEngineLoop::GetInstance()->GetHandle();
	const bool IsGameWindowActive = Window != nullptr && GetForegroundWindow() == Window;
	POINT MousePosition = {};
	RECT ClientRect = {};
	__bMousePositionValid = IsGameWindowActive && GetCursorPos(&MousePosition)
		&& ScreenToClient(Window, &MousePosition) && GetClientRect(Window, &ClientRect)
		&& PtInRect(&ClientRect, MousePosition);
	if (__bMousePositionValid)
		__MousePosition = FVector2D(MousePosition.x, MousePosition.y);

	for (size_t KeyIndex = 0; KeyIndex < KeyCount; ++KeyIndex)
	{
		const EKey Key = static_cast<EKey>(KeyIndex);
		const int VirtualKey = ToVirtualKey(Key);
		FKeyStateData& KeyState = __KeyStates[KeyIndex];

		const bool bCurrentActive = IsGameWindowActive && VirtualKey >= 0 &&
			((GetAsyncKeyState(VirtualKey) & 0x8000) != 0);

		if (!KeyState.bPrevActive && !bCurrentActive)
			KeyState.State = EKeyState::None;
		else if (!KeyState.bPrevActive && bCurrentActive)
			KeyState.State = EKeyState::Pressed;
		else if (KeyState.bPrevActive && bCurrentActive)
			KeyState.State = EKeyState::Repeat;
		else
			KeyState.State = EKeyState::Released;

		KeyState.bPrevActive = bCurrentActive;
	}
}

void InputEngineSystem::TG_PostUpdateWork(float _DeltaTime)
{
	UNREFERENCED_PARAMETER(_DeltaTime);
}

void InputEngineSystem::Render()
{
}

bool InputEngineSystem::IsKeyPressed(EKey _Key) const
{
	return GetKeyState(_Key) == EKeyState::Pressed;
}

bool InputEngineSystem::IsKeyRepeat(EKey _Key) const
{
	return GetKeyState(_Key) == EKeyState::Repeat;
}

bool InputEngineSystem::IsKeyReleased(EKey _Key) const
{
	return GetKeyState(_Key) == EKeyState::Released;
}

EKeyState InputEngineSystem::GetKeyState(EKey _Key) const
{
	const size_t KeyIndex = static_cast<size_t>(_Key);

	if (KeyIndex >= KeyCount)
		return EKeyState::None;

	return __KeyStates[KeyIndex].State;
}

bool InputEngineSystem::GetMousePosition(FVector2D& _Position) const
{
	if (!__bMousePositionValid)
		return false;
	_Position = __MousePosition;
	return true;
}

int InputEngineSystem::ToVirtualKey(EKey _Key)
{
	switch (_Key)
	{
	case EKey::LeftMouseButton: return VK_LBUTTON;
	case EKey::RightMouseButton: return VK_RBUTTON;
	case EKey::MiddleMouseButton: return VK_MBUTTON;
	case EKey::ThumbMouseButton: return VK_XBUTTON1;
	case EKey::ThumbMouseButton2: return VK_XBUTTON2;

	case EKey::Backspace: return VK_BACK;
	case EKey::Tab: return VK_TAB;
	case EKey::Enter: return VK_RETURN;
	case EKey::Pause: return VK_PAUSE;
	case EKey::CapsLock: return VK_CAPITAL;
	case EKey::Escape: return VK_ESCAPE;
	case EKey::SpaceBar: return VK_SPACE;
	case EKey::PageUp: return VK_PRIOR;
	case EKey::PageDown: return VK_NEXT;
	case EKey::End: return VK_END;
	case EKey::Home: return VK_HOME;
	case EKey::Left: return VK_LEFT;
	case EKey::Up: return VK_UP;
	case EKey::Right: return VK_RIGHT;
	case EKey::Down: return VK_DOWN;
	case EKey::PrintScreen: return VK_SNAPSHOT;
	case EKey::Insert: return VK_INSERT;
	case EKey::Delete: return VK_DELETE;

	case EKey::Num0: return '0'; case EKey::Num1: return '1';
	case EKey::Num2: return '2'; case EKey::Num3: return '3';
	case EKey::Num4: return '4'; case EKey::Num5: return '5';
	case EKey::Num6: return '6'; case EKey::Num7: return '7';
	case EKey::Num8: return '8'; case EKey::Num9: return '9';

	case EKey::A: return 'A'; case EKey::B: return 'B'; case EKey::C: return 'C';
	case EKey::D: return 'D'; case EKey::E: return 'E'; case EKey::F: return 'F';
	case EKey::G: return 'G'; case EKey::H: return 'H'; case EKey::I: return 'I';
	case EKey::J: return 'J'; case EKey::K: return 'K'; case EKey::L: return 'L';
	case EKey::M: return 'M'; case EKey::N: return 'N'; case EKey::O: return 'O';
	case EKey::P: return 'P'; case EKey::Q: return 'Q'; case EKey::R: return 'R';
	case EKey::S: return 'S'; case EKey::T: return 'T'; case EKey::U: return 'U';
	case EKey::V: return 'V'; case EKey::W: return 'W'; case EKey::X: return 'X';
	case EKey::Y: return 'Y'; case EKey::Z: return 'Z';

	case EKey::LeftWindows: return VK_LWIN;
	case EKey::RightWindows: return VK_RWIN;
	case EKey::Application: return VK_APPS;

	case EKey::Numpad0: return VK_NUMPAD0; case EKey::Numpad1: return VK_NUMPAD1;
	case EKey::Numpad2: return VK_NUMPAD2; case EKey::Numpad3: return VK_NUMPAD3;
	case EKey::Numpad4: return VK_NUMPAD4; case EKey::Numpad5: return VK_NUMPAD5;
	case EKey::Numpad6: return VK_NUMPAD6; case EKey::Numpad7: return VK_NUMPAD7;
	case EKey::Numpad8: return VK_NUMPAD8; case EKey::Numpad9: return VK_NUMPAD9;
	case EKey::NumpadMultiply: return VK_MULTIPLY;
	case EKey::NumpadAdd: return VK_ADD;
	case EKey::NumpadSubtract: return VK_SUBTRACT;
	case EKey::NumpadDecimal: return VK_DECIMAL;
	case EKey::NumpadDivide: return VK_DIVIDE;

	case EKey::F1: return VK_F1; case EKey::F2: return VK_F2;
	case EKey::F3: return VK_F3; case EKey::F4: return VK_F4;
	case EKey::F5: return VK_F5; case EKey::F6: return VK_F6;
	case EKey::F7: return VK_F7; case EKey::F8: return VK_F8;
	case EKey::F9: return VK_F9; case EKey::F10: return VK_F10;
	case EKey::F11: return VK_F11; case EKey::F12: return VK_F12;
	case EKey::F13: return VK_F13; case EKey::F14: return VK_F14;
	case EKey::F15: return VK_F15; case EKey::F16: return VK_F16;
	case EKey::F17: return VK_F17; case EKey::F18: return VK_F18;
	case EKey::F19: return VK_F19; case EKey::F20: return VK_F20;
	case EKey::F21: return VK_F21; case EKey::F22: return VK_F22;
	case EKey::F23: return VK_F23; case EKey::F24: return VK_F24;

	case EKey::NumLock: return VK_NUMLOCK;
	case EKey::ScrollLock: return VK_SCROLL;
	case EKey::LeftShift: return VK_LSHIFT;
	case EKey::RightShift: return VK_RSHIFT;
	case EKey::LeftControl: return VK_LCONTROL;
	case EKey::RightControl: return VK_RCONTROL;
	case EKey::LeftAlt: return VK_LMENU;
	case EKey::RightAlt: return VK_RMENU;

	case EKey::Semicolon: return VK_OEM_1;
	case EKey::Equals: return VK_OEM_PLUS;
	case EKey::Comma: return VK_OEM_COMMA;
	case EKey::Minus: return VK_OEM_MINUS;
	case EKey::Period: return VK_OEM_PERIOD;
	case EKey::Slash: return VK_OEM_2;
	case EKey::Grave: return VK_OEM_3;
	case EKey::LeftBracket: return VK_OEM_4;
	case EKey::Backslash: return VK_OEM_5;
	case EKey::RightBracket: return VK_OEM_6;
	case EKey::Apostrophe: return VK_OEM_7;

	case EKey::BrowserBack: return VK_BROWSER_BACK;
	case EKey::BrowserForward: return VK_BROWSER_FORWARD;
	case EKey::BrowserRefresh: return VK_BROWSER_REFRESH;
	case EKey::BrowserStop: return VK_BROWSER_STOP;
	case EKey::BrowserSearch: return VK_BROWSER_SEARCH;
	case EKey::BrowserFavorites: return VK_BROWSER_FAVORITES;
	case EKey::BrowserHome: return VK_BROWSER_HOME;
	case EKey::VolumeMute: return VK_VOLUME_MUTE;
	case EKey::VolumeDown: return VK_VOLUME_DOWN;
	case EKey::VolumeUp: return VK_VOLUME_UP;
	case EKey::MediaNextTrack: return VK_MEDIA_NEXT_TRACK;
	case EKey::MediaPreviousTrack: return VK_MEDIA_PREV_TRACK;
	case EKey::MediaStop: return VK_MEDIA_STOP;
	case EKey::MediaPlayPause: return VK_MEDIA_PLAY_PAUSE;
	case EKey::LaunchMail: return VK_LAUNCH_MAIL;
	case EKey::LaunchMediaSelect: return VK_LAUNCH_MEDIA_SELECT;
	case EKey::LaunchApplication1: return VK_LAUNCH_APP1;
	case EKey::LaunchApplication2: return VK_LAUNCH_APP2;
	case EKey::Hangul: return VK_HANGUL;
	case EKey::Hanja: return VK_HANJA;
	case EKey::OEM102: return VK_OEM_102;
	}

	return -1;
}
