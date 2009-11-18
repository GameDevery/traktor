#include "Input/RumbleEffect.h"
#include "Core/Math/Hermite.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

struct LF { static float value(const InputRumble& rumble) { return rumble.lowFrequencyRumble; } };
struct HF { static float value(const InputRumble& rumble) { return rumble.highFrequencyRumble; } };

template < typename F >
struct RumbleAccessor
{
	static inline float time(const std::pair< float, InputRumble >& key)
	{
		return key.first;
	}

	static inline float value(const std::pair< float, InputRumble >& key)
	{
		return F::value(key.second);
	}

	static inline float combine(
		float v0, float w0,
		float v1, float w1,
		float v2, float w2,
		float v3, float w3
	)
	{
		return v0 * w0 + v1 * w1 + v2 * w2 + v3 * w3;
	}
};

class MemberInputRumble : public MemberComplex
{
public:
	typedef InputRumble value_type;

	MemberInputRumble(const std::wstring& name, value_type& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		s >> Member< float >(L"lowFrequencyRumble", m_ref.lowFrequencyRumble);
		s >> Member< float >(L"highFrequencyRumble", m_ref.highFrequencyRumble);
		return true;
	}

private:
	value_type& m_ref;
};

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.input.RumbleEffect", 0, RumbleEffect, ISerializable)

float RumbleEffect::getDuration() const
{
	return !m_rumbleKeys.empty() ? m_rumbleKeys.back().first : 0.0f;
}

void RumbleEffect::setRumbleKeys(const std::vector< std::pair< float, InputRumble > >& rumbleKeys)
{
	m_rumbleKeys = rumbleKeys;
}

void RumbleEffect::getRumble(float time, InputRumble& outRumble) const
{
	outRumble.lowFrequencyRumble = Hermite< std::pair< float, InputRumble >, float, float, RumbleAccessor< LF > >::evaluate(&m_rumbleKeys[0], m_rumbleKeys.size(), time);
	outRumble.highFrequencyRumble = Hermite< std::pair< float, InputRumble >, float, float, RumbleAccessor< HF > >::evaluate(&m_rumbleKeys[0], m_rumbleKeys.size(), time);
}

bool RumbleEffect::serialize(ISerializer& s)
{
	s >> MemberStlVector
		< 
			std::pair< float, InputRumble >,
			MemberStlPair< float, InputRumble, Member< float >, MemberInputRumble >
		>
		(L"rumbleKeys", m_rumbleKeys);
	return true;
}

	}
}
