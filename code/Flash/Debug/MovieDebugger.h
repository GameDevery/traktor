#pragma once

#include "Core/Object.h"
#include "Core/Timer/Timer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace flash
	{

class Movie;
class SpriteInstance;

/*! \brief
 * \ingroup Flash
 */
class T_DLLCLASS MovieDebugger : public Object
{
	T_RTTI_CLASS;

public:
	MovieDebugger(net::BidirectionalObjectTransport* transport, const std::wstring& name);

	void postExecuteFrame(
		const Movie* movie,
		const SpriteInstance* movieInstance,
		const Vector4& stageTransform,
		int32_t viewWidth,
		int32_t viewHeight
	) const;

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	mutable int32_t m_captureFrames;
};

	}
}

