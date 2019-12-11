#pragma once

#include "Core/Object.h"

namespace traktor
{
	namespace db
	{

class Context;

/*! Transaction action.
 * \ingroup Database
 */
class Action : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool execute(Context* context) = 0;

	virtual bool undo(Context* context) = 0;

	virtual void clean(Context* context) = 0;

	virtual bool redundant(const Action* action) const = 0;
};

	}
}

