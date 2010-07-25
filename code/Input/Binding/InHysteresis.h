#ifndef traktor_input_InHysteresis_H
#define traktor_input_InHysteresis_H

#include "Input/Binding/IInputNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS InHysteresis : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	InHysteresis();

	InHysteresis(IInputNode* source, float limitMin, float limitMax, float outputMin, float outputMax);
	
	virtual Ref< Instance > createInstance() const;

	virtual float evaluate(
		Instance* instance,
		const InputValueSet& valueSet,
		float T,
		float dT
	) const;

	virtual bool serialize(ISerializer& s);
	
private:
	Ref< IInputNode > m_source;
	float m_limit[2];
	float m_output[2];
};

	}
}

#endif	// traktor_input_InHysteresis_H
