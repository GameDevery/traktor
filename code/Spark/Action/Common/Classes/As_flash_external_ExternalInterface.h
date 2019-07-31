#pragma once

#include <map>
#include "Spark/Action/ActionObject.h"

namespace traktor
{
	namespace spark
	{

struct CallArgs;

struct IExternalCall
{
	virtual ~IExternalCall() {}

	virtual ActionValue dispatchExternalCall(const std::string& methodName, int32_t argc, const ActionValue* argv) = 0;
};

class As_flash_external_ExternalInterface : public ActionObject
{
	T_RTTI_CLASS;

public:
	As_flash_external_ExternalInterface(ActionContext* context);

	void setExternalCall(IExternalCall* externalCall);

	ActionValue dispatchCallback(const std::string& methodName, int32_t argc, const ActionValue* argv);

protected:
	virtual void trace(visitor_t visitor) const override final;

	virtual void dereference() override final;

private:
	struct Callback
	{
		Ref< ActionObject > instance;
		Ref< ActionFunction > method;
	};

	std::map< std::string, Callback > m_callbacks;
	IExternalCall* m_externalCall;

	void ExternalInterface_get_available(CallArgs& ca);

	void ExternalInterface_addCallback(CallArgs& ca);

	void ExternalInterface_call(CallArgs& ca);
};

	}
}

