#ifndef traktor_net_FloatTemplate_H
#define traktor_net_FloatTemplate_H

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

class T_DLLCLASS FloatTemplate : public IValueTemplate
{
	T_RTTI_CLASS;

public:
	FloatTemplate();

	explicit FloatTemplate(float min, float max, bool lowPrecision);

	explicit FloatTemplate(float min, float max, float idle, bool lowPrecision);

	virtual void pack(BitWriter& writer, const IValue* V) const;

	virtual Ref< const IValue > unpack(BitReader& reader) const;

	virtual bool equal(const IValue* Vl, const IValue* Vr) const;

	virtual Ref< const IValue > extrapolate(const IValue* Vn2, float Tn2, const IValue* Vn1, float Tn1, const IValue* V0, float T0, const IValue* V, float T) const;

private:
	float m_min;
	float m_max;
	float m_idle;
	bool m_lowPrecision;
	bool m_haveIdle;
};

	}
}

#endif	// traktor_net_FloatTemplate_H
