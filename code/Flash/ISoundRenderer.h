#ifndef traktor_flash_ISoundRenderer_H
#define traktor_flash_ISoundRenderer_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashSound;

/*! \brief Sound rendering interface.
 * \ingroup Flash
 */
class T_DLLCLASS ISoundRenderer : public Object
{
	T_RTTI_CLASS;

public:
	virtual void destroy() = 0;

	virtual void play(const FlashSound& sound) = 0;
};

	}
}

#endif	// traktor_flash_ISoundRenderer_H
