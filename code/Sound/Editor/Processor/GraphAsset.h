#pragma once

#include <vector>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/RefSet.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"

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

class Graph;

class T_DLLCLASS GraphAsset : public ISerializable
{
	T_RTTI_CLASS;

public:
	GraphAsset();

	const Guid& getCategory() const { return m_category; }

	float getGain() const { return m_gain; }

	float getPresence() const { return m_presence; }

	float getPresenceRate() const { return m_presenceRate; }

	float getRange() const { return m_range; }

	Graph* getGraph() { return m_graph; }

	const Graph* getGraph() const { return m_graph; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_category;
	float m_gain;
	float m_presence;
	float m_presenceRate;
	float m_range;
	Ref< Graph > m_graph;
};

	}
}
