#include "Runtime/IEnvironment.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/Stage.h"
#include "Runtime/Engine/VideoLayer.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Profiler.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Video/Video.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.VideoLayer", VideoLayer, Layer)

VideoLayer::VideoLayer(
	Stage* stage,
	const std::wstring& name,
	bool permitTransition,
	IEnvironment* environment,
	const resource::Proxy< video::Video >& video,
	const resource::Proxy< render::Shader >& shader,
	const Aabb2& screenBounds,
	bool visible,
	bool autoPlay,
	bool repeat
)
:	Layer(stage, name, permitTransition)
,	m_environment(environment)
,	m_video(video)
,	m_shader(shader)
,	m_screenBounds(screenBounds)
,	m_repeat(repeat)
,	m_playing(autoPlay)
,	m_visible(visible)
{
}

void VideoLayer::destroy()
{
	safeDestroy(m_screenRenderer);
	Layer::destroy();
}

void VideoLayer::play()
{
	m_playing = true;
}

void VideoLayer::stop()
{
	m_playing = false;
}

void VideoLayer::rewind()
{
	if (m_video)
		m_video->rewind();
}

void VideoLayer::show()
{
	m_visible = true;
}

void VideoLayer::hide()
{
	m_visible = false;
}

bool VideoLayer::isPlaying() const
{
	return m_playing;
}

bool VideoLayer::isVisible() const
{
	return m_visible;
}

void VideoLayer::setScreenBounds(const Aabb2& screenBounds)
{
	m_screenBounds = screenBounds;
}

Aabb2 VideoLayer::getScreenBounds() const
{
	return m_screenBounds;
}

void VideoLayer::setRepeat(bool repeat)
{
	m_repeat = repeat;
}

bool VideoLayer::getRepeat() const
{
	return m_repeat;
}

void VideoLayer::transition(Layer* fromLayer)
{
}

void VideoLayer::prepare(const UpdateInfo& info)
{
}

void VideoLayer::update(const UpdateInfo& info)
{
	T_PROFILER_SCOPE(L"VideoLayer update");

	if (m_playing)
	{
		if (!m_video->update(info.getSimulationDeltaTime()))
		{
			getStage()->invokeScript("videoFinished", 0, 0);

			if (!m_repeat)
				m_playing = false;
			else
			{
				m_video->rewind();
				m_video->update(info.getSimulationDeltaTime());
			}
		}
	}
}

void VideoLayer::build(const UpdateInfo& info, uint32_t frame)
{
}

void VideoLayer::render(uint32_t frame)
{
	if (!m_visible)
		return;

	if (!m_screenRenderer)
	{
		m_screenRenderer = new render::ScreenRenderer();
		if (!m_screenRenderer->create(m_environment->getRender()->getRenderSystem()))
		{
			m_screenRenderer = nullptr;
			return;
		}
	}

	render::ISimpleTexture* texture = m_video->getTexture();
	if (texture)
	{
		m_shader->setTextureParameter(L"Texture", texture);
		m_shader->setVectorParameter(L"Bounds", Vector4(m_screenBounds.mn.x, m_screenBounds.mn.y, m_screenBounds.mx.x, m_screenBounds.mx.y));

		m_screenRenderer->draw(
			m_environment->getRender()->getRenderView(),
			m_shader
		);
	}
}

void VideoLayer::flush()
{
}

void VideoLayer::preReconfigured()
{
}

void VideoLayer::postReconfigured()
{
}

void VideoLayer::suspend()
{
}

void VideoLayer::resume()
{
}

	}
}
