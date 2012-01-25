#ifndef traktor_spray_SequenceInstance_H
#define traktor_spray_SequenceInstance_H

#include "Core/Object.h"
#include "Spray/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

struct Context;
class ITriggerInstance;

/*! \brief Sequence instance.
 * \ingroup Spray
 */
class T_DLLCLASS SequenceInstance : public Object
{
	T_RTTI_CLASS;

public:
	void update(Context& context, float T);

private:
	friend class Sequence;

	struct Key
	{
		float T;
		Ref< ITriggerInstance > trigger;	
	};

	std::vector< Key > m_keys;
	uint32_t m_index;
	float m_lastT;

	SequenceInstance(const std::vector< Key >& keys);
};

	}
}

#endif	// traktor_spray_SequenceInstance_H
