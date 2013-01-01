#ifndef traktor_input_InTrigger_H
#define traktor_input_InTrigger_H

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

/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InTrigger : public IInputNode
{
	T_RTTI_CLASS;
	
public:
	enum Flank
	{
		FlPositive,
		FlNegative
	};
	
	InTrigger();

	InTrigger(IInputNode* source, Flank flank, float duration);
	
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
	Flank m_flank;
	float m_duration;
};

	}
}

#endif	// traktor_input_InTrigger_H
