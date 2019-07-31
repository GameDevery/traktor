#include "Core/Log/Log.h"
#include "Spark/Action/ActionContext.h"
#include "Spark/Action/ActionFunctionNative.h"
#include "Spark/Action/Common/Array.h"
#include "Spark/Action/Common/Classes/AsFunction.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.AsFunction", AsFunction, ActionClass)

AsFunction::AsFunction(ActionContext* context)
:	ActionClass(context, "Function")
{
	Ref< ActionObject > prototype = new ActionObject(context);

	prototype->setMember(ActionContext::IdApply, ActionValue(createNativeFunction(context, this, &AsFunction::Function_apply)));
	prototype->setMember(ActionContext::IdCall, ActionValue(createNativeFunction(context, this, &AsFunction::Function_call)));
	prototype->setMember(ActionContext::IdToString, ActionValue(createNativeFunction(context, this, &AsFunction::Function_toString)));

	prototype->setMember(ActionContext::IdConstructor, ActionValue(this));
	prototype->setReadOnly();

	setMember(ActionContext::IdPrototype, ActionValue(prototype));
}

void AsFunction::initialize(ActionObject* self)
{
}

void AsFunction::construct(ActionObject* self, const ActionValueArray& args)
{
}

ActionValue AsFunction::xplicit(const ActionValueArray& args)
{
	return ActionValue();
}

void AsFunction::Function_apply(CallArgs& ca)
{
	if (ca.args.size() < 2)
	{
		log::error << L"Function.apply, incorrect number of arguments" << Endl;
		return;
	}

	ActionFunction* function = dynamic_type_cast< ActionFunction* >(ca.self);
	if (!function)
	{
		log::error << L"Function.apply, invalid object" << Endl;
		return;
	}

	Ref< ActionObject > self = ca.args[0].getObjectAlways(ca.context);
	Ref< Array > args = ca.args[1].getObjectAlways(ca.context)->getRelay< Array >();

	if (args)
	{
		const AlignedVector< ActionValue >& argValues = args->getValues();
		ActionValueArray argv(getContext()->getPool(), uint32_t(argValues.size()));
		for (uint32_t i = 0; i < uint32_t(argValues.size()); ++i)
			argv[i] = argValues[i];
		ca.ret = function->call(self, argv);
	}
	else
		ca.ret = function->call(self);
}

void AsFunction::Function_call(CallArgs& ca)
{
	if (ca.args.size() < 1)
	{
		log::error << L"Function.call, incorrect number of arguments" << Endl;
		return;
	}

	ActionFunction* function = dynamic_type_cast< ActionFunction* >(ca.self);
	if (!function)
	{
		log::error << L"Function.apply, invalid object" << Endl;
		return;
	}

	Ref< ActionObject > self = ca.args[0].getObjectAlways(ca.context);

	ActionValueArray args(ca.context->getPool(), ca.args.size() - 1);
	for (uint32_t i = 0; i < ca.args.size() - 1; ++i)
		args[i] = ca.args[i + 1];

	ca.ret = function->call(self, args);
}

void AsFunction::Function_toString(CallArgs& ca)
{
	ca.ret = ActionValue("[type Function]");
}

	}
}
