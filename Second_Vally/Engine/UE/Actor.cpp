#include "pch.h"
#include "Actor.h"
#include "Material.h"
#include "CameraEngineSystem.h"
#include <cmath>
#include <limits>

void AActor::Render()
{
    RenderMaterial();
}

void AActor::SetMaterial(std::shared_ptr<Material> _Material)
{
    __Material = std::move(_Material);
}

bool AActor::RenderMaterial() const
{
    if (!__Material || __Scale.X <= 0 || __Scale.Y <= 0) return false;
    const FVector2D RenderPosition = CameraEngineSystem::GetInstance()->GetRenderPos(__Pos);
    const double Bounds[] = { RenderPosition.X - __Scale.X * 0.5, RenderPosition.Y - __Scale.Y * 0.5,
        RenderPosition.X + __Scale.X * 0.5, RenderPosition.Y + __Scale.Y * 0.5 };
    for (double Bound : Bounds)
        if (!std::isfinite(Bound) || Bound < (std::numeric_limits<LONG>::min)() ||
            Bound > (std::numeric_limits<LONG>::max)()) return false;
    const RECT Destination = { static_cast<LONG>(Bounds[0]), static_cast<LONG>(Bounds[1]),
        static_cast<LONG>(Bounds[2]), static_cast<LONG>(Bounds[3]) };
    return __Material->Draw(GetRenderTarget(), Destination, __Rotation);
}

void AActor::Tick(float _DeltaTime)
{
}

void AActor::TG_PostUpdateWork(float _DeltaTime)
{
}

void AActor::PreInitializeComponents()
{
}

void AActor::PostInitializeComponents()
{
}

void AActor::BeginPlay()
{
}

void AActor::EndPlay()
{
}

void AActor::Destroy()
{
}

void AActor::Destroyed()
{
}

void AActor::OnCollisionEnter(UCollider* _Other)
{
}

void AActor::OnCollisionStay(UCollider* _Other)
{
}

void AActor::OnCollisionExit(UCollider* _Other)
{
}
