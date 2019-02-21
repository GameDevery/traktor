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

/*! \brief Simple 2d texture.
 * \ingroup Render
 */
class T_DLLCLASS ISimpleTexture : public ITexture
{
	T_RTTI_CLASS;
	
public:
	/*! \brief Get width in pixels of texture.
	 *
	 * \return Texture width in pixels.
	 */
	virtual int32_t getWidth() const = 0;

	/*! \brief Get height in pixels of texture.
	 *
	 * \return Texture height in pixels.
	 */
	virtual int32_t getHeight() const = 0;

	/*! \brief Lock access to texture data.
	 *
	 * \param level Mip level.
	 * \param lock Information about locked region.
	 * \return True if locked.
	 */
	virtual bool lock(int32_t level, Lock& lock) = 0;

	/*! \brief Unlock access to texture data.
	 *
	 * \param level Mip level.
	 */
	virtual void unlock(int32_t level) = 0;

	/*! \brief Get API specific internal handle.
	 *
	 * \return API handle to texture.
	 */
	virtual void* getInternalHandle() = 0;
};

	}
}
