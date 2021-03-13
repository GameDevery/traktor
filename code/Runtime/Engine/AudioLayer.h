#pragma once

#include "Runtime/Engine/Layer.h"
#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
#include "Sound/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RUNTIME_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class ISoundHandle;
class Sound;

	}

	namespace runtime
	{

class IEnvironment;

/*! Stage audio layer.
 * \ingroup Runtime
 */
class T_DLLCLASS AudioLayer : public Layer
{
	T_RTTI_CLASS;

public:
	AudioLayer(
		Stage* stage,
		const std::wstring& name,
		bool permitTransition,
		IEnvironment* environment,
		const resource::Proxy< sound::Sound >& sound,
		bool autoPlay,
		bool repeat
	);

	virtual void destroy() override;

	/*! Play sound. */
	void play();

	/*! Stop sound. */
	void stop();

	/*! Fade off sound. */
	void fadeOff();

	/*! Set sound effect parameter.
	 *
	 * \param parameterName Name of effect parameter.
	 * \param value Effect parameter value.
	 */
	void setParameter(const std::wstring& parameterName, float value);

	/*! Tween effect parameter over a period of time.
	 *
	 * \param parameterName Name of effect parameter.
	 * \param fromValue Effect parameter "from" value.
	 * \param toValue Effect parameter "to" value.
	 * \param duration Duration in seconds for transition.
	 */
	void tweenParameter(const std::wstring& parameterName, float fromValue, float toValue, float duration);

	virtual void transition(Layer* fromLayer) override final;

	virtual void prepare(const UpdateInfo& info) override final;

	virtual void update(const UpdateInfo& info) override final;

	virtual void setup(const UpdateInfo& info, render::RenderGraph& renderGraph) override final;

	virtual void preReconfigured() override final;

	virtual void postReconfigured() override final;

	virtual void suspend() override final;

	virtual void resume() override final;

private:
	struct Tween
	{
		sound::handle_t parameter;
		float fromValue;
		float toValue;
		float duration;
		float time;
	};

	IEnvironment* m_environment;
	resource::Proxy< sound::Sound > m_sound;
	Ref< sound::ISoundHandle > m_handle;
	bool m_autoPlay;
	bool m_repeat;
	AlignedVector< Tween > m_tweens;
};

	}
}

