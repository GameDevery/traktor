#pragma once

#include <string>
#include "Runtime/Target/IRemoteEvent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace runtime
	{

/*! \brief Application generic command events.
 * \ingroup Runtime
 *
 * These events are sent remotely from the editor to the
 * running target.
 */
class T_DLLCLASS CommandEvent : public IRemoteEvent
{
	T_RTTI_CLASS;

public:
	CommandEvent();

	CommandEvent(const std::wstring& function);

	const std::wstring& getFunction() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_function;
};

	}
}
