#pragma once

#include <string>
#include "Core/Serialization/ISerializable.h"

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

/*! Log statement from running target.
 * \ingroup Runtime
 */
class T_DLLCLASS TargetLog : public ISerializable
{
	T_RTTI_CLASS;

public:
	TargetLog();

	TargetLog(uint32_t threadId, int32_t level, const std::wstring& text);

	uint32_t getThreadId() const { return m_threadId; }

	int32_t getLevel() const { return m_level; }

	const std::wstring& getText() const { return m_text; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_threadId;
	int32_t m_level;
	std::wstring m_text;
};

	}
}

