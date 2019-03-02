#pragma once

#include "Render/ITexture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Cube texture.
 * \ingroup Render
 */
class T_DLLCLASS ICubeTexture : public ITexture
{
	T_RTTI_CLASS;

public:
	enum Side
	{
		SdPositiveX	= 0,
		SdNegativeX = 1,
		SdPositiveY = 2,
		SdNegativeY = 3,
		SdPositiveZ = 4,
		SdNegativeZ = 5
	};

	/*! \brief Size of cube map side in pixels.
	 */
	virtual int32_t getSide() const = 0;

	/*! \brief Lock access to texture data.
	 *
	 * \param side Cube side.
	 * \param level Mip level.
	 * \param lock Information about locked region.
	 * \return True if locked.
	 */
	virtual bool lock(int32_t side, int32_t level, Lock& lock) = 0;

	/*! \brief Unlock access to texture data.
	 *
	 * \param side Cube side.
	 * \param level Mip level.
	 */
	virtual void unlock(int32_t side, int32_t level) = 0;
};

	}
}
