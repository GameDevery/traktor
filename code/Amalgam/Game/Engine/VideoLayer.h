#pragma once

#include "Amalgam/Game/Engine/Layer.h"
#include "Core/Math/Aabb2.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_AMALGAM_GAME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace amalgam
	{

class IEnvironment;

	}

	namespace render
	{

class ScreenRenderer;
class Shader;

	}

	namespace video
	{

class Video;

	}

	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class T_DLLCLASS VideoLayer : public Layer
{
	T_RTTI_CLASS;

public:
	VideoLayer(
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
	);

	virtual void destroy() override;

	void play();

	void stop();

	void rewind();

	void show();

	void hide();

	bool isPlaying() const;

	bool isVisible() const;

	void setScreenBounds(const Aabb2& screenBounds);

	Aabb2 getScreenBounds() const;

	void setRepeat(bool repeat);

	bool getRepeat() const;

	virtual void transition(Layer* fromLayer) override final;

	virtual void prepare(const UpdateInfo& info) override final;

	virtual void update(const UpdateInfo& info) override final;

	virtual void build(const UpdateInfo& info, uint32_t frame) override final;

	virtual void render(render::EyeType eye, uint32_t frame) override final;

	virtual void flush() override final;

	virtual void preReconfigured() override final;

	virtual void postReconfigured() override final;

	virtual void suspend() override final;

	virtual void resume() override final;

private:
	Ref< IEnvironment > m_environment;
	resource::Proxy< video::Video > m_video;
	resource::Proxy< render::Shader > m_shader;
	Ref< render::ScreenRenderer > m_screenRenderer;
	Aabb2 m_screenBounds;
	bool m_repeat;
	bool m_playing;
	bool m_visible;
};

	}
}

