#include "pch.h"
#include "LaunchEngineLoop.h"
#include "Engine.h"
#include "AppTimeEngineSystem.h"
#include "InputEngineSystem.h"
#include "LevelEngineSystem.h"
#include "EventEngineSystem.h"
#include "CollisionEngineSystem.h"
#include "CameraEngineSystem.h"
#include "PathEngineSystem.h"
#include "ResourceEngineSystem.h"

HRESULT FEngineLoop::Initialize(POINT _Resolution)
{
	__Hwnd = GetActiveWindow();

	if (!__Hwnd)
		return E_HANDLE;
	
	if (FAILED(ResolutionInitialize(_Resolution)))
		return E_FAIL;

	if (FAILED(CreateBackBuffer()))
		return E_FAIL;

	if (FAILED(EngineSystemBootstrapper()))
		return E_FAIL;

	if (FAILED(GEngine->Initialize()))
		return E_FAIL;

	return S_OK;
}

HRESULT FEngineLoop::EngineSystemBootstrapper()
{
	InputEngineSystem::GetInstance();
	AppTimeEngineSystem::GetInstance();
	PathEngineSystem::GetInstance();
	ResourceEngineSystem::GetInstance();
	EventEngineSystem::GetInstance();
	LevelEngineSystem::GetInstance();
	CameraEngineSystem::GetInstance();
	CollisionEngineSystem::GetInstance();

	return S_OK;
}

void FEngineLoop::Deinitialize()
{
	GEngine->Deinitialize();
	ReleaseBackBuffer();
}

HRESULT FEngineLoop::CreateBackBuffer()
{
	if (__Resolution.x <= 0 || __Resolution.y <= 0)
		return E_INVALIDARG;

	wil::unique_hdc_window WindowDC = wil::GetDC(__Hwnd);
	if (!WindowDC)
		return HRESULT_FROM_WIN32(GetLastError());

	__BackBufferDC.reset(CreateCompatibleDC(WindowDC.get()));
	if (!__BackBufferDC)
		return HRESULT_FROM_WIN32(GetLastError());

	__BackBufferBitmap.reset(CreateCompatibleBitmap(
		WindowDC.get(),
		__Resolution.x,
		__Resolution.y));

	if (!__BackBufferBitmap)
	{
		const DWORD ErrorCode = GetLastError();
		ReleaseBackBuffer();
		return HRESULT_FROM_WIN32(ErrorCode);
	}

	__OldBackBufferBitmap = static_cast<HBITMAP>(
		SelectObject(__BackBufferDC.get(), __BackBufferBitmap.get()));

	if (nullptr == __OldBackBufferBitmap || HGDI_ERROR == __OldBackBufferBitmap)
	{
		__OldBackBufferBitmap = nullptr;
		ReleaseBackBuffer();
		return E_FAIL;
	}

	IRenderable::SetRenderTarget(__BackBufferDC.get());

	return S_OK;
}

void FEngineLoop::ReleaseBackBuffer()
{
	IRenderable::SetRenderTarget(nullptr);

	if (__BackBufferDC && nullptr != __OldBackBufferBitmap)
		SelectObject(__BackBufferDC.get(), __OldBackBufferBitmap);

	__OldBackBufferBitmap = nullptr;
	__BackBufferBitmap.reset();
	__BackBufferDC.reset();
	__Resolution = {};
}

void FEngineLoop::ClearBackBuffer()
{
	const RECT BackBufferRect =
	{
		0,
		0,
		__Resolution.x,
		__Resolution.y
	};

	FillRect(
		__BackBufferDC.get(),
		&BackBufferRect,
		GetSysColorBrush(COLOR_WINDOW));
}

void FEngineLoop::PresentBackBuffer()
{
	if (!__BackBufferDC)
		return;

	wil::unique_hdc_window WindowDC = wil::GetDC(__Hwnd);
	if (!WindowDC)
		return;

	BitBlt(
		WindowDC.get(),
		0,
		0,
		__Resolution.x,
		__Resolution.y,
		__BackBufferDC.get(),
		0,
		0,
		SRCCOPY);
}

HRESULT FEngineLoop::ResolutionInitialize(POINT _Resolution)
{
	const bool bBorderlessFullscreen = _Resolution.x <= 0 || _Resolution.y <= 0;

	if (bBorderlessFullscreen)
	{
		const HMONITOR Monitor = MonitorFromWindow(__Hwnd, MONITOR_DEFAULTTONEAREST);

		MONITORINFO MonitorInfo = {};
		MonitorInfo.cbSize = sizeof(MonitorInfo);

		if (!GetMonitorInfo(Monitor, &MonitorInfo))
		{
			const DWORD ErrorCode = GetLastError();
			return HRESULT_FROM_WIN32(ErrorCode);
		}

		SetWindowLongPtr(__Hwnd, GWL_STYLE, WS_POPUP);

		SetMenu(__Hwnd, nullptr);

		const RECT& MonitorRect = MonitorInfo.rcMonitor;

		__Resolution.x = MonitorRect.right - MonitorRect.left;
		__Resolution.y = MonitorRect.bottom - MonitorRect.top;

		if (!SetWindowPos(
			__Hwnd,

			HWND_TOP,
			MonitorRect.left,
			MonitorRect.top,
			MonitorRect.right - MonitorRect.left,
			MonitorRect.bottom - MonitorRect.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW))
		{
			const DWORD ErrorCode = GetLastError();
			return HRESULT_FROM_WIN32(ErrorCode);
		}
	}
	else
	{
		constexpr DWORD WindowStyle = WS_OVERLAPPEDWINDOW;
		__Resolution = _Resolution;

		SetWindowLongPtr(__Hwnd, GWL_STYLE, WindowStyle);

		RECT WindowRect = { 0, 0, _Resolution.x, _Resolution.y };

		if (!AdjustWindowRect(&WindowRect, WindowStyle, GetMenu(__Hwnd) != nullptr))
		{
			const DWORD ErrorCode = GetLastError();
			return HRESULT_FROM_WIN32(ErrorCode);
		}

		if (!SetWindowPos(
			__Hwnd,
			HWND_TOP,
			0,
			0,
			WindowRect.right - WindowRect.left,
			WindowRect.bottom - WindowRect.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW))
		{
			const DWORD ErrorCode = GetLastError();
			return HRESULT_FROM_WIN32(ErrorCode);
		}
	}

	return S_OK;
}

void FEngineLoop::Progress()
{
	Update();
	Render();
}

void FEngineLoop::Update()
{
	GEngine->Tick(__DeltaTime);
	GEngine->TG_PostUpdateWork(__DeltaTime);
}

void FEngineLoop::Render()
{
	ClearBackBuffer();

	GEngine->Render();

	PresentBackBuffer();
}
