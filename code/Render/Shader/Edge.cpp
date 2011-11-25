#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Render/Shader/Edge.h"
#include "Render/Shader/InputPin.h"
#include "Render/Shader/Node.h"
#include "Render/Shader/OutputPin.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct OutputPinAccessor
{
	static const OutputPin* get(Ref< Node >& node, const std::wstring& name) {
		return node->findOutputPin(name);
	}
};

struct InputPinAccessor
{
	static const InputPin* get(Ref< Node >& node, const std::wstring& name) {
		return node->findInputPin(name);
	}
};

template < typename PinType, typename PinAccessor >
class MemberPin : public MemberComplex
{
public:
	MemberPin(const wchar_t* const name, PinType*& pin)
	:	MemberComplex(name, true)
	,	m_pin(pin)
	{
	}

	virtual bool serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::SdWrite)
		{
			Ref< Node > node = m_pin->getNode();
			s >> MemberRef< Node >(L"node", node);
			std::wstring name = m_pin->getName();
			s >> Member< std::wstring >(L"name", name);
		}
		else	// SdRead
		{
			Ref< Node > node;
			s >> MemberRef< Node >(L"node", node);
			std::wstring name;
			s >> Member< std::wstring >(L"name", name);
			m_pin = PinAccessor::get(node, name);
		}
		return true;
	}

private:
	PinType*& m_pin;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Edge", 0, Edge, ISerializable)

Edge::Edge(const OutputPin* source, const InputPin* destination)
:	m_source(source)
,	m_destination(destination)
{
}

void Edge::setSource(const OutputPin* source)
{
	m_source = source;
}

const OutputPin* Edge::getSource() const
{
	return m_source;
}

void Edge::setDestination(const InputPin* destination)
{
	m_destination = destination;
}

const InputPin* Edge::getDestination() const
{
	return m_destination;
}

bool Edge::serialize(ISerializer& s)
{
	s >> MemberPin< const OutputPin, OutputPinAccessor >(L"source", m_source);
	s >> MemberPin< const InputPin, InputPinAccessor >(L"destination", m_destination);
	return true;
}

	}
}
