#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/ISoundBuffer.h"
#include "Sound/Processor/Nodes/Parameter.h"

namespace traktor
{
	namespace sound
	{
		namespace
		{

const ImmutableNode::OutputPinDesc c_Parameter_o[] =
{
	{ L"Output", NptScalar },
	{ nullptr }
};

class ParameterCursor : public RefCountImpl< ISoundBufferCursor >
{
public:
	handle_t m_id;
	float m_value;

	ParameterCursor(handle_t id, float defaultValue)
	:	m_id(id)
	,	m_value(defaultValue)
	{
	}

	virtual void setParameter(handle_t id, float parameter) override final
	{
		if (id == m_id)
			m_value = parameter;
	}

	virtual void disableRepeat() override final {}

	virtual void reset() override final {}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.Parameter", 1, Parameter, ImmutableNode)

Parameter::Parameter()
:	ImmutableNode(nullptr, c_Parameter_o)
{
}

bool Parameter::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

Ref< ISoundBufferCursor > Parameter::createCursor() const
{
	return new ParameterCursor(getParameterHandle(m_name), m_defaultValue);
}

bool Parameter::getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outParameter) const
{
	ParameterCursor* parameterCursor = static_cast< ParameterCursor* >(cursor);
	outParameter = parameterCursor->m_value;
	return true;
}

bool Parameter::getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const IAudioMixer* mixer, SoundBlock& outBlock) const
{
	return false;
}

void Parameter::serialize(ISerializer& s)
{
	ImmutableNode::serialize(s);

	s >> Member< std::wstring >(L"name", m_name);

	if (s.getVersion< Parameter >() >= 1)
		s >> Member< float >(L"defaultValue", m_defaultValue);
}

	}
}
