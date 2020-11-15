#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace shape
	{

class IblProbe;

class T_DLLCLASS TracerEnvironment : public Object
{
	T_RTTI_CLASS;

public:
	explicit TracerEnvironment(const IblProbe* environment);

	const IblProbe* getEnvironment() const { return m_environment; }

private:
	Ref< const IblProbe > m_environment;
};

	}
}