#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Sound/Processor/InputPin.h"
#include "Sound/Processor/Node.h"
#include "Sound/Processor/OutputPin.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_VERSION_CLASS(L"traktor.sound.Node", 0, Node, ISerializable)

Node::Node()
:	m_id(Guid::create())
,	m_position(0, 0)
{
}

void Node::setId(const Guid& instanceId)
{
	m_id = instanceId;
}

const Guid& Node::getId() const
{
	return m_id;
}

void Node::setComment(const std::wstring& comment)
{
	m_comment = comment;
}

const std::wstring& Node::getComment() const
{
	return m_comment;
}

std::wstring Node::getInformation() const
{
	return L"";
}

void Node::setPosition(const std::pair< int, int >& position)
{
	m_position = position;
}

const std::pair< int, int >& Node::getPosition() const
{
	return m_position;
}

const InputPin* Node::findInputPin(const std::wstring& name) const
{
	int count = getInputPinCount();
	for (int i = 0; i < count; ++i)
	{
		const InputPin* inputPin = getInputPin(i);
		T_ASSERT (inputPin);

		if (inputPin->getName() == name)
			return inputPin;
	}
	return 0;
}

const OutputPin* Node::findOutputPin(const std::wstring& name) const
{
	int count = getOutputPinCount();
	for (int i = 0; i < count; ++i)
	{
		const OutputPin* outputPin = getOutputPin(i);
		T_ASSERT (outputPin);

		if (outputPin->getName() == name)
			return outputPin;
	}
	return 0;
}

void Node::serialize(ISerializer& s)
{
	if (s.getVersion< Node >() >= 1)
		s >> Member< Guid >(L"id", m_id, AttributePrivate());
	else if (s.getDirection() == ISerializer::SdRead)
		m_id = Guid();

	s >> Member< std::wstring >(L"comment", m_comment, AttributeMultiLine());
	s >> MemberStlPair< int, int >(L"position", m_position, AttributePrivate());
}

	}
}
