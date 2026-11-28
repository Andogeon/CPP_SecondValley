#include "pch.h"
#include "Renderable.h"

namespace
{
	HDC GRenderTarget = nullptr;
}

void IRenderable::SetRenderTarget(HDC _DC)
{
	GRenderTarget = _DC;
}

HDC IRenderable::GetRenderTarget()
{
	return GRenderTarget;
}
