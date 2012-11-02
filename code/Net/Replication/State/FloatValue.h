#ifndef traktor_net_FloatValue_H
#define traktor_net_FloatValue_H

#include "Net/Replication/State/IValue.h"

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

class T_DLLCLASS FloatValue : public IValue
{
	T_RTTI_CLASS;

public:
	typedef float value_t;
	typedef float value_argument_t;
	typedef float value_return_t;

	FloatValue(float value)
	:	m_value(value)
	{
	}

	operator float () const { return m_value; }

private:
	float m_value;
};

	}
}

#endif	// traktor_net_FloatValue_H
