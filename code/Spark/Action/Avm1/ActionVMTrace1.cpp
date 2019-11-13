#include <map>
#include "Core/Misc/String.h"
#include "Spark/Action/ActionFrame.h"
#include "Spark/Action/ActionObjectRelay.h"
#include "Spark/Action/ActionSuper.h"
#include "Spark/Action/Avm1/ActionOperations.h"
#include "Spark/Action/Avm1/ActionVMTrace1.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

std::map< intptr_t, uint32_t > g_objectIds;
uint32_t g_nextObjectId = 1;

uint32_t getObjectId(void* object)
{
	std::map< intptr_t, uint32_t >::const_iterator i = g_objectIds.find(intptr_t(object));
	if (i != g_objectIds.end())
		return i->second;

	uint32_t id = g_nextObjectId++;
	g_objectIds.insert(std::make_pair(intptr_t(object), id));

	return id;
}

std::wstring describeValue(const ActionValue& v)
{
	StringOutputStream ss;

	if (v.isUndefined())
		ss << L"undefined (void)";
	else if (v.isBoolean())
		ss << v.getWideString() << L" (boolean)";
	else if (v.isNumeric())
		ss <<v.getWideString() << L" (numeric)";
	else if (v.isString())
		ss << L"\"" << v.getWideString() << L"\" (string)";
	else if (v.isObject())
	{
		ActionObject* object = v.getObject();
		if (object)
		{
			ActionFunction* fn = dynamic_type_cast< ActionFunction* >(object);
			if (fn)
			{
#if defined(_DEBUG)
				if (is_a< ActionSuper >(fn))
					ss << L"[type Super] (function @" << getObjectId(fn) << L" \"" << mbstows(fn->getName()) << L"\"";
				else
					ss << L"[type Function] (function @" << getObjectId(fn) << L" \"" << mbstows(fn->getName()) << L"\"";
#else
				if (is_a< ActionSuper >(fn))
					ss << L"[type Super] (function @" << getObjectId(fn);
				else
					ss << L"[type Function] (function @" << getObjectId(fn);
#endif
			}
			else
				ss << L"[object Object] (object @" << getObjectId(object);

			ActionObjectRelay* relay = object->getRelay();
			if (relay)
				ss << L", relay @" << getObjectId(relay) << L" " << type_name(relay);

			ss << L")";
		}
		else
			ss << L"null (object)";
	}
	else
		ss << L"garbage";

	return ss.str();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ActionVMTrace1", ActionVMTrace1, Object)

ActionVMTrace1::ActionVMTrace1()
{
	g_objectIds.clear();
	g_nextObjectId = 1;
}

ActionVMTrace1::~ActionVMTrace1()
{
}

void ActionVMTrace1::beginDispatcher()
{
	m_stream = &log::info;
	(*m_stream) << L"Begin dispatcher" << Endl;
}

void ActionVMTrace1::endDispatcher()
{
	(*m_stream) << L"End dispatcher" << Endl;
}

void ActionVMTrace1::preDispatch(const ExecutionState& state, const OperationInfo& info)
{
	(*m_stream) << mbstows(info.name) << Endl;
	(*m_stream) << Endl;

	(*m_stream) << L"Stack (pre):" << Endl;
	(*m_stream) << IncreaseIndent;

	const ActionValueStack& stack = state.frame->getStack();
	for (int32_t i = 0; i < stack.depth(); ++i)
		(*m_stream) << i << L". " << describeValue(stack.top(-i)) << Endl;

	(*m_stream) << DecreaseIndent;

	(*m_stream) << L"Registers (pre):" << Endl;
	(*m_stream) << IncreaseIndent;

	const ActionValueArray& registers = state.frame->getRegisters();
	for (uint32_t i = 0; i < registers.size(); ++i)
	{
		if (!registers[i].isUndefined())
			(*m_stream) << uint32_t(i) << L". " << describeValue(registers[i]) << Endl;
	}

	(*m_stream) << DecreaseIndent;

	(*m_stream) << Endl;
	(*m_stream) << IncreaseIndent;
}

void ActionVMTrace1::postDispatch(const ExecutionState& state, const OperationInfo& info)
{
	(*m_stream) << DecreaseIndent;
	(*m_stream) << Endl;

	(*m_stream) << L"Stack (post):" << Endl;
	(*m_stream) << IncreaseIndent;

	const ActionValueStack& stack = state.frame->getStack();
	for (int32_t i = 0; i < stack.depth(); ++i)
		(*m_stream) << i << L". " << describeValue(stack.top(-i)) << Endl;

	(*m_stream) << DecreaseIndent;

	(*m_stream) << L"Registers (post):" << Endl;
	(*m_stream) << IncreaseIndent;

	const ActionValueArray& registers = state.frame->getRegisters();
	for (uint32_t i = 0; i < registers.size(); ++i)
	{
		if (!registers[i].isUndefined())
			(*m_stream) << uint32_t(i) << L". " << describeValue(registers[i]) << Endl;
	}

	(*m_stream) << DecreaseIndent;

	(*m_stream) << L"===========================================================" << Endl;
}

OutputStream& ActionVMTrace1::getTraceStream()
{
	return (*m_stream);
}

	}
}
