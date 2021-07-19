#pragma once

#include "Core/Guid.h"
#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SoundAsset : public editor::Asset
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) override final;

	void setStream(bool stream) { m_stream = stream; }

	bool getStream() const { return m_stream; }

	void setPreload(bool preload) { m_preload = preload; }

	bool getPreload() const { return m_preload; }

	void setCompressed(bool compressed) { m_compressed = compressed; }

	bool getCompressed() const { return m_compressed; }

	float getGain() const { return m_gain; }

private:
	friend class SoundPipeline;

	Guid m_category;
	bool m_stream = false;
	bool m_preload = false;
	bool m_compressed = true;
	float m_gain = 0.0f;
};

	}
}

