/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_AccTextureCache_H
#define traktor_flash_AccTextureCache_H

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace render
	{

class IRenderSystem;
class ISimpleTexture;

	}

	namespace resource
	{

class IResourceManager;

	}

	namespace flash
	{

class AccBitmapRect;
class Bitmap;

/*! \brief Texture cache for accelerated rendering.
 * \ingroup Flash
 */
class AccTextureCache : public Object
{
public:
	AccTextureCache(
		resource::IResourceManager* resourceManager,
		render::IRenderSystem* renderSystem,
		bool reuseTextures
	);

	virtual ~AccTextureCache();

	void destroy();

	void clear();

	Ref< AccBitmapRect > getBitmapTexture(const Bitmap& bitmap);

	void freeTexture(render::ISimpleTexture* texture);

private:
	Ref< resource::IResourceManager > m_resourceManager;
	Ref< render::IRenderSystem > m_renderSystem;
	RefArray< render::ISimpleTexture > m_freeTextures;
	bool m_reuseTextures;
};

	}
}

#endif	// traktor_flash_AccTextureCache_H
