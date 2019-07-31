#include "Spark/Action/ActionFunctionNative.h"
#include "Spark/Action/Common/Classes/As_mx_transitions_easing_None.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.As_mx_transitions_easing_None", As_mx_transitions_easing_None, ActionObject)

As_mx_transitions_easing_None::As_mx_transitions_easing_None(ActionContext* context)
:	ActionObject(context)
{
	setMember("easeIn", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_None::None_easeIn)));
	setMember("easeInOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_None::None_easeInOut)));
	setMember("easeOut", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_None::None_easeOut)));
	setMember("easeNone", ActionValue(createNativeFunction(context, this, &As_mx_transitions_easing_None::None_easeNone)));
}

void As_mx_transitions_easing_None::None_easeIn(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_None::None_easeInOut(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_None::None_easeOut(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();
	ca.ret = ActionValue(B + C * T / D);
}

void As_mx_transitions_easing_None::None_easeNone(CallArgs& ca)
{
	float T = ca.args[0].getFloat();
	float B = ca.args[1].getFloat();
	float C = ca.args[2].getFloat();
	float D = ca.args[3].getFloat();
	ca.ret = ActionValue(B + C * T / D);
}

	}
}
