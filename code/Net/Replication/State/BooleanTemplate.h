#pragma once

#include "Net/Replication/State/IValueTemplate.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_NET_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class T_DLLCLASS BooleanTemplate : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	explicit BooleanTemplate(const wchar_t* const tag, float threshold = 0.0f);

	virtual const TypeInfo& getValueType() const override final;

	virtual uint32_t getMaxPackedDataSize() const override final;

	virtual void pack(BitWriter& writer, const IValue* V) const override final;

	virtual Ref< const IValue > unpack(BitReader& reader) const override final;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, float T) const override final;

	virtual bool threshold(const IValue* Vn1, const IValue* V) const override final;

private:
	const wchar_t* const m_tag;
	float m_threshold;
};

	}
}

