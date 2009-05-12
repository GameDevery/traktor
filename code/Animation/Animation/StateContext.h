#ifndef traktor_animation_StateContext_H
#define traktor_animation_StateContext_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace animation
	{

/*! \brief State evaluation context.
 * \ingroup Animation
 */
class T_DLLCLASS StateContext : public Object
{
	T_RTTI_CLASS(StateContext)

public:
	StateContext();

	inline void setTime(float time)
	{
		m_time = time;
	}

	inline float getTime() const
	{
		return m_time;
	}

	inline void setDuration(float duration)
	{
		m_duration = duration;
	}

	inline float getDuration() const
	{
		return m_duration;
	}

private:
	float m_time;
	float m_duration;
};

	}
}

#endif	// traktor_animation_StateContext_H
