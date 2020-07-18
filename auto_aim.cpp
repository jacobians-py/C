#include "autoaim.h"
#include "iostream"
#include "algorithm"

#include "raw.h"

#define M_PI		3.14159265358979323846
#define M_PI_F		((float)(M_PI))
#define DEG2RAD(x)  ( (float)(x) * (float)(M_PI_F / 180.f) )
#define RAD2DEG(x)  ( (float)(x) * (float)(180.f / M_PI_F) )

namespace AutoAim
{
	float bestFOV = 0.f;
	Classes::FRotator idealAngDelta;

	const float AutoAimFOV = 15.f;

	void BeginFrame()
	{
		bestFOV = AutoAimFOV;
		idealAngDelta = { 0,0,0 };
	}

	Classes::FRotator Clamp(Classes::FRotator r)
	{
		if (r.Yaw > 180.f)
			r.Yaw -= 360.f;
		else if (r.Yaw < -180.f)
			r.Yaw += 360.f;

		if (r.Pitch > 180.f)
			r.Pitch -= 360.f;
		else if (r.Pitch < -180.f)
			r.Pitch += 360.f;

		if (r.Pitch < -89.f)
			r.Pitch = -89.f;
		else if (r.Pitch > 89.f)
			r.Pitch = 89.f;

		r.Roll = 0.f;

		return r;
	}

	Classes::FRotator Vec2Rot(Classes::FVector vec)
	{
		Classes::FRotator rot;

		rot.Yaw = RAD2DEG(std::atan2f(vec.Y, vec.X));
		rot.Pitch = RAD2DEG(std::atan2f(vec.Z, std::sqrtf(vec.X * vec.X + vec.Y * vec.Y)));
		rot.Roll = 0.f;

		return rot;
	}

	Classes::FVector GetPlayerVelocity(Classes::ATslCharacter* p)
	{
		if (p->RootComponent)
			return p->RootComponent->ComponentVelocity;

		return { 0.f, 0.f, 0.f };
	}

	void EvaluateTarget(Classes::ATslCharacter * p)
	{
		if (p->Team)
			return;

		if (!Engine::IsVisible(p))
			return;

		auto pos = Engine::GetBonePosition(p, 15);
		auto delta = (pos - G::localPos);
		auto dist = delta.Size();

		if (p->Health < 1.f)
			return;

		Classes::FRotator ideal = Clamp(Vec2Rot(delta));
		auto angDelta = Clamp(ideal - G::localRot);
		float angFOV = angDelta.Size();

		if (angFOV < bestFOV)
		{
			bestFOV = angFOV;
			idealAngDelta = angDelta;
		}
	}

	void SetViewAngles(Classes::FRotator ang)
	{
		G::localChar->Controller->ControlRotation = Clamp(ang);
	}

	void AimToTarget()
	{
		if (bestFOV >= AutoAimFOV)
			return;

		SetViewAngles(G::localRot + idealAngDelta * 0.4f);
	}
}
