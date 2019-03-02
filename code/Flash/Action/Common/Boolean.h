#pragma once

#include "Flash/Action/ActionObjectRelay.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript boolean wrapper.
 * \ingroup Flash
 */
class T_DLLCLASS Boolean : public ActionObjectRelay
{
	T_RTTI_CLASS;

public:
	Boolean(bool value);

	void set(bool value) { m_value = value; }

	bool get() const { return m_value; }

private:
	bool m_value;
};

	}
}

