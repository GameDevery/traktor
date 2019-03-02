#pragma once

#include "Resource/IdProxy.h"
#include "Sound/Processor/ImmutableNode.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class Sound;

class T_DLLCLASS Source : public ImmutableNode
{
	T_RTTI_CLASS;

public:
	Source();

	virtual bool bind(resource::IResourceManager* resourceManager) override final;

	virtual Ref< ISoundBufferCursor > createCursor() const override final;

	virtual bool getScalar(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, float& outScalar) const override final;

	virtual bool getBlock(ISoundBufferCursor* cursor, const GraphEvaluator* evaluator, const ISoundMixer* mixer, SoundBlock& outBlock) const override final;

	virtual void serialize(ISerializer& s) override final;

	const resource::IdProxy< Sound >& getSound() const { return m_sound; }

private:
	resource::IdProxy< Sound > m_sound;
};

	}
}
