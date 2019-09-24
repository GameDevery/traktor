#include <cstring>
#include "Core/Log/Log.h"
#include "Spark/BitmapImage.h"
#include "Spark/BitmapResource.h"
#include "Spark/BitmapTexture.h"
#include "Spark/Acc/AccBitmapRect.h"
#include "Spark/Acc/AccTextureCache.h"
#include "Render/IRenderSystem.h"
#include "Render/ISimpleTexture.h"
#include "Resource/IResourceManager.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

class AccCachedTexture : public render::ISimpleTexture
{
public:
	AccCachedTexture(AccTextureCache* cache, render::ISimpleTexture* texture)
	:	m_cache(cache)
	,	m_texture(texture)
	{
	}

	virtual ~AccCachedTexture()
	{
		destroy();
	}

	virtual void destroy() override final
	{
		if (m_texture)
		{
			m_cache->freeTexture(m_texture);
			m_texture = 0;
		}
	}

	virtual render::ITexture* resolve() override final
	{
		return m_texture;
	}

	virtual int32_t getWidth() const override final
	{
		return m_texture->getWidth();
	}

	virtual int32_t getHeight() const override final
	{
		return m_texture->getHeight();
	}

	virtual int32_t getMips() const override final
	{
		return m_texture->getMips();
	}

	virtual bool lock(int32_t level, Lock& lock) override final
	{
		return m_texture->lock(level, lock);
	}

	virtual void unlock(int32_t level) override final
	{
		m_texture->unlock(level);
	}

	virtual void* getInternalHandle() override final
	{
		return m_texture->getInternalHandle();
	}

private:
	Ref< AccTextureCache > m_cache;
	Ref< render::ISimpleTexture > m_texture;
};

		}

AccTextureCache::AccTextureCache(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	bool reuseTextures
)
:	m_resourceManager(resourceManager)
,	m_renderSystem(renderSystem)
,	m_reuseTextures(reuseTextures)
{
}

AccTextureCache::~AccTextureCache()
{
	T_EXCEPTION_GUARD_BEGIN

	destroy();

	T_EXCEPTION_GUARD_END
}

void AccTextureCache::destroy()
{
	m_resourceManager = 0;
	m_renderSystem = 0;
}

void AccTextureCache::clear()
{
	m_freeTextures.clear();
}

Ref< AccBitmapRect > AccTextureCache::getBitmapTexture(const Bitmap& bitmap)
{
	AccBitmapRect* bmr = static_cast< AccBitmapRect* >(bitmap.getCacheObject());
	if (bmr)
		return bmr;

	if (const BitmapResource* bitmapResource = dynamic_type_cast< const BitmapResource* >(&bitmap))
	{
		resource::Proxy< render::ISimpleTexture > texture;

		m_resourceManager->bind(
			resource::Id< render::ISimpleTexture >(bitmapResource->getResourceId()),
			texture
		);

		float w = float(bitmapResource->getAtlasWidth());
		float h = float(bitmapResource->getAtlasHeight());

		Ref< AccBitmapRect > br = new AccBitmapRect(
			texture,
			bitmapResource->getX() / w,
			bitmapResource->getY() / h,
			bitmapResource->getWidth() / w,
			bitmapResource->getHeight() / h
		);

		bitmap.setCacheObject(br);
		return br;
	}
	else if (const BitmapTexture* bitmapTexture = dynamic_type_cast< const BitmapTexture* >(&bitmap))
	{
		Ref< AccBitmapRect > br = new AccBitmapRect(
			resource::Proxy< render::ISimpleTexture >(bitmapTexture->getTexture()),
			0.0f,
			0.0f,
			1.0f,
			1.0f
		);

		bitmap.setCacheObject(br);
		return br;
	}
	else if (const BitmapImage* bitmapData = dynamic_type_cast< const BitmapImage* >(&bitmap))
	{
		Ref< render::ISimpleTexture > texture;

		// Check if any free texture matching requested size.
		if (m_reuseTextures)
		{
			for (RefArray< render::ISimpleTexture >::iterator i = m_freeTextures.begin(); i != m_freeTextures.end(); ++i)
			{
				if (i->getWidth() == bitmapData->getWidth() && i->getHeight() == bitmapData->getHeight())
				{
					texture = *i;
					m_freeTextures.erase(i);
					break;
				}
			}
		}

		// No such texture found; create new texture.
		if (!texture)
		{
			render::SimpleTextureCreateDesc desc;

			desc.width = bitmapData->getWidth();
			desc.height = bitmapData->getHeight();
			desc.mipCount = 1;
			desc.format = render::TfR8G8B8A8;
			desc.immutable = false;

			texture = resource::Proxy< render::ISimpleTexture >(m_renderSystem->createSimpleTexture(desc, T_FILE_LINE_W));
		}

		if (!texture)
			return 0;

		render::ITexture::Lock tl;
		if (texture->lock(0, tl))
		{
			const uint8_t* s = reinterpret_cast< const uint8_t* >(bitmapData->getBits());
			uint8_t* d = static_cast< uint8_t* >(tl.bits);

			for (uint32_t y = 0; y < bitmapData->getHeight(); ++y)
			{
				std::memcpy(d, s, bitmapData->getWidth() * 4);
				s += bitmapData->getWidth() * 4;
				d += tl.pitch;
			}

			texture->unlock(0);
		}

		Ref< AccBitmapRect > br = new AccBitmapRect(
			resource::Proxy< render::ISimpleTexture >(new AccCachedTexture(this, texture)),
			0.0f,
			0.0f,
			1.0f,
			1.0f
		);

		bitmap.setCacheObject(br);
		return br;
	}

	return 0;
}

void AccTextureCache::freeTexture(render::ISimpleTexture* texture)
{
	if (!texture)
		return;

	if (m_reuseTextures)
		m_freeTextures.push_back(texture);
	else
		texture->destroy();
}

	}
}
