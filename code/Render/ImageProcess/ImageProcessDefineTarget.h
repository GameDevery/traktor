#pragma once

#include "Core/Math/Color4f.h"
#include "Render/Types.h"
#include "Render/ImageProcess/ImageProcessDefine.h"

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

/*! \brief Define post processing render target.
 * \ingroup Render
 */
class T_DLLCLASS ImageProcessDefineTarget : public ImageProcessDefine
{
	T_RTTI_CLASS;

public:
	ImageProcessDefineTarget();

	virtual bool define(ImageProcess* imageProcess, resource::IResourceManager* resourceManager, IRenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight) override final;

	virtual void serialize(ISerializer& s) override final;

	const std::wstring& getId() const { return m_id; }

	bool persistent() const { return m_persistent; }

private:
	std::wstring m_id;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_screenWidthDenom;
	uint32_t m_screenHeightDenom;
	uint32_t m_maxWidth;
	uint32_t m_maxHeight;
	TextureFormat m_format;
	bool m_depthStencil;
	bool m_preferTiled;
	bool m_generateMips;
	int32_t m_multiSample;
	Color4f m_clearColor;
	bool m_persistent;
};

	}
}
