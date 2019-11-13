#include <cstring>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Timer/Timer.h"
#include "Spark/Action/ActionContext.h"
#include "Spark/Action/ActionFrame.h"
#include "Spark/Action/Avm1/ActionOperations.h"
#include "Spark/Action/Avm1/ActionVMImage1.h"
#if defined(_DEBUG)
#	include "Spark/Action/Avm1/ActionVMTrace1.h"
#endif

//#define T_TRACE_ENABLE

namespace traktor
{
	namespace spark
	{
		namespace
		{

Timer s_timer;

#if defined(__IOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)

template < int Size >
struct UnalignedView {};

template < >
struct UnalignedView< 1 > { uint8_t value; };

template < >
struct UnalignedView< 2 > { uint16_t value __attribute__(( packed )); };

template < >
struct UnalignedView< 4 > { uint32_t value __attribute__(( packed )); };

template < typename T >
T unalignedRead(const void* ptr)
{
	return ((UnalignedView< sizeof(T) >*)ptr)->value;
}

#else

template < typename T >
T unalignedRead(const void *ptr)
{
	return *(T T_UNALIGNED *)(ptr);
}

#endif

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.ActionVMImage1", 0, ActionVMImage1, IActionVMImage)

ActionVMImage1::ActionVMImage1()
:	m_prepared(false)
{
}

ActionVMImage1::ActionVMImage1(const uint8_t* byteCode, uint32_t length)
:	m_prepared(false)
{
	if (length > 0)
	{
		m_byteCode.resize(length);
		std::memcpy(m_byteCode.ptr(), byteCode, length);
	}
}

void ActionVMImage1::prepare()
{
	if (m_prepared)
		return;

	PreparationState state;
	state.image = this;
	state.pc = m_byteCode.ptr();
	state.npc = state.pc + 1;
	state.data = 0;
	state.length = 0;

	const uint8_t* end = state.pc + m_byteCode.size();
	while (state.pc < end)
	{
		const uint8_t op = *state.pc;

		state.npc = state.pc + 1;
		state.data = 0;

		// Decode instruction data.
		state.length = 1;
		if (op & 0x80)
		{
#if defined(__EMSCRIPTEN__)
			uint16_t length = unalignedRead< uint16_t >(state.pc + 1);
#else
			uint16_t& length = *reinterpret_cast< uint16_t* >(state.pc + 1);
#endif
#if defined(T_BIG_ENDIAN)
			swap8in32(length);
#endif
			state.length = length;
			state.data = state.pc + 3;
			state.npc = state.data + state.length;
		}

		// Get instruction preparation handler and dispatch.
		const OperationInfo& info = c_operationInfos[op];
		T_ASSERT(info.op == op);

		if (info.prepare)
			info.prepare(state);

		// Update program counter.
		state.pc = state.npc;
	}

	m_prepared = true;
}

uint16_t ActionVMImage1::addConstData(const ActionValue& cd)
{
	m_constData.push_back(cd);
	return uint16_t(m_constData.size() - 1);
}

void ActionVMImage1::execute(ActionFrame* frame) const
{
	const_cast< ActionVMImage1* >(this)->nonConstExecute(frame);
}

void ActionVMImage1::serialize(ISerializer& s)
{
	T_ASSERT(!m_prepared);

	uint32_t size = uint32_t(m_byteCode.size());
	s >> Member< uint32_t >(L"byteCodeSize", size);

	if (s.getDirection() == ISerializer::SdRead)
		m_byteCode.resize(size);

	void* data = m_byteCode.ptr();

	if (size > 0)
		s >> Member< void* >(L"byteCode", data, size);

	s >> MemberAlignedVector< ActionValue, MemberComposite< ActionValue > >(L"constData", m_constData);
}

void ActionVMImage1::nonConstExecute(ActionFrame* frame)
{
	// Prepare image on first execution.
	if (!m_prepared)
	{
		prepare();
		T_ASSERT(m_prepared);
	}

	// Setup execution state.
	ExecutionState state;
	state.image = this;
	state.frame = frame;
	state.pc = m_byteCode.c_ptr();
	state.npc = state.pc + 1;
	state.data = 0;
	state.length = 0;
	state.timer = &s_timer;

	// Cache frequently used instances.
	state.context = frame->getContext();
	state.self = frame->getSelf();
	state.global = frame->getContext()->getGlobal();
	state.movieClip = frame->getContext()->getMovieClip();
	state.trace = 0;

#if defined(_DEBUG) && defined(T_TRACE_ENABLE)
	ActionVMTrace1 trace;

	trace.beginDispatcher();
	state.trace = &trace.getTraceStream();

	const ActionDictionary* dictionary = state.frame->getDictionary();
	if (dictionary)
	{
		*state.trace << L"Dictionary:" << Endl;
		*state.trace << IncreaseIndent;

		const AlignedVector< ActionValue >& table = dictionary->getTable();
		for (uint32_t i = 0; i < table.size(); ++i)
			*state.trace << i << L". " << table[i].getWideString() << Endl;

		*state.trace << DecreaseIndent;
	}
#endif

	const uint8_t* end = state.pc + m_byteCode.size();
	while (state.pc < end)
	{
		const uint8_t op = *state.pc;

		if (op == AopEnd || op == AopReturn)
			break;

		state.npc = state.pc + 1;
		state.data = 0;

		// Decode instruction data.
		state.length = 1;
		if (op & 0x80)
		{
			state.length = unalignedRead< uint16_t >(state.pc + 1);
			state.data = state.pc + 3;
			state.npc = state.data + state.length;
		}

		// Get instruction handler and dispatch.
		const OperationInfo& info = c_operationInfos[op];
		T_ASSERT(info.op == op);
		T_ASSERT(info.execute != 0);

#if defined(_DEBUG) && defined(T_TRACE_ENABLE)
		trace.preDispatch(state, info);
#endif
		info.execute(state);
#if defined(_DEBUG) && defined(T_TRACE_ENABLE)
		trace.postDispatch(state, info);
#endif

		// Update program counter.
		state.pc = state.npc;
	}

#if defined(_DEBUG) && defined(T_TRACE_ENABLE)
	trace.endDispatcher();
#endif
}

	}
}
