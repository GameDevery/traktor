#include "Core/Log/Log.h"
#include "Core/Misc/Endian.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Flash/Action/Avm1/ActionOperations.h"
#include "Flash/Action/Avm1/ActionVMImage1.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.ActionVMImage1", 0, ActionVMImage1, IActionVMImage)

ActionVMImage1::ActionVMImage1()
{
}

ActionVMImage1::ActionVMImage1(const uint8_t* byteCode, uint32_t length)
{
	if (length > 0)
	{
		m_byteCode.resize(length);
		std::memcpy(m_byteCode.ptr(), byteCode, length);
	}
}

void ActionVMImage1::prepare()
{
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

		if (op == AopEnd || op == AopReturn)
			break;

		state.npc = state.pc + 1;
		state.data = 0;

		// Decode instruction data.
		state.length = 1;
		if (op & 0x80)
		{
			uint16_t& length = *reinterpret_cast< uint16_t* >(state.pc + 1);
#if defined(T_BIG_ENDIAN)
			swap8in32(length);
#endif
			state.length = length;
			state.data = state.pc + 3;
			state.npc = state.data + state.length;
		}

		// Get instruction preparation handler and dispatch.
		const OperationInfo& info = c_operationInfos[op];
		T_ASSERT (info.op == op);

		if (info.prepare)
			info.prepare(state);

		// Update program counter.
		state.pc = state.npc;
	}
}

uint16_t ActionVMImage1::addConstData(const ActionValue& cd)
{
	m_constData.push_back(cd);
	return uint16_t(m_constData.size() - 1);
}

bool ActionVMImage1::serialize(ISerializer& s)
{
	uint32_t size = m_byteCode.size();

	s >> Member< uint32_t >(L"byteCodeSize", size);

	if (s.getDirection() == ISerializer::SdRead)
		m_byteCode.resize(size);

	void* data = m_byteCode.ptr();
	
	if (size > 0)
		s >> Member< void* >(L"byteCode", data, size);

	s >> MemberAlignedVector< ActionValue, MemberComposite< ActionValue > >(L"constData", m_constData);

	if (s.getDirection() == ISerializer::SdRead)
		prepare();

	return true;
}

	}
}
