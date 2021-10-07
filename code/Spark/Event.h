#pragma once

#include "Core/RefArray.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Any;
class IRuntimeDelegate;

	namespace spark
	{

/*! Event subject.
 * \ingroup Spark
 */
class T_DLLCLASS Event : public Object
{
	T_RTTI_CLASS;

public:
	IRuntimeDelegate* add(IRuntimeDelegate* rd);

	void remove(IRuntimeDelegate* rd);

	void removeAll();

	bool empty() const;

	void issue();

	void issue(int32_t argc, const Any* argv);

private:
	RefArray< IRuntimeDelegate > m_rds;
};

	}
}
