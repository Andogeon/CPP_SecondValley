#include "pch.h"
#include "CollisionEngineSystem.h"
#include "Collider.h"
#include "CameraEngineSystem.h"
#include "LevelEngineSystem.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

HRESULT CollisionEngineSystem::Initialize()
{
	bCanEverTick = true;
	bCanEverRender = true;
	return S_OK;
}

void CollisionEngineSystem::Deinitialize()
{
	__CollisionInfo.clear();

	InspectionOff();
}

void CollisionEngineSystem::Tick(float _DeltaTime)
{
}

void CollisionEngineSystem::TG_PostUpdateWork(float _DeltaTime)
{
	ULevel* Level = LevelEngineSystem::GetInstance()->GetCurrentLevel();
	if (Level != nullptr && Level->IsActive())
		Update(*Level);
}

void CollisionEngineSystem::Render()
{
	ULevel* Level = LevelEngineSystem::GetInstance()->GetCurrentLevel();
	HDC DC = GetRenderTarget();
	if (Level == nullptr || !Level->IsActive() || DC == nullptr)
		return;

	const int SavedDC = SaveDC(DC);
	if (SavedDC == 0)
		return;

	SelectObject(DC, GetStockObject(DC_PEN));
	SelectObject(DC, GetStockObject(NULL_BRUSH));

	for (size_t GroupIndex = 0; GroupIndex < GroupCount; ++GroupIndex)
	{
		for (const auto& Actor : Level->GetActors(static_cast<EActorGroup>(GroupIndex)))
		{
			for (const auto& Component : Actor->GetComponents())
			{
				const auto* Collider = dynamic_cast<const UCollider*>(Component.get());
				if (Collider == nullptr || !Collider->IsEnabled())
					continue;

				const FVector2D Scale = Collider->GetScale();
				if (!(Scale.X > 0.0 && Scale.Y > 0.0))
					continue;

				auto Corners = Collider->GetCorners();
				for (FVector2D& Corner : Corners)
					Corner = CameraEngineSystem::GetInstance()->GetRenderPos(Corner);
				const bool bValidCorners = std::all_of(Corners.begin(), Corners.end(), [](const FVector2D& _Point)
				{
					return std::isfinite(_Point.X) && std::isfinite(_Point.Y)
						&& _Point.X >= (std::numeric_limits<LONG>::min)() && _Point.X <= (std::numeric_limits<LONG>::max)()
						&& _Point.Y >= (std::numeric_limits<LONG>::min)() && _Point.Y <= (std::numeric_limits<LONG>::max)();
				});
				if (!bValidCorners)
					continue;

				POINT Points[4];
				for (size_t Index = 0; Index < Corners.size(); ++Index)
					Points[Index] = { static_cast<LONG>(Corners[Index].X), static_cast<LONG>(Corners[Index].Y) };

				SetDCPenColor(DC, Collider->IsColliding() ? RGB(255, 0, 0) : RGB(0, 255, 0));
				Polygon(DC, Points, 4);
			}
		}
	}

	RestoreDC(DC, SavedDC);
}

void CollisionEngineSystem::RequestInspection(EActorGroup _Left, EActorGroup _Right)
{
	SetInspection(_Left, _Right, true);
}

void CollisionEngineSystem::SetInspection(EActorGroup _Left, EActorGroup _Right, bool _Enabled)
{
	if (_Left >= EActorGroup::Count || _Right >= EActorGroup::Count)
		throw std::out_of_range("Invalid collision group.");

	if (_Right < _Left)
		std::swap(_Left, _Right);

	const size_t Row = static_cast<size_t>(_Left);
	const uint32_t Mask = uint32_t{ 1 } << static_cast<size_t>(_Right);
	if (_Enabled)
		__Channels[Row] |= Mask;
	else
		__Channels[Row] &= ~Mask;
}

bool CollisionEngineSystem::IsInspection(EActorGroup _Left, EActorGroup _Right) const
{
	if (_Left >= EActorGroup::Count || _Right >= EActorGroup::Count)
		return false;

	if (_Right < _Left)
		std::swap(_Left, _Right);

	return (__Channels[static_cast<size_t>(_Left)] & (uint32_t{ 1 } << static_cast<size_t>(_Right))) != 0;
}

void CollisionEngineSystem::InspectionOff()
{
	__Channels.fill(0);
}

bool CollisionEngineSystem::IsCollision(const UCollider& _Left, const UCollider& _Right)
{
	const FVector2D LeftPos = _Left.GetPos();
	const FVector2D RightPos = _Right.GetPos();
	const FVector2D LeftScale = _Left.GetScale();
	const FVector2D RightScale = _Right.GetScale();
	for (double Value : { LeftPos.X, LeftPos.Y, RightPos.X, RightPos.Y,
		LeftScale.X, LeftScale.Y, RightScale.X, RightScale.Y, _Left.GetRotation(), _Right.GetRotation() })
	{
		if (!std::isfinite(Value))
			return false;
	}

	if (!(LeftScale.X > 0.0 && LeftScale.Y > 0.0 && RightScale.X > 0.0 && RightScale.Y > 0.0))
		return false;

	const auto LeftAxes = _Left.GetAxes();
	const auto RightAxes = _Right.GetAxes();
	const FVector2D Delta = RightPos - LeftPos;
	const auto Dot = [](const FVector2D& _A, const FVector2D& _B)
	{
		return _A.X * _B.X + _A.Y * _B.Y;
	};

	for (const FVector2D& Axis : { LeftAxes[0], LeftAxes[1], RightAxes[0], RightAxes[1] })
	{
		const double Distance = std::abs(Dot(Delta, Axis));
		const double LeftRadius = LeftScale.X * 0.5 * std::abs(Dot(LeftAxes[0], Axis))
			+ LeftScale.Y * 0.5 * std::abs(Dot(LeftAxes[1], Axis));
		const double RightRadius = RightScale.X * 0.5 * std::abs(Dot(RightAxes[0], Axis))
			+ RightScale.Y * 0.5 * std::abs(Dot(RightAxes[1], Axis));
		const double RadiusSum = LeftRadius + RightRadius;
		if (!std::isfinite(Distance) || !std::isfinite(RadiusSum) || Distance >= RadiusSum)
			return false;
	}
	return true;
}

void CollisionEngineSystem::Notify(FCollisionInfo& _Info, ECollisionPhase _Phase)
{
	UCollider* Left = _Info.Left;
	UCollider* Right = _Info.Right;
	switch (_Phase)
	{
	case ECollisionPhase::Enter:
		_Info.bWasColliding = true;
		++Left->__CollisionCount;
		++Right->__CollisionCount;
		Left->GetActor()->OnCollisionEnter(Right);
		Right->GetActor()->OnCollisionEnter(Left);
		break;
	case ECollisionPhase::Stay:
		Left->GetActor()->OnCollisionStay(Right);
		Right->GetActor()->OnCollisionStay(Left);
		break;
	case ECollisionPhase::Exit:
		_Info.bWasColliding = false;
		--Left->__CollisionCount;
		--Right->__CollisionCount;
		Left->GetActor()->OnCollisionExit(Right);
		Right->GetActor()->OnCollisionExit(Left);
		break;
	}
}

void CollisionEngineSystem::ChannelInspection(EActorGroup _Left, EActorGroup _Right, const FColliderGroups& _Groups)
{
	const auto& LeftColliders = _Groups[static_cast<size_t>(_Left)];
	const auto& RightColliders = _Groups[static_cast<size_t>(_Right)];
	for (size_t LeftIndex = 0; LeftIndex < LeftColliders.size(); ++LeftIndex)
	{
		const size_t FirstRight = _Left == _Right ? LeftIndex + 1 : 0;

		for (size_t RightIndex = FirstRight; RightIndex < RightColliders.size(); ++RightIndex)
		{
			if (!IsInspection(_Left, _Right))
				return;

			UCollider* Left = LeftColliders[LeftIndex];
			UCollider* Right = RightColliders[RightIndex];
			if (Left->GetActor() == Right->GetActor())
				continue;

			const uint64_t LeftID = Left->GetID();
			const uint64_t RightID = Right->GetID();
			const auto IDs = std::minmax(LeftID, RightID);
			const FPairKey Key{ IDs.first, IDs.second };
			auto [Iterator, Inserted] = __CollisionInfo.try_emplace(Key,
				FCollisionInfo{ Left, Right, _Left, _Right, false });
			FCollisionInfo& Info = Iterator->second;

			const bool bCurrentCollision = Left->IsEnabled() && Right->IsEnabled() && IsCollision(*Left, *Right);
			if (bCurrentCollision)
			{
				if (Info.bWasColliding)
					Notify(Info, ECollisionPhase::Stay);
				else
					Notify(Info, ECollisionPhase::Enter);
			}
			else if (Info.bWasColliding)
			{
				Notify(Info, ECollisionPhase::Exit);
			}
		}
	}
}

void CollisionEngineSystem::Update(ULevel& _Level)
{
	FColliderGroups Groups;
	for (size_t GroupIndex = 0; GroupIndex < GroupCount; ++GroupIndex)
	{
		for (const auto& Actor : _Level.GetActors(static_cast<EActorGroup>(GroupIndex)))
			for (const auto& Component : Actor->GetComponents())
				if (auto* Collider = dynamic_cast<UCollider*>(Component.get()))
					Groups[GroupIndex].push_back(Collider);
	}

	for (size_t Row = 0; Row < GroupCount; ++Row)
	{
		for (size_t Col = Row; Col < GroupCount; ++Col)
		{
			const auto LeftGroup = static_cast<EActorGroup>(Row);
			const auto RightGroup = static_cast<EActorGroup>(Col);
			if (IsInspection(LeftGroup, RightGroup))
			{
				ChannelInspection(LeftGroup, RightGroup, Groups);
			}
		}
	}

	bool bEndedCollision;
	do
	{
		bEndedCollision = false;
		for (auto& [Key, Info] : __CollisionInfo)
		{
			if (Info.bWasColliding && (!IsInspection(Info.LeftGroup, Info.RightGroup)
				|| !Info.Left->IsEnabled() || !Info.Right->IsEnabled()))
			{
				Notify(Info, ECollisionPhase::Exit);
				bEndedCollision = true;
			}
		}
	} while (bEndedCollision);
}

void CollisionEngineSystem::RemoveActor(AActor* _Actor)
{
	for (auto Iterator = __CollisionInfo.begin(); Iterator != __CollisionInfo.end();)
	{
		FCollisionInfo& Info = Iterator->second;
		if (Info.Left->GetActor() == _Actor || Info.Right->GetActor() == _Actor)
		{
			if (Info.bWasColliding)
				Notify(Info, ECollisionPhase::Exit);

			Iterator = __CollisionInfo.erase(Iterator);
		}
		else
		{
			++Iterator;
		}
	}
}
