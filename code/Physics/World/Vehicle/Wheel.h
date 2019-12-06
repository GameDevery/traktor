#pragma once

#include <Core/Ref.h>
#include <Core/Object.h>
#include <Core/Math/Vector4.h>

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class WheelData;

class T_DLLCLASS Wheel : public Object
{
	T_RTTI_CLASS;

public:
	Ref< const WheelData > data;
	float angle;
	float velocity;
	Vector4 direction;
	Vector4 directionPerp;
	float suspensionLength;
	bool contact;
	float contactFudge;
	int32_t contactMaterial;
	Vector4 contactPosition;
	Vector4 contactNormal;
	Vector4 contactVelocity;

	explicit Wheel(const WheelData* data);

	const WheelData* getData() const { return data; }

	float getAngle() const { return angle; }

	float getVelocity() const { return velocity; }

	const Vector4& getDirection() const { return direction; }

	const Vector4& getDirectionPerp() const { return directionPerp; }

	float getSuspensionLength() const { return suspensionLength; }

	bool getContact() const { return contact; }

	const Vector4& getContactPosition() const { return contactPosition; }

	const Vector4& getContactNormal() const { return contactNormal; }

	const Vector4& getContactVelocity() const { return contactVelocity; }
};

	}
}

