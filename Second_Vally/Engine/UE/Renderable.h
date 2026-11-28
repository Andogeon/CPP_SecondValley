#pragma once

#include "CoreMinimal.h"

class ENGINE_API IRenderable
{
public:
	virtual ~IRenderable() = default;

	virtual void Render() = 0;

	static void SetRenderTarget(HDC _DC);
	static HDC GetRenderTarget();
	bool IsRenderable() const { return bCanEverRender; }

protected:
	bool    bCanEverRender = true;
};
