#pragma once

#include "Core/Math/Aabb2.h"
#include "Core/Math/Aabb3.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/MemberComplex.h"

namespace traktor
{

class MemberAabb2 : public MemberComplex
{
public:
	MemberAabb2(const wchar_t* const name, Aabb2& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< Vector2 >(L"mn", m_ref.mn);
		s >> Member< Vector2 >(L"mx", m_ref.mx);
	}

private:
	Aabb2& m_ref;
};

class MemberAabb3 : public MemberComplex
{
public:
	MemberAabb3(const wchar_t* const name, Aabb3& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const override final
	{
		s >> Member< Vector4 >(L"mn", m_ref.mn, AttributePoint());
		s >> Member< Vector4 >(L"mx", m_ref.mx, AttributePoint());
	}

private:
	Aabb3& m_ref;
};

}

