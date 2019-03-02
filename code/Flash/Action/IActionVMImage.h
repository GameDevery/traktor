#pragma once

#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace flash
	{

class ActionFrame;

/*! \brief ActionScript virtual machine image interface.
 * \ingroup Flash
 */
class IActionVMImage : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! \brief Execute image. */
	virtual void execute(ActionFrame* frame) const = 0;
};

	}
}

