#pragma once

#include "EngineLoop.h"
#include "ThirdParty/WIL/include/wil/resource.h"

class LAUNCH_API FEngineLoop : public IEngineLoop, public TSingleton<FEngineLoop>
{
	DECLARE_SINGLETON(FEngineLoop)

private:
	~FEngineLoop() = default;

public:
	virtual HRESULT Initialize(POINT _Resolution = {}) override;
	virtual void Deinitialize() override;

	HRESULT ResolutionInitialize(POINT _Resolution);
	HRESULT EngineSystemBootstrapper();
	HRESULT CreateBackBuffer();
	void ClearBackBuffer();
	void ReleaseBackBuffer();
	void PresentBackBuffer();

	virtual void Progress() override;

	void Update();
	void Render();

public:
	inline HWND	GetHandle() const { return __Hwnd; }
	inline POINT GetResolution() const { return __Resolution; }

	inline void SetDeltaTime(const double _DeltaTime) { __DeltaTime = _DeltaTime; }
	
private:
	double	__DeltaTime = 0.0;

	wil::unique_hdc		__BackBufferDC;
	wil::unique_hbitmap	__BackBufferBitmap;
	HBITMAP				__OldBackBufferBitmap = nullptr;
	POINT				__Resolution = {};

private:
	HWND	__Hwnd = nullptr;
};
