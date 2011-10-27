#include <limits>
#include "Core/Misc/String.h"
#include "Flash/Action/ActionFunctionNative.h"
#include "Flash/Action/Classes/Number.h"
#include "Flash/Action/Avm1/Classes/AsNumber.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.AsNumber", AsNumber, ActionClass)

AsNumber::AsNumber(ActionContext* context)
:	ActionClass(context, "Number")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember("MAX_VALUE", ActionValue(std::numeric_limits< avm_number_t >::max()));
	prototype->setMember("MIN_VALUE", ActionValue(std::numeric_limits< avm_number_t >::min()));
	prototype->setMember("NaN", ActionValue(std::numeric_limits< avm_number_t >::signaling_NaN()));
	prototype->setMember("NEGATIVE_INFINITY", ActionValue(-std::numeric_limits< avm_number_t >::infinity()));
	prototype->setMember("POSITIVE_INFINITY", ActionValue(std::numeric_limits< avm_number_t >::infinity()));
	prototype->setMember("toString", ActionValue(createNativeFunction(context, this, &AsNumber::Number_toString)));
	prototype->setMember("valueOf", ActionValue(createNativeFunction(context, this, &AsNumber::Number_valueOf)));

	prototype->setMember("constructor", ActionValue(this));
	prototype->setReadOnly();

	setMember("prototype", ActionValue(prototype));
}

void AsNumber::initialize(ActionObject* self)
{
}

void AsNumber::construct(ActionObject* self, const ActionValueArray& args)
{
	Ref< Number > n;
	if (args.size() > 0)
		n = new Number(args[0].getNumber());
	else
		n = new Number(avm_number_t(0));
	self->setRelay(n);
}

ActionValue AsNumber::xplicit(const ActionValueArray& args)
{
	if (args.size() > 0)
		return ActionValue(args[0].getNumber());
	else
		return ActionValue();
}

std::wstring AsNumber::Number_toString(const Number* self) const
{
	return traktor::toString(self->get());
}

avm_number_t AsNumber::Number_valueOf(const Number* self) const
{
	return self->get();
}

	}
}
