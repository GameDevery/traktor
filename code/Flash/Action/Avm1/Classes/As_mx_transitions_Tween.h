#ifndef traktor_flash_As_mx_transitions_Tween_H
#define traktor_flash_As_mx_transitions_Tween_H

#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

/*! \brief mx.transitions.Tween class.
 * \ingroup Flash
 */
class As_mx_transitions_Tween : public ActionClass
{
	T_RTTI_CLASS;

public:
	As_mx_transitions_Tween(ActionContext* context);

	virtual void initialize(ActionObject* self);

	virtual void construct(ActionObject* self, const ActionValueArray& args);

	virtual ActionValue xplicit(const ActionValueArray& args);
};

	}
}

#endif	// traktor_flash_As_mx_transitions_Tween_H
