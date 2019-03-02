#pragma once

#include <string>
#include "Core/Object.h"

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

/*! \brief Socket address.
 * \ingroup Net
 */
class T_DLLCLASS SocketAddress : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool valid() const = 0;

	virtual std::wstring getHostName() const = 0;
};

	}
}

