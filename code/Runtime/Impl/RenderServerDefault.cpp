#include <cmath>
#include <limits>
#include "Runtime/IEnvironment.h"
#include "Runtime/Impl/RenderServerDefault.h"
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/IVRCompositor.h"
#include "Render/Image2/ImageGraphFactory.h"
#include "Render/Resource/AliasTextureFactory.h"
#include "Render/Resource/ShaderFactory.h"
#include "Render/Resource/SequenceTextureFactory.h"
#include "Render/Resource/TextureFactory.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace runtime
	{
		namespace
		{

const float c_maxAspectRatio = 8.0f;

int32_t sanitizeMultiSample(int32_t multiSample)
{
	switch (multiSample)
	{
	case 1:
		return 0;
	case 2:
		return 2;
	case 3:
		return 2;
	case 4:
		return 4;
	default:
		return 0;
	}
}

int32_t skipMipsFromQuality(int32_t quality)
{
	const int32_t c_skipMips[] =
	{
		3,	// Disabled
		3,	// Low
		2,	// Medium
		1,	// High
		0	// Ultra
	};
	return c_skipMips[quality];
}

int32_t maxAnisotropyFromQuality(int32_t quality)
{
#if defined(__ANDROID__) || defined(__IOS__)
	const int32_t c_maxAnisotropy[] =
	{
		1,	// Disabled
		1,	// Low
		1,	// Medium
		4,	// High
		8	// Ultra
	};
#else
	const int32_t c_maxAnisotropy[] =
	{
		1,	// Disabled
		1,	// Low
		4,	// Medium
		8,	// High
		16	// Ultra
	};
#endif
	return c_maxAnisotropy[quality];
}

bool findDisplayMode(render::IRenderSystem* renderSystem, const render::DisplayMode& criteria, render::DisplayMode& outBestMatch)
{
	int32_t bestMatch = std::numeric_limits< int32_t >::max();
	int32_t bestRefreshRate = 0;
	uint32_t bestDisplayModeIndex = 0;

	uint32_t displayModeCount = renderSystem->getDisplayModeCount();
	if (!displayModeCount)
	{
		render::DisplayMode currentMode = renderSystem->getCurrentDisplayMode();
		if (currentMode.width != 0 && currentMode.height != 0)
		{
			log::warning << L"Unable to enumerate display modes; using current display mode (" << currentMode.width << L" * " << currentMode.height << L") as fail safe" << Endl;
			outBestMatch = currentMode;
		}
		else
		{
			log::warning << L"Unable to enumerate display modes; using criteria display mode (" << criteria.width << L" * " << criteria.height << L") as fail safe" << Endl;
			outBestMatch = criteria;
		}
		return true;
	}

	const uint32_t c_preferColorBits[] = { 15, 16, 32, 24 };
	for (uint32_t i = 0; i < sizeof_array(c_preferColorBits); ++i)
	{
		for (uint32_t j = 0; j < displayModeCount; ++j)
		{
			render::DisplayMode check = renderSystem->getDisplayMode(j);
			if (check.colorBits != c_preferColorBits[i])
				continue;

			int32_t match =
				traktor::abs((int32_t)(check.width - criteria.width)) +
				traktor::abs((int32_t)(check.height - criteria.height));

			if (
				match < bestMatch ||
				(
					match == bestMatch &&
					check.refreshRate > bestRefreshRate
				)
			)
			{
				bestDisplayModeIndex = j;
				bestMatch = match;
				bestRefreshRate = check.refreshRate;
			}
		}
	}

	if (bestMatch != std::numeric_limits< int32_t >::max())
	{
		outBestMatch = renderSystem->getDisplayMode(bestDisplayModeIndex);
	}
	else
	{
		log::warning << L"Unable to find matching display mode; using current display mode as fail safe" << Endl;
		outBestMatch = renderSystem->getCurrentDisplayMode();
	}

	return true;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.RenderServerDefault", RenderServerDefault, RenderServer)

RenderServerDefault::RenderServerDefault()
:	m_screenAspectRatio(1.0f)
{
}

bool RenderServerDefault::create(const PropertyGroup* defaultSettings, PropertyGroup* settings, const SystemApplication& sysapp)
{
	std::wstring renderType = defaultSettings->getProperty< std::wstring >(L"Render.Type");
	std::wstring captureRenderType = settings->getProperty< std::wstring >(L"Render.CaptureType");
	std::wstring vrCompositorType = settings->getProperty< std::wstring >(L"Render.VRCompositorType");

	Ref< render::IRenderSystem > renderSystem = dynamic_type_cast< render::IRenderSystem* >(TypeInfo::createInstance(renderType.c_str()));
	if (!renderSystem)
		return false;

	Ref< render::IRenderSystem > captureRenderSystem;
	if (!captureRenderType.empty())
	{
		captureRenderSystem = dynamic_type_cast< render::IRenderSystem* >(TypeInfo::createInstance(captureRenderType.c_str()));
		if (!captureRenderSystem)
			return false;

		std::swap(captureRenderSystem, renderSystem);
	}

	Ref< render::IVRCompositor > vrCompositor;
	if (!vrCompositorType.empty())
	{
		vrCompositor = dynamic_type_cast< render::IVRCompositor* >(TypeInfo::createInstance(vrCompositorType.c_str()));
		if (!vrCompositor)
			return false;
	}

	int32_t textureQuality = settings->getProperty< int32_t >(L"Render.TextureQuality", 2);

	render::RenderSystemDesc rsd;
	rsd.capture = captureRenderSystem;
	rsd.sysapp = sysapp;
	rsd.adapter = settings->getProperty< int32_t >(L"Render.Adapter", -1);
	rsd.mipBias = settings->getProperty< float >(L"Render.MipBias", 0.0f);
	rsd.maxAnisotropy = maxAnisotropyFromQuality(textureQuality);
	rsd.validation = settings->getProperty< bool >(L"Render.Validation", false);
	rsd.programCache = settings->getProperty< bool >(L"Render.UseProgramCache", true);
	rsd.verbose = true;

	if (!renderSystem->create(rsd))
	{
		log::error << L"Render server failed; unable to create render system." << Endl;
		return false;
	}

	m_originalDisplayMode = renderSystem->getCurrentDisplayMode();

	m_screenAspectRatio = renderSystem->getDisplayAspectRatio();
	if (m_screenAspectRatio <= 0.001f)
	{
		if (m_originalDisplayMode.width > 0 && m_originalDisplayMode.height > 0)
			m_screenAspectRatio = float(m_originalDisplayMode.width) / m_originalDisplayMode.height;
		else
		{
			log::warning << L"Unable to get display aspect ratio; assuming square display ratio." << Endl;
			m_screenAspectRatio = 1.0f;
		}
	}

	m_renderViewDesc.depthBits = settings->getProperty< int32_t >(L"Render.DepthBits", 24);
	m_renderViewDesc.stencilBits = settings->getProperty< int32_t >(L"Render.StencilBits", 8);
	m_renderViewDesc.multiSample = settings->getProperty< int32_t >(L"Render.MultiSample", 0);
	m_renderViewDesc.waitVBlanks = settings->getProperty< int32_t >(L"Render.WaitVBlanks", 1);
	m_renderViewDesc.title = settings->getProperty< std::wstring >(L"Render.Title", L"Traktor");
	m_renderViewDesc.fullscreen = settings->getProperty< bool >(L"Render.FullScreen", false);

#if defined(_PS3)

	switch (m_originalDisplayMode.height)
	{
	case 720:
		log::info << L"Using HD television settings" << Endl;
		m_renderViewDesc.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionHD/Width", 1280);
		m_renderViewDesc.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionHD/Height", 720);
		m_renderViewDesc.multiSample = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionHD/MultiSample", 0);
		break;

	case 1080:
		log::info << L"Using FullHD television settings" << Endl;
		m_renderViewDesc.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionFullHD/Width", 1440);
		m_renderViewDesc.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionFullHD/Height", 1080);
		m_renderViewDesc.multiSample = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionFullHD/MultiSample", 0);
		break;

	default:
		log::info << L"Using default television settings" << Endl;
		m_renderViewDesc.displayMode.width = m_originalDisplayMode.width;
		m_renderViewDesc.displayMode.height = m_originalDisplayMode.height;
		m_renderViewDesc.multiSample = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionStandard/MultiSample", 0);
	}

	m_renderViewDesc.displayMode.colorBits = 24;

#else

	// Get display mode from settings; use default settings if none is provided.
	if (m_renderViewDesc.fullscreen)
	{
		m_renderViewDesc.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode/Width", m_originalDisplayMode.width);
		m_renderViewDesc.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode/Height", m_originalDisplayMode.height);
	}
	else
	{
		int32_t defaultDenominator = settings->getProperty< int32_t >(L"Render.DisplayMode.Window/DefaultDenominator", 2);
		int32_t defaultWidth = m_originalDisplayMode.width / defaultDenominator;
		int32_t defaultHeight = m_originalDisplayMode.height / defaultDenominator;

		// Clamp width if display is wider than 16:10.
		float ratio = float(defaultWidth) / defaultHeight;
		if (ratio > 16.0f / 10.0f)
			defaultWidth = (defaultHeight * 16) / 10;

		m_renderViewDesc.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode.Window/Width", defaultWidth);
		m_renderViewDesc.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode.Window/Height", defaultHeight);
	}

	m_renderViewDesc.displayMode.colorBits = 24;

#endif

	// Ensure no invalid multi-sample configuration is entered.
	m_renderViewDesc.multiSample = sanitizeMultiSample(m_renderViewDesc.multiSample);

	// Ensure display mode is still supported; else find closest match.
	if (m_renderViewDesc.fullscreen)
	{
		if (!findDisplayMode(renderSystem, m_renderViewDesc.displayMode, m_renderViewDesc.displayMode))
		{
			log::error << L"Render server failed; unable to find an acceptable display mode." << Endl;
			renderSystem->destroy();
			return false;
		}
	}

	Ref< render::IRenderView > renderView = renderSystem->createRenderView(m_renderViewDesc);
	if (!renderView)
	{
		log::error << L"Render server failed; unable to create render view." << Endl;
		renderSystem->destroy();
		return false;
	}

	// We've successfully created the render view; update settings to reflect found display mode.
#if !defined(_PS3)
	if (m_renderViewDesc.fullscreen)
	{
		settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Width", m_renderViewDesc.displayMode.width);
		settings->setProperty< PropertyInteger >(L"Render.DisplayMode/Height", m_renderViewDesc.displayMode.height);
	}
#endif

	m_renderSystem = renderSystem;
	m_renderView = renderView;
	return true;
}

void RenderServerDefault::destroy()
{
	safeClose(m_renderView);
	safeDestroy(m_renderSystem);
}

void RenderServerDefault::createResourceFactories(IEnvironment* environment)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();

	int32_t textureQuality = environment->getSettings()->getProperty< int32_t >(L"Render.TextureQuality", 2);
	int32_t skipMips = skipMipsFromQuality(textureQuality);

	m_textureFactory = new render::TextureFactory(m_renderSystem, skipMips);

	resourceManager->addFactory(m_textureFactory);
	resourceManager->addFactory(new render::AliasTextureFactory());
	resourceManager->addFactory(new render::SequenceTextureFactory());
	resourceManager->addFactory(new render::ShaderFactory(m_renderSystem));
	resourceManager->addFactory(new render::ImageGraphFactory(m_renderSystem));
}

int32_t RenderServerDefault::reconfigure(IEnvironment* environment, const PropertyGroup* settings)
{
	resource::IResourceManager* resourceManager = environment->getResource()->getResourceManager();
	int32_t result = CrUnaffected;

	render::RenderViewDefaultDesc rvdd;
	rvdd.depthBits = settings->getProperty< int32_t >(L"Render.DepthBits", 16);
	rvdd.stencilBits = settings->getProperty< int32_t >(L"Render.StencilBits", 8);
	rvdd.multiSample = settings->getProperty< int32_t >(L"Render.MultiSample", 4);
	rvdd.waitVBlanks = settings->getProperty< int32_t >(L"Render.WaitVBlanks", 1);
	rvdd.fullscreen = settings->getProperty< bool >(L"Render.FullScreen", false);
	rvdd.title = settings->getProperty< std::wstring >(L"Render.Title", L"Traktor");

#if defined(_PS3)

	rvdd.displayMode.stereoscopic = settings->getProperty< bool >(L"Render.Stereoscopic", false);
	if (!rvdd.displayMode.stereoscopic)
	{
		switch (m_originalDisplayMode.height)
		{
		case 720:
			log::info << L"Using HD television settings" << Endl;
			rvdd.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionHD/Width", 1280);
			rvdd.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionHD/Height", 720);
			rvdd.multiSample = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionHD/MultiSample", 0);
			break;

		case 1080:
			log::info << L"Using FullHD television settings" << Endl;
			rvdd.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionFullHD/Width", 1440);
			rvdd.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionFullHD/Height", 1080);
			rvdd.multiSample = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionFullHD/MultiSample", 0);
			break;

		default:
			log::info << L"Using default television settings" << Endl;
			rvdd.displayMode.width = m_originalDisplayMode.width;
			rvdd.displayMode.height = m_originalDisplayMode.height;
			rvdd.multiSample = settings->getProperty< int32_t >(L"Render.DisplayMode/TelevisionStandard/MultiSample", 0);
		}
	}
	else
	{
		log::info << L"Using 3D television settings" << Endl;
		rvdd.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode/Television3D/Width", 960);
		rvdd.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode/Television3D/Height", 720);
		rvdd.multiSample = settings->getProperty< int32_t >(L"Render.DisplayMode/Television3D/MultiSample", 0);
	}
	rvdd.displayMode.colorBits = 24;

#else

	if (rvdd.fullscreen)
	{
		rvdd.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode/Width", m_originalDisplayMode.width);
		rvdd.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode/Height", m_originalDisplayMode.height);
	}
	else
	{
		rvdd.displayMode.width = settings->getProperty< int32_t >(L"Render.DisplayMode.Window/Width", m_originalDisplayMode.width / 2);
		rvdd.displayMode.height = settings->getProperty< int32_t >(L"Render.DisplayMode.Window/Height", m_originalDisplayMode.height / 2);
	}

	rvdd.displayMode.colorBits = 24;

#endif

	// Ensure display mode is still supported; else find closest match.
	if (rvdd.fullscreen)
	{
		if (!findDisplayMode(m_renderSystem, rvdd.displayMode, rvdd.displayMode))
		{
			log::error << L"Unable to find an acceptable display mode; unable to continue" << Endl;
			return CrFailed;
		}
	}

	// Check if we need to reset render view.
	if (
		m_renderViewDesc.depthBits != rvdd.depthBits ||
		m_renderViewDesc.stencilBits != rvdd.stencilBits ||
		m_renderViewDesc.multiSample != rvdd.multiSample ||
		m_renderViewDesc.waitVBlanks != rvdd.waitVBlanks ||
		m_renderViewDesc.fullscreen != rvdd.fullscreen ||
		m_renderViewDesc.displayMode.width != rvdd.displayMode.width ||
		m_renderViewDesc.displayMode.height != rvdd.displayMode.height
	)
	{
		T_DEBUG(L"Render view settings changed; resetting view...");
		render::RenderViewDefaultDesc current = m_renderViewDesc;

		// Reset primary render view, restore descriptor if fail.
		m_renderViewDesc = rvdd;
		if (!m_renderView->reset(rvdd))
		{
			log::error << L"Failed to apply changes to render view; current is kept" << Endl;
			m_renderViewDesc = current;
			return CrFailed;
		}

		result = CrAccepted;
	}
	else
		T_DEBUG(L"Render view settings unchanged");

	int32_t textureQuality = settings->getProperty< int32_t >(L"Render.TextureQuality", 2);

	// Update texture quality; manifest through skipping high-detail mips.
	int32_t skipMips = skipMipsFromQuality(textureQuality);
	if (skipMips != m_textureFactory->getSkipMips())
	{
		m_textureFactory->setSkipMips(skipMips);

		// Unload all texture resources from resource manager.
		resourceManager->unload(type_of< render::ITexture >());

		// Ensure all unloaded textures are purged; ie totally evicted from video memory.
		m_renderSystem->purge();

		// Reload all texture resources; this time using new skip mips setting.
		resourceManager->reload(type_of< render::ITexture >(), false);
		result |= CrAccepted;
	}

	// Reset render system.
	render::RenderSystemDesc rsd;
	rsd.adapter = settings->getProperty< int32_t >(L"Render.Adapter", -1);
	rsd.mipBias = settings->getProperty< float >(L"Render.MipBias", 0.0f);
	rsd.maxAnisotropy = maxAnisotropyFromQuality(textureQuality);
	if (!m_renderSystem->reset(rsd))
		return CrFailed;

	return result;
}

RenderServer::UpdateResult RenderServerDefault::update(PropertyGroup* settings)
{
	RenderServer::update(settings);

#if !defined(_PS3)

	if (!m_renderView)
		return UrSuccess;

	render::RenderEvent evt;
	while (m_renderView->nextEvent(evt))
	{
		if (evt.type == render::ReClose)
			return UrTerminate;
		else if (evt.type == render::ReToggleFullScreen)
		{
			settings->setProperty< PropertyBoolean >(L"Render.FullScreen", !m_renderViewDesc.fullscreen);
			return UrReconfigure;
		}
		else if (evt.type == render::ReSetWindowed)
		{
			settings->setProperty< PropertyBoolean >(L"Render.FullScreen", false);
			return UrReconfigure;
		}
		else if (evt.type == render::ReSetFullScreen)
		{
			settings->setProperty< PropertyBoolean >(L"Render.FullScreen", true);
			return UrReconfigure;
		}
		else if (evt.type == render::ReResize)
		{
			if (!m_renderViewDesc.fullscreen)
			{
				settings->setProperty< PropertyInteger >(L"Render.DisplayMode.Window/Width", evt.resize.width);
				settings->setProperty< PropertyInteger >(L"Render.DisplayMode.Window/Height", evt.resize.height);
				return UrReconfigure;
			}
		}
		else if (evt.type == render::ReLost)
		{
			m_renderViewDesc.displayMode.width = 0;
			m_renderViewDesc.displayMode.height = 0;
			return UrReconfigure;
		}
	}

#endif

	return UrSuccess;
}

render::IRenderSystem* RenderServerDefault::getRenderSystem()
{
	return m_renderSystem;
}

render::IRenderView* RenderServerDefault::getRenderView()
{
	return m_renderView;
}

int32_t RenderServerDefault::getWidth() const
{
	return m_renderView->getWidth();
}

int32_t RenderServerDefault::getHeight() const
{
	return m_renderView->getHeight();
}

float RenderServerDefault::getScreenAspectRatio() const
{
	return m_screenAspectRatio;
}

float RenderServerDefault::getViewAspectRatio() const
{
	float aspectRatio = float(getWidth()) / getHeight();
	return min(aspectRatio, c_maxAspectRatio);
}

float RenderServerDefault::getAspectRatio() const
{
	return m_renderView->isFullScreen() ? getScreenAspectRatio() : getViewAspectRatio();
}

int32_t RenderServerDefault::getMultiSample() const
{
	return m_renderViewDesc.multiSample;
}

	}
}
