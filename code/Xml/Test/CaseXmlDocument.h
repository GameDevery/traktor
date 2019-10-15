#pragma once

#include "Core/Test/Case.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_XML_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace xml
	{
		namespace test
		{

class T_DLLCLASS CaseXmlDocument : public traktor::test::Case
{
	T_RTTI_CLASS;

public:
	virtual void run() override final;
};

		}
	}
}

