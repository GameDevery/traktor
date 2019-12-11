#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

class StateNode;
class Transition;

/*! Animation state graph.
 * \ingroup Animation
 */
class T_DLLCLASS StateGraph : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addState(StateNode* state);

	void removeState(StateNode* state);

	const RefArray< StateNode >& getStates() const;

	void addTransition(Transition* transition);

	void removeTransition(Transition* transition);

	const RefArray< Transition >& getTransitions() const;

	void setRootState(StateNode* rootState);

	Ref< StateNode > getRootState() const;

	virtual void serialize(ISerializer& s) override final;

private:
	RefArray< StateNode > m_states;
	RefArray< Transition > m_transitions;
	Ref< StateNode > m_rootState;
};

	}
}

