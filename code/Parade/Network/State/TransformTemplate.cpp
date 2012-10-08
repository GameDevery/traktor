#include "Core/Io/BitReader.h"
#include "Core/Io/BitWriter.h"
#include "Parade/Network/State/TransformValue.h"
#include "Parade/Network/State/TransformTemplate.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.TransformTemplate", TransformTemplate, IValueTemplate)

void TransformTemplate::pack(BitWriter& writer, const IValue* V) const
{
	Transform v = *checked_type_cast< const TransformValue* >(V);

	float T_MATH_ALIGN16 e[4 + 4];
	v.translation().storeAligned(&e[0]);
	v.rotation().e.storeAligned(&e[4]);

	for (uint32_t i = 0; i < 3; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[i]);

	for (uint32_t i = 0; i < 4; ++i)
		writer.writeUnsigned(32, *(uint32_t*)&e[4 + i]);
}

Ref< const IValue > TransformTemplate::unpack(BitReader& reader) const
{
	uint32_t u[4 + 4];

	for (uint32_t i = 0; i < 3; ++i)
		u[i] = reader.readUnsigned(32);

	for (uint32_t i = 0; i < 4; ++i)
		u[4 + i] = reader.readUnsigned(32);

	return new TransformValue(Transform(
		Vector4(
			*(float*)&u[0],
			*(float*)&u[1],
			*(float*)&u[2],
			1.0f
		),
		Quaternion(
			*(float*)&u[4],
			*(float*)&u[5],
			*(float*)&u[6],
			*(float*)&u[7]
		)
	));
}

Ref< const IValue > TransformTemplate::extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const
{
	Transform tn1 = *checked_type_cast< const TransformValue* >(Vn1);
	Transform t0 = *checked_type_cast< const TransformValue* >(V0);

	float k = (T - Tn1) / (T0 - Tn1);
	Transform t = lerp(t0, t0 * (tn1.inverse() * t0), Scalar(k));

	return new TransformValue(t);
}

	}
}
