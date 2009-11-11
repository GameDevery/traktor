#include "Flash/AccTextureCache.h"
#include "Flash/FlashFillStyle.h"
#include "Flash/FlashBitmap.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

SwfColor interpolateGradient(const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords, float f)
{
	if (colorRecords.size() <= 1)
		return colorRecords[0].color;

	int i = 0;
	while (i < int(colorRecords.size() - 2) && colorRecords[i + 1].ratio < f)
		++i;

	const FlashFillStyle::ColorRecord& a = colorRecords[i];
	const FlashFillStyle::ColorRecord& b = colorRecords[i + 1];

	if (f <= a.ratio)
		return a.color;

	if (f >= b.ratio)
		return b.color;

	if ((b.ratio - a.ratio) > 0.0f)
		f = (f - a.ratio) / (b.ratio - a.ratio);
	else
		f = 0.0f;

	SwfColor color =
	{
		uint8_t(a.color.red   * (1.0f - f) + b.color.red   * f),
		uint8_t(a.color.green * (1.0f - f) + b.color.green * f),
		uint8_t(a.color.blue  * (1.0f - f) + b.color.blue  * f),
		uint8_t(a.color.alpha * (1.0f - f) + b.color.alpha * f)
	};

	return color;
}

		}

AccTextureCache::AccTextureCache(render::IRenderSystem* renderSystem)
:	m_renderSystem(renderSystem)
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
	clear();
	m_renderSystem = 0;
}

void AccTextureCache::clear()
{
	for (std::map< uint32_t, Ref< render::ITexture > >::iterator i = m_cache.begin(); i != m_cache.end(); ++i)
	{
		if (i->second)
			i->second->destroy();
	}
	m_cache.clear();
}

Ref< render::ITexture > AccTextureCache::getGradientTexture(const FlashFillStyle& style)
{
	Ref< render::ISimpleTexture > texture;

	uint32_t hash = reinterpret_cast< uint32_t >(&style);
	std::map< uint32_t, Ref< render::ITexture > >::iterator it = m_cache.find(hash);
	if (it != m_cache.end())
		return it->second;

	const AlignedVector< FlashFillStyle::ColorRecord >& colorRecords = style.getColorRecords();
	T_ASSERT (colorRecords.size() > 1);

	if (style.getGradientType() == FlashFillStyle::GtLinear)
	{
		SwfColor gradientBitmap[256];

		int x1 = 0;
		for (int i = 1; i < int(colorRecords.size()); ++i)
		{
			int x2 = int(colorRecords[i].ratio * 256);
			for (int x = x1; x < x2; ++x)
			{
				float f = float(x - x1) / (x2 - x1);
				SwfColor& c = gradientBitmap[x];
				c.red = uint8_t(colorRecords[i - 1].color.red * (1.0f - f) + colorRecords[i].color.red * f);
				c.green = uint8_t(colorRecords[i - 1].color.green * (1.0f - f) + colorRecords[i].color.green * f);
				c.blue = uint8_t(colorRecords[i - 1].color.blue * (1.0f - f) + colorRecords[i].color.blue * f);
				c.alpha = uint8_t(colorRecords[i - 1].color.alpha * (1.0f - f) + colorRecords[i].color.alpha * f);
			}
			x1 = x2;
		}

		render::SimpleTextureCreateDesc desc;

		desc.width = 256;
		desc.height = 1;
		desc.mipCount = 1;
		desc.format = render::TfR8G8B8A8;
		desc.immutable = true;
		desc.initialData[0].data = gradientBitmap;
		desc.initialData[0].pitch = 256 * 4;

		texture = m_renderSystem->createSimpleTexture(desc);
	}
	else if (style.getGradientType() == FlashFillStyle::GtRadial)
	{
		SwfColor gradientBitmap[64 * 64];

		for (int y = 0; y < 64; ++y)
		{
			for (int x = 0; x < 64; ++x)
			{
				float fx = x / 31.5f - 1.0f;
				float fy = y / 31.5f - 1.0f;
				float f = sqrtf(fx * fx + fy * fy);
				gradientBitmap[x + y * 64] = interpolateGradient(colorRecords, f);
			}
		}

		render::SimpleTextureCreateDesc desc;

		desc.width = 64;
		desc.height = 64;
		desc.mipCount = 1;
		desc.format = render::TfR8G8B8A8;
		desc.immutable = true;
		desc.initialData[0].data = gradientBitmap;
		desc.initialData[0].pitch = 64 * 4;

		texture = m_renderSystem->createSimpleTexture(desc);
	}

	T_ASSERT (texture);
	m_cache[hash] = texture;

	return texture;
}

Ref< render::ITexture > AccTextureCache::getBitmapTexture(const FlashBitmap& bitmap)
{
	uint32_t hash = reinterpret_cast< uint32_t >(&bitmap);
	std::map< uint32_t, Ref< render::ITexture > >::iterator it = m_cache.find(hash);
	if (it != m_cache.end())
		return it->second;

	render::SimpleTextureCreateDesc desc;

	desc.width = bitmap.getBitsWidth();
	desc.height = bitmap.getBitsHeight();
	desc.mipCount = 1;
	desc.format = render::TfR8G8B8A8;
	desc.immutable = true;
	desc.initialData[0].data = bitmap.getBits();
	desc.initialData[0].pitch = bitmap.getBitsWidth() * 4;

	Ref< render::ISimpleTexture > texture = m_renderSystem->createSimpleTexture(desc);
	T_ASSERT (texture);

	m_cache[hash] = texture;
	return texture;
}

	}
}
